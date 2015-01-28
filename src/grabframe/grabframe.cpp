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
 *  Description:  
 * =====================================================================================
 */
    void
getImage ( HIDS cam )
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
    //is_ForceTrigger(cam);		
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

    // Save the image.
    swprintf(buffer, 100, L"images/cam%d-%010d.bmp", cam, framenumber);
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
    int num_cams;
    HIDS *camera;
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

    int cami;
    for( cami=0; cami<num_cams; ++cami ) camera[cami]=initCam(cami+1);

    int i=0;
    while(i<20)
    {
        ++i;
        for( cami=0; cami<num_cams; ++cami )
        {
            getImage(camera[cami]);
        }
       
    }
    for( cami=0; cami<num_cams; ++cami ) is_ExitCamera(camera[cami]);

    free (camera);
    camera	= NULL;
    printf("success\n");
}				/* ----------  end of function main  ---------- */


