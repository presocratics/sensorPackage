#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <cv.h>
#include <cvaux.h>
#include <ml.h>
#include <cxcore.h> 
#include <ueye.h>
#include <cvwimage.h>
#include <highgui.h>
#include <cxmisc.h>
#include <cstring>

using namespace cv;
using namespace std;
int main(int argc, char** argv )
{
	HIDS hCam = 1;
	HIDS hCam2 = 2;
	
	char* imgMem[10];
	int memId[10];
	
 	unsigned int pixelClockRange[3];
	unsigned int maxPixelClock;

	unsigned int pixelClockRange2[3];
	unsigned int maxPixelClock2;	
	
	IMAGE_FILE_PARAMS ImageFileParams;	
	
	char* imgMem2[10];
	int memId2[10];
	IMAGE_FILE_PARAMS ImageFileParams2;

	ImageFileParams.nQuality = 0;
	ImageFileParams.nFileType = IS_IMG_BMP;
	wchar_t buffer[100];

	ImageFileParams2.nQuality = 0;
	ImageFileParams2.nFileType = IS_IMG_BMP;
	wchar_t buffer2[100];
	
	UEYEIMAGEINFO ImageInfo;

	UEYEIMAGEINFO ImageInfo2;
	
	int pnNum;
	char* activeMem;

	int pnNum2;
	char* activeMem2;
	
	if(is_InitCamera (&hCam, NULL)!= IS_SUCCESS){
		printf("cam 1 failed");     
  	 	return 0;
     	}	


	is_SetColorMode( hCam, IS_CM_MONO8);

	if(is_InitCamera (&hCam2, NULL)!= IS_SUCCESS){
		printf("cam 2 failed");     
	 	return 0;
	}
	is_SetColorMode( hCam2, IS_CM_MONO8);
	int img_width=1600, img_height=1200, img_bpp=8,img_step, img_data_size;
	for(int i=0; i<10; ++i )
	{
		is_AllocImageMem( hCam, img_width,img_height,img_bpp,&imgMem[i],&memId[i]);
		is_AddToSequence( hCam, imgMem[i],memId[i] );
	}

	for(int i=0; i<10; ++i )
	{
		is_AllocImageMem( hCam2, img_width,img_height,img_bpp,&imgMem2[i],&memId2[i]);
		is_AddToSequence( hCam2, imgMem2[i],memId2[i] );
	}
	
	is_PixelClock(hCam, IS_PIXELCLOCK_CMD_GET_RANGE,(void*)pixelClockRange,sizeof(pixelClockRange));
	maxPixelClock = pixelClockRange[1];
	is_PixelClock(hCam, IS_PIXELCLOCK_CMD_SET,(void*)&maxPixelClock,sizeof(maxPixelClock));

	is_PixelClock(hCam2, IS_PIXELCLOCK_CMD_GET_RANGE,(void*)pixelClockRange2,sizeof(pixelClockRange2));
	maxPixelClock2 = pixelClockRange2[1];
	is_PixelClock(hCam2, IS_PIXELCLOCK_CMD_SET,(void*)&maxPixelClock2,sizeof(maxPixelClock2));
	//printf("pixel cl range : %d %d %d %d\n", pixelClockRange[3], pixelClockRange[2],pixelClockRange[1], pixelClockRange[0]);
			
	IplImage * img;
	img=cvCreateImage(cvSize(img_width, img_height), IPL_DEPTH_8U, 1); 
	cvNamedWindow( "img1", CV_WINDOW_AUTOSIZE );

	IplImage* smallImg;
	smallImg=cvCreateImage(cvSize(img_width/4, img_height/4), IPL_DEPTH_8U, 1); 
	
	IplImage * img2;
	img2=cvCreateImage(cvSize(img_width, img_height), IPL_DEPTH_8U, 1); 
	cvNamedWindow( "img2", CV_WINDOW_AUTOSIZE );	
	
	IplImage* smallImg2;
	smallImg2=cvCreateImage(cvSize(img_width/4, img_height/4), IPL_DEPTH_8U, 1); 

	
	is_EnableEvent(hCam, IS_SET_EVENT_FRAME);
	is_SetExternalTrigger(hCam, IS_SET_TRIGGER_HI_LO);
	is_CaptureVideo(hCam, IS_DONT_WAIT);

	double dblEnable = 1;
	double dblDummy = 0;
	int ret = is_SetAutoParameter(hCam, IS_SET_ENABLE_AUTO_GAIN, &dblEnable, &dblDummy);
	ret = is_SetAutoParameter(hCam, IS_SET_ENABLE_AUTO_SENSOR_GAIN, &dblEnable, &dblDummy);
	ret = is_SetAutoParameter(hCam, IS_SET_ENABLE_AUTO_SHUTTER, &dblEnable, &dblDummy);	
	ret = is_SetAutoParameter(hCam, IS_SET_ENABLE_AUTO_SENSOR_SHUTTER, &dblEnable, &dblDummy);	
	
	

	is_EnableEvent(hCam2, IS_SET_EVENT_FRAME);
	is_SetExternalTrigger(hCam2, IS_SET_TRIGGER_HI_LO);
	is_CaptureVideo(hCam2, IS_DONT_WAIT);

	ret = is_SetAutoParameter(hCam2, IS_SET_ENABLE_AUTO_GAIN, &dblEnable, &dblDummy);
	ret = is_SetAutoParameter(hCam2, IS_SET_ENABLE_AUTO_SENSOR_GAIN, &dblEnable, &dblDummy);
	ret = is_SetAutoParameter(hCam2, IS_SET_ENABLE_AUTO_SHUTTER, &dblEnable, &dblDummy);	
	ret = is_SetAutoParameter(hCam2, IS_SET_ENABLE_AUTO_SENSOR_SHUTTER, &dblEnable, &dblDummy);
	
	//IO_FLASH_PARAMS flashParams;
	//INT nRet = is_IO(hCam, IS_IO_CMD_FLASH_GET_GLOBAL_PARAMS,(void*)&flashParams, sizeof(flashParams));
	//flashParams.u32Duration = 0;
	//flashParams.s32Delay = 0;
	//nRet = is_IO(hCam, IS_IO_CMD_FLASH_SET_PARAMS,(void*)&flashParams, sizeof(flashParams));
	//nRet = is_IO(hCam, IS_IO_CMD_FLASH_GET_GLOBAL_PARAMS,(void*)&flashParams, sizeof(flashParams));	
	//INT nDelay   = flashParams.s32Delay;
        //UINT nDuration = flashParams.u32Duration;	
	//printf("flash params: %0d %0d \n", nDelay, nDuration); 	
	//nMode = IO_FLASH_MODE_CONSTANT_HIGH;
	UINT nMode = IO_FLASH_MODE_TRIGGER_LO_ACTIVE;
	//UINT theMode;	
	INT nRet = is_IO(hCam, IS_IO_CMD_FLASH_SET_MODE, (void*)&nMode, sizeof(nMode));	
	//nRet = is_IO(hCam, IS_IO_CMD_FLASH_GET_MODE, (void*)&theMode, sizeof(theMode));
	//printf("current flash mode %0d setmode %0d \n", theMode, nMode); 

	//IO_FLASH_PARAMS flashParams2;
	//nRet = is_IO(hCam2, IS_IO_CMD_FLASH_GET_GLOBAL_PARAMS,(void*)&flashParams2, sizeof(flashParams2));
	//flashParams.u32Duration = 0;
	//flashParams.s32Delay = 0;
	//nRet = is_IO(hCam, IS_IO_CMD_FLASH_SET_PARAMS,(void*)&flashParams, sizeof(flashParams));
	//nRet = is_IO(hCam, IS_IO_CMD_FLASH_GET_GLOBAL_PARAMS,(void*)&flashParams, sizeof(flashParams));	
	//nDelay   = flashParams2.s32Delay;
        //nDuration = flashParams2.u32Duration;	
	//printf("flash params: %0d %0d \n", nDelay, nDuration); 	
	nMode = IO_FLASH_MODE_TRIGGER_LO_ACTIVE;
		
	nRet = is_IO(hCam2, IS_IO_CMD_FLASH_SET_MODE, (void*)&nMode, sizeof(nMode));	
	//nRet = is_IO(hCam2, IS_IO_CMD_FLASH_GET_MODE, (void*)&theMode, sizeof(theMode));
	//printf("current flash mode %0d setmode %0d \n", theMode, nMode); 		
		
	FILE * pFile;
	pFile = fopen ("/media/kevin/010991e2-97ff-4163-a8e9-724f4dfb3950/myfile.txt","w");
	
	FILE * pFile2;
	pFile2 = fopen ("/media/kevin/010991e2-97ff-4163-a8e9-724f4dfb3950/myfile2.txt","w");

	int k = 0;	
        int numFrames = 15000;
	int failedcam1 = 0;
	int failedcam2 = 0;
	while(k < numFrames)
	{
		
		swprintf(buffer, 100, L"/media/kevin/010991e2-97ff-4163-a8e9-724f4dfb3950/testingL%09d.bmp", k);
		//swprintf(buffer, 100, L"/home/kevin/IDSCamera/ImageData/testingL%09d.bmp", k);		
		//swprintf(buffer, 100, L"/media/kevin/CBA3-7AAD/ImageData/testingL%09d.bmp", k);
		ImageFileParams.pwchFileName = buffer;
		
		swprintf(buffer2, 100, L"/media/kevin/010991e2-97ff-4163-a8e9-724f4dfb3950/testingR%09d.bmp", k);
		//swprintf(buffer, 100, L"/media/kevin/CBA3-7AAD/ImageData/testingR%09d.bmp", k);
		//swprintf(buffer, 100, L"/home/kevin/IDSCamera/ImageData/testingR%09d.bmp", k);
		ImageFileParams2.pwchFileName = buffer2;
		
		//is_ForceTrigger(hCam);		
		INT nRet = is_WaitEvent(hCam, IS_SET_EVENT_FRAME, 50);
		printf("\nwaitEvent 1: %0d \n", nRet);
		is_GetActSeqBuf(hCam, &pnNum, &activeMem, NULL);
		if(nRet == 122)
		{
			failedcam1 = failedcam1 + 1;
		}	
		if(k == numFrames -1)
		{
			nMode = IO_FLASH_MODE_CONSTANT_LOW;//low is high. high is low. Due to external circuit wiring.		
			nRet = is_IO(hCam, IS_IO_CMD_FLASH_SET_MODE, (void*)&nMode, sizeof(nMode));
		}
	
		//is_ForceTrigger(hCam2);			
		nRet = is_WaitEvent(hCam2, IS_SET_EVENT_FRAME, 50);		
		printf("waitEvent 2: %0d \n", nRet);					
		is_GetActSeqBuf(hCam2, &pnNum2, &activeMem2, NULL);
		if(k == numFrames -1)
		{
			nMode = IO_FLASH_MODE_CONSTANT_LOW;		
			nRet = is_IO(hCam2, IS_IO_CMD_FLASH_SET_MODE, (void*)&nMode, sizeof(nMode));		
		}

		if(nRet == 122)
		{
			failedcam2 = failedcam2 + 1;
		}			
		pnNum = pnNum - 1;printf("pnNum : %0d \n", pnNum); if (pnNum == 0){pnNum = 10;}
		pnNum2 = pnNum2 -1;if (pnNum2 == 0){pnNum2 = 10;}

		is_LockSeqBuf (hCam, IS_IGNORE_PARAMETER, imgMem[pnNum-1]);
		is_LockSeqBuf (hCam2, IS_IGNORE_PARAMETER, imgMem2[pnNum2-1]);		
		
		printf("pnNum : %0d \n", pnNum);
		//printf("pnNum2 : %0d \n", pnNum2);
//		printf("memId[pnNum-1] : %d \n", memId[pnNum-1]);
		//is_RenderBitmap (hCam, memId[pnNum-1], hwnd, IS_RENDER_FIT_TO_WINDOW | IS_RENDER_MIRROR_UPDOWN);
		nRet = is_GetImageInfo( hCam, memId[pnNum-1], &ImageInfo, sizeof(ImageInfo));
		//printf("nRet: %d  issuu: %d \n", nRet, IS_SUCCESS);		
		unsigned long long u64TimestampDevice;
		u64TimestampDevice = ImageInfo.u64TimestampDevice;  
		fprintf( pFile,"%020lld \n",u64TimestampDevice);
		fprintf( pFile,"%02d %02d %04d %02d %02d %02d %03d\n",
				ImageInfo.TimestampSystem.wDay, 
       				ImageInfo.TimestampSystem.wMonth,
				ImageInfo.TimestampSystem.wYear,
				ImageInfo.TimestampSystem.wHour,
				ImageInfo.TimestampSystem.wMinute,
				ImageInfo.TimestampSystem.wSecond,
				ImageInfo.TimestampSystem.wMilliseconds);
		
		nRet = is_GetImageInfo( hCam2, memId2[pnNum2-1], &ImageInfo2, sizeof(ImageInfo2));
		unsigned long long u64TimestampDevice2;
		u64TimestampDevice2 = ImageInfo2.u64TimestampDevice;  
		fprintf( pFile2,"%020lld \n",u64TimestampDevice2);
		fprintf( pFile2,"%02d %02d %04d %02d %02d %02d %03d\n",
				ImageInfo2.TimestampSystem.wDay, 
       				ImageInfo2.TimestampSystem.wMonth,
				ImageInfo2.TimestampSystem.wYear,
				ImageInfo2.TimestampSystem.wHour,
				ImageInfo2.TimestampSystem.wMinute,
				ImageInfo2.TimestampSystem.wSecond,
				ImageInfo2.TimestampSystem.wMilliseconds);
		

		unsigned int ID = memId[pnNum-1];
		ImageFileParams.pnImageID = &ID;
		ImageFileParams.ppcImageMem = &imgMem[pnNum-1];
		is_ImageFile(hCam, IS_IMAGE_FILE_CMD_SAVE, (void*)&ImageFileParams,sizeof(ImageFileParams));
		
		unsigned int ID2 = memId2[pnNum2-1];
		ImageFileParams2.pnImageID = &ID2;
		ImageFileParams2.ppcImageMem = &imgMem2[pnNum2-1];
		is_ImageFile(hCam2, IS_IMAGE_FILE_CMD_SAVE, (void*)&ImageFileParams2,sizeof(ImageFileParams2));
		if(k%10 == 0)
		{
			img->imageData=imgMem[pnNum-1];
			img->imageDataOrigin=imgMem[pnNum-1];
			cvResize(img, smallImg, CV_INTER_LINEAR );	
			cvShowImage("img1",smallImg);
			
			img2->imageData=imgMem2[pnNum2-1];
			img2->imageDataOrigin=imgMem2[pnNum2-1];
			cvResize(img2, smallImg2, CV_INTER_LINEAR );	
			cvShowImage("img2",smallImg2);	
	
			cvWaitKey(1);
		}
		is_UnlockSeqBuf(hCam, IS_IGNORE_PARAMETER, imgMem[pnNum-1]);	
			
		is_UnlockSeqBuf(hCam2, IS_IGNORE_PARAMETER, imgMem2[pnNum2-1]);
		printf("\nkval: %0d \n", k);		
		k++;
		
		
	}	
	FILE * pFile3;
	pFile3 = fopen ("/media/kevin/010991e2-97ff-4163-a8e9-724f4dfb3950/camfail.txt","w");
	fprintf( pFile3,"cam1: %02d cam2: %02d",
				failedcam1, 
       				failedcam2);		
	cvDestroyWindow( "img1" );
	cvDestroyWindow( "img2" );	
	fclose(pFile);
	fclose(pFile2);
	fclose(pFile3);
	is_StopLiveVideo (hCam, IS_FORCE_VIDEO_STOP);
	is_StopLiveVideo (hCam2, IS_FORCE_VIDEO_STOP);
	is_ClearSequence (hCam);
	is_ClearSequence (hCam2);
	for(int i=0; i<10; ++i )
	{
		is_FreeImageMem(hCam, imgMem[i], memId[i]);
	}
	for(int i=0; i<10; ++i )
	{
		is_FreeImageMem(hCam2, imgMem2[i], memId2[i]);
	}
		
	is_DisableEvent(hCam, IS_SET_EVENT_FRAME);
	is_DisableEvent(hCam2, IS_SET_EVENT_FRAME);
	is_ExitCamera(hCam);
	is_ExitCamera(hCam2);
	//cvReleaseImage(&img);
	//cvReleaseImage(&img2);
	//cvReleaseImage(&smallImg);
	//cvReleaseImage(&smallImg2);
}
/*
	is_SetDisplayMode (hCam, IS_SET_DM_DIB);
	is_SetDisplayMode (hCam2, IS_SET_DM_DIB);
     	is_SetColorMode (hCam, IS_CM_RGB8_PACKED);
	is_SetColorMode (hCam2, IS_CM_RGB8_PACKED);
     	//is_SetImageSize (hCam, img_width, img_height);
	double enable = 1;
	double disable = 0;
     	is_SetAutoParameter (hCam, IS_SET_ENABLE_AUTO_GAIN, &enable, 0);
	is_SetAutoParameter (hCam, IS_SET_ENABLE_AUTO_WHITEBALANCE, &enable, 0);
     	is_SetAutoParameter (hCam, IS_SET_ENABLE_AUTO_FRAMERATE, &enable, 0);
	is_SetAutoParameter (hCam, IS_SET_ENABLE_AUTO_SENSOR_FRAMERATE, &enable, 0);
     	is_SetAutoParameter (hCam, IS_SET_ENABLE_AUTO_SHUTTER, &enable, 0);
     	is_SetAutoParameter (hCam, IS_SET_ENABLE_AUTO_SENSOR_GAIN, &enable, 0);
     	is_SetAutoParameter (hCam, IS_SET_ENABLE_AUTO_SENSOR_WHITEBALANCE,&enable,0);
     	is_SetAutoParameter (hCam, IS_SET_ENABLE_AUTO_SENSOR_SHUTTER, &enable, 0);	
	is_SetAutoParameter (hCam, IS_SET_ENABLE_AUTO_SENSOR_GAIN_SHUTTER, &enable, 0);	

	is_SetAutoParameter (hCam2, IS_SET_ENABLE_AUTO_GAIN, &enable, 0);
	is_SetAutoParameter (hCam2, IS_SET_ENABLE_AUTO_WHITEBALANCE, &enable, 0);
     	is_SetAutoParameter (hCam2, IS_SET_ENABLE_AUTO_FRAMERATE, &enable, 0);
	is_SetAutoParameter (hCam2, IS_SET_ENABLE_AUTO_SENSOR_FRAMERATE, &enable, 0);
     	is_SetAutoParameter (hCam2, IS_SET_ENABLE_AUTO_SHUTTER, &enable, 0);
     	is_SetAutoParameter (hCam2, IS_SET_ENABLE_AUTO_SENSOR_GAIN, &enable, 0);
     	is_SetAutoParameter (hCam2, IS_SET_ENABLE_AUTO_SENSOR_WHITEBALANCE,&enable,0);
     	is_SetAutoParameter (hCam2, IS_SET_ENABLE_AUTO_SENSOR_SHUTTER, &enable, 0);	
	is_SetAutoParameter (hCam2, IS_SET_ENABLE_AUTO_SENSOR_GAIN_SHUTTER, &enable, 0);	
	char key;
	IplImage * img;
	IplImage * img2;		
	img=cvCreateImage(cvSize(img_width, img_height), IPL_DEPTH_8U, 3); 
	img2=cvCreateImage(cvSize(img_width, img_height), IPL_DEPTH_8U, 3); 
	cvNamedWindow( "img1", CV_WINDOW_AUTOSIZE );
	cvNamedWindow( "img2", CV_WINDOW_AUTOSIZE );
	key = cvWaitKey();	
	while(key != 27)	
	{
		void *pMemVoid;
		void *pMemVoid2;
		is_FreezeVideo(hCam, IS_WAIT);
		is_GetImageMem (hCam, &pMemVoid);

		is_FreezeVideo(hCam2, IS_WAIT);
		is_GetImageMem (hCam2, &pMemVoid2);		
		
		img->imageData=(char*)pMemVoid;
		img->imageDataOrigin=(char*)pMemVoid;
	
		img2->imageData=(char*)pMemVoid2;
		img2->imageDataOrigin=(char*)pMemVoid2;
		//cout << "hello";		
		cvShowImage("img1",img);
		cvSaveImage("/home/kevin/IDSCamera/test.bmp", img);
	
		cvShowImage("img2",img2);
		cvSaveImage("/home/kevin/IDSCamera/test2.bmp", img2);
		key = cvWaitKey();
		//cout << "hello";
		//cvReleaseImage(&img);
		//cout << "hello";
	}
	//cvReleaseImage( &img );
	
	cvDestroyWindow( "img1" );
	is_FreeImageMem (hCam, imgMem, memId);

	cvDestroyWindow( "img2" );
	is_FreeImageMem (hCam2, imgMem2, memId2);
	
	//printf("freemem: %d\n",nRet);
	//printf("nRet: %d\n",nRet);
	is_ExitCamera(hCam);
	is_ExitCamera(hCam2);

	//cvReleaseImage( &img);

	//is_SetDisplayMode (hCam, IS_SET_DM_OPENGL);
	//is_CaptureVideo(hCam,IS_GET_LIVE);
	//is_ExitCamera(hCam);
	//INT* pnNumCams;
	//is_GetNumberOfCameras(pnNumCams);
	
    	/*IplImage* src1;
	src1 = cvLoadImage("/media/kevin/CBA3-7AAD/image.jpg", 3); 
			cvShowImage("image", src1);
			cvWaitKey();
   
*/	 /*if ( argc != 2 )
    {
            if ( argc != 2 )
    {
        printf("usage: DisplayImage.out <Image_Path>\n");
        return -1;
    }

    Mat image;
    image = imread( argv[1], 1 );

    if ( !image.data )
    {
        printf("No image data \n");
        return -1;
    }
    namedWindow("Display Image", CV_WINDOW_AUTOSIZE );
    imshow("Display Image", image);

    waitKey(0);

    return 0;
printf("usage: DisplayImage.out <Image_Path>\n");
        return -1;
    }

    Mat image;
    image = imread( argv[1], 1 );

    if ( !image.data )
    {
        printf("No image data \n");
        return -1;
    }
    namedWindow("Display Image", CV_WINDOW_AUTOSIZE );
    imshow("Display Image", image);

    waitKey(0);

    return 0;*/


	/*cvNamedWindow( "Example2", CV_WINDOW_AUTOSIZE );
	CvCapture* capture = cvCreateCameraCapture(0);
	IplImage* frame;
	while(1) {
		frame = cvQueryFrame( capture );
		if( !frame ) break;
		cvShowImage( "Example2", frame );
		char c = cvWaitKey(33);
		if( c == 27 ) break;
	}
	cvReleaseCapture( &capture );
	cvDestroyWindow( "Example2" );
	printf("eye carumba \n");	
	return 0;*/






//INT nNumCam;

/*is_GetNumberOfCameras( &nNumCam );
	UEYE_CAMERA_LIST* pucl;
	pucl = (UEYE_CAMERA_LIST*) new BYTE [sizeof (DWORD) + nNumCam * sizeof (UEYE_CAMERA_INFO)];
	pucl->dwCount = nNumCam;
	is_GetCameraList(pucl);
	for (int iCamera = 0; iCamera < (int)pucl->dwCount; iCamera++) {
	        printf("Camera %i Id: %d", iCamera,
 	      pucl->uci[iCamera].dwCameraID);
        }
  	
	printf("\nnumCams: %d\n", nNumCam);
	return 0;*/


