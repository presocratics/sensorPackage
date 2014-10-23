/*
 * =====================================================================================
 *
 *       Filename:  grabframe.cpp
 *
 *    Description:  grabs frame and saves it to outdir
 *
 *        Version:  1.0
 *        Created:  10/02/2014 10:24:53 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Hong-Bin Yoon (HBY), yoon48@illinois.edu
 *   Organization:  Aerospace Robotics and Controls Lab (ARC)
 *
 * =====================================================================================
 */

#include "grabframe.h"

int set_interface_attribs(int fd, int speed, int parity)
{
        struct termios tty;
        if( tcgetattr(fd, &tty)==-1 )
            err_sys("tcgetattr");

        cfsetospeed (&tty, speed);
        cfsetispeed (&tty, speed);

        tty.c_iflag &= ~(IGNBRK|BRKINT|ICRNL|INLCR|PARMRK|INPCK|ISTRIP|IXON);
        tty.c_oflag &= ~(OCRNL | ONLCR | ONLRET |
                     ONOCR | OFILL | OLCUC | OPOST);                // no remapping, no delays
        tty.c_lflag &= ~(ECHO|ECHONL|IEXTEN|ISIG);;                // no signaling chars, no echo,
        tty.c_lflag |= ICANON;
                                        // no canonical processing

        tty.c_cflag &= ~(CSIZE|PARENB);
        tty.c_cflag |= CS8 ;

        if( tcsetattr(fd, TCSANOW, &tty)==-1 )
            err_sys("tcsetattr");
        return 0;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  initGPS
 *  Description:  Connects to the GPS, turns on event logging, and turns on
 *  trigger pulse.
 * =====================================================================================
 */
    void
initGPS ( char* device, int *spanFD )
{
    if( (*spanFD=open( device, O_RDWR | O_NOCTTY ))==-1 )
        err_sys("open: %s", device);
    set_interface_attribs( *spanFD, B115200, 0 );
    // turn off current logging
    //if( write( *spanFD, UNLOGALL, strlen(UNLOGALL) )==-1 )
    //    err_sys("write: %s", UNLOGALL);
    // enable input event port
    if( write( *spanFD, ENABLE_EVENT_IN, strlen(ENABLE_EVENT_IN) )==-1 )
        err_sys("write: %s", ENABLE_EVENT_IN);
    // enable input event log
    if( write( *spanFD, LOG_EVENT_IN, strlen(LOG_EVENT_IN) )==-1 )
        err_sys("write: %s", LOG_EVENT_IN);
    // turn on triggering
    if( write( *spanFD, ENABLE_TRIGGER, strlen(ENABLE_TRIGGER) )==-1 )
        err_sys("write: %s", ENABLE_TRIGGER);
    return;
}		/* -----  end of function initGPS  ----- */


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  err_ueye
 *  Description:  
 * =====================================================================================
 */
    void
err_ueye ( HIDS cam, int result )
{
    char *str;
    is_GetError( cam, &result, &str );
    fprintf( stderr, "ueye error %d: %s\n", result, str );
    return;
}		/* -----  end of function err_ueye  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  initCam
 *  Description:  Connects to camera, allocates memory, enables trigger mode.
 * =====================================================================================
 */
    HIDS
initCam ()
{
    HIDS cam;
    int frameWidth, frameHeight;
    int bitsPerPixel;
    int rv;
    char* frameMemory[SEQSIZE];
    int memoryID[SEQSIZE];
    unsigned int pixelClockRange[3];
    unsigned int maxPixelClock;
    double on = 1;
    double empty;
    cam = (HIDS) 0;
    if( (rv=is_InitCamera( &cam, NULL ))!=IS_SUCCESS )
        err_ueye(cam, rv);
    if( (rv=is_SetColorMode( cam, IS_CM_MONO8))!=IS_SUCCESS )
        err_ueye(cam, rv);
    if( (rv=is_SetAutoParameter( cam, IS_SET_ENABLE_AUTO_GAIN, &on, &empty))!=IS_SUCCESS )
        err_ueye(cam, rv);
    bitsPerPixel=8;
    frameWidth=1600;
    frameHeight=1200;

    // Initialize memory
    if( (rv=is_ClearSequence(cam))!=IS_SUCCESS )
        err_ueye(cam, rv);
    int i;
    for( i=0; i<SEQSIZE; ++i )
    {
        if( (rv=is_AllocImageMem( cam, frameWidth, frameHeight, bitsPerPixel,
                        &frameMemory[i], &memoryID[i]))!=IS_SUCCESS )
        {
            err_ueye(cam, rv);
        }
        if( (rv=is_AddToSequence( cam, frameMemory[i], memoryID[i] ))!=IS_SUCCESS )
        {
            err_ueye(cam, rv);
        }
    }

    // Set max available pixel clock
    if( (rv=is_PixelClock(cam, IS_PIXELCLOCK_CMD_GET_RANGE,
                    (void*)pixelClockRange, 
                    sizeof(pixelClockRange)))!=IS_SUCCESS )
    {
        err_ueye(cam, rv);
    }
    maxPixelClock = pixelClockRange[1];
    if( (rv=is_PixelClock(cam, IS_PIXELCLOCK_CMD_SET,(void*)&maxPixelClock, 
                    sizeof(maxPixelClock)))!=IS_SUCCESS )
    {
        err_ueye(cam, rv);
    }

    // Set external trigger
    if( (rv=is_SetExternalTrigger(cam, IS_SET_TRIGGER_LO_HI))!=IS_SUCCESS )
        err_ueye(cam, rv);

    // Begin transmission
    if( (rv=is_CaptureVideo(cam, IS_WAIT))!=IS_SUCCESS )
        err_ueye(cam, rv);
    if( (rv=is_EnableEvent(cam, IS_SET_EVENT_FRAME))!=IS_SUCCESS )
        err_ueye(cam, rv);

    return cam;
}		/* -----  end of function initCam  ----- */

/* --------------------------------------------------------------------------
Calculate a CRC value to be used by CRC calculation functions.
-------------------------------------------------------------------------- */
unsigned long CRC32Value(int i)
{
    int j;
    unsigned long ulCRC;
    ulCRC = i;
    for ( j = 8 ; j > 0; j-- )
    {
        if ( ulCRC & 1 )
            ulCRC = ( ulCRC >> 1 ) ^ CRC32_POLYNOMIAL;
        else
            ulCRC >>= 1;
    }
    return ulCRC;
}

/* --------------------------------------------------------------------------
Calculates the CRC-32 of a block of data all at once
-------------------------------------------------------------------------- */
unsigned long CalculateBlockCRC32( unsigned long ulCount, /* Number of bytes in the data block */
                                   unsigned char *ucBuffer ) /* Data block */
{
    unsigned long ulTemp1;
    unsigned long ulTemp2;
    unsigned long ulCRC = 0;
    while ( ulCount-- != 0 )
    {
        ulTemp1 = ( ulCRC >> 8 ) & 0x00FFFFFFL;
        ulTemp2 = CRC32Value( ((int) ulCRC ^ *ucBuffer++ ) & 0xff );
        ulCRC = ulTemp1 ^ ulTemp2;
    }
    return( ulCRC );
}
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  extractMsg
 *  Description:  Points msg to the beginning of the message, points crc to the
 *  beginning of the crc, sets the * to '\0'.
 * =====================================================================================
 */
void extractMsg( char *buf, char **msg, char **crc )
{
    unsigned long len;
    *msg=buf+1;
    len=strlen(*msg);
    len-=11; // Exclude the checksum.
    *(*msg+len)='\0';
    *crc=*msg+len+1;
    return;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  isValid
 *  Description:  Returns true if calculated CRC matches given CRC.
 * =====================================================================================
 */
int isValid( char *buf )
{
    unsigned long int len, CRC, givenCRC;
    char *msg, *givenASCII;

    extractMsg( buf, &msg, &givenASCII ); 

    len=strlen(msg);
    CRC = CalculateBlockCRC32( len, (unsigned char *)msg );

    sscanf( givenASCII, "%lx", &givenCRC);
    return (int)(CRC-givenCRC)==0 ; // Not sure why I need the typecast.
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  getLatestTimestamp
 *  Description:  returns 0 on success, -1 on failure.
 * =====================================================================================
 */
    int
getLatestTimestamp ( FILE *gps, char *buf )
{
    buf[0]='a';
    while(buf[0]!='#')
        fgets( buf, MAXMSG, gps );
    /* Checksum */
    if( !isValid( buf ) )
    {
        fprintf( stderr, "Bad checksum: %s\n", buf );
        return -1;
    }
    return 0;
}		/* -----  end of function getLatestTimestamp  ----- */
/*
 * ===  FUNCTION  ======================================================================
 *         Name:  main
 *  Description:  
 * =====================================================================================
 */
int main(int argc, char* argv[])
{
    if( argc!=2 )
    {
        printf("Usage: %s device\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int gps;
    FILE *gpsFile;
    HIDS camera;
    IMAGE_FILE_PARAMS ImageFileParams;

    initGPS( argv[1], &gps );
    gpsFile = fdopen( gps, "r" );

    camera = initCam( );

    ImageFileParams.pwchFileName = NULL;
    ImageFileParams.pnImageID = NULL;
    ImageFileParams.ppcImageMem = NULL;
    ImageFileParams.nFileType = IS_IMG_BMP;
    ImageFileParams.nQuality=100;
    fflush(gpsFile);
    tcflush(gps, TCIFLUSH);

    int i=0;
    while(1)
    {
        char buf[MAXMSG];
        wchar_t buffer[100];
        int rv;
        char *currentFrame;
        if( (rv=is_WaitEvent(camera, IS_SET_EVENT_FRAME, INFINITE))!=IS_SUCCESS )
            err_ueye(camera, rv);
        if( (rv=is_GetActSeqBuf(camera, NULL, NULL, &currentFrame))!=IS_SUCCESS )
            err_ueye(camera, rv);
        if( (rv=is_LockSeqBuf(camera, IS_IGNORE_PARAMETER, currentFrame))!=IS_SUCCESS )
            err_ueye(camera, rv);
        if( (rv=is_UnlockSeqBuf(camera, IS_IGNORE_PARAMETER, currentFrame))!=IS_SUCCESS )
            err_ueye(camera, rv);
        swprintf(buffer, 100, L"images/%010d.bmp",i);
        ImageFileParams.pwchFileName = buffer;
        if( (rv=is_ImageFile( camera, IS_IMAGE_FILE_CMD_SAVE, (void*) &ImageFileParams,
                sizeof(ImageFileParams) ))!=IS_SUCCESS )
        {
            err_ueye(camera, rv);
        }
        if( getLatestTimestamp(gpsFile, buf)==-1 )
            fprintf(stderr, "bad CRC\n");
        else
            printf("%d,%s\n", ++i, buf);
        fflush(gpsFile);
    }
    if( write( gps, UNLOGALL, strlen(UNLOGALL) )==-1 )
        err_sys("write: %s", UNLOGALL);
    close(gps);
    is_ExitCamera(camera);
    printf("success\n");
}				/* ----------  end of function main  ---------- */


