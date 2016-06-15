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
    NoResponseID=[101,263,264,268,507,616,812,1068,1461]
    """Reads the header of the message"""
    if isShort:
        format='=BHHL'
        header_data=ser.read(9)
        fields=['Message_Length','Message_ID','Week_Number','Milliseconds']
    else:
        header_length=ser.read(1)
        msg_id_data=struct.unpack('=H',ser.read(2))
        fields=['Message_ID','Message_Type','Port_Address','Message_Length',
                'Sequence','Idle_Time','Time_Status','Week','ms','Receiver_Status',
                'Reserved','Receiver_SW_Version']

        if msg_id_data[0] in NoResponseID:
            num_to_read=ord(header_length)-6
            format='=cBHHBcHLLHH'
        else:
            num_to_read=ord(header_length)-2
            format='=cBHHBcHLLHHL'
            fields.append('Response_ID')
        header_data=ser.read(num_to_read)
    header_data=struct.unpack(format,header_data)
    if not isShort:
        header_data=msg_id_data+header_data
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
    message_ids={42:"bestpos", 726:"bestutm", 812:"corrimudata",
                 813:"corrimudatas", 1362:"imuratecorrimus", 1305:"imuratepvas",
                 263:"insatt", 319:"insatts", 1457:"insattx", 264:"inscov",
                 320:"inscovs", 507:"inspva", 508:"inspvas", 1465:"inspvax",
                 1067:"mark1pva", 1068:"mark2pva", 616:"mark2time",
                 231:"marktime", 268:"rawimu", 325:"rawimus", 1462:"rawimusx",
                 1461:"rawimux", 101:"time", }

    '''These are the message formats as defined in the oem6 manual'''
    message_fmts={"bestpos":'=IIdddfI3f4sff5Bccc',
                  "bestutm":'=IILL3dfI3f4s2f5B3c',
                  "corrimudata":'=Iddddddd',
                  "corrimudatas":'=Iddddddd',
                  "imuratecorrimus":'=I7d',
                  "imuratepvas":'=I10dI',
                  "insatt":'=L4dI',
                  "insatts":'=L4dI',
                  "insattx":'=II3d3fIH',
                  "inscov":'=Id9d9d9d',
                  "inscovs":'=Id9d9d9d',
                  "inspva": '=IddddddddddI',
                  "inspvas":'=IddddddddddI',
                  "inspvax":'=2I3df6d9fIH',
                  "mark1pva":'=IddddddddddI',
                  "mark2pva":'=IddddddddddI',
                  "mark2time":'=lddddI',
                  "marktime":'=lddddI',
                  "rawimu":'=Ldlllllll',
                  "rawimus":'=Ldlllllll',
                  "rawimux":'=BBHdI6l',
                  "rawimusx":'=BBHdI6l',
                  "time":'=I3dL4BLI',
                 }

    '''Field names as defined by oem6 manual. Spaces are replaced with '_' and
    non alphanumeric characters are deleted or abbreviated. Capitalizations are
    retained wherever possible. In the oem6 documentation, field names may vary
    slightly between commands (e.g. 'sol stat' and 'sol status') and these
    variations are carried over here.  Therefore make sure to consult the exact
    command when determining field names.'''
    fields={"bestpos":['sol_stat','pos_type','lat',
                       'lon','hgt','undulation','datum_id',
                       'lat_sig','lon_sig','hgt_sig','stn_id','diff_age',
                       'sol_age','SVs','solnSVs','ggL1','ggL1L2',
                       'Reserved','ext_sol_stat','Reserved2','sig_mask'],
            "bestutm":['sol_status', 'pos_type', 'znumber', 'zletter',
                       'northing', 'easting', 'hgt', 'undulation', 'datum_id',
                       'nstd', 'estd', 'hgtstd', 'stn_id', 'diff_age',
                       'sol_age', 'SVs', 'solnSVs', 'ggL1', 'solnMultiSV',
                       'Reserved', 'ext_sol_stat',
                       'Galileo_and_BeiDou_sig_mask',
                       'GPS_and_GLONASS_sig_mask'],
            "imuratecorrimus":['Week', 'Seconds', 'PitchRate', 'RollRate',
                               'YawRate', 'LateralAcc', 'LongitudinalAcc',
                               'VerticalAcc'],
            "corrimudata":['Week','Seconds','PitchRate','RollRate','YawRate',
                           'LateralAcc','LongitudinalAcc','VerticalAcc'],
            "corrimudatas":['Week','Seconds','PitchRate','RollRate','YawRate',
                            'LateralAcc','LongitudinalAcc','VerticalAcc'],
            "inscov":['Week','Seconds_into_Week', 'Position_Covariance0',
                      'Position_Covariance1', 'Position_Covariance2',
                      'Position_Covariance3', 'Position_Covariance4',
                      'Position_Covariance5', 'Position_Covariance6',
                      'Position_Covariance7', 'Position_Covariance8',
                      'Attitude_Covariance0', 'Attitude_Covariance1',
                      'Attitude_Covariance2', 'Attitude_Covariance3',
                      'Attitude_Covariance4', 'Attitude_Covariance5',
                      'Attitude_Covariance6', 'Attitude_Covariance7',
                      'Attitude_Covariance8', 'Velocity_Covariance0',
                      'Velocity_Covariance1', 'Velocity_Covariance2',
                      'Velocity_Covariance3', 'Velocity_Covariance4',
                      'Velocity_Covariance5', 'Velocity_Covariance6',
                      'Velocity_Covariance7', 'Velocity_Covariance8'],
            "inscovs":['Week','Seconds_into_Week', 'Position_Covariance0',
                       'Position_Covariance1', 'Position_Covariance2',
                       'Position_Covariance3', 'Position_Covariance4',
                       'Position_Covariance5', 'Position_Covariance6',
                       'Position_Covariance7', 'Position_Covariance8',
                       'Attitude_Covariance0', 'Attitude_Covariance1',
                       'Attitude_Covariance2', 'Attitude_Covariance3',
                       'Attitude_Covariance4', 'Attitude_Covariance5',
                       'Attitude_Covariance6', 'Attitude_Covariance7',
                       'Attitude_Covariance8', 'Velocity_Covariance0',
                       'Velocity_Covariance1', 'Velocity_Covariance2',
                       'Velocity_Covariance3', 'Velocity_Covariance4',
                       'Velocity_Covariance5', 'Velocity_Covariance6',
                       'Velocity_Covariance7', 'Velocity_Covariance8'],
            "inspva":['Week','Seconds','Latitude','Longitude','Height',
                      'North_Velocity','East_Velocity','Up_Velocity',
                      'Roll','Pitch','Azimuth','Status'],
            "inspvas":['Week','Seconds','Latitude','Longitude','Height',
                       'North_Velocity','East_Velocity','Up_Velocity',
                       'Roll','Pitch','Azimuth','Status'],
            "inspvax":['INS_Status', 'Pos_Type', 'Lat', 'Long', 'Height',
                       'Undulation', 'North_Vel', 'East_Vel', 'Up_Vel', 'Roll',
                       'Pitch', 'Azimuth', 'Latstd', 'Longstd', 'Heightstd',
                       'North_Velstd', 'East_Velstd', 'Up_Velstd', 'Rollstd',
                       'Pitchstd', 'Azimuthstd', 'Ext_sol_stat',
                       'Time_Since_Update'],
            "insatt":['Week', 'Seconds_into_week', 'Roll', 'Pitch', 'Azimuth',
                      'Status'],
            "insatts":['Week', 'Seconds_into_week', 'Roll', 'Pitch', 'Azimuth',
                      'Status'],
            "insattx":['INS_Status', 'Pos_Type', 'Roll', 'Pitch', 'Azimuth',
                       'Rollstd', 'Pitchstd', 'Azimuthstd', 'Ext_sol_stat',
                       'Time_Since_Update'],
            "imuratepvas":['Week', 'Seconds', 'Latitude', 'Longitude', 'Height',
                           'North_Velocity', 'East_Velocity', 'Up_Velocity',
                           'Roll', 'Pitch', 'Azimuth', 'Status'],
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
            "rawimux":['IMU_Error', 'IMU_Type', 'GNSS_Week',
                       'GNSS_Week_Seconds', 'IMU_Status', 'Z_Accel',
                       'minusY_Accel', 'X_Accel', 'Z_Gyro', 'minusY_Gyro',
                       'X_Gyro'],
            "rawimusx":['IMU_Error', 'IMU_Type', 'GNSS_Week',
                       'GNSS_Week_Seconds', 'IMU_Status', 'Z_Accel',
                       'minusY_Accel', 'X_Accel', 'Z_Gyro', 'minusY_Gyro',
                       'X_Gyro'],
            "marktime":['week','seconds','offset','offset_std','utc_offset','status'],
            "mark2time":['week','seconds','offset','offset_std','utc_offset','status'],
            "time":['clock_status', 'offset', 'offset_std', 'utc_offset',
                    'utc_year', 'utc_month', 'utc_day', 'utc_hour', 'utc_min',
                    'utc_ms', 'utc_status'],
           }
    return message_ids,message_fmts,fields

def main():
    message_ids,message_fmts,fields=get_defines()
    ser=serial.Serial("/dev/ttyUSB0",baudrate=115200)
    while True:
        isShort=getStart(ser)
        header=readHeader(ser,isShort)
        msg_name=message_ids[header.Message_ID]
        format=message_fmts[msg_name]
        message_data=read_message(ser,header.Message_Length,format)
        MsgTuple=namedtuple(msg_name,fields[msg_name])
        message=MsgTuple._make(message_data)
        output=list(header)
        output.extend(list(message))
        print(",".join(map(str,output)))

if __name__=='__main__':
    main()

