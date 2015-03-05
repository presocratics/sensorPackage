#!/usr/bin/env python2.7
# checkerboard.py
# Martin Miller
# Created: 2015/03/04
# Reads in an image and searches for checkerboard pattern.
import cv2
import sys
import numpy as np

def main():
    criteria = (cv2.TERM_CRITERIA_EPS + cv2.TERM_CRITERIA_MAX_ITER, 30, 0.001)

    objp = np.zeros((6*9,3), np.float32)
    objp[:,:2] = np.mgrid[0:9,0:6].T.reshape(-1,2)

    objpoints = []
    imgpoints = []

    img=cv2.imread(sys.argv[1])
    gray = cv2.cvtColor(img,cv2.COLOR_BGR2GRAY)

    rv,corners=cv2.findChessboardCorners(gray,(9,6), None)

    if rv is True:
        objpoints.append(objp)
        cv2.cornerSubPix(gray, corners, (11,11), (-1,-1), criteria)
        imgpoints.append(corners)

        cv2.drawChessboardCorners(img, (9,6), corners, rv)
        cv2.imshow('img', img)
        cv2.waitKey()
    print rv

if __name__=='__main__':
    main()

