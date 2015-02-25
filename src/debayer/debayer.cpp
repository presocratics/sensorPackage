/*
 * =====================================================================================
 *
 *       Filename:  debayer.cpp
 *
 *    Description:  Debayer image
 *
 *        Version:  1.0
 *        Created:  02/24/2015 05:53:55 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Martin Miller (MHM), miller7@illinois.edu
 *   Organization:  Aerospace Robotics and Controls Lab (ARC)
 *
 * =====================================================================================
 */


#include <cv.h>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
int main(int argc, char **argv)
{
    cv::Mat in=cv::imread(argv[1], CV_LOAD_IMAGE_GRAYSCALE);
    cv::Mat out;
    cvtColor(in, out, CV_BayerBG2BGR ,3);
    cv::imwrite(argv[2], out);
	return 0;
}

