#pragma once
#include "opencv2/opencv.hpp"  
//#include "opencv2/highgui/highgui.hpp"
//#include "opencv2/core/core.hpp"
//#include "opencv2/highgui/highgui.hpp"
//#include "opencv2/video/background_segm.hpp"
#include "GDICaptureClass.h"

using namespace cv;

class ImageClass
{
public:
	ImageClass(GDICaptureClass* Cap);
	~ImageClass();

	GDICaptureClass* m_Cap;
	int nWidth;
	int nHeight;
	BYTE *src;

private:
	bool CV_Init();
	bool Cut_Image();
	bool Resize();
	bool Bilinear_Interpolation();
	bool Gaussian_Blur();
	bool GrayScale();
	bool Thresholding();



};
