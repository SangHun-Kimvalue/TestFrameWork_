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

	ImageClass(int wid, int hei, BYTE* src);
	~ImageClass();

	GDICaptureClass* m_Cap;
	int nWidth;
	int nHeight;
	BYTE *src;

private:

	int c_x;
	int c_y;
	int c_wid;
	int c_hei;
	Mat ori_image;
	Mat fix_image;
	int base_height;


	bool CV_Init();
	Mat Resize(Mat fix_image);
	Mat Refactoring(Mat fix_image);
	Mat Bilinear_Interpolation(Mat fix_image);
	Mat Gaussian_Blur(Mat fix_image, int sigmaX, int sigmaY);
	Mat GrayScale(Mat fix_image);
	Mat Thresholding(Mat fix_image);
	void ShowImage(Mat showimage);
	BYTE* Mat2Byte(Mat fix_image);


};

