/*
 * =====================================================================================
 *
 *       Filename:  takePic.cpp
 *
 *    Description:  This is a lousy program that will take a picture when you
 *    a SIGUSR.
 *
 *        Version:  1.0
 *        Created:  03/04/2015 09:22:29 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Martin Miller (MHM), miller7@illinois.edu
 *   Organization:  Aerospace Robotics and Controls Lab (ARC)
 *
 * =====================================================================================
 */


#include <iostream>
#include <uEye.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <wchar.h>
//#include "ourerr.hpp"
#include <unistd.h>
#include <sys/fcntl.h>
#include <signal.h>
#include <cv.h>
#include <opencv2/highgui/highgui.hpp>
#define WIDTH 1600
#define HEIGHT 1200

HIDS cam;
int fNo=1;

HIDS initCam ( );
void err_ueye ( HIDS cam, int result, char *msg );
void autoExp ( HIDS cam );
void autoGain ( HIDS cam );
void signal_callback_handler ( int signum );
void getPic ( int save );

int main()
{
    // Initialize the camera.
    cam=initCam();
    signal(SIGUSR1, signal_callback_handler);
    signal(SIGUSR2, signal_callback_handler);

    //int rv;
    while (1) {
        getPic(0);
    }
	return 0;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  initCam
 *  Description:  Connects to camera, allocates memory, enables trigger mode.
 * =====================================================================================
 */
    HIDS
initCam ( )
{
    HIDS cam=0;
    int rv;
    char *pmem=NULL;
    int memid=0;
    if( (rv=is_InitCamera( &cam, NULL ))!=IS_SUCCESS )
    {
        err_ueye(cam, rv, "InitCamera.");
        fprintf(stderr, "Failed to init cam %d\n", cam );
        exit(EXIT_FAILURE);
    }

    // Set camera exposure modes
    if( (rv=is_SetColorMode( cam, IS_CM_MONO8))!=IS_SUCCESS ) {
        err_ueye(cam, rv, "SetColorMode.");
        exit(EXIT_FAILURE);
    }
    //autoGain(cam);
    autoExp(cam);

    // Initialize memory
    if ((rv=is_ClearSequence(cam))!=IS_SUCCESS) {
        err_ueye(cam, rv, "ClearSequence.");
        exit(EXIT_FAILURE);
    }
    if ((rv=is_AllocImageMem(cam, WIDTH, HEIGHT, 8, &pmem, &memid)!=IS_SUCCESS)) { 
        err_ueye(cam, rv, "Allocate image memory.");
        exit(EXIT_FAILURE);
    }
    if ((rv=is_SetImageMem(cam, pmem, memid)!=IS_SUCCESS)) { 
        err_ueye(cam, rv, "Set image memory.");
        exit(EXIT_FAILURE);
    }

    // Begin transmission
    if ((rv=is_CaptureVideo(cam, IS_WAIT))!=IS_SUCCESS) {
        err_ueye(cam, rv, "CaptureVideo.");
        exit(EXIT_FAILURE);
    }
    if( (rv=is_EnableEvent(cam, IS_SET_EVENT_FRAME))!=IS_SUCCESS ) {
        err_ueye(cam, rv, "Set event frame.");
        exit(EXIT_FAILURE);
    }

    return cam;
}		/* -----  end of function initCam  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  err_ueye
 *  Description:  Passes error value to uEye library, prints error message and
 *  user-defined string (msg) to STDERR.
 * =====================================================================================
 */
    void
err_ueye ( HIDS cam, int result, char *msg )
{
    char *str;
    is_GetError( cam, &result, &str );
    fprintf( stderr, "%s ueye error %d: %s\n", msg, result, str );
    return;
}		/* -----  end of function err_ueye  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  autoExp
 *  Description:  
 * =====================================================================================
 */
    void
autoExp ( HIDS cam )
{
    int rv;
    double on=1;
    double empty;
    if ((rv=is_SetAutoParameter(cam, IS_SET_ENABLE_AUTO_SHUTTER, &on, &empty))!=IS_SUCCESS)
        err_ueye(cam, rv, "Set auto shutter.");
    return ;
}		/* -----  end of function autoExp  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  autoGain
 *  Description:  
 * =====================================================================================
 */
    void
autoGain ( HIDS cam )
{
    int rv;
    double on = 1;
    double empty;
    if( (rv=is_SetAutoParameter( cam, IS_SET_ENABLE_AUTO_GAIN, &on, &empty))!=IS_SUCCESS )
        err_ueye(cam, rv, "SetAutoGain.");
    return ;
}		/* -----  end of function autoGain  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  signal_callback_handler
 *  Description:  Handles SIGINT (^C).
 * =====================================================================================
 */
    void
signal_callback_handler ( int signum )
{
    int rv;
    printf("Caught signal %d\n", signum);
    if (signum==12) {
        if( (rv=is_ExitCamera(cam))!=IS_SUCCESS )
            err_ueye(cam, rv, "Exit camera.");
        if( (rv=is_DisableEvent(cam, IS_SET_EVENT_FRAME))!=IS_SUCCESS)
            err_ueye(cam, rv, "Disable event.");
        exit( signum );
    } else {
        // Save Pic
        getPic ( 1 );
    }
}		/* -----  end of function signal_callback_handler  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  getPic
 *  Description:  
 * =====================================================================================
 */
    void
getPic ( int save )
{
    cv::Mat frame(HEIGHT, WIDTH, CV_8UC1);
    void *pMemVoid;
    is_WaitEvent(cam, IS_SET_EVENT_FRAME, 1000);

    is_GetImageMem(cam, &pMemVoid);
    frame.data= (uchar*) pMemVoid;
    if (save) {
        char fn[100];
        sprintf(fn, "%d.jpg", fNo++);
        cv::imwrite(fn,frame);
        printf("Saved %s\n", fn);
    }
    cv::imshow("frame", frame);
    cv::waitKey(100);

    return ;
}		/* -----  end of function getPic  ----- */
