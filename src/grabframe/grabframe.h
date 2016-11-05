#ifndef  grabframe_INC
#define  grabframe_INC
#include <cv.h>
#include <errno.h>
#include <opencv2/highgui/highgui.hpp>
#include <signal.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <uEye.h>
#include <unistd.h>
#include <wchar.h>
#include "ourerr.hpp"

#define UNLOGALL "unlogall\r\n"            /*  */
#define ENABLE_EVENT_IN "EVENTINCONTROL MARK2 ENABLE\r\n"            /*  */
#define LOG_EVENT_IN "LOG MARK2TIMEA ONNEW\r\n"            /*  */
#define ENABLE_TRIGGER "EVENTOUTCONTROL MARK1 ENABLE POSITIVE 10000000 10000000\r\n"            /*  */

#define SEQSIZE 128            /*  */
#define MAXMSG 1024            /*  */
#define CRC32_POLYNOMIAL 0xEDB88320L

#define MAXOFFSET 5000
#define MAXSHUTTER 32
#define MAXGAIN 20
#define PIXELCLOCK 128

void err_ueye ( HIDS cam, int result, char *msg );
void capabilities (HIDS cam);
void auto_info (HIDS cam);
HIDS initCam ( int cam_num );
void getImage ( HIDS cam, char *dir, uint64_t frameno, int show);
void autoGain ( HIDS cam );
void incrShutter ( HIDS cam, double delta );
void autoShutter( HIDS cam);
void toggleGainBoost ( HIDS cam );
#endif   /* ----- #ifndef grabframe_INC  ----- */
