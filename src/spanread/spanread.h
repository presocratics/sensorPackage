#ifndef  gpstest_INC
#define  gpstest_INC
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sstream>
#include <errno.h>
#include "ourerr.hpp"
#include <vector>
#include <iterator>
#include <termios.h>
#include <unistd.h>
#include <sys/fcntl.h>

// command line options
#define ARGGPS "-gps"
#define ARGIMU "-imu"
#define ARGATT "-att"
#define ARGPASHR "-pashr"            /*  */
#define ARGRAWIMU "-rawimu"            /*  */
// 
#define PARAMSZ 7            /*  */
#define DEVICE 0            /*  */
#define TYPE_GPS 1
#define TYPE_ATT 2
#define TYPE_IMU 3
#define TYPE_DT 4
#define TYPE_PASHR 5            /*  */
#define TYPE_RAWIMUE 6            /*  */
// message names
#define GPSNAME "#BESTPOSA"
#define ATTNAME "#INSATTA"
#define IMUNAME "#CORRIMUDATAA"
#define PASHRNAME "$PASHR"            /*  */
#define RAWIMUNAME "#RAWIMUA"            /*  */

#define CRC32_POLYNOMIAL 0xEDB88320L
#define MAXMSG 2048           /*  */

// messages
#define GETBESTPOS "LOG usb1 BESTPOSA ontime 1\r\n" /* Get best position message. */
#define GETATT "LOG usb1 INSATTA ontime 1\r\n" /* Get best position message. */
#define GETIMUDATA "LOG usb1 CORRIMUDATAA ontime 0.01\r\n"            /*  */
#define GETPASHR "LOG usb1 pashra ontime 1\r\n"            /*  */
#define STOPLOGGING "unlogall\r\n"            /*  */

unsigned long CRC32Value(int i);
unsigned long CalculateBlockCRC32( unsigned long ulCount, unsigned char *ucBuffer );
int set_interface_attribs(int fd, int speed, int parity);
void set_blocking(int fd, int should_block);
void extractMsg( char *buf, char **msg, char **crc );
int isValid( char *buf );
void help( char *arg0);
void readargs ( int argc, char **argv, char **p );

#endif   /* ----- #ifndef gpsTest_INC  ----- */
