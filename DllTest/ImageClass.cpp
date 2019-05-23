#include "ImageClass.h"

void Test_convert() {

	
	//imdecode(, IMREAD_COLOR);		//IMREAD_GRAYSCALE
	
}

ImageClass::ImageClass(int wid, int hei, BYTE* src) : nWidth(wid), nHeight(hei), src(src){

	CV_Init();

	Cut_Image();
	ShowImage();
}

ImageClass::~ImageClass()
{
}

bool ImageClass::CV_Init() {

	c_x = 0; 	c_y = 0;  	c_wid = 100; 	c_hei = 100;		//wid, hei 원본 사이즈 넘으면 안됨 주의.

	if (c_x + c_wid > nWidth) 
		std::cerr << "넓이 오류" << std::endl;
	else if(c_y + c_hei > nHeight)
		std::cerr << "높이 오류" << std::endl;


	// Read the image file
	std::string imagepath = IMAGEPATH;
	std::string filename = imagepath + "Lady with a Guitar.PNG";

	image = Mat(nWidth, nHeight, CV_8UC4, src);
	if (image.empty()){
		std::cout << "Could not open or find the image" << std::endl;
		std::cin.get(); //wait for any key press
		return false;
	}

	return true;
}

void ImageClass::ShowImage() {

	//imshow("sub", cropimage);				//Show image
	namedWindow("main");					// Create a window
	imshow("main", image);				// Show our image inside the created window.
	waitKey(0);								// Wait for any keystroke in the window

	destroyWindow("main");					//destroy the created window
}

void ImageClass::Cut_Image() {

	Rect rect(c_x, c_y, c_wid, c_hei);		// x,y ,wid, hei

	cropimage = image(rect);

}

bool ImageClass::Resize() {


	return true;
}



