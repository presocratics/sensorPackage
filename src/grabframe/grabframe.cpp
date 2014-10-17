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


#include <iostream>
#include <uEye.h>
#include <stddef.h>
#include <stdio.h>
#include <opencv2/highgui/highgui.hpp>
#include <string>
#include <stdexcept>
#include <wchar.h>

void err_sys(INT n)
{
    if(n != IS_SUCCESS)
    {
        for(int i=0;i<10;i++)
        {
            printf("-(!) SOMETHING WENT WRONG... \n");
            sleep(1);
        }
        sleep(10);
    }
    return;
}

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  main
 *  Description:  
 * =====================================================================================
 */

 int main(int argc, char* argv[])
{

    HIDS hCam = 1;

    printf("Success-Code: %d\n",IS_SUCCESS);

    /*
    *=====================================
    *  CAMERA INITIALIZATION
    *=====================================
    */
    
    /* Initialize Camera */
    INT nRet = is_InitCamera (&hCam, NULL);
    printf("Status Init %d\n",nRet);
    err_sys(nRet); 

    /* PixelClock */
    UINT nPixelClockDefault = 128;
    nRet = is_PixelClock(hCam, IS_PIXELCLOCK_CMD_SET,
                          (void*)&nPixelClockDefault,
                          sizeof(nPixelClockDefault));
 
    printf("Status is_PixelClock %d\n",nRet);
    err_sys(nRet); 
    /* Set ColorMode */
    //INT colorMode = IS_CM_CBYCRY_PACKED;
    INT colorMode = IS_CM_MONO8;
    //INT colorMode = IS_CM_BGR8_PACKED;
    //INT colorMode = IS_CM_SENSOR_RAW8;
    nRet = is_SetColorMode(hCam,colorMode);
    printf("Status SetColorMode %d\n",nRet);
    err_sys(nRet); 

    UINT formatID = 20;
    /* ImageSize= 1600x1200 */
    int w = 1600;
    int h = 1200;
    nRet = is_ImageFormat(hCam, IMGFRMT_CMD_SET_FORMAT, &formatID, 4);
    printf("Status ImageFormat %d\n",nRet);
 
    err_sys(nRet); 
    /* Memory Allocation */
    char* pMem = NULL;
    int memID = 0;
    nRet = is_AllocImageMem(hCam, w, h, 8, &pMem, &memID);
    printf("Status AllocImage %d\n",nRet);
    err_sys(nRet); 

    nRet = is_SetImageMem(hCam, pMem, memID);
    printf("Status SetImageMem %d\n",nRet);
    err_sys(nRet); 

    /* Set Display Mode */
    INT displayMode = IS_SET_DM_DIB;
    nRet = is_SetDisplayMode (hCam, displayMode);
    printf("Status displayMode %d\n",nRet);

    /* Set Auto Settings */
    double on = 1;
    double empty;
    nRet = is_SetAutoParameter(hCam, IS_SET_ENABLE_AUTO_WHITEBALANCE, &on, &empty);
    printf("Auto White Balance %d\n", nRet);

    nRet = is_SetAutoParameter(hCam, IS_SET_ENABLE_AUTO_GAIN, &on, &empty);
    printf("Auto Gain %d\n", nRet);


     
    // is_SetExternalTrigger (HIDS hCam, INT nTriggerMode) // TODO: use when
    // external trigger
    //
    //INT captureMode = IS_SET_CM_FRAME;
    //nRet = is_SetCaptureMode(hCam, captureMode);
    /*  
     double enable = 1;
     double disable = 0;
     is_SetAutoParameter (hCam, IS_SET_ENABLE_AUTO_GAIN, &enable, 0);
     is_SetAutoParameter (hCam, IS_SET_ENABLE_AUTO_WHITEBALANCE, &enable, 0);
     is_SetAutoParameter (hCam, IS_SET_ENABLE_AUTO_FRAMERATE, &disable, 0);
     is_SetAutoParameter (hCam, IS_SET_ENABLE_AUTO_SHUTTER, &disable, 0);
     is_SetAutoParameter (hCam, IS_SET_ENABLE_AUTO_SENSOR_GAIN, &enable, 0);
     is_SetAutoParameter (hCam, IS_SET_ENABLE_AUTO_SENSOR_WHITEBALANCE,&enable,0);
     is_SetAutoParameter (hCam, IS_SET_ENABLE_AUTO_SENSOR_SHUTTER, &disable, 0);
*/


     double fps;
     //double FPS = 100;
     //double NEWFPS;
     //is_SetFrameRate(hCam,FPS,&NEWFPS);
    /* LOOP */

    int n=1;
    IMAGE_FILE_PARAMS ImageFileParams;

    ImageFileParams.pwchFileName = NULL;
    ImageFileParams.pnImageID = NULL;
    ImageFileParams.ppcImageMem = NULL;

    ImageFileParams.nFileType = IS_IMG_BMP;
    ImageFileParams.nQuality=100;

    
    nRet =  is_SetExternalTrigger (hCam, IS_SET_TRIGGER_LO_HI);
    printf("isSetTrigger %d\n",nRet);
    
    nRet =  is_SetTimeout (hCam, IS_TRIGGER_TIMEOUT, 10000);
    printf("isTriggerTimeout %d\n",nRet);
    printf("passed EnableEvent\n");
/*  
    UINT nMode;

    nRet = is_IO(hCam, IS_GPIO_INPUT , (void*)&nMode, sizeof(nMode));
    printf("is_gpio_in %d\n",nRet);
    
    nRet = is_IO(hCam, IO_FLASH_MODE_TRIGGER_HI_ACTIVE, (void*)&nMode, sizeof(nMode));
    printf("trig %d\n",nRet);

    nRet = is_IO(hCam, IO_FLASH_MODE_GPIO_1, (void*)&nMode, sizeof(nMode));
    printf("gpio %d\n",nRet);

    nRet = is_IO(hCam, IO_GPIO_1 , (void*)&nMode, sizeof(nMode));
    printf("IO_gpio_1 %d\n",nRet);
    sleep(1);

*/
    
    is_EnableEvent( hCam, IS_SET_EVENT_FRAME );
    
    
    


    nRet = is_CaptureVideo(hCam, IS_DONT_WAIT)==IS_SUCCESS;
     
    /* Check Trigger Mode */
/*  
    INT nSupportedTriggerModes = is_SetExternalTrigger(hCam, IS_GET_SUPPORTED_TRIGGER_MODE);
    
    if ((nSupportedTriggerModes & IS_SET_TRIGGER_SOFTWARE) == IS_SET_TRIGGER_SOFTWARE)
    {
        printf("using SW Trigger \n");
    }

    if ((nSupportedTriggerModes & IS_SET_TRIGGER_HI_LO) == IS_SET_TRIGGER_HI_LO)
    {
        printf("using HI_LO Trigger \n");
    } 

    if ((nSupportedTriggerModes & IS_SET_TRIGGER_LO_HI) == IS_SET_TRIGGER_LO_HI)
    {
        printf("using LO_HI Trigger \n");
    }
 */
    /* #define IO_FLASH_MODE_OFF                   0
#define IO_FLASH_MODE_TRIGGER_LO_ACTIVE     1
#define IO_FLASH_MODE_TRIGGER_HI_ACTIVE     2
#define IO_FLASH_MODE_CONSTANT_HIGH         3
#define IO_FLASH_MODE_CONSTANT_LOW          4
#define IO_FLASH_MODE_FREERUN_LO_ACTIVE     5
#define IO_FLASH_MODE_FREERUN_HI_ACTIVE  */
    while(1)
    {
        wchar_t buffer[100];
        //if(is_FreezeVideo(hCam, IS_WAIT)==IS_SUCCESS)
        //if(1)
        {
            printf("waiting for trigger\n");
            is_WaitEvent( hCam, IS_SET_EVENT_FRAME, 1000000 );
            printf("waited\n");
            cv::Mat frame(h,w,CV_8UC1);
            void* pMemVoid;
            is_GetImageMem(hCam, &pMemVoid);
            frame.data = (uchar*) pMemVoid;
            is_GetFramesPerSecond(hCam, &fps);
            printf("frame rate: %f\n",fps);

            swprintf(buffer, 100, L"images/%010d.bmp",n);
            ImageFileParams.pwchFileName = buffer;
            cv::imshow("frame", frame);
            cv::waitKey(1);
            is_ImageFile( hCam, IS_IMAGE_FILE_CMD_SAVE, (void*)
                    &ImageFileParams, sizeof(ImageFileParams) );
            
            n++;
            printf("processing image: %d\n",n);
        }
        //else
        {
            //printf("-(!) ERROR unable to freeze frame\n");

        }

   
    }

   
    is_ExitCamera(hCam);
    return EXIT_SUCCESS;






}				/* ----------  end of function main  ---------- */


