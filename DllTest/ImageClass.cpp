#include "ImageClass.h"

bool RGBSaveBMP(BYTE *input, int nWidth, int nHeight, int index) {

	BITMAPFILEHEADER bf;
	BITMAPINFOHEADER bi;

	BYTE *image = input;
	//unsigned char *image = (unsigned char*)malloc(sizeof(unsigned char)*nWidth*nHeight * 3);
	//memcpy();
	FILE *file;
	char title[1024];

	int i = index;
	if ( i > 10)
		i = 0;

	sprintf_s(title, "CCapture_%d.bmp", i++);
	fopen_s(&file, title, "wb");

	if (image != NULL)
	{
		if (file != NULL)
		{
			memset(&bf, 0, sizeof(bf));
			memset(&bi, 0, sizeof(bi));

			bf.bfType = 'MB';
			bf.bfSize = sizeof(bf) + sizeof(bi) + nWidth * nHeight * 4;
			bf.bfOffBits = sizeof(bf) + sizeof(bi);
			bi.biSize = sizeof(bi);
			bi.biWidth = nWidth;
			bi.biHeight = nHeight;
			bi.biPlanes = 1;
			bi.biBitCount = 32;
			bi.biSizeImage = nWidth * nHeight * 4;

			fwrite(&bf, sizeof(bf), 1, file);
			fwrite(&bi, sizeof(bi), 1, file);
			fwrite(image, sizeof(unsigned char), nHeight*nWidth * 4, file);

			fclose(file);
		}
	}
	return true;
}

ImageClass::ImageClass(){}

ImageClass::ImageClass(int wid, int hei, BYTE* src, int String_Type, int Base_length) 
	: nWidth(wid), nHeight(hei), src(src), String_Type(String_Type), Base_length(Base_length){

	base_height = 75;				// ����ڰ� ��������� ������ ���������� ������ ���� ������ 50 ~ 70% �� ���� �Ǿ� �ְ� 
									// ������ ũ�⸦ 35�� ����� ũ��� ����� ���ؼ��� ���� ���̸� 75�� �����ؾ���.
	base_width = 100;

	
	ori_image = CV_Init();
	//bitwise_not(ori_image, ori_image);

	ShowImage(ori_image, c_wid, c_hei);

	fix_image = ori_image.clone();
	//fix_image = Refactoring(ori_image);
	
	Rect rect(c_x, c_y, c_wid, c_hei);		// x,y ,wid, hei

	//fix_image

	(ori_image(rect)).copyTo(fix_image);			//rect ��ŭ crop
	ShowImage(fix_image, c_wid, c_hei);

	//fix_image = GrayScale(ori_image);			//�׷��� �̹����ϸ� src�� �̻�����.(pixel ä�ζ����ε�.)
	//ShowImage(fix_image);

	//Thresholding(fix_image);
	//ShowImage(fix_image);

	//this->src = Mat2Byte(ori_image);
	
	this->src = (BYTE*)fix_image.data;
	RGBSaveBMP(this->src, wid, hei, 5);
	
	this->src = (BYTE*)ori_image.data;
	RGBSaveBMP(this->src, wid, hei, 6);

}

ImageClass::~ImageClass()
{
}

Mat ImageClass::CV_Init() {

	c_x = 0; 	c_y = 0;  	c_wid = 400; 	c_hei = 400;		//wid, hei ���� ������ ������ �ȵ� ����.

	if (c_x + c_wid > nWidth) 
		std::cerr << "���� ����" << std::endl;
	else if(c_y + c_hei > nHeight)
		std::cerr << "���� ����" << std::endl;

	// Read the image file
	//std::string imagepath = IMAGEPATH;
	//std::string filename = imagepath + "Lady with a Guitar.PNG";
	
	//ori_image.create(nWidth, nHeight, CV_8UC(4));
	//ori_image.data = src;

	//ori_image.at();

	ori_image = Mat(nWidth, nHeight, CV_8UC(4), src);
	//ori_image.resize();
	if (ori_image.empty()){
		std::cout << "Could not open or find the image" << std::endl;
		std::cin.get(); //wait for any key press
		return ori_image;
	}
	//ShowImage(ori_image, nWidth, nHeight);

	return ori_image;
}

void ImageClass::ShowImage(Mat Image, int wid, int hei) {

	//imshow("sub", cropimage);				//Show image
	namedWindow("main", WINDOW_AUTOSIZE);					// Create a window
	imshow("main", Image);				// Show our image inside the created window.
	waitKey(0);								// Wait for any keystroke in the window

	destroyWindow("main");					//destroy the created window
}

Mat ImageClass::Resize_Num(Mat fix_image) {

	Mat re_image;

	//std::cout << "���� ���� �� ����" << c_wid << " " << c_hei << std::endl;

	//CV_EXPORTS_W void resize(InputArray src, OutputArray dst,
	//	Size dsize, double fx = 0, double fy = 0,
	//	int interpolation = INTER_LINEAR);
	//cv::resize(fix_image, fix_image, cv::Size(wid, hei), 0, 0, INTER_LINEAR);
	//�̹����� ����Ϸ��� �Ϲ������� INTER_AREA �������� ������ �������� �̹����� Ȯ���Ϸ��� �Ϲ������� 
	//INTER_LINEAR (���� ������ �� ������ ������ ���Դϴ�) �Ǵ� INTER_CUBIC (����)�� ���� �� ���Դϴ�.

	base_width = base_height;

	int base = c_wid - base_width;

	if (base == 0) {					//�������� �� ���̰� �ƴ϶� �ۼ��������� ���̿� ���̸� ����ϸ� ������??
		return fix_image;
	}
	else if (base > 0) {				//���
		
		float percent = (float)c_wid - (float)base / (float)c_wid;
		int temp_hei = c_hei * percent;
		int temp_wid = c_wid - base;
		
		//�̹����� ����ġ ���� �� �۾����� �Ǹ� ���̻� ������ �Ұ����ϴٰ� �Ǵ��� �״�� ����Ѵ�.
		if ((temp_hei < base_height) || (temp_wid < base_width))
			return fix_image;

		else {
			//�̹��� ���ø��̼� (�������� �ݴ�Ǵ� ������ LowPass ���͵� ����)�� ��ȣ�Ǵ� �÷���		//INTER_AREA >> Bilinear_Interpolation�� �ڼ���
			cv::resize(fix_image, re_image, cv::Size(temp_wid, temp_hei), 0, 0, INTER_AREA);
			c_wid = temp_wid;
			c_hei = temp_hei;

			return re_image;
		}
	}
	else if (base < 0) {			//Ȯ��

		float percent = (float)c_wid - (float)base / (float)c_wid;
		int temp_hei = c_hei * percent;
		int temp_wid = c_wid - base;

		cv::resize(fix_image, re_image, cv::Size(temp_wid, temp_hei), 0, 0, INTER_LINEAR);			//�缱�� ������  >>  Bilinear_Interpolation 
		c_wid = temp_wid;
		c_hei = temp_hei;

		return re_image;
	}
	return re_image;
}

Mat ImageClass::Resize_String(Mat fix_image) {
	
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
	else if (base > 0) {				//���
		//�̹����� ����ġ ���� �� �۾����� �Ǹ� ���̻� ������ �Ұ����ϴٰ� �Ǵ��� �״�� ����Ѵ�.
		if (c_hei - base < base_height || (c_wid - base < base_width))	
			return fix_image;
		else {
			float percent = (float)c_hei - (float)base / (float)c_hei;
			int temp_wid = c_wid * percent;
			int temp_hei = c_hei - base;

			//�̹��� ���ø��̼� (�������� �ݴ�Ǵ� ������ LowPass ���͵� ����)�� ��ȣ�Ǵ� �÷���		//INTER_AREA >> Bilinear_Interpolation�� �ڼ���
			cv::resize(fix_image, re_image, cv::Size(temp_wid, temp_hei), 0, 0, INTER_AREA);
			c_wid = temp_wid;
			c_hei = temp_hei;

			return re_image;
		}
	}
	else if (base < 0) {			//Ȯ��

		float percent = ((float)c_hei - (float)base) / (float)c_hei;

		int temp_wid = c_wid * percent;
		int temp_hei = c_hei - base;

		cv::resize(fix_image, re_image, cv::Size(temp_wid, temp_hei), 0, 0, INTER_LINEAR);			//�缱�� ������  >>  Bilinear_Interpolation 
		c_wid = temp_wid;
		c_hei = temp_hei;

		return re_image;
	}



	/*else if (base > 0) {				//���
		if (c_wid <= base) {
			//�̹����� �¿�� �۾����� �Ǹ� ���̻� ������ �Ұ����ϴٰ� �Ǵ��� �״�� ����Ѵ�.
			return fix_image;
		}
		else if (c_wid > base) {
			if(c_wid - base < c_wid)
				c_wid = c_wid - base;
			cv::resize(fix_image, re_image, cv::Size(c_wid, base_height), 0, 0, INTER_AREA);		//�̹��� ���ø��̼� (�������� �ݴ�Ǵ� ������ LowPass ���͵� ����)�� ��ȣ�Ǵ� �÷���	
			// ���̿� ���� ������ ���̸�ŭ ���̵� ���� ����.
			return re_image;
		}
	}

	else if (base < 0) {			//Ȯ��
		cv::resize(fix_image, re_image, cv::Size(c_wid + base, c_hei + base), 0, 0, INTER_LINEAR);			//�缱�� ������  >>  Bilinear_Interpolation 
		c_wid = c_wid + base;
		c_hei = c_hei + base;
		return re_image;
	}*/

	return re_image;
}

Mat ImageClass::Refactoring(Mat ori_image) {


	Rect rect(c_x, c_y, c_wid, c_hei);		// x, y ,wid, hei

	fix_image = ori_image(rect);			//rect ��ŭ crop
	ShowImage(fix_image, c_wid, c_hei);

	if(String_Type == 4)
		fix_image = Resize_Num(fix_image);
	else 
		fix_image = Resize_String(fix_image);
	ShowImage(fix_image, c_wid, c_hei);

	//std::cout << "���� ���� �� ����" << c_wid - base << " " << base_height << std::endl;

	//���� �߰�

	//fix_image = Gaussian_Blur(fix_image, 3, 3);

	//Canny(fix_image, fix_image, 50, 200);           // �ܰ��� ���⵵ �غ���.

	return fix_image;
}

Mat ImageClass::GrayScale(Mat fix_image) {

	Mat GrayImage;
	
	cv::cvtColor(fix_image, GrayImage, COLOR_RGB2GRAY);

	return GrayImage;
}

Mat ImageClass::Thresholding(Mat fix_image) {

	Mat ThreshImage;




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

BYTE* ImageClass::Mat2Byte(Mat fix_image) {				//����

	int size = fix_image.rows * fix_image.cols;

	src = /*(BYTE*)*/fix_image.data;

	//std::memcpy(src, fix_image.data, size * sizeof(BYTE));

	//imencode(".png", fix_image, src, src);


	return src;
}
