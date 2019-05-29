#pragma once
//#include "opencv2/opencv.hpp"  
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
//#include "opencv2/video/background_segm.hpp"
#include "GDICaptureClass.h"

#define IMAGEPATH "..\\Libs\\Test_Image\\";

using namespace cv;

class ImageClass
{
public:

	ImageClass();
	ImageClass(int wid, int hei, BYTE* src, int String_Type, int Base_length);
	~ImageClass();

	GDICaptureClass* m_Cap;
	int nWidth;
	int nHeight;
	BYTE *src;

	int c_wid;
	int c_hei;

private:

	int c_x;
	int c_y;

	Mat ori_image;
	Mat fix_image;

	int base_height;
	int base_width;

	int Base_length;
	int String_Type;

	Mat CV_Init();
	Mat CV_Ini_t();
	Mat Resize_Num(Mat fix_image);
	Mat Resize_String(Mat fix_image);
	Mat Refactoring(Mat fix_image);
	Mat Bilinear_Interpolation(Mat fix_image);
	Mat Gaussian_Blur(Mat fix_image, int sigmaX, int sigmaY);
	Mat GrayScale(Mat fix_image);
	Mat Thresholding(Mat fix_image);
	void ShowImage(Mat showimage);
	BYTE* Mat2Byte(Mat fix_image, int index);


};

