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

void Save2png(Mat inputimage, std::string name) {

	std::string temp = name + ".png";

	imwrite(temp.c_str(), inputimage);
}

ImageClass::ImageClass(){}

ImageClass::ImageClass(int wid, int hei, BYTE* src, int String_Type, int Base_length) 
	: src(src), base_length(Base_length) {

	/*
	//�ӵ� ���� ���ɼ� >> �ٽ� 4ä�η� ��ȯ�ϴ°� �ƴ� tesseract ���� 1ä�η� �������ټ�����?
	//Base length�� �°� Resize ���� �ʿ���.
	//����þ��� �ʹ� �帰������ ����.

	//ori_image = CV_Init(wid, hei, 44, 0, 150, 40);			
	
	//fix_image = Crop(ori_image);

	//fix_image = Resize(fix_image, String_Type);

	//fix_image = GrayScale(fix_image);				//�׷��� �̹����ϸ� src�� �̻�����.(pixel ä�ζ����ε�.)  �ذ�

	//fix_image = Gaussian_Blur(fix_image, 3, 3);

	//ShowImage(fix_image);

	////Save2png(ori_image, "ori");
	//Save2png(fix_image, "Thresh");

	//this->src = Mat2Byte(fix_image, 1, 4);
	*/
}

ImageClass::~ImageClass()
{
	Release();
}

//bool ImageClass::PreImageProcess(int String_Type, int String_length) {
//
//	fix_image = Crop(ori_image);
//
//	fix_image = Resize(fix_image, String_Type, String_length);
//
//	fix_image = GrayScale(fix_image);				//�׷��� �̹����ϸ� src�� �̻�����.(pixel ä�ζ����ε�.)  �ذ�
//
//	fix_image = Gaussian_Blur(fix_image, 3, 3);
//
//	ShowImage(fix_image);
//
//	//Save2png(ori_image, "ori");
//	//Save2png(fix_image, "Thresh");
//
//	this->src = Mat2Byte(fix_image, 1, 4);
//
//	return true;
//}


Mat ImageClass::CV_Init(int ori_wid, int ori_hei, int x, int y, int wid, int hei, unsigned char* src) {

	std::string Ipath = IMAGEPATH;
	Ipath = Ipath + "ocr.bmp";
	base_height = 75;											// ����ڰ� ��������� ������ ���������� ������ ���� ������ 50 ~ 70% �� ���� �Ǿ� �ְ� 
	base_width = 100;											// ������ ũ�⸦ 35�� ����� ũ��� ����� ���ؼ��� ���� ���̸� 75�� �����ؾ���.
																
	c_x = x; 	c_y = y;  	c_wid = wid; 	c_hei = hei;		//wid, hei ���� ������ ������ �ȵ� ����.
															
	if (c_x + c_wid > ori_wid) 
		std::cerr << "���� ����" << std::endl;
	else if(c_y + c_hei > ori_hei)
		std::cerr << "���� ����" << std::endl;

	ori_image = Mat(ori_hei, ori_wid, CV_8UC(4), src);
	if (ori_image.empty()) {
		std::cout << "Could not open or find the image" << std::endl;
		std::cin.get(); //wait for any key press
		return ori_image;
	}

	//Mat DecodeImg = ori_image; // imdecode(ori_image, IMREAD_COLOR);
	std::vector<uchar> OutBuffer;
	OutBuffer.push_back((uchar)src);
	bool res = imencode(".bmp", ori_image, OutBuffer);

	OutBuffer.clear();

	return ori_image;
}

void ImageClass::Release() {
	ori_image.release();
	fix_image.release();
}

//�̹����� �����ִ� ����׿� 
void ImageClass::ShowImage(Mat Image) {

	//imshow("sub", cropimage);				//Show image
	namedWindow("main", WINDOW_AUTOSIZE);					// Create a window
	//namedWindow("main", WINDOW_NORMAL);					// Create a window
	imshow("main", Image);				// Show our image inside the created window.
	waitKey(0);								// Wait for any keystroke in the window

	destroyWindow("main");					//destroy the created window
}

//String_Type�� Num�϶� ���̸� �߽����� ��ȯ
Mat ImageClass::Resize_Num(Mat ori_image) {

	Mat re_image;

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
	return re_image;
}

//String_Type�� �����϶� ���̸� �߽����� ��ȯ		//INTER_AREA >> Bilinear_Interpolation�� �ڼ���			//�缱�� ������  >>  Bilinear_Interpolation 
Mat ImageClass::Resize_String(Mat ori_image, int String_length) {		//������ �ƴ� �ܾ ������ ����.	//������ ������ ��ŭ �ֱ�����.		>> ��� ����� ���� �����غ�����/
	
	Mat re_image;

	//CV_EXPORTS_W void resize(InputArray src, OutputArray dst,
	//	Size dsize, double fx = 0, double fy = 0,
	//	int interpolation = INTER_LINEAR);
	//cv::resize(fix_image, fix_image, cv::Size(wid, hei), 0, 0, INTER_LINEAR);
	//�̹����� ����Ϸ��� �Ϲ������� INTER_AREA �������� ������ �������� �̹����� Ȯ���Ϸ��� �Ϲ������� 
	//INTER_LINEAR (���� ������ �� ������ ������ ���Դϴ�) �Ǵ� INTER_CUBIC (����)�� ���� �� ���Դϴ�.
	
	base_width = String_length * 37;		//������ ���̽� ������ ���̺��ٴ� ũ�� �����ؾ���(�� ���� ������ ������ ����) (37����Ʈ�� ���� �̻����� ����)

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

	return re_image;
}

//�νķ��� �ʹ� �����Ƽ� ���.
Mat ImageClass::C_Canny(Mat ori_image) {

	Mat canny_image;
	int lowThreshold = 50;
	int highThreshold = 150;

	//�׷���, ����þ�, �Һ� Ŀ�η� ���� ���� ���� ����, ���� �翷�� ������ ���� ����, ����ȭ   ����(ĳ�� ���� ����Ʈ �̷�)
	//void cv::Canny(InputArray image, OutputArray edges, double threshold1, double threshold2, int apertureSize = 3, bool 	L2gradient = false)
	Canny(ori_image, canny_image, lowThreshold, highThreshold);			//���� �Ӱ谪, ���� �Ӱ谪�� 1 : 2 �� 1 : 3 ������ ����.
	ShowImage(canny_image);

	return canny_image;
}

//�̹��� �ڸ���
Mat ImageClass::Crop(Mat ori_image) {

	Mat crop_image;
	Rect rect(c_x, c_y, c_wid, c_hei);							// x, y ,wid, hei

	(ori_image(rect)).copyTo(crop_image);						//rect ��ŭ crop
	//fix_image = ori_image(rect);			//rect ��ŭ crop

	return crop_image;
}

//�̹��� ������ ����			String_Type�� �°� �޼��� ȣ��		>>  ���̿� ���̿� ���߾� ������. (������ ���ڿ�, ���� ���� ��Ȳ ���)
Mat ImageClass::Resize(Mat ori_image, int String_Type, int String_length) {

	Mat resize_image;

	if (ori_image.cols < ori_image.rows) {
		resize_image = Resize_Num(ori_image);
	}
	else {
		resize_image = Resize_String(ori_image, String_length);
	}

	//if(String_Type == 3)
	//	resize_image = Resize_Num(ori_image);
	//else 
	//	resize_image = Resize_String(ori_image, String_length);

	return resize_image;
}

//�׷��� �̹��� ���� >> BGRA 4ä�ο��� 1ä�η� ����Ǳ� ������ �ٽ� 4ä�� BGRA�� ���� �۾� ����.  >> �׼������� 1ä�η� ���̹����� ������
Mat ImageClass::GrayScale(Mat ori_image) {

	Mat GrayImage;
	Mat Thresholding_Image;

	GrayImage = Mat(ori_image.size(), CV_8UC4);
	cvtColor(ori_image, GrayImage, COLOR_BGRA2GRAY);
	
	//ShowImage(GrayImage);
	Thresholding_Image = Thresholding(GrayImage);
	//cvtColor(Thresholding_Image, ori_image, COLOR_GRAY2BGRA);					

	return Thresholding_Image;//ThreshImage�� �ٽ� 4ä�η� �����ϴ� �������� ori_image�� dst�� ��������.
}

// 1ä�� �׷��� �̹����� �Ű������� �޾Ƶ���
Mat ImageClass::Thresholding(Mat ori_image) {

	Mat ThreshImage;			//�Ű����� src�� �ݵ�� �׷��� �̹������;��� �ȱ׷��� ����.

	//src	�ҽ� 8 ��Ʈ ���� ä�� �̹���.
	//DST	src�� ���� ũ�� �� ������ ������ ��� �̹���.
	//maxValue	������ �����Ǵ� �ȼ��� �Ҵ� �� 0�� �ƴ� ��
	//���� �� ���	���� �� thresholding �˰����� ����մϴ�.	//	ADAPTIVE_THRESH_MEAN_C / ADAPTIVE_THRESH_GAUSSIAN_C
	//�Ӱ� �� ����	thresholding Ÿ���� THRESH_BINARY �Ǵ� THRESH_BINARY_INV�� �ƴϸ� �ȵȴ� .
	//blockSize	�ȼ��� �Ӱ� ���� ����ϴ� �� ���Ǵ� �ȼ� ���� ������ ũ�� : 3, 5, 7 ��.
	//����	����� ��� �Ǵ� ���� ��տ��� ���ϴ�(�Ʒ� ���� ���� ����).�Ϲ������� ��������� 0 �Ǵ� ���� �� �� �ֽ��ϴ�.

	//threshold(fix_image, ThreshImage, 127, 255, THRESH_BINARY);				

	adaptiveThreshold(ori_image, ThreshImage, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, 11, 5);			//11�� ������ ������.
	
	//fix_image = Gaussian_Blur(ThreshImage, 3, 3);
	//Save2png(fix_image, "Thresh_11_5_blur");
	//adaptiveThreshold(ori_image, ThreshImage, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, 21, 5);
	//fix_image = Gaussian_Blur(ThreshImage, 3, 3);
	//Save2png(fix_image, "Thresh_21_5_blur");
	//adaptiveThreshold(ori_image, ThreshImage, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, 31, 5);
	//fix_image = Gaussian_Blur(ThreshImage, 3, 3);
	//Save2png(fix_image, "Thresh_31_5_blur");

	return ThreshImage;		
}

//�����..   �缱�� �������� �̹� Resize���� ����Ǿ� �־� �ʿ䰡 ������.
Mat ImageClass::Bilinear_Interpolation(Mat ori_image){
	

	//INTER_AREA�� ����.
	//INTER_AREA �÷��״� �̹��� Ȯ�뿡 ��� INTER_LINEAR�� ȣ���Ѵ�.(����� �ణ �ٸ�)
	//����� ��� wid�� hei �� 2�� ����̰�, ��Ʈ ä�μ��� 2�� �ƴϸ� INTER_AREA�� �����. �׿��� ��쿡�� INTER_LINEAR_EXACT �̳� INTER_LINEAR�� ���
	//����ϴ� ��쿡�� ���ǰ� ������ ��� ���� ��հ� �����.(Ŀ�ο��� ���ϰ� ���� Not Mask)
	//https://medium.com/@wenrudong/what-is-opencvs-inter-area-actually-doing-282a626a09b3

	//���� �ƴ� ���� P,Q ������ �� R�� �����Ҷ�, �� ���� ���� �������� ���踦 ���´ٰ� �����ϰ� P,Q�� ���� �Ÿ��� �̿��� R�� �����Ѵ�.
	//R = P * (dx) + Q * (1-dx)		�� �̿��Ͽ� x ��� y���� �ι� ���� ������ ������ ���� ����ġ�� �����Ѵ�.
	//R = A * (dx)(dy) + B * (1-dx)(dy) + C * (dx)(1-dy) + D * (1-dx)(1-dy)

	//cv::resize(fix_image, fix_image, cv::Size(c_wid - base, base_height), 0, 0, INTER_LINEAR); ���� �̹� ���� �Ǿ�����.


	return ori_image;

}

//����þ� �帲 ȿ��
Mat ImageClass::Gaussian_Blur(Mat ori_image) {			//�ñ׸��� 0�̸� �ڵ����� ����(MASK �ڵ����)

	Mat Gasu_image;
	//void GaussianBlur(InputArray src, OutputArray dst, double sigmaX, double sigmaY=0, int borderType=BORDER_DEFAULT )
	// GaussianBlur( src, dst, Size( i, i ), 0, 0 );				//�Ÿ��� ���� ����ġ ����ũ��		, BorderType�� ũ�� ������ ���µ���.
	GaussianBlur(ori_image, Gasu_image, Size(3, 3), BORDER_DEFAULT);

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

	ShowImage(Gasu_image);
	Save2png(Gasu_image, "Lastest_PNG");

	return Gasu_image;
}

//�̹����� �ٽ� BYTE ������ ���� (����x)
BYTE* ImageClass::Mat2Byte(Mat input_image, int index, int depth) {				//����

	int size = input_image.rows * input_image.cols;
	
	//std::memcpy(src, fix_image.data, size * sizeof(BYTE));
	//imencode(".png", fix_image, src, src);

	this->src = (BYTE*)input_image.data;
	//RGBSaveBMP(this->src, input_image.rows, input_image.cols, index, depth);

	return src;
}

//�̹����� �ٽ� BYTE ������ ���� (����o)
BYTE* ImageClass::Mat2Byte(Mat input_image, int index, int depth, int save) {				//����

	int size = input_image.rows * input_image.cols;

	//std::memcpy(src, fix_image.data, size * sizeof(BYTE));
	//imencode(".png", fix_image, src, src);

	this->src = (BYTE*)input_image.data;
	RGBSaveBMP(this->src, 1000, 1000, index, depth);

	return src;
}