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
err_ueye ( HIDS cam, int result, char *msg )
{
    char *str;
    is_GetError( cam, &result, &str );
    fprintf( stderr, "%s ueye error %d: %s\n", msg, result, str );
    return;
}		/* -----  end of function err_ueye  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  initCam
 *  Description:  Connects to camera, allocates memory, enables trigger mode.
 * =====================================================================================
 */
    HIDS
initCam ( int cam_num )
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
    cam = (HIDS) cam_num;
    if( (rv=is_InitCamera( &cam, NULL ))!=IS_SUCCESS )
        err_ueye(cam, rv, "InitCamera.");

    // Set camera exposure modes
    if( (rv=is_SetColorMode( cam, IS_CM_MONO8))!=IS_SUCCESS )
        err_ueye(cam, rv, "SetColorMode.");
    if( (rv=is_SetAutoParameter( cam, IS_SET_ENABLE_AUTO_GAIN, &on, &empty))!=IS_SUCCESS )
        err_ueye(cam, rv, "SetAutoGain.");
    if( (rv=is_SetAutoParameter( cam, IS_SET_ENABLE_AUTO_SHUTTER, &on, &empty))!=IS_SUCCESS )
        err_ueye(cam, rv, "EnableAutoShutter.");
    bitsPerPixel=8;
    frameWidth=1600;
    frameHeight=1200;

    // Initialize memory
    if( (rv=is_ClearSequence(cam))!=IS_SUCCESS )
        err_ueye(cam, rv, "ClearSequence.");
    int i;
    for( i=0; i<SEQSIZE; ++i )
    {
        if( (rv=is_AllocImageMem( cam, frameWidth, frameHeight, bitsPerPixel,
                        &frameMemory[i], &memoryID[i]))!=IS_SUCCESS )
        {
            err_ueye(cam, rv, "AllocImageMem.");
        }
        if( (rv=is_AddToSequence( cam, frameMemory[i], memoryID[i] ))!=IS_SUCCESS )
        {
            err_ueye(cam, rv, "AddToSequence.");
        }
    }

    // Set max available pixel clock
    if( (rv=is_PixelClock(cam, IS_PIXELCLOCK_CMD_GET_RANGE,
                    (void*)pixelClockRange, 
                    sizeof(pixelClockRange)))!=IS_SUCCESS )
    {
        err_ueye(cam, rv, "Get pixel clock range.");
    }
    maxPixelClock = pixelClockRange[1];
    if( (rv=is_PixelClock(cam, IS_PIXELCLOCK_CMD_SET,(void*)&maxPixelClock, 
                    sizeof(maxPixelClock)))!=IS_SUCCESS )
    {
        err_ueye(cam, rv, "Set pixel clock.");
    }

    // Set external trigger input
    if( (rv=is_SetExternalTrigger(cam, IS_SET_TRIGGER_HI_LO))!=IS_SUCCESS )
        err_ueye(cam, rv, "Set external trigger hi lo.");

    // Set flash strobe output
    UINT nMode = IO_FLASH_MODE_TRIGGER_LO_ACTIVE;
    if( (rv=is_IO( cam, IS_IO_CMD_FLASH_SET_MODE, (void *) &nMode, sizeof(nMode)))!=IS_SUCCESS )
        err_ueye(cam, rv, "Set flash mode trigger lo active.");

    // Begin transmission
    if( (rv=is_CaptureVideo(cam, IS_DONT_WAIT))!=IS_SUCCESS )
        err_ueye(cam, rv, "CaptureVideo.");
    if( (rv=is_EnableEvent(cam, IS_SET_EVENT_FRAME))!=IS_SUCCESS )
        err_ueye(cam, rv, "Set event frame.");

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
 *         Name:  getImage
 *  Description:  
 * =====================================================================================
 */
    void
getImage ( HIDS cam, wchar_t *buffer )
{
    IMAGE_FILE_PARAMS ImageFileParams;
    int rv;
    char *currentFrame;

    ImageFileParams.pwchFileName = NULL;
    ImageFileParams.pnImageID = NULL;
    ImageFileParams.ppcImageMem = NULL;
    ImageFileParams.nFileType = IS_IMG_BMP;
    ImageFileParams.nQuality=100;

    is_ForceTrigger(cam);		
    if( (rv=is_WaitEvent(cam, IS_SET_EVENT_FRAME, 500))!=IS_SUCCESS )
        err_ueye(cam, rv, "Wait Event.");
    if( (rv=is_GetActSeqBuf(cam, NULL, NULL, &currentFrame))!=IS_SUCCESS )
        err_ueye(cam, rv, "GetActSeqBuf.");
    if( (rv=is_LockSeqBuf(cam, IS_IGNORE_PARAMETER, currentFrame))!=IS_SUCCESS )
        err_ueye(cam, rv, "LockSeqBuf.");

    ImageFileParams.pwchFileName = buffer;
    if( (rv=is_ImageFile( cam, IS_IMAGE_FILE_CMD_SAVE, (void*) &ImageFileParams,
            sizeof(ImageFileParams) ))!=IS_SUCCESS )
    {
        err_ueye(cam, rv, "Save image.");
    }

    if( (rv=is_UnlockSeqBuf(cam, IS_IGNORE_PARAMETER, currentFrame))!=IS_SUCCESS )
        err_ueye(cam, rv, "UnlockSeqBuf.");
    return;
}		/* -----  end of function getImage  ----- */

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  main
 *  Description:  
 * =====================================================================================
 */
int main(int argc, char* argv[])
{
    HIDS camera1, camera2;

    camera1 = initCam(1);
    camera2 = initCam(2);


    int i=0;
    while(i<30)
    {
        //int frameId;
        //uint64_t framenumber;
        wchar_t buffer[100];
        swprintf(buffer, 100, L"images/R%010d.bmp",++i);
        getImage(camera1, buffer);

        swprintf(buffer, 100, L"images/L%010d.bmp",i);
        getImage(camera2, buffer);
        //UEYEIMAGEINFO ImageInfo;
        //if( (rv=is_GetImageInfo( camera, frameId, &ImageInfo, sizeof(ImageInfo)))!=IS_SUCCESS )
        //    err_ueye(camera, rv);
        //framenumber=ImageInfo.u64FrameNumber;
        //printf("%lu\n", framenumber);
       
     //   if( getLatestTimestamp(gpsFile, buf)==-1 )
      //      fprintf(stderr, "bad CRC\n");
       // else
        //    printf("%d,%s\n", ++i, buf);
        //fflush(gpsFile);
    }
    //if( write( gps, UNLOGALL, strlen(UNLOGALL) )==-1 )
     //   err_sys("write: %s", UNLOGALL);
    //close(gps);
    is_ExitCamera(camera1);
    is_ExitCamera(camera2);
    printf("success\n");
}				/* ----------  end of function main  ---------- */


