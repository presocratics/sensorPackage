#!/usr/bin/env python3
# pybin.py
# Martin Miller
# Created: 2015/07/28
"""Reads binary messages from SPAN"""
import serial
import struct
from collections import namedtuple

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
    return isShort

def readHeader(ser,isShort=False):
    """Reads the header of the message"""
    if isShort:
        format='=BHHL'
        header_data=ser.read(9)
        fields=['Message_Length','Message_ID','Week_Number','Milliseconds']
    else:
        header_length=ser.read(1)
        num_to_read=ord(header_length)
        format='=HbBHHBcHLLHHL'
        header_data=ser.read(num_to_read)
        fields=['Message_ID','Message_Type','Port_Address','Message_Length',
                'Sequence','Idle_Time','Time_Status','Week','ms','Receiver_Status',
                'Reserved','Receiver_SW_Version','Response_ID']
    header_data=struct.unpack(format,header_data)
    tuple=namedtuple('header',fields)

    return tuple._make(header_data)


def read_message(ser,msg_len,format):
    """Reads according to format"""
    message=ser.read(msg_len)
    message=struct.unpack(format,message)
    return message

def read_CRC(ser):
    """Reads 32-bin CRC value"""
    crc=ser.read(4)
    crc=struct.unpack('=4s',crc)
    return crc

def get_defines():
    """Returns constant data"""
    '''message ids for each message as defined in the oem6 manual'''
    message_ids={42:"bestpos",
                 812:"corrimudata",
                 813:"corrimudatas",
                 264:"inscov",
                 320:"inscovs",
                 507:"inspva",
                 508:"inspvas",
                 1067:"mark1pva",
                 1068:"mark2pva",
                 268:"rawimu",
                 325:"rawimus",
                }

    '''These are the message formats as defined in the oem6 manual'''
    message_fmts={"bestpos":'=IIdddfIfff4sffBBBBBccc',
                 "corrimudata":'=Iddddddd',
                 "corrimudatas":'=Iddddddd',
                 "inscov":'=Id9d9d9d',
                 "inscovs":'=Id9d9d9d',
                 "inspva": '=IddddddddddI',
                 "inspvas":'=IddddddddddI',
                 "mark1pva":'=IddddddddddI',
                 "mark2pva":'=IddddddddddI',
                 "rawimu":'=Idlllllll',
                 "rawimus":'=Idlllllll',
                 }

    '''Field names as defined by oem6 manual. Spaces are replaced with '_' and
    non alphanumeric characters are deleted or abbreviated. Capitalizations are
    retained wherever possible.'''
    fields={"bestpos":['sol_stat','pos_type','lat',
                      'lon','hgt','undulation','datum_id',
                      'lat_sig','lon_sig','hgt_sig','stn_id','diff_age',
                      'sol_age','SVs','solnSVs','ggL1','ggL1L2',
                      'Reserved','ext_sol_stat','Reserved2','sig_mask'],
            "corrimudata":['Week','Seconds','PitchRate','RollRate','YawRate',
                           'LateralAcc','LongitudinalAcc','VerticalAcc'],
            "corrimudatas":['Week','Seconds','PitchRate','RollRate','YawRate',
                           'LateralAcc','LongitudinalAcc','VerticalAcc'],
            "inscov":['Week','Seconds_into_Week',
                      'Position_Covariance0',
                      'Position_Covariance1',
                      'Position_Covariance2',
                      'Position_Covariance3',
                      'Position_Covariance4',
                      'Position_Covariance5',
                      'Position_Covariance6',
                      'Position_Covariance7',
                      'Position_Covariance8',
                      'Attitude_Covariance0',
                      'Attitude_Covariance1',
                      'Attitude_Covariance2',
                      'Attitude_Covariance3',
                      'Attitude_Covariance4',
                      'Attitude_Covariance5',
                      'Attitude_Covariance6',
                      'Attitude_Covariance7',
                      'Attitude_Covariance8',
                      'Velocity_Covariance0',
                      'Velocity_Covariance1',
                      'Velocity_Covariance2',
                      'Velocity_Covariance3',
                      'Velocity_Covariance4',
                      'Velocity_Covariance5',
                      'Velocity_Covariance6',
                      'Velocity_Covariance7',
                      'Velocity_Covariance8'],
            "inscovs":['Week','Seconds_into_Week',
                      'Position_Covariance0',
                      'Position_Covariance1',
                      'Position_Covariance2',
                      'Position_Covariance3',
                      'Position_Covariance4',
                      'Position_Covariance5',
                      'Position_Covariance6',
                      'Position_Covariance7',
                      'Position_Covariance8',
                      'Attitude_Covariance0',
                      'Attitude_Covariance1',
                      'Attitude_Covariance2',
                      'Attitude_Covariance3',
                      'Attitude_Covariance4',
                      'Attitude_Covariance5',
                      'Attitude_Covariance6',
                      'Attitude_Covariance7',
                      'Attitude_Covariance8',
                      'Velocity_Covariance0',
                      'Velocity_Covariance1',
                      'Velocity_Covariance2',
                      'Velocity_Covariance3',
                      'Velocity_Covariance4',
                      'Velocity_Covariance5',
                      'Velocity_Covariance6',
                      'Velocity_Covariance7',
                      'Velocity_Covariance8'],
            "inspva":['Week','Seconds','Latitude','Longitude','Height',
                      'North_Velocity','East_Velocity','Up_Velocity',
                      'Roll','Pitch','Azimuth','Status'],
            "inspvas":['Week','Seconds','Latitude','Longitude','Height',
                      'North_Velocity','East_Velocity','Up_Velocity',
                      'Roll','Pitch','Azimuth','Status'],
            "mark1pva":['Week','Seconds','Latitude','Longitude','Height',
                        'North_Velocity','East_Velocity','Up_Velocity',
                        'Roll','Pitch','Azimuth','Status'],
            "mark2pva":['Week','Seconds','Latitude','Longitude','Height',
                        'North_Velocity','East_Velocity','Up_Velocity',
                        'Roll','Pitch','Azimuth','Status'],
            "rawimu":['Week','Seconds_into_Week','IMU_Status','Z_Accel_Output',
                      'minusY_Accel_Output','X_Accel_Output','Z_Gyro_Output',
                      'minusY_Gyro_Output','X_Gyro_Output'],
            "rawimus":['Week','Seconds_into_Week','IMU_Status','Z_Accel_Output',
                      'minusY_Accel_Output','X_Accel_Output','Z_Gyro_Output',
                      'minusY_Gyro_Output','X_Gyro_Output'],
           }
    return message_ids,message_fmts,fields

def main():
    message_ids,message_fmts,fields=get_defines()
    ser=serial.Serial("/dev/ttyUSB0",baudrate=19200)
    while True:
        isShort=getStart(ser)
        header=readHeader(ser,isShort)

        msg_name=message_ids[header.Message_ID]
        format=message_fmts[msg_name]
        message_data=read_message(ser,header.Message_Length,format)
        print(message_data)
        MsgTuple=namedtuple(msg_name,fields[msg_name])
        message=MsgTuple._make(message_data)
        output=list(header)
        output.extend(list(message))
        print(",".join(map(str,output)))

if __name__=='__main__':
    main()

