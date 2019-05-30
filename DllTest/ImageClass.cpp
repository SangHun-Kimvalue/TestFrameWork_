#include "ImageClass.h"

bool RGBSaveBMP(BYTE *input, int nWidth, int nHeight, int index, int depth) {

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
			bf.bfSize = sizeof(bf) + sizeof(bi) + nWidth * nHeight * depth;
			bf.bfOffBits = sizeof(bf) + sizeof(bi);
			bi.biSize = sizeof(bi);
			bi.biWidth = nWidth;
			bi.biHeight = nHeight;
			bi.biPlanes = 1;
			bi.biBitCount = depth * 8;
			bi.biSizeImage = nWidth * nHeight * depth;

			fwrite(&bf, sizeof(bf), 1, file);
			fwrite(&bi, sizeof(bi), 1, file);
			fwrite(image, sizeof(unsigned char), nHeight*nWidth * depth, file);

			fclose(file);
		}
	}
	return true;
}

void Save2png(Mat inputimage) {

	imwrite("fix.png", inputimage);

}

ImageClass::ImageClass(){}

ImageClass::ImageClass(int wid, int hei, BYTE* src, int String_Type, int Base_length) 
	: nWidth(wid), nHeight(hei), src(src), String_Type(String_Type), Base_length(Base_length){

	base_height = 75;				// ����ڰ� ��������� ������ ���������� ������ ���� ������ 50 ~ 70% �� ���� �Ǿ� �ְ� 
									// ������ ũ�⸦ 35�� ����� ũ��� ����� ���ؼ��� ���� ���̸� 75�� �����ؾ���.
	base_width = 100;

	// 1. ���̿� �ʺ� ���� ���� �� �׸��� ��ħ(linesize �� ���� �ε�)
	// 2. �׷��� �̹����� �ϸ� �׼������� �ν��� ����	>> ä�μ� �̻��ѵ�(1���� ����)(BGRA >> GRAY)(ä��)		//tesseract ���� 1ä�η� �������ټ�����?
	// 2.2 �׷��� �̹��������� �н�  >> ������ Ȧ�忡�� 1ä�� �̹����� �ٽ� 4ä�η� ����������.

	ori_image = CV_Ini_t(44, 0, 150, 40);			//���� ���� �ٸ��� ����
	ShowImage(ori_image);
	//bitwise_not(ori_image, ori_image);			//�ȼ� ����

	fix_image = Refactoring(ori_image);	

	fix_image = GrayScale(fix_image);				//�׷��� �̹����ϸ� src�� �̻�����.(pixel ä�ζ����ε�.)  �ذ�
	
	fix_image = Thresholding(fix_image);
	ShowImage(fix_image);
	
	fix_image = Gaussian_Blur(fix_image, 3, 3);
	ShowImage(fix_image);

	//Canny(fix_image, fix_image, 50, 200);           // �ܰ��� ���⵵ �غ���.

	Save2png(ori_image);
	Save2png(fix_image);

	//this->src = Mat2Byte(ori_image, 0, 4);
	this->src = Mat2Byte(fix_image, 1, 4);
	
}

ImageClass::~ImageClass()
{
}

Mat ImageClass::CV_Ini_t(int x, int y, int wid, int hei) {
	
	
	c_x = x; 	c_y = y;  	c_wid = wid; 	c_hei = hei;

	std::string Ipath = IMAGEPATH;
	Ipath  = Ipath  + "ocr.bmp";

	ori_image = Mat(nWidth, nHeight, CV_8UC(4), src);
	
	//Mat DecodeImg = ori_image; // imdecode(ori_image, IMREAD_COLOR);
	std::vector<uchar> OutBuffer;
	OutBuffer.push_back((uchar)src);
	bool res = imencode(".bmp", ori_image, OutBuffer);

	//ori_image = Mat(1, OutBuffer.size(), CV_8UC4, OutBuffer.data());
	//Mat DecodeImg = imdecode(ori_image, IMREAD_COLOR);

	//ori_image = imread(Ipath.c_str(), IMREAD_COLOR);			//BGR�� ����(���� �ε�)
	
	return ori_image;
}

Mat ImageClass::CV_Init() {

	c_x = 0; 	c_y = 0;  	c_wid = 500; 	c_hei = 500;		//wid, hei ���� ������ ������ �ȵ� ����.

	if (c_x + c_wid > nWidth) 
		std::cerr << "���� ����" << std::endl;
	else if(c_y + c_hei > nHeight)
		std::cerr << "���� ����" << std::endl;

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

void ImageClass::ShowImage(Mat Image) {

	//imshow("sub", cropimage);				//Show image
	namedWindow("main", WINDOW_AUTOSIZE);					// Create a window
	//namedWindow("main", WINDOW_NORMAL);					// Create a window
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

	float percent = (float)c_wid - (float)base / (float)c_wid;
	int temp_hei = c_hei * percent;
	int temp_wid = c_wid - base;

	if (base == 0) {					//�������� �� ���̰� �ƴ϶� �ۼ��������� ���̿� ���̸� ����ϸ� ������??
		return fix_image;
	}
	else if (base > 0) {				//���
		
		//�̹����� ����ġ ���� �� �۾����� �ȵ����� ���� ��ġ�� �ѹ� �� ���� ����Ѵ�.
		if (temp_hei < base_height || (temp_wid < base_width)) {
			temp_hei = temp_hei + base_height;
			temp_wid = temp_wid + base_width;
			cv::resize(fix_image, re_image, cv::Size(temp_wid, temp_hei), 0, 0, INTER_AREA);
			//ShowImage(re_image);
			c_wid = temp_wid;
			c_hei = temp_hei;
			return re_image;
		}

		else {
			//�̹��� ���ø��̼� (�������� �ݴ�Ǵ� ������ LowPass ���͵� ����)�� ��ȣ�Ǵ� �÷���		//INTER_AREA >> Bilinear_Interpolation�� �ڼ���
			cv::resize(fix_image, re_image, cv::Size(temp_wid, temp_hei), 0, 0, INTER_AREA);
			c_wid = temp_wid;
			c_hei = temp_hei;

			return re_image;
		}
	}
	else if (base < 0) {			//Ȯ��

		cv::resize(fix_image, re_image, cv::Size(temp_wid, temp_hei), 0, 0, INTER_LINEAR);			//�缱�� ������  >>  Bilinear_Interpolation 
		c_wid = temp_wid;
		c_hei = temp_hei;

		return re_image;
	}
	return re_image;
}

Mat ImageClass::Resize_String(Mat ori_image) {
	
	Mat re_image;

	//std::cout << "���� ���� �� ����" << c_wid << " " << c_hei << std::endl;

	//CV_EXPORTS_W void resize(InputArray src, OutputArray dst,
	//	Size dsize, double fx = 0, double fy = 0,
	//	int interpolation = INTER_LINEAR);
	//cv::resize(fix_image, fix_image, cv::Size(wid, hei), 0, 0, INTER_LINEAR);
	//�̹����� ����Ϸ��� �Ϲ������� INTER_AREA �������� ������ �������� �̹����� Ȯ���Ϸ��� �Ϲ������� 
	//INTER_LINEAR (���� ������ �� ������ ������ ���Դϴ�) �Ǵ� INTER_CUBIC (����)�� ���� �� ���Դϴ�.
	
	int base = c_hei - base_height;
	float percent = ((float)c_hei - (float)base) / (float)c_hei;

	int temp_wid = c_wid * percent;
	int temp_hei = c_hei - base;

	if (base == 0) {					//�������� �� ���̰� �ƴ϶� �ۼ��������� ���̿� ���̸� ����ϸ� ������??
		return ori_image;
	}
	else if (base > 0) {				//���
		//�̹����� ����ġ ���� �� �۾����� �ȵ����� ���� ��ġ�� �ѹ� �� ���� ����Ѵ�.
		if (temp_hei < base_height || (temp_wid < base_width)) {
			temp_hei = temp_hei + base_height;
			temp_wid = temp_wid + base_width;
			cv::resize(ori_image, re_image, cv::Size(temp_wid, temp_hei), 0, 0, INTER_AREA);
			//ShowImage(re_image);
			c_wid = temp_wid;
			c_hei = temp_hei;
			return re_image;
		}
		else {

			//�̹��� ���ø��̼� (�������� �ݴ�Ǵ� ������ LowPass ���͵� ����)�� ��ȣ�Ǵ� �÷���		//INTER_AREA >> Bilinear_Interpolation�� �ڼ���
			cv::resize(ori_image, re_image, cv::Size(temp_wid, temp_hei), 0, 0, INTER_AREA);
			c_wid = temp_wid;
			c_hei = temp_hei;

			return re_image;
		}
	}
	else if (base < 0) {			//Ȯ��

		cv::resize(ori_image, re_image, cv::Size(temp_wid, temp_hei), 0, 0, INTER_LINEAR);			//�缱�� ������  >>  Bilinear_Interpolation 
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

	Rect rect(c_x, c_y, c_wid, c_hei);							// x, y ,wid, hei

	(ori_image(rect)).copyTo(fix_image);						//rect ��ŭ crop
	//fix_image = ori_image(rect);			//rect ��ŭ crop

	if(String_Type == 4)
		fix_image = Resize_Num(fix_image);
	else 
		fix_image = Resize_String(fix_image);

	//std::cout << "���� ���� �� ����" << c_wid - base << " " << base_height << std::endl;


	return fix_image;
}

Mat ImageClass::GrayScale(Mat fix_image) {

	Mat GrayImage;
	
	GrayImage = Mat(fix_image.size(), CV_8UC4);
	cvtColor(fix_image, GrayImage, COLOR_BGRA2GRAY);
	
	
	return GrayImage;
}

Mat ImageClass::Thresholding(Mat fix_image) {

	Mat ThreshImage;

	//src	�ҽ� 8 ��Ʈ ���� ä�� �̹���.
	//DST	src�� ���� ũ�� �� ������ ������ ��� �̹���.
	//maxValue	������ �����Ǵ� �ȼ��� �Ҵ� �� 0�� �ƴ� ��
	//���� �� ���	���� �� thresholding �˰����� ����մϴ�.	//	ADAPTIVE_THRESH_MEAN_C / ADAPTIVE_THRESH_GAUSSIAN_C
	//�Ӱ� �� ����	thresholding Ÿ���� THRESH_BINARY �Ǵ� THRESH_BINARY_INV�� �ƴϸ� �ȵȴ� .
	//blockSize	�ȼ��� �Ӱ� ���� ����ϴ� �� ���Ǵ� �ȼ� ���� ������ ũ�� : 3, 5, 7 ��.
	//����	����� ��� �Ǵ� ���� ��տ��� ���ϴ�(�Ʒ� ���� ���� ����).�Ϲ������� ��������� 0 �Ǵ� ���� �� �� �ֽ��ϴ�.

	//threshold(fix_image, ThreshImage, 127, 255, THRESH_BINARY);
	adaptiveThreshold(fix_image, ThreshImage, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, 5, 10);
	cvtColor(ThreshImage, fix_image, COLOR_GRAY2BGRA);


	return ThreshImage;
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

BYTE* ImageClass::Mat2Byte(Mat input_image, int index, int depth) {				//����

	int size = input_image.rows * input_image.cols;
	
	//std::memcpy(src, fix_image.data, size * sizeof(BYTE));
	//imencode(".png", fix_image, src, src);

	this->src = (BYTE*)input_image.data;
	//RGBSaveBMP(this->src, input_image.rows, input_image.cols, index, depth);

	return src;
}

BYTE* ImageClass::Mat2Byte(Mat input_image, int index, int depth, int save) {				//����

	int size = input_image.rows * input_image.cols;

	//std::memcpy(src, fix_image.data, size * sizeof(BYTE));
	//imencode(".png", fix_image, src, src);

	this->src = (BYTE*)input_image.data;
	RGBSaveBMP(this->src, 1000, 1000, index, depth);

	return src;
}