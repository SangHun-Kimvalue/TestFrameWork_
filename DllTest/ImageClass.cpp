#include "ImageClass.h"

ImageClass::ImageClass(int wid, int hei, BYTE* src) : nWidth(wid), nHeight(hei), src(src){

	base_height = 50;

	CV_Init();
	Refactoring();
	ShowImage(fix_image);

}

ImageClass::~ImageClass()
{
}

bool ImageClass::CV_Init() {

	c_x = 0; 	c_y = 0;  	c_wid = 100; 	c_hei = 40;		//wid, hei ���� ������ ������ �ȵ� ����.

	if (c_x + c_wid > nWidth) 
		std::cerr << "���� ����" << std::endl;
	else if(c_y + c_hei > nHeight)
		std::cerr << "���� ����" << std::endl;

	// Read the image file
	//std::string imagepath = IMAGEPATH;
	//std::string filename = imagepath + "Lady with a Guitar.PNG";

	ori_image = Mat(nWidth, nHeight, CV_8UC4, src);
	if (ori_image.empty()){
		std::cout << "Could not open or find the image" << std::endl;
		std::cin.get(); //wait for any key press
		return false;
	}

	return true;
}

void ImageClass::ShowImage(Mat Image) {

	//imshow("sub", cropimage);				//Show image
	namedWindow("main");					// Create a window
	imshow("main", Image);				// Show our image inside the created window.
	waitKey(0);								// Wait for any keystroke in the window

	destroyWindow("main");					//destroy the created window
}


void ImageClass::Refactoring() {


	Rect rect(c_x, c_y, c_wid, c_hei);		// x,y ,wid, hei

	fix_image = ori_image(rect);			//rect ��ŭ crop

	//CV_EXPORTS_W void resize(InputArray src, OutputArray dst,
	//	Size dsize, double fx = 0, double fy = 0,
	//	int interpolation = INTER_LINEAR);
	
	//cv::resize(fix_image, fix_image, cv::Size(wid, hei), 0, 0, INTER_LINEAR);
	//�̹����� ����Ϸ��� �Ϲ������� INTER_AREA �������� ���� �� �������� �̹����� Ȯ���Ϸ��� �Ϲ������� 
	//INTER_CUBIC (������) �Ǵ� INTER_LINEAR (���� ������ �� ������ ������ ���Դϴ�)�� ���� �� ���Դϴ�.

	int base = c_hei - base_height;

	if(base == 0){
		return ;
	}
	else if (base > 0) {		//���
		cv::resize(fix_image, fix_image, cv::Size(c_wid - base, base_height), 0, 0, INTER_AREA);
	}
	else if(base < 0) {			//Ȯ��
		cv::resize(fix_image, fix_image, cv::Size(c_wid - base, base_height), 0, 0, INTER_LINEAR);
	}


	return ;
}



