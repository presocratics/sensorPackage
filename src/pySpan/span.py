#!/usr/bin/env python2.7
# span.py
# Martin Miller
# Created: 2014/12/22
# Uses python for the serial connection to the GPS
# Usage: ./span.py [-I] [--fps N] [-d serial device]
# -d	Specify serial device (default: /dev/ttyUSB0)
# -I	turns off image handling
# -fps	set fps of trigger output. (default: 25)
from optparse import OptionParser
import serial
import sys
import signal

ser=serial.Serial()

def signal_handler(signal, frame):
    ser.close()
    sys.exit("Killed by user.")

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
#TODO test for timeout
            msg=ser.readline(300)
            if msg.find("BESTPOSA")!=-1:
                break
        if msg.find("FINESTEERING")!=-1:
            break
        print "Waiting for finesteering"
    print "Finesteering achieved"
    return

def setInitAttitude(ser):
    """Initializes attitude"""
    ser.write("SETINITATTITUDE 0 0 90 5 5 5\r\n")
    return

def unlogall(ser):
    """Turns off all other logging"""
    ser.write("unlogall\r\n")
    return

def logINSPVASA(ser, rate):
    """Turns on INSPVASA. Position, velocity, attitude, short msg ASCII"""
    ser.write("LOG usb1 INSPVASA ontime %f\r\n" % (rate))
    return

def logImages(ser, fps):
    """Sets up external trigger output at fps and turns on 2 trigger inputs"""
    ser.write("EVENTINCONTROL MARK1 ENABLE\r\n")
    ser.write("EVENTINCONTROL MARK2 ENABLE\r\n")

    ser.write("MARK1TIMEA ONNEW\r\n")
    ser.write("MARK2TIMEA ONNEW\r\n")

    """Convert fps to nanosecond half period"""
    T=int(500e6/int(fps))

    ser.write("EVENTOUTCONTROL MARK1 ENABLE POSITIVE %d %d\r\n" % (T, T))


def logACC(ser, rate):
    """Turns on acceleration logging"""
    ser.write("LOG usb1 CORRIMUDATASA ontime %f\r\n" % (rate))
    return

def main():
    parser=OptionParser()
    parser.add_option("-d", "--device", dest="device", action="store",
                      help="Specify serial device.", default="/dev/ttyUSB0")
    parser.add_option("-I", "--no-image", dest="doImage", action="store_false",
                      help="Turn off image handling.", default=True)
    parser.add_option("-f", "--fps", dest="fps", action="store",
                      help="Set fps of trigger output.", default=25)

    (options, args)=parser.parse_args()
    ser=connectToGPS(options.device)
    signal.signal(signal.SIGINT, signal_handler)
    unlogall(ser)
    if options.doImage is True:
        logImages(ser, options.fps)
    waitForFix(ser)
    setInitAttitude(ser)
    logINSPVASA(ser, 1)  # 1Hz
    logACC(ser, .02) # 50Hz
    print "Logging has begun. cat or tail the device to read."
    ser.close()
    sys.exit(0)

if __name__=='__main__':
    main()

