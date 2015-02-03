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

// These are made global so that we can access them after SIGINT.
HIDS *camera; 
char **dirs; 
int num_cams;

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  signal_callback_handler
 *  Description:  Handles SIGINT (^C).
 * =====================================================================================
 */
    void
signal_callback_handler ( int signum )
{
    printf("Caught signal %d\n", signum);
    int i;
    for( i=0; i<num_cams; ++i )
    {
        int rv;
        printf("Closing cam %d of %d (%d)\n", i+1, num_cams, camera[i]);
        if( (rv=is_ExitCamera(camera[i]))!=IS_SUCCESS );
            err_ueye(camera[i], rv, "Exit camera.");
        free (dirs[i]);
    }
    free(dirs);
    camera=NULL;
    dirs	= NULL;
    exit( signum );
}		/* -----  end of function signal_callback_handler  ----- */

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
    {
        err_ueye(cam, rv, "InitCamera.");
        fprintf(stderr, "Failed to init cam %d\n", cam );
        exit(EXIT_FAILURE);
    }

    // Set camera exposure modes
    if( (rv=is_SetColorMode( cam, IS_CM_MONO8))!=IS_SUCCESS )
        err_ueye(cam, rv, "SetColorMode.");
    if( (rv=is_SetAutoParameter( cam, IS_SET_ENABLE_AUTO_GAIN, &on, &empty))!=IS_SUCCESS )
        err_ueye(cam, rv, "SetAutoGain.");
    //if( (rv=is_SetAutoParameter( cam, IS_SET_ENABLE_AUTO_SHUTTER, &on, &empty))!=IS_SUCCESS )
     //   err_ueye(cam, rv, "EnableAutoShutter.");
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

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  getImage
 *  Description:  Gets the next image from cam, saves it, and prints its info to
 *  STDOUT.
 * =====================================================================================
 */
    void
getImage ( HIDS cam, char *dir, int show )
{
    wchar_t buffer[100];
    IMAGE_FILE_PARAMS ImageFileParams;
    int rv;
    char *currentFrame;
    int timedout;

    int frameId;
    uint64_t framenumber;
    uint64_t u64TimestampDevice;
    UEYEIMAGEINFO ImageInfo;

    // Setup file saving params.
    ImageFileParams.pwchFileName = NULL;
    ImageFileParams.pnImageID = NULL;
    ImageFileParams.ppcImageMem = NULL;
    ImageFileParams.nFileType = IS_IMG_BMP;
    ImageFileParams.nQuality=100;

    // Manage memory for the next frame.
    if( (rv=is_GetActiveImageMem(cam, &currentFrame, &frameId))!=IS_SUCCESS )
        err_ueye(cam, rv, "GetActSeqBuf.");
    timedout=0;
    if( (rv=is_WaitEvent(cam, IS_SET_EVENT_FRAME, 500))==IS_NO_SUCCESS )
    {
        err_ueye(cam, rv, "Wait Event.");
    }
    else if( rv==IS_TIMED_OUT )
    {
        timedout=1;
    }
    if( (rv=is_LockSeqBuf(cam, IS_IGNORE_PARAMETER, currentFrame))!=IS_SUCCESS )
        err_ueye(cam, rv, "LockSeqBuf.");

    // Get info about the latest frame.
    if( (rv=is_GetImageInfo( cam, frameId, &ImageInfo, sizeof(ImageInfo)))!=IS_SUCCESS )
        err_ueye(cam, rv, "GetImageInfo.");

    framenumber=ImageInfo.u64FrameNumber;
    u64TimestampDevice = ImageInfo.u64TimestampDevice;  

    // Print logging info.
    printf("%d,%lu,%020ld,%02d/%02d/%04d %02d:%02d:%02d.%03d,%d\n", 
            cam,
            framenumber,
            u64TimestampDevice,
            ImageInfo.TimestampSystem.wMonth,
            ImageInfo.TimestampSystem.wDay, 
            ImageInfo.TimestampSystem.wYear,
            ImageInfo.TimestampSystem.wHour,
            ImageInfo.TimestampSystem.wMinute,
            ImageInfo.TimestampSystem.wSecond,
            ImageInfo.TimestampSystem.wMilliseconds,
            timedout);

    // Write to a Mat
    if( show==1 )
    {
        cv::Mat image(1200,1600,CV_8UC1, NULL, 1600);
        image.data = (uchar *) currentFrame;
        cv::imshow("image", image);
        cv::waitKey(1);
    }
    // Save the image.
    swprintf(buffer, 100, L"%s/%010d.bmp", dir, framenumber);
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
 *         Name:  incrGain
 *  Description:  Adjusts gain by delta.
 * =====================================================================================
 */
    void
incrGain ( HIDS cam, int delta )
{
    // Get current value
    int rv;
    int nm=IS_IGNORE_PARAMETER;
    if( (rv=is_SetHardwareGain(cam, IS_GET_MASTER_GAIN, nm, nm, nm))==IS_NO_SUCCESS )
        err_ueye(cam, rv, "Get master gain.");
    printf("gain: %d\n", rv);
    rv+=delta;
    // Set new value
    if( (rv=is_SetHardwareGain(cam, rv, nm, nm, nm))!=IS_SUCCESS )
        err_ueye(cam, rv, "Set master gain.");

    return;
}		/* -----  end of function incrGain  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  incrShutter
 *  Description:  
 * =====================================================================================
 */
    void
incrShutter ( HIDS cam, int delta )
{
    // Get current value
    int rv;
    double exposure;
    double range[3];
    if( (rv=is_Exposure(cam, IS_EXPOSURE_CMD_GET_EXPOSURE, (void*) &exposure,
                    sizeof(exposure)))!=IS_SUCCESS )
        err_ueye(cam, rv, "Get exposure.");
    printf("exposure: %f\n", exposure);
    if( (rv=is_Exposure(cam, IS_EXPOSURE_CMD_GET_EXPOSURE_RANGE, (void *) &range, 
                    sizeof(range)))!=IS_SUCCESS )
        err_ueye(cam, rv, "Get exposure range.");
    printf("min: %f max: %f incr: %f\n", range[0], range[1], range[2]);
    exposure+=delta;
    printf("new exposure: %f\n", exposure);
    if( (rv=is_Exposure(cam, IS_EXPOSURE_CMD_SET_EXPOSURE, (void*) &exposure, 
                    sizeof(exposure)))!=IS_SUCCESS)
        err_ueye(cam, rv, "Set exposure.");

    return ;
}		/* -----  end of function incrShutter  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  toggleGainBoost
 *  Description:  
 * =====================================================================================
 */
    void
toggleGainBoost ( HIDS cam )
{
    int rv;
    if( (rv=is_SetGainBoost(cam, IS_GET_GAINBOOST))==IS_NO_SUCCESS )
        err_ueye(cam, rv, "Get gain boost.");
    if( rv==IS_SET_GAINBOOST_ON )
    {
        rv=IS_SET_GAINBOOST_OFF;
    }
    else
    {
        rv=IS_SET_GAINBOOST_ON;
    }
    if( (rv=is_SetGainBoost(cam, rv))==IS_NO_SUCCESS )
        err_ueye(cam, rv, "Set gain boost.");

    return ;
}		/* -----  end of function toggleGainBoost  ----- */

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  main
 *  Description:  
 * =====================================================================================
 */
int main(int argc, char* argv[])
{
    char parentdir[100];

    // Get time for image storage dir.
    char timestr[200];
    time_t t;
    struct tm *tmp;

    // Register signal and signal handler.
    signal(SIGINT, signal_callback_handler);
    t=time(NULL);
    tmp=localtime(&t);

    if( strftime(timestr, sizeof(timestr), "%F-%H%M%S", tmp)==0 )
        err_sys("strftime");
    sprintf(parentdir, "./images/%s", timestr);
    if( mkdir(parentdir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH )==-1 )
        err_sys("mkdir %s", parentdir);

    if( is_GetNumberOfCameras(&num_cams)!=IS_SUCCESS )
    {
        fprintf(stderr, "Error retrieving number of cameras.\n");
        exit(EXIT_FAILURE);
    }
    
    camera	= (HIDS *) calloc ( (size_t)(num_cams), sizeof(HIDS) );
    if ( camera==NULL ) {
        fprintf ( stderr, "\ndynamic memory allocation failed\n" );
        exit (EXIT_FAILURE);
    }
    
    dirs	= (char **) calloc ( (size_t)(num_cams), sizeof(char*) );
    if ( dirs==NULL ) {
        fprintf ( stderr, "\ndynamic memory allocation failed\n" );
        exit (EXIT_FAILURE);
    }

    int cami;
    for( cami=0; cami<num_cams; ++cami ) 
    {
        camera[cami]=initCam(0);
        // Prepare directories to store images
        
        dirs[cami]	= (char *)calloc ( (size_t)(100), sizeof(char) );
        if ( dirs[cami]==NULL ) {
            fprintf ( stderr, "\ndynamic memory allocation failed\n" );
            exit (EXIT_FAILURE);
        }
        sprintf(dirs[cami], "%s/%d", parentdir, cami+1);
        if( mkdir(dirs[cami], S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH )==-1 )
            err_sys("mkdir %s", dirs[cami]);
    }

    int i=0;
    while(1)
    {
        // Handle key presses
        char key;
        key=cv::waitKey(1);
        
        switch ( key ) {
            case 'q':	
                signal_callback_handler ( SIGINT );
                break;

            case 'j':	
                printf("Gain down.\n");
                for( cami=0; cami<num_cams; ++cami ) incrGain(camera[cami], -5);
                break;

            case 'k':	
                printf("Gain up.\n");
                for( cami=0; cami<num_cams; ++cami ) incrGain(camera[cami], +1);
                break;

            case 'h':	
                printf("Decrease shutter time.\n");
                for( cami=0; cami<num_cams; ++cami ) incrShutter(camera[cami], -1);
                break;

            case 'l':	
                printf("Increase shutter time.\n");
                for( cami=0; cami<num_cams; ++cami ) incrShutter(camera[cami], +5);
                break;

            case 'b':	
                for( cami=0; cami<num_cams; ++cami ) toggleGainBoost(camera[cami]);
                break;


            default:	
                break;
        }				/* -----  end switch  ----- */



        ++i;
        for( cami=0; cami<num_cams; ++cami )
        {
            int show=(i%10==0) && (cami==0) ? 1 : 0;
            getImage(camera[cami], dirs[cami], show);
        }
    }
    for( cami=0; cami<num_cams; ++cami ) 
    {
        int rv;
        if( (rv=is_ExitCamera(camera[cami]))!=IS_SUCCESS )
            err_ueye(camera[cami], rv, "Exit camera.");
        free (dirs[cami]);
    }

    free (camera);
    camera	= NULL;
    free (dirs);
    dirs	= NULL;
    printf("success\n");
}				/* ----------  end of function main  ---------- */


