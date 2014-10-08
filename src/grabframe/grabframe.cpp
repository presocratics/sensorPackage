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
 
    /* PixelClock */
    UINT nPixelClockDefault = 128;
    nRet = is_PixelClock(hCam, IS_PIXELCLOCK_CMD_SET,
                          (void*)&nPixelClockDefault,
                          sizeof(nPixelClockDefault));
 
    printf("Status is_PixelClock %d\n",nRet);
 
    /* Set ColorMode */
    //INT colorMode = IS_CM_CBYCRY_PACKED;
    INT colorMode = IS_CM_MONO8;
    //INT colorMode = IS_CM_BGR8_PACKED;
    //INT colorMode = IS_CM_SENSOR_RAW8;
    nRet = is_SetColorMode(hCam,colorMode);
    printf("Status SetColorMode %d\n",nRet);
   

    UINT formatID = 4;
    /* ImageSize= 1600x1200 */
    int w = 1600;
    int h = 1200;
    nRet = is_ImageFormat(hCam, IMGFRMT_CMD_SET_FORMAT, &formatID, 4);

    printf("Status ImageFormat %d\n",nRet);
 
    /* Memory Allocation */
    char* pMem = NULL;
    int memID = 0;
    nRet = is_AllocImageMem(hCam, w, h, 8, &pMem, &memID);
    printf("Status AllocImage %d\n",nRet);

    nRet = is_SetImageMem(hCam, pMem, memID);
    printf("Status SetImageMem %d\n",nRet);
   
    /* Set Display Mode */
    INT displayMode = IS_SET_DM_DIB;
    nRet = is_SetDisplayMode (hCam, displayMode);
    printf("Status displayMode %d\n",nRet);


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
     double FPS = 100;
     double NEWFPS;
     is_SetFrameRate(hCam,FPS,&NEWFPS);
    /* LOOP */

    printf("Status Initialization SUCCESS!\n");

    int n=1;
    char buffer[100];
    while(1)
    {
        cv::Mat frame(h,w,CV_8UC1, NULL,w);
        //cv::Mat frame(h,w,CV_8UC3, NULL);

        if(is_FreezeVideo(hCam, IS_WAIT)==IS_SUCCESS) // use trigger e.g. IS_SET_TRIGGER_LO_HI()
        //if(is_CaptureVideo(hCam, IS_GET_LIVE)==IS_SUCCESS)
        {
            void* pMemVoid;
            is_GetImageMem(hCam, &pMemVoid);
            frame.data = (uchar*)pMemVoid;
            //std::vector<cv::Mat> layers;
            //cv::split(frame, layers);

            is_GetFramesPerSecond(hCam, &fps);
            printf("frame rate: %f\n",fps);
            sprintf(buffer, "images/%010d.png",n);
            
            std::vector<int> compression_params;
            compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
            compression_params.push_back(3);

            try{
               // imwrite(buffer, frame, compression_params);
            }
            catch (std::runtime_error& ex) {
                fprintf(stderr, "Exception converting image to PNG format: %s\n", ex.what());
                return 1;
            }

            //cv::imshow("frame", frame);
            //cv::waitKey(1);
            n++;
            printf("processing image: %d\n",n);
        }
        else
        {
            //printf("-(!) ERROR unable to freeze frame\n");

        }

   
    }

   
    is_ExitCamera(hCam);
    return EXIT_SUCCESS;






}				/* ----------  end of function main  ---------- */


