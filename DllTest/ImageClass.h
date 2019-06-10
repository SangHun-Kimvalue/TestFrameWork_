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
	ImageClass(int i);
	ImageClass(int wid, int hei, BYTE* src, int String_Type, int Base_length);
	~ImageClass();

	GDICaptureClass* m_Cap;

	int c_x;
	int c_y;
	int c_wid;
	int c_hei;
	BYTE *src;

	Mat ori_image;
	Mat fix_image;

	Mat CV_Init(int ori_wid, int ori_hei, int x, int y, int wid, int hei, unsigned char* src);
	Mat Resize(Mat fix_image, int String_Type, int String_length);
	Mat Crop(Mat fix_image);
	Mat Bilinear_Interpolation(Mat fix_image);
	Mat Gaussian_Blur(Mat fix_image);
	Mat GrayScale(Mat fix_image);
	Mat C_Canny(Mat ori_image);

	void Release();

	void ShowImage(Mat showimage);
	BYTE* Mat2Byte(Mat fix_image, int index, int depth);


private:

	int base_height;
	int base_width;
	int base_length;

	Mat Resize_Num(Mat fix_image);
	Mat Resize_String(Mat fix_image, int String_length);
	Mat Thresholding(Mat fix_image);
	BYTE* Mat2Byte(Mat fix_image, int index, int depth, int save);

	int index;

};

