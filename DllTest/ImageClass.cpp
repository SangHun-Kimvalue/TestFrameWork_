#include "ImageClass.h"

ImageClass::ImageClass(GDICaptureClass* Cap) : m_Cap(Cap)
{
	nWidth = Cap->nWidth;
	nHeight = Cap->nHeight;
	src = Cap->src;
}

ImageClass::~ImageClass()
{
}

bool ImageClass::Cut_Image() {

	// Read the image file
	Mat image = imread("D:/My OpenCV Website/Lady with a Guitar.jpg");

	// Check for failure
	if (image.empty())
	{
		std::cout << "Could not open or find the image" << std::endl;
		std::cin.get(); //wait for any key press
	}

	String windowName = "The Guitar"; //Name of the window

	namedWindow(windowName); // Create a window

	imshow(windowName, image); // Show our image inside the created window.

	waitKey(0); // Wait for any keystroke in the window

	destroyWindow(windowName); //destroy the created window

	return true;
}



