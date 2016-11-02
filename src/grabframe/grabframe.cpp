/*
 * =====================================================================================
 *
 *       Filename:  grabframe.cpp
 *
 *    Description:  grabs frame and saves it to outdir
 *          Usage:  grabframe [parent_directory] [-d] [-1]
 *                  parent_directory: directory to store image in. Default: "."
 *                  -d  debug_mode: Images are not saved. No timeout on event
 *                  wait handler. Cannot be used with parent_directory.
 *                  -1 once: Implies -d, and exits after one frame is captured.
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
#define BINNING /* if defined, use binning. comment out to turn off */
// These are made global so that we can access them after SIGINT.
HIDS *camera; 
char **dirs; 
int64_t *offset;
int num_cams;
#ifndef __linux
HANDLE frameEvent[2];
#endif
int debug_mode = 0;

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
        if( (rv=is_ExitCamera(camera[i]))!=IS_SUCCESS )
            err_ueye(camera[i], rv, "Exit camera.");
        free (dirs[i]);
        if( (rv=is_DisableEvent(camera[i], IS_SET_EVENT_FRAME))!=IS_SUCCESS)
            err_ueye(camera[i], rv, "Disable event.");
    }
    free(dirs);
    free(camera);
    free (offset);
    offset	= NULL;
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
    fprintf( stderr, "cam %d: %s ueye error %d: %s\n", cam, msg, result, str );
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
    unsigned int desiredPixelClock=96;
    cam = (HIDS) cam_num;
    IS_RECT rectAOI;
    if( (rv=is_InitCamera( &cam, NULL ))!=IS_SUCCESS )
    {
        err_ueye(cam, rv, "InitCamera.");
        fprintf(stderr, "Failed to init cam %d\n", cam );
        exit(EXIT_FAILURE);
    }

    // Set camera exposure modes
    if( (rv=is_SetColorMode( cam, IS_CM_SENSOR_RAW8))!=IS_SUCCESS ) {
        err_ueye(cam, rv, "SetColorMode.");
        exit(EXIT_FAILURE);
    }
    autoGain(cam);
    bitsPerPixel=8;
#ifdef BINNING
    frameWidth=800;
    frameHeight=600;
#else
    frameWidth=1600;
    frameHeight=1200;
#endif

    // Initialize memory
    if( (rv=is_ClearSequence(cam))!=IS_SUCCESS ) {
        err_ueye(cam, rv, "ClearSequence.");
        exit(EXIT_FAILURE);
    }
    int i;
    for( i=0; i<SEQSIZE; ++i )
    {
        if( (rv=is_AllocImageMem( cam, frameWidth, frameHeight, bitsPerPixel,
                        &frameMemory[i], &memoryID[i]))!=IS_SUCCESS )
        {
            err_ueye(cam, rv, "AllocImageMem.");
            exit(EXIT_FAILURE);
        }
        if( (rv=is_AddToSequence( cam, frameMemory[i], memoryID[i] ))!=IS_SUCCESS )
        {
            err_ueye(cam, rv, "AddToSequence.");
            exit(EXIT_FAILURE);
        }
    }

    if( (rv=is_InitImageQueue(cam,0))!=IS_SUCCESS ) {
        err_ueye(cam, rv, "Init image queue");
        exit(EXIT_FAILURE);
    }

    // Set pixelclock to 64MHz. According to documentation, setting the
    // pixelclock higher will result in more frames lost. Therefore, it is not
    // a good idea to set this value to maxPixelClock unless we are actually
    // using the highest framerates.
    if( (rv=is_PixelClock(cam, IS_PIXELCLOCK_CMD_SET,(void*)&desiredPixelClock, 
                    sizeof(desiredPixelClock)))!=IS_SUCCESS )
    {
        err_ueye(cam, rv, "Set pixel clock.");
        exit(EXIT_FAILURE);
    }
    // Set exposure to 30ms. This will allow us to easily shoot at 25Hz.
    double exptime=3.;
    if( (rv=is_Exposure(cam, IS_EXPOSURE_CMD_SET_EXPOSURE, (void*)&exptime,
                    sizeof(exptime)))!=IS_SUCCESS )
    {
        err_ueye(cam, rv, "Set exposure.");
        exit(EXIT_FAILURE);
    }

    // Set external trigger input
    if( (rv=is_SetExternalTrigger(cam, IS_SET_TRIGGER_HI_LO))!=IS_SUCCESS ) {
        err_ueye(cam, rv, "Set external trigger hi lo.");
        exit(EXIT_FAILURE);
    }

    // Set flash strobe output
    UINT nMode = IO_FLASH_MODE_TRIGGER_HI_ACTIVE;
    if( (rv=is_IO( cam, IS_IO_CMD_FLASH_SET_MODE, (void *) &nMode, sizeof(nMode)))!=IS_SUCCESS ) {
        err_ueye(cam, rv, "Set flash mode trigger lo active.");
        exit(EXIT_FAILURE);
    }

#ifdef BINNING
    if( (rv=is_SetBinning(cam, IS_BINNING_2X_VERTICAL|IS_BINNING_2X_HORIZONTAL))!=IS_SUCCESS ) {
        err_ueye(cam, rv, "Set Binning.");
        exit(EXIT_FAILURE);
    }
#endif
    //Setting the auto brightness Area Of Interest (AOI).
    rectAOI.s32X = 360;
    rectAOI.s32Y = 270;
    rectAOI.s32Width = 80;
    rectAOI.s32Height = 60;
    if( (rv=is_AOI(cam,IS_AOI_AUTO_BRIGHTNESS_SET_AOI, (void*)&rectAOI, sizeof(rectAOI)))!= IS_SUCCESS ) {
        err_ueye(cam, rv, "Set AOI Auto Brightness.");
        exit(EXIT_FAILURE);
    }

    // Begin transmission
    if( (rv=is_CaptureVideo(cam, IS_DONT_WAIT))!=IS_SUCCESS ) {
        err_ueye(cam, rv, "CaptureVideo.");
        exit(EXIT_FAILURE);
    }
#ifndef __linux // Using Windows
    if( (rv=is_InitEvent(cam, frameEvent[cam-1], IS_SET_EVENT_FRAME))!=IS_SUCCESS ) {
        err_ueye(cam, rv, "Init event (Win).");
        exit(EXIT_FAILURE);
    }
#endif
    if( (rv=is_EnableEvent(cam, IS_SET_EVENT_FRAME))!=IS_SUCCESS ) {
        err_ueye(cam, rv, "Set event frame.");
        exit(EXIT_FAILURE);
    }

    return cam;
}		/* -----  end of function initCam  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  getImage
 *  Description:  Gets the next image from cam, saves it, and prints its info to
 *  STDOUT. Returns the camera time.
 * =====================================================================================
 */
    uint64_t
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
    timedout=0;
    if( (rv=is_WaitForNextImage(cam, (debug_mode) ? INFINITE : INFINITE, &currentFrame, &frameId))!=IS_SUCCESS )
    {
        err_ueye(cam, rv, "Wait for next image.");
    }
    else if( rv==IS_TIMED_OUT )
    {
        timedout=1;
    }

    // Get capture status
    UEYE_CAPTURE_STATUS_INFO capStat;
    if( (rv=is_CaptureStatus(cam, IS_CAPTURE_STATUS_INFO_CMD_GET, (void*) &capStat,
            sizeof(capStat)))==IS_SUCCESS)
    {
        if( capStat.adwCapStatusCnt_Detail[IS_CAP_STATUS_USB_TRANSFER_FAILED]!=0 )
        {
            fprintf(stderr, "Cam %d USB transfer failed. Operate fewer cameras on bus.\n", cam);
        }
        if( capStat.adwCapStatusCnt_Detail[IS_CAP_STATUS_API_NO_DEST_MEM]!=0 )
        {
            fprintf(stderr, "Cam %d No destination memory. Reduce FPS.\n", cam);
        }
        if( capStat.adwCapStatusCnt_Detail[IS_CAP_STATUS_API_CONVERSION_FAILED]!=0 )
        {
            fprintf(stderr, "Cam %d Conversion failed.\n", cam);
        }
        if( capStat.adwCapStatusCnt_Detail[IS_CAP_STATUS_API_IMAGE_LOCKED] )
        {
            fprintf(stderr, "Cam %d All destination buffers locked. Reduce FPS.\n", cam);
        }
        if( capStat.adwCapStatusCnt_Detail[IS_CAP_STATUS_DRV_OUT_OF_BUFFERS] )
        {
            fprintf(stderr, "Cam %d Out of Buffers. Reduce FPS.\n", cam);
        }
        if( capStat.adwCapStatusCnt_Detail[IS_CAP_STATUS_DRV_DEVICE_NOT_READY] )
        {
            fprintf(stderr, "Cam %d Device not ready. Check connection.\n", cam);
        }
        if( capStat.adwCapStatusCnt_Detail[IS_CAP_STATUS_DEV_TIMEOUT] )
        {
            fprintf(stderr, "Cam %d Image capture timeout. Reduce exposure time.\n", cam);
        }
    }
    is_CaptureStatus(cam, IS_CAPTURE_STATUS_INFO_CMD_RESET, NULL, 0);
    // Get info about the latest frame.
    if( (rv=is_GetImageInfo( cam, frameId, &ImageInfo, sizeof(ImageInfo)))!=IS_SUCCESS )
        err_ueye(cam, rv, "GetImageInfo.");

    framenumber=ImageInfo.u64FrameNumber;
    u64TimestampDevice = ImageInfo.u64TimestampDevice;  

    // Print logging info.
    printf("%d,%lu,%020ld,%02d/%02d/%04d %02d:%02d:%02d.%03d,%d", 
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
#ifdef BINNING
        cv::Mat image(600,800,CV_8UC1, NULL, 800);
        cv::Mat color(600,800,CV_8UC3, NULL, 800);
#else
        cv::Mat image(1200,1600,CV_8UC1, NULL, 1600);
        cv::Mat color(1200,1600,CV_8UC3, NULL, 1600);
#endif
        image.data = (uchar *) currentFrame;
        cvtColor(image, color, CV_BayerBG2BGR, 3);
        cv::imshow("image", color);
        cv::waitKey(1);
    }
    // Save the image.
    if (debug_mode==0) {
        swprintf(buffer, 100, L"%s/%010d.bmp", dir, framenumber);
        ImageFileParams.pwchFileName = buffer;
        if( (rv=is_ImageFile( cam, IS_IMAGE_FILE_CMD_SAVE, (void*) &ImageFileParams,
                sizeof(ImageFileParams) ))!=IS_SUCCESS )
        {
            err_ueye(cam, rv, "Save image.");
        }
    }

    if( (rv=is_UnlockSeqBuf(cam, IS_IGNORE_PARAMETER, currentFrame))!=IS_SUCCESS )
        err_ueye(cam, rv, "UnlockSeqBuf.");
    return u64TimestampDevice;
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
    //printf("gain: %d\n", rv);
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
incrShutter ( HIDS cam, double delta )
{
    // Get current value
    int rv;
    double exposure;
    if( (rv=is_Exposure(cam, IS_EXPOSURE_CMD_GET_EXPOSURE, (void*) &exposure,
                    sizeof(exposure)))!=IS_SUCCESS )
        err_ueye(cam, rv, "Get exposure.");
    //if( (rv=is_Exposure(cam, IS_EXPOSURE_CMD_GET_EXPOSURE_RANGE, (void *) &range, 
     //               sizeof(range)))!=IS_SUCCESS )
      //  err_ueye(cam, rv, "Get exposure range.");
    if (exposure+delta<7 && exposure+delta>0)
    {
        exposure+=delta;
    }
    else
    {
        fprintf(stderr,"Exposure :%f out of range (0,7)ms. Aborting.\n", exposure+delta);
    }
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
 *         Name:  main
 *  Description:  
 * =====================================================================================
 */
int main(int argc, char* argv[])
{
    // in debug mode, not images are written
    char pparent[100];
    char parentdir[100];
    char timestr[200];

    int once=0;

    // Get time for image storage dir.
    time_t t;
    struct tm *tmp;

    // Register signal and signal handler.
    signal(SIGINT, signal_callback_handler);
    t=time(NULL);
    tmp=localtime(&t);

    if (argc==2) { // Check for debug mode
        if (!strcmp("-d",argv[1])) {
            debug_mode=1;
        } else if (!strcmp("-1",argv[1])) { // Check for one time mode
                once=1;
                debug_mode=1;
        } else if (!strcmp("-h", argv[1])) { // Check for help
            fprintf(stderr, "%s [-d] [-1] [parent directory]\n \
                    -d\tDebug mode: Captured images are not saved.\n \
                    -1\tOne-time mode: Captures only one image and exits. Implies -d.\n \
                    PARENT DIRECTORY\tDirectory where camera images are stored. Timestamped subfolders are created.\n", argv[0]);
            exit(0);
        }
    } else if (argc>2) {
        fprintf(stderr, "grabframe takes at most one argument: <parent_directory>, -d, -1.\n \
                Use -h for help.\n");
        exit(1);
    }

#ifdef __linux
    if( strftime(timestr, sizeof(timestr), "%F-%H%M%S", tmp)==0 ) {
        err_sys("strftime");
        exit(EXIT_FAILURE);
    }
#else
    strftime(timestr, sizeof(timestr), "%Y-%m-%d-%H%M%S", tmp);
#endif
    if(argc==2 && debug_mode==0) // Parent dir is set
    {
        sprintf(pparent, "%s/images", argv[1]);
    }
    else
    {
        sprintf(pparent, "./images");
    }
    sprintf(parentdir, "%s/%s", pparent, timestr);
#ifdef __linux
    if( debug_mode==0 && mkdir(parentdir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH )==-1 ) {
        err_sys("mkdir %s", parentdir);
        exit(EXIT_FAILURE);
    }
#else
    if (debug_mode==0) _mkdir(parentdir);
#endif

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
    
    offset	= (int64_t *) calloc ( (size_t)(num_cams-1), sizeof(int64_t) );
    if ( offset==NULL ) {
        fprintf ( stderr, "\ndynamic memory allocation failed\n" );
        exit (EXIT_FAILURE);
    }




#ifndef __linux
    frameEvent[0]=CreateEvent(NULL,FALSE,FALSE,NULL);
    frameEvent[1]=CreateEvent(NULL,FALSE,FALSE,NULL);
#endif

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
        sprintf(dirs[cami], "%s/cam%d", parentdir, cami);
#ifdef __linux
        if( debug_mode==0 && mkdir(dirs[cami], S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH )==-1 ) {
            err_sys("mkdir %s", dirs[cami]);
            exit(EXIT_FAILURE);
        }
#else
        if (debug_mode==0) _mkdir(dirs[cami]);
#endif
    }
    fprintf(stderr,"Cameras are ready\n");

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

            case 'j':	// Decrease gain.
                for( cami=0; cami<num_cams; ++cami ) incrGain(camera[cami], -2);
                break;

            case 'k':	// Increase gain.
                for( cami=0; cami<num_cams; ++cami ) incrGain(camera[cami], +1);
                break;

            case 'h':	// Decrease exposure time.
                for( cami=0; cami<num_cams; ++cami ) incrShutter(camera[cami], -0.25);
                break;

            case 'l':	// Increase exposure time.
                for( cami=0; cami<num_cams; ++cami ) incrShutter(camera[cami], 0.25);
                break;

            case 'b':	// Toggle gain boost.
                for( cami=0; cami<num_cams; ++cami ) toggleGainBoost(camera[cami]);
                break;

            case 'a':	// Turn auto gain.
                for( cami=0; cami<num_cams; ++cami ) autoGain(camera[cami]);
                break;

            default:	
                break;
        }				/* -----  end switch  ----- */



        ++i;
        uint64_t prev,cur;
        int failed=0;
        for( cami=0; cami<num_cams; ++cami )
        {
            int show=(i%10==0) && (cami==0) ? 1 : 0;
            cur = getImage(camera[cami], dirs[cami], show);

            // Check offset for all cameras after the first.
            if (cami>0) 
            {
                if (offset[cami-1]==0) { // offset is unset
                    offset[cami-1]=cur-prev;
                } else if (abs(offset[cami-1]-(cur-prev))>MAXOFFSET) { 
                    // The current offset is too far from the initial.
                    ++failed;
                }
            }

            // Handle printing.
            if (cami<num_cams-1) {
                printf(",");
            } else {
                printf(",%d\n", failed);
            }

            prev = cur;

        }
        if (once) break;
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
    exit(0);
    free (offset);
    offset	= NULL;
}				/* ----------  end of function main  ---------- */


