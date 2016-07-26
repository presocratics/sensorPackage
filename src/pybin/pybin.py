#!/usr/bin/env python3
# pybin.py
# Martin Miller
# Created: 2015/07/28
"""Reads binary messages from SPAN"""
import serial
import struct
from collections import namedtuple
import sys
import yaml

def getStart(ser):
    """Waits reads serial until start bits are detected"""
    seq=[[b'\xaa'],[b'\x44'],[b'\x12',b'\x13']]
    nstates=len(seq)
    curstate=0
    isShort=False
    byte=""
    while curstate<nstates:
        byte=ser.read(1)
        if byte in seq[curstate]:
            curstate+=1
        else:
            curstate=0
    if byte==b'\x13':
        isShort=True
    return byte,isShort

def readHeader(ser,isShort=False):
    """Reads the header of the message"""
    if isShort:
        format='=BHHL'
        raw_header=ser.read(9)
        fields=['Message_Length','Message_ID','Week_Number','Milliseconds']
    else:
        header_length=ser.read(1)
        num_to_read=ord(header_length)-4
        format='=HcBHHBcHLLHH'
        raw_header=ser.read(num_to_read)
        fields=['Message_ID','Message_Type','Port_Address','Message_Length',
                'Sequence','Idle_Time','Time_Status','Week','ms','Receiver_Status',
                'Reserved','Receiver_SW_Version']
    header_data=struct.unpack(format,raw_header)
    tuple=namedtuple('header',fields)
    if not isShort:
        raw_header=header_length+raw_header

    return raw_header,tuple._make(header_data)

def read_message(ser,msg_len,format):
    """Reads according to format"""
    rawmessage=ser.read(msg_len)
    message=struct.unpack(format,rawmessage)
    return rawmessage,message

def read_CRC(ser):
    """Reads 32-bit CRC value"""
    crc=ser.read(4)
    crc=struct.unpack('=4s',crc)
    return crc

def CRC32Value(ulCRC):
    """Calculate intermediate value for calc_CRC()"""
    CRC32_POLYNOMIAL=0xEDB88320
    for j in range(8):
        if ulCRC & 1:
            ulCRC = (ulCRC>>1) ^ CRC32_POLYNOMIAL
        else:
            ulCRC>>=1
    return ulCRC

def calc_CRC(msg):
    """Calculates 32-bit CRC value"""
    L=int(len(msg))
    ulCRC=0
    for i in range(L):
        byte=msg[i:i+1]
        byte=int.from_bytes(byte,'big')
        ulTemp1 = (ulCRC>>8) & 0x00ffffff
        ulTemp2 = CRC32Value((ulCRC ^ byte) & 0xff)
        ulCRC = ulTemp1 ^ ulTemp2
    return ulCRC.to_bytes(4,'little')

def main():
    Messages = {}
    with open('message.yaml','r') as fin:
        Messages = yaml.safe_load(fin.read())
    if len(sys.argv)>1:
        ser=open(sys.argv[1],'rb')
    else:
        ser=serial.Serial("/dev/ttyUSB0",baudrate=115200)
    while True:
        rawstart,isShort=getStart(ser)
        rawhdr,header=readHeader(ser,isShort)

        format=Messages[header.Message_ID]['format']
        fields=Messages[header.Message_ID]['fields']
        message_name=Messages[header.Message_ID]['message_name']

        rawmsg,message_data=read_message(ser,header.Message_Length,format)
        crc=read_CRC(ser)
        if rawstart==b'\x13':
            raw=b'\xaa\x44\x13'
        else:
            raw=b'\xaa\x44\x12'
        raw+=rawhdr
        raw+=rawmsg
        crc_calcd=calc_CRC(raw)

        MsgTuple=namedtuple(message_name,fields)
        message=MsgTuple._make(message_data)
        output=list(header)
        output.extend(list(message))

        if crc_calcd==crc[0]:
            print(",".join(map(str,output)))
        else:
            print("Error: ", ",".join(map(str,output)),file=sys.stderr)

if __name__=='__main__':
    main()

