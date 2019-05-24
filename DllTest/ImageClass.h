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
	void Refactoring();
	bool Bilinear_Interpolation();
	bool Gaussian_Blur();
	bool GrayScale();
	bool Thresholding();
	void ShowImage(Mat image);

};

