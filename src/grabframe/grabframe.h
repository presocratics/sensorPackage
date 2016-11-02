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

void err_ueye ( HIDS cam, int result, char *msg );
HIDS initCam ( int cam_num );
uint64_t getImage ( HIDS cam, char *dir, int show);
void autoGain ( HIDS cam );
void incrShutter ( HIDS cam, double delta );
#endif   /* ----- #ifndef grabframe_INC  ----- */
