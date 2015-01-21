#!/usr/bin/env python2.7
# span.py
# Martin Miller
# Created: 2014/12/22
# Uses python for the serial connection to the GPS
# Usage: ./span.py [serial device]
import serial
import sys
import signal

ser=serial.Serial()

def signal_handler(signal, frame):
    ser.close()
    sys.exit(0)

def connectToGPS(device="/dev/ttyUSB0"):
    """Continually attempt to connect to the GPS"""
    try:
        ser=serial.Serial(device, baudrate=115200, timeout=10)
    except serial.serialutil.SerialException, e:
        print "Cannot connect to GPS device."
        sys.exit(e)
    return ser

def waitForFix(ser):
    """Poll device until it has a fix"""
    while 1:
        ser.write("LOG usb1 BESTPOSA once\r\n")
        while 1:
            msg=ser.readline(300)
            if msg.find("BESTPOSA")!=-1:
                break
        if msg.find("FINESTEERING")!=-1:
            break
        print "Waiting for finesteering"
    print "Finesteering achieved"
    return

def setInitAttidue(ser):
    """Initializes attitude"""
    ser.write("SETINITATTITUDE 0 0 90 5 5 5\r\n")
    return

def logINSPVASA(ser, rate):
    """Turns on INSPVASA. Position, velocity, attitude, short msg ASCII"""
    ser.write("LOG usb1 INSPVASA ontime %f\r\n" % (rate))
    return

def logACC(ser, rate):
    """Turns on acceleration logging"""
    ser.write("LOG usb1 CORRIMUDATASA ontime %f\r\n" % (rate))
    return

def main():
    if len(sys.argv)>1:
        ser=connectToGPS(sys.argv[1])
    else:
        ser=connectToGPS()
    signal.signal(signal.SIGINT, signal_handler)
    waitForFix(ser)
    setInitAttitude(ser)
    logINSPVAS(ser, 1)  # 1Hz
    logACC(ser, .02) # 50Hz
    print "Logging has begun. cat or tail the device to read."
    ser.close()
    sys.exit(0)

if __name__=='__main__':
    main()

