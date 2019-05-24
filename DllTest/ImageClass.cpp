#include "ImageClass.h"

ImageClass::ImageClass(int wid, int hei, BYTE* src) : nWidth(wid), nHeight(hei), src(src){

	base_height = 150;

	CV_Init();
	Refactoring(fix_image);

}

ImageClass::~ImageClass()
{
}

bool ImageClass::CV_Init() {

	c_x = 0; 	c_y = 0;  	c_wid = 100; 	c_hei = 400;		//wid, hei ���� ������ ������ �ȵ� ����.

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

Mat ImageClass::Resize(Mat fix_image) {
	
	Mat re_image;

	//std::cout << "���� ���� �� ����" << c_wid << " " << c_hei << std::endl;

	//CV_EXPORTS_W void resize(InputArray src, OutputArray dst,
	//	Size dsize, double fx = 0, double fy = 0,
	//	int interpolation = INTER_LINEAR);
	//cv::resize(fix_image, fix_image, cv::Size(wid, hei), 0, 0, INTER_LINEAR);
	//�̹����� ����Ϸ��� �Ϲ������� INTER_AREA �������� ������ �������� �̹����� Ȯ���Ϸ��� �Ϲ������� 
	//INTER_LINEAR (���� ������ �� ������ ������ ���Դϴ�) �Ǵ� INTER_CUBIC (����)�� ���� �� ���Դϴ�.

	int base = c_hei - base_height;

	if (base == 0) {					//�������� �� ���̰� �ƴ϶� �ۼ��������� ���̿� ���̸� ����ϸ� ������??
		return fix_image;
	}
	else if (base > 0) {			//���
		if (c_wid < base) {
			//�̹����� �¿�� �۾����� �Ǹ� ���̻� ������ �Ұ����ϴٰ� �Ǵ��� �״�� ����Ѵ�.
			return fix_image;
		}
		else if (c_wid > base) {
			cv::resize(fix_image, re_image, cv::Size(c_wid - base, base_height), 0, 0, INTER_AREA);		//�̹��� ���ø��̼� (�������� �ݴ� LowPass ���͵� ����)�� ��ȣ�Ǵ� �÷���	
			// ���̿� ���� ������ ���̸�ŭ ���̵� ���� ����.
			return re_image;
		}
	}
	else if (base < 0) {			//Ȯ��
		cv::resize(fix_image, re_image, cv::Size(c_wid - base, base_height), 0, 0, INTER_LINEAR);			//�缱�� ������  >>  Bilinear_Interpolation 
		return re_image;
	}

	return re_image;
}

Mat ImageClass::Refactoring(Mat fix_image) {


	Rect rect(c_x, c_y, c_wid, c_hei);		// x,y ,wid, hei

	fix_image = ori_image(rect);			//rect ��ŭ crop
	ShowImage(fix_image);

	
	fix_image = Resize(fix_image);
	ShowImage(fix_image);

	//std::cout << "���� ���� �� ����" << c_wid - base << " " << base_height << std::endl;

	//���� �߰�

	fix_image = Gaussian_Blur(fix_image, 3, 3);
	ShowImage(fix_image);

	return fix_image;
}

Mat ImageClass::Bilinear_Interpolation(Mat fix_image){
	

	//INTER_AREA�� ����.
	//INTER_AREA �÷��״� �̹��� Ȯ�뿡 ��� INTER_LINEAR�� ȣ���Ѵ�.(����� �ణ �ٸ�)
	//����� ��� wid�� hei �� 2�� ����̰�, ��Ʈ ä�μ��� 2�� �ƴϸ� INTER_AREA�� �����. �׿��� ��쿡�� INTER_LINEAR_EXACT �̳� INTER_LINEAR�� ���
	//����ϴ� ��쿡�� ���ǰ� ������ ��� ���� ��հ� �����.(Ŀ�ο��� ���ϰ� ���� Not Mask)
	//https://medium.com/@wenrudong/what-is-opencvs-inter-area-actually-doing-282a626a09b3

	//���� �ƴ� ���� P,Q ������ �� R�� �����Ҷ�, �� ���� ���� �������� ���踦 ���´ٰ� �����ϰ� P,Q�� ���� �Ÿ��� �̿��� R�� �����Ѵ�.
	//R = P * (dx) + Q * (1-dx)		�� �̿��Ͽ� x ��� y���� �ι� ���� ������ ������ ���� ����ġ�� �����Ѵ�.
	//R = A * (dx)(dy) + B * (1-dx)(dy) + C * (dx)(1-dy) + D * (1-dx)(1-dy)

	//cv::resize(fix_image, fix_image, cv::Size(c_wid - base, base_height), 0, 0, INTER_LINEAR); ���� �̹� ���� �Ǿ�����.


	return fix_image;

}

Mat ImageClass::Gaussian_Blur(Mat fix_image, int sigmaX, int sigmaY) {			//�ñ׸��� 0�̸� �ڵ����� ����(MASK �ڵ����)

	Mat Gasu_image;

	//void GaussianBlur(InputArray src, OutputArray dst, Size ksize, double sigmaX, double sigmaY=0, int borderType=BORDER_DEFAULT )
	// GaussianBlur( src, dst, Size( i, i ), 0, 0 );				//�Ÿ��� ���� ����ġ ����ũ��
	GaussianBlur(fix_image, Gasu_image, Size(sigmaX, sigmaX), 1.5);
	
	//���� ���� ������		�Ÿ� �� �ȼ��� ������� ���� ����ġ ����ũ��
	//void bilateralFilter(InputArray src, OutputArray dst, int d, double sigmaColor, double sigmaSpace, int borderType=BORDER_DEFAULT )
	//src: �ҽ� �̹���
	//dst: ��ǥ �̹���, �� ó���� �̹����� ���� ����
	//d: Diameter of each pixel neighborhood that is used during filtering. 
	//sigmaColor: Filter sigma in the color space. A larger value of the parameter means that farther colors			//�� ����� ����
	//			within the pixel neighborhood will be mixed together, resulting in larger areas of semi-equal color.
	//sigmaSpace: Filter sigma in the coordinate space. A larger value of the parameter means that					//�Ÿ��� ����
	//			farther pixels will influence each other as long as their colors are close enough. 
	//Mat Bilateral;
	//Bilateral.create(fix_image, CV_8UC3);
	//bilateralFilter(fix_image, Bilateral, 3, 15, 15);			//CV_8UC1 �̳� 3�� ��ߵǴµ� 4�� �����־ ������ ��ȯ�� �ʿ�.

	return Gasu_image;
}

