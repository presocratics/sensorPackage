#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <string>

#define PATHINPUT "/home/sdw/Desktop/clib_rename/datainput/"
#define PATHOUTPUT "/home/sdw/Desktop/clib_rename/dataset/"
using namespace std;
using namespace cv;

int main (int argc, const char* argv[])
{
	if(argc != 4)
	{
		cout << "wrong input" << endl;
		return -1;
	}

	//input acqired
	const char* mode = argv[1];
	string path_input = argv[2];
	string path_output = argv[3];
	//means there is only one camera
	if ( *mode == '1' )
	{
		string line_input;
		string comma = ",";
  		while(cin>>line_input)
  		{
  			int pos = line_input.find(comma);
			//parsing the path
			string frame_num = line_input.substr(0,pos);
			string frame_ts = line_input.substr(pos+1,line_input.length());
			string frame_need_rename = path_input + frame_num +".bmp";
			string frame_need_parse = path_output + frame_ts + ".bmp";
			Mat in=imread(frame_need_rename, CV_LOAD_IMAGE_GRAYSCALE);
    		Mat out;
    		cvtColor(in, out, CV_BayerBG2BGR);
    		imwrite(frame_need_parse,out);
    	}
    }
    //means there are two cameras
    else if ( *mode == '2')
    {
    	string line_input;
		string comma = ",";
  		while(cin>>line_input)
  		{
  			int pos = line_input.find(comma);
			//parsing the path
			string frame_num = line_input.substr(0,pos);
			string frame_ts = line_input.substr(pos+1,line_input.length());
			//get the path for the frame that need to be renamed
			string frame_need_rename_cam0 = path_input + "cam0/" + frame_num + ".bmp";
			string frame_need_rename_cam1 = path_input + "cam1/" + frame_num + ".bmp";
			//get the output path
			string frame_need_parse_cam0 = path_output + "cam0/" + frame_ts + ".bmp";
			string frame_need_parse_cam1 = path_output + "cam1/" + frame_ts + ".bmp";

			Mat in_0 = imread(frame_need_rename_cam0, CV_LOAD_IMAGE_GRAYSCALE);
			Mat in_1 = imread(frame_need_rename_cam1, CV_LOAD_IMAGE_GRAYSCALE);
    		Mat out_0;
    		Mat out_1;
    		cvtColor(in_0, out_0, CV_BayerBG2BGR ,3);
    		cvtColor(in_1, out_1, CV_BayerBG2BGR ,3);

    		imwrite(frame_need_parse_cam0,out_0);
    		imwrite(frame_need_parse_cam1,out_1);
    	}
    }
    else
    {
    	cout << "wrong mode number" << endl;
    	return -1;
    }
	return 0;
}
