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
import time

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
        print "Waiting for finesteering"
        ser.write("LOG usb1 BESTPOSA once\r\n")
        while 1:
#TODO test for timeout
            msg=ser.readline(300)
            if msg.find("BESTPOSA")!=-1:
                break
        if msg.find("FINESTEERING")!=-1:
            break
    print "Finesteering achieved"
    return

def setInitAttitude(ser):
    """Initializes attitude"""
    ser.write("SETINITATTITUDE 0 0 90 5 5 5\r\n")
    if isOK(ser) is False:
        exit("SETINITATTITUDE failed.")
    return

def isOK(ser):
    """Returns True if SPAN responds with OK"""
    while 1:
        msg=ser.readline(300)
        if msg.find("<")!=-1:
            break
    if msg.find("<OK")!=-1:
        return True
    return False

def unlogall(ser):
    """Turns off all other logging"""
    while 1:
        print("Running unlogall and waiting for confirmation.")
        ser.write("unlogall\r\n")
        time.sleep(1)
        if isOK(ser) is True:
            break
    print("Unlogall ran successfully")
    return

def logINSPVASA(ser, rate):
    """Turns on INSPVASA. Position, velocity, attitude, short msg ASCII"""
    ser.write("LOG usb1 INSPVASA ontime %f\r\n" % (rate))
    if isOK(ser) is False:
        exit("INSPVASA failed.")
    return

def logImages(ser, fps):
    """Sets up external trigger output at fps and turns on 2 trigger inputs"""
    msg="EVENTINCONTROL MARK1 ENABLE"
    if sendCommand(ser, msg) is False:
        exit("message failed: %s" % (msg))

    msg="EVENTINCONTROL MARK2 ENABLE"
    if sendCommand(ser, msg) is False:
        exit("message failed: %s" % (msg))

    msg="log MARK1TIMEA ONNEW"
    if sendCommand(ser, msg) is False:
        exit("message failed: %s" % (msg))

    msg="log MARK2TIMEA ONNEW"
    if sendCommand(ser, msg) is False:
        exit("message failed: %s" % (msg))

    """Convert fps to nanosecond half period"""
    T=int(500e6/int(fps))

    msg="EVENTOUTCONTROL MARK1 ENABLE POSITIVE %d %d" % (T, T)
    if sendCommand(ser, msg) is False:
        exit("message failed: %s" % (msg))


def logACC(ser, rate):
    """Turns on acceleration logging"""
    msg="LOG usb1 CORRIMUDATASA ontime %f" % (rate)
    if sendCommand(ser, msg) is False:
        exit("corrimudatasa Failed.")
    return

def sendCommand(ser, msg):
    """Sends a command and verifies it is received correctly"""
    ser.write("%s\r\n" % (msg))
    return isOK(ser)

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
    print "Logging has begun."
    ser.close()
    sys.exit(0)

if __name__=='__main__':
    main()

