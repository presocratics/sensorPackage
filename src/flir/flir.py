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
    seq=[[b'\x81'],[b'\xff']]
    nstates=len(seq)
    curstate=0
    byte=""
    while curstate<nstates:
        byte=ser.read(1)
        if byte in seq[curstate]:
            curstate+=1
        else:
            curstate=0
    return

def readHeader(ser):
    """Reads the header of the message"""
    ser.read(6) # Read flags, num bytes, checksum
    raw_header=ser.read(2)
    fmt='=BB'

    fields=['Key','Length']
    header_data=struct.unpack(fmt,raw_header)
    tuple=namedtuple('header',fields)

    return tuple._make(header_data)

def read_message(ser,msg_len,fmt):
    """Reads according to format"""
    rawmessage=ser.read(msg_len)
    message=struct.unpack(fmt,rawmessage)
    return message

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
        getStart(ser)
        header=readHeader(ser)

        try:
            fmt=Messages[header.Key]['format']
        except KeyError:
            continue
        if Messages[header.Key]['length']!=header.Length:
            continue
        fields=Messages[header.Key]['fields']
        message_name=Messages[header.Key]['message_name']

        message_data=read_message(ser,header.Length,fmt)

        MsgTuple=namedtuple(message_name,fields)
        message=MsgTuple._make(message_data)
        print(message)
        print(message.target_latitude*360/2**32,message.target_longitude*360/2**32)
        output=list(header)
        output.extend(list(message))


if __name__=='__main__':
    main()

