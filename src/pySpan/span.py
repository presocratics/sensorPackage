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
            msg=ser.readline(300)
            if "BESTPOSA" in msg:
                break
        if "FINESTEERING" in msg:
            break
    print "Finesteering achieved"
    return

def waitForINS(ser):
    """Poll device until it has an inertial solution"""
    while 1:
        print "Waiting for inertial solution"
        ser.write("LOG usb1 inspvasa once\r\n")
        while 1:
            msg=ser.readline(300)
            if "INSPVASA" in msg:
                break
        if "INS_SOLUTION_GOOD" in msg:
            break
        status=msg.split(",")[13]
        print status
    print "Inertial solution achieved"
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

def logINSPVAS(ser, rate, binary=False):
    """Turns on INSPVASA. Position, velocity, attitude, short msg ASCII"""
    type="A"
    if binary is True:
        type="B"
    msg="LOG usb1 INSPVAS%c ontime %f" % (type,rate)
    if sendCommand(ser, msg) is False:
        exit("message failed: %s" % (msg))
    return

def logImages(ser, fps, binary=False):
    """Sets up external trigger output at fps and turns on 2 trigger inputs"""
    type="A"
    if binary is True:
        type="B"
    msg="EVENTINCONTROL MARK2 ENABLE"
    if sendCommand(ser, msg) is False:
        exit("message failed: %s" % (msg))

    msg="log MARK2TIME%c ONNEW" % (type)
    if sendCommand(ser, msg) is False:
        exit("message failed: %s" % (msg))

    """Convert fps to nanosecond half period"""
    T=int(500e6/int(fps))

    msg="EVENTOUTCONTROL MARK1 ENABLE POSITIVE %d %d" % (T, T)
    if sendCommand(ser, msg) is False:
        exit("message failed: %s" % (msg))


def logACC(ser, rate, binary=False):
    """Turns on acceleration logging"""
    type="A"
    if binary is True:
        type="B"
    msg="LOG usb1 CORRIMUDATAS%c ontime %f" % (type,rate)
    if sendCommand(ser, msg) is False:
        exit("%s Failed." % (msg))
    return

def logRawimu(ser, rate, binary=False):
    """Logs RAWIMU"""
    if binary is True:
        msg="LOG usb1 RAWIMUSB onnew "
    else:
        msg="LOG usb1 RAWIMUSB onnew"
    if sendCommand(ser, msg) is False:
        exit("%s Failed." % (msg))
    return

def sendCommand(ser, msg):
    """Sends a command and verifies it is received correctly"""
    ser.write("%s\r\n" % (msg))
    return isOK(ser)

def logStatus(ser, rate, binary=False):
    """Gets a full message to see status of Span unit"""
    type="a"
    if binary is True:
        type="b"
    msg="LOG usb1 bestpos%c ontime %f" % (type,rate)
    if sendCommand(ser, msg) is False:
        exit("%s Failed." % (msg))
    return

def main():
    parser=OptionParser()
    parser.add_option("-d", "--device", dest="device", action="store",
                      help="Specify serial device.", default="/dev/ttyUSB0")
    parser.add_option("-I", "--no-image", dest="doImage", action="store_false",
                      help="Turn off image handling.", default=True)
    parser.add_option("-b", "--binary", dest="binary", action="store_true",
                      help="Use binary communication.", default=False)
    parser.add_option("-s", "--set-init-attitude", dest="initAtt", action="store_true",
                      help="Manually set attitude to 0 0 90 5 5 5. Needed when \
                      moving slowly. Otherwise automatically set.", default=False)
    parser.add_option("-f", "--fps", dest="fps", action="store",
                      help="Set fps of trigger output.", default=25)
    parser.add_option("-r", "--raw", dest="raw", action="store_true",
                      help="Collect rawimu.", default=False)
    parser.add_option("--no-pva", dest="pva", action="store_false",
                      help="Do not collect pva. Implies -r.", default=True)

    (options, args)=parser.parse_args()
    ser=connectToGPS(options.device)
    signal.signal(signal.SIGINT, signal_handler)
    unlogall(ser)
    if options.doImage is True:
        logImages(ser, options.fps, options.binary)
    if options.pva is True:
        waitForFix(ser)
    if options.initAtt is True:
        setInitAttitude(ser)
    if options.pva is True:
        waitForINS(ser)
        logINSPVAS(ser, .1, options.binary)  # 10Hz
    if options.binary is True:
        imurate=0.01
    else:
        imurate=0.02
    if options.pva is False or options.raw is True:
        logRawimu(ser, imurate, options.binary)
    else:
        logACC(ser, imurate, options.binary) # 50Hz
    logStatus(ser, 10, options.binary) # .1Hz
    print "Logging has begun."
    ser.close()
    sys.exit(0)

if __name__=='__main__':
    main()

