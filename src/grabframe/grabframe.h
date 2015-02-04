#ifndef  grabframe_INC
#define  grabframe_INC
#include <uEye.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <wchar.h>
#ifdef __linux
#include "ourerr.hpp"
#include <unistd.h>
#endif
#include <sys/fcntl.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <signal.h>
#include <cv.h>
#include <opencv2/highgui/highgui.hpp>

#define UNLOGALL "unlogall\r\n"            /*  */
#define ENABLE_EVENT_IN "EVENTINCONTROL MARK2 ENABLE\r\n"            /*  */
#define LOG_EVENT_IN "LOG MARK2TIMEA ONNEW\r\n"            /*  */
#define ENABLE_TRIGGER "EVENTOUTCONTROL MARK1 ENABLE POSITIVE 10000000 10000000\r\n"            /*  */

#define SEQSIZE 10            /*  */
#define MAXMSG 1024            /*  */
#define CRC32_POLYNOMIAL 0xEDB88320L

void err_ueye ( HIDS cam, int result, char *msg );
HIDS initCam ( int cam_num );
void getImage ( HIDS cam, char *dir, int show);
void autoGain ( HIDS cam );
#endif   /* ----- #ifndef grabframe_INC  ----- */
