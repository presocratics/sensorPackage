#!/usr/bin/env python2.7
# getimu.py
# Martin Miller
# Created: 2015/03/11
# Garbage that just gets a single rawimu value and echoes it
import serial
import time
import sys
import signal


def connectToGPS(device="/dev/ttyUSB0"):
    """Continually attempt to connect to the GPS"""
    try:
        ser=serial.Serial(device, baudrate=115200, timeout=10)
    except serial.serialutil.SerialException, e:
        print "Cannot connect to GPS device."
        sys.exit(e)
    return ser

def signal_handler(signal, frame):
    ser=connectToGPS()
    logIMU(ser)
    return

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

def isOK(ser):
    """Returns True if SPAN responds with OK"""
    while 1:
        msg=ser.readline(300)
        if msg.find("<")!=-1:
            break
    if msg.find("<OK")!=-1:
        return True
    return False

def logIMU(ser):
    """Gets one RAWIMU reading"""
    msg="LOG usb2 RAWIMUS once"
    if sendCommand(ser, msg) is False:
        exit("rawimus Failed.")
    return

def sendCommand(ser, msg):
    """Sends a command and verifies it is received correctly"""
    ser.write("%s\r\n" % (msg))
    return

def main():
    ser=connectToGPS()
    signal.signal(signal.SIGUSR1, signal_handler)
    unlogall(ser)
    while 1:
        """Whatever, I just need to chill til i get a signal"""
        time.sleep(10)

if __name__=='__main__':
    main()

