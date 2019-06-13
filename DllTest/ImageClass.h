#pragma once
//#include "opencv2/opencv.hpp"  
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
//#include "opencv2/video/background_segm.hpp"
#include "GDICaptureClass.h"

#include <ctime>

#define IMAGEPATH "..\\Libs\\Test_Image\\";

using namespace cv;

class ImageClass
{
public:

	ImageClass();
	ImageClass(int i);
	~ImageClass();

	GDICaptureClass* m_Cap;

	int c_x;
	int c_y;
	int c_wid;
	int c_hei;


	Mat fix_image;

	Mat Create_Mat(int ori_wid, int ori_hei, unsigned char* src);
	bool Init(int ori_wid, int ori_hei, int x, int y, int wid, int hei);
	Mat Resize(Mat fix_image, int String_length);
	Mat Crop(Mat fix_image);
	Mat Bilinear_Interpolation(Mat fix_image);
	Mat Gaussian_Blur(Mat fix_image);
	Mat GrayScale(Mat fix_image);
	Mat Thresholding(Mat fix_image);
	Mat C_Canny(Mat ori_image);

	void Release();

	void ShowImage(Mat showimage);
	//BYTE* Mat2Byte(Mat fix_image, int index, int depth);


private:

	Mat ori_image;

	int base_height;
	int base_width;
	int base_length;
	int best_font_size;

	void Reverse_check(Mat fix_image);
	void Reverse_check_Ran(Mat fix_image);
	//void Reverse_check(int ori_wid, int ori_hei, unsigned char* src);
	Mat Resize_Num(Mat fix_image);
	Mat Resize_String(Mat fix_image, int String_length);
	//BYTE* Mat2Byte(Mat fix_image, int index, int depth, int save);

	bool Reverse_Color;
	int index;

};

