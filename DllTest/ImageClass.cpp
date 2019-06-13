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

ImageClass::ImageClass() : Reverse_Color(false) {}

ImageClass::ImageClass(int i): Reverse_Color(false) {}

ImageClass::~ImageClass()
{
	Release();
}

void ImageClass::Reverse_check(Mat fix_image) {

	Mat InputMat = fix_image.clone();

	float wid = InputMat.cols;
	float hei = InputMat.rows;
	float percent = 0;

	uchar pixsum = 0;
	int temp = 0;

	if (wid > hei) {
		percent = wid / hei;
		for (int i = 0; i < hei; i++) {
			int y = i;	int x = (y * percent);
			pixsum = InputMat.at<uchar>(y, x);
			temp += (int)pixsum;
			//InputMat.at<uchar>(y, x) = 0;
		}
		int ave = temp / hei;
		if (ave < 127) {
			Reverse_Color = true;
		}
		else
			Reverse_Color = false;
	}
	else {
		percent = hei / wid;
		for (int i = 0; i < wid; i++) {
			int x = i;	int y = (x * percent);
			pixsum = InputMat.at<uchar>(y, x);
			temp += (int)pixsum;
			//InputMat.at<uchar>(y, x) = 0;
		}
		int ave = temp / wid;
		if (ave < 127) {
			Reverse_Color = true;
		}
		else
			Reverse_Color = false;
	}

	//ShowImage(InputMat);
	//std::cout << "Reverse_Color : " << Reverse_Color << std::endl;
	InputMat.release();

	return;
}

void ImageClass::Reverse_check_Ran(Mat fix_image) {

	Mat InputMat;

	InputMat = fix_image.clone();
	int count = 20;

	if (InputMat.cols > 50 && InputMat.rows > 10) {

		uchar pixsum = 0;
		int temp = 0;
		int scope = (InputMat.rows * InputMat.cols);

		for (int i = 0; i < count; i++) {
			
			long pos = (((long)rand()<<15)|rand()) % scope;
			pixsum = InputMat.at<uchar>(pos);
			temp += (int)pixsum;
			InputMat.at<uchar>(pos) = 0;
			//std::cout << "pos : " << pos << std::endl;
		}

		if (temp / count < 127) {
			Reverse_Color = true;
		}
		else
			Reverse_Color = false;
	}
	//ShowImage(InputMat);
	//std::cout << "Reverse_Color : " << Reverse_Color << std::endl;
	InputMat.release();

	return ;
}

bool ImageClass::Init(int ori_wid, int ori_hei, int x, int y, int wid, int hei) {

	//std::string Ipath = IMAGEPATH;
	//Ipath = Ipath + "ocr.bmp";

	//base_height = 150;											// ����ڰ� ��������� ������ ���������� ������ ���� ������ 50 ~ 70% �� ���� �Ǿ� �ְ� 
	//base_width = 200;												// ������ ũ�⸦ 35�� ����� ũ��� ����� ���ؼ��� ���� ���̸� 75�� �����ؾ���.
	best_font_size = 75;

	c_x = x; 	c_y = y;  	c_wid = wid; 	c_hei = hei;		//wid, hei ���� ������ ������ �ȵ� ����.
															
	if (c_x + c_wid > ori_wid) {
		std::cerr << "���� ����" << std::endl;
		return false;
	}
	else if (c_y + c_hei > ori_hei) {
		std::cerr << "���� ����" << std::endl;
		return false;
	}

	srand((unsigned int)time(NULL));

	return true;
}

Mat ImageClass::Create_Mat(int ori_wid, int ori_hei, unsigned char* src) {

	Release();

	ori_image = Mat(ori_hei, ori_wid, CV_8UC(4), src);
	if (ori_image.empty()) {
		std::cout << "Could not open or find the image" << std::endl;
		std::cin.get(); //wait for any key press
		return ori_image;
	}

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

//String_Type�� Num�϶� ���̸� �߽����� ��ȯ			//���
Mat ImageClass::Resize_Num(Mat ori_image) {

	Mat re_image;

	//CV_EXPORTS_W void resize(InputArray src, OutputArray dst,
	//	Size dsize, double fx = 0, double fy = 0,
	//	int interpolation = INTER_LINEAR);
	//cv::resize(fix_image, fix_image, cv::Size(wid, hei), 0, 0, INTER_LINEAR);
	//�̹����� ����Ϸ��� �Ϲ������� INTER_AREA �������� ������ �������� �̹����� Ȯ���Ϸ��� �Ϲ������� 
	//INTER_LINEAR (���� ������ �� ������ ������ ���Դϴ�) �Ǵ� INTER_CUBIC (����)�� ���� �� ���Դϴ�.

	base_width = base_height;

	int base = fix_image.cols - base_width;

	float percent = ((float)fix_image.cols - (float)base) / (float)fix_image.cols;
	int temp_hei = fix_image.rows * percent;
	int temp_wid = fix_image.cols - base;

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
			//c_wid = temp_wid;
			//c_hei = temp_hei;
			return re_image;
		}

		else {
			//�̹��� ���ø��̼� (�������� �ݴ�Ǵ� ������ LowPass ���͵� ����)�� ��ȣ�Ǵ� �÷���		//INTER_AREA >> Bilinear_Interpolation�� �ڼ���
			cv::resize(ori_image, re_image, cv::Size(temp_wid, temp_hei), 0, 0, INTER_AREA);
			//c_wid = temp_wid;
			//c_hei = temp_hei;

			return re_image;
		}
	}
	else if (base < 0) {			//Ȯ��

		cv::resize(ori_image, re_image, cv::Size(temp_wid, temp_hei), 0, 0, INTER_LINEAR);			//�缱�� ������  >>  Bilinear_Interpolation 
		//c_wid = temp_wid;
		//c_hei = temp_hei;

		return re_image;
	}
	return re_image;
}

//String_Type�� �����϶� ���̸� �߽����� ��ȯ		//���	//INTER_AREA >> Bilinear_Interpolation�� �ڼ���			//�缱�� ������  >>  Bilinear_Interpolation 
Mat ImageClass::Resize_String(Mat ori_image, int String_length) {		//������ �ƴ� �ܾ ������ ����.	//������ ������ ��ŭ �ֱ�����.		>> ��� ����� ���� �����غ�����/
	
	Mat re_image;
	int temp_wid;
	int temp_hei;
	base_width = String_length * 37;		//������ ���̽� ������ ���̺��ٴ� ũ�� �����ؾ���(�� ���� ������ ������ ����) (37����Ʈ�� ���� �̻����� ����)

	int base = ori_image.rows - base_height;
	float percent = ((float)ori_image.rows - (float)base) / (float)ori_image.rows;
	if (percent > 2) {
		percent = 2;
		temp_wid = ori_image.cols * percent;
		temp_hei = ori_image.rows * percent;
	}

	temp_wid = ori_image.cols * percent;
	temp_hei = base_height;

	if (base == 0) {
		return ori_image;
	}
	else if (base > 0) {				//���
		//�̹����� ����ġ ���� �� �۾����� �ȵ�		//�ʹ� ���̸� �۾��� �ƿ� ¥�ε�.( ����ڰ� ���� ���� ������ ���ؾ���.)
		if (temp_hei > base_height && (temp_wid > base_width) && percent > 0.5f) {
			//temp_hei = temp_hei + base_height;
			//temp_wid = temp_wid + base_width;
			//�̹��� ���ø��̼� (�������� �ݴ�Ǵ� ������ LowPass ���͵� ����)�� ��ȣ�Ǵ� �÷���		//INTER_AREA >> Bilinear_Interpolation�� �ڼ���
			cv::resize(ori_image, re_image, cv::Size(temp_wid, temp_hei), 0, 0, INTER_AREA);
			return re_image;
		}
		else {
			base = base_width - temp_wid;
			percent = ((float)ori_image.cols + (float)base) / (float)ori_image.cols;
			if (percent > 2) {
				percent = 2;
				temp_wid = ori_image.cols * percent;
				temp_hei = ori_image.rows * percent;
			}
			temp_wid = base_width;
			temp_hei = ori_image.rows * percent;

			cv::resize(ori_image, re_image, cv::Size(temp_wid, temp_hei), 0, 0, INTER_LINEAR);

			return re_image;
		}
	}
	else if (base < 0) {			//Ȯ��

		cv::resize(ori_image, re_image, cv::Size(temp_wid, temp_hei), 0, 0, INTER_LINEAR);			//�缱�� ������  >>  Bilinear_Interpolation 

		return re_image;
	}

	return re_image;
}

//�νķ��� �ʹ� �����Ƽ� ���.		������ ����
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

//�̹��� ������ ����		//Base_String�� ���̿� ����ȭ �Ͽ� ������ 	//INTER_AREA >> Bilinear_Interpolation�� �ڼ���			//�缱�� ������  >>  Bilinear_Interpolation 
Mat ImageClass::Resize(Mat ori_image, int String_length) {
	
	Mat re_image;
	int wid = ori_image.cols;		int hei = ori_image.rows; 
	float temp_wid = 0;				float temp_hei = 0;
	int base = 0;					float percent = 0;

	if (String_length < 3)					//wid�� 300������ �Ǿ� Tesseract ���� �ּ����� ���ϰ� �ɸ�
		String_length = 3;
	base_width = String_length * best_font_size;		//������ ���̽� ������ ���̺��ٴ� ũ�� �����ؾ���(�� ���� ������ ������ ����) (37����Ʈ�� ���� �̻����� ����)

	base = wid - base_width;
	percent = ((float)wid - (float)base) / (float)wid;

	if(base >= 0){
	//if (base == 0)
	//		return ori_image;
	//else if (base > 0) {	//���
		//temp_hei = hei * percent;
		//temp_wid = base_width;
		//if (temp_hei >= base_height) {
		//	//�̹��� ���ø��̼� (�������� �ݴ�Ǵ� ������ LowPass ���͵� ����)�� ��ȣ�Ǵ� �÷���		//INTER_AREA >> Bilinear_Interpolation�� �ڼ���
		//	cv::resize(ori_image, re_image, cv::Size(temp_wid, temp_hei), 0, 0, INTER_AREA);
		//	return re_image;
		//}
		//else {
		//	base = base_height - temp_hei;
		//	percent = (float)(temp_hei + base) / (float)temp_hei;
		//	temp_wid = temp_wid * percent;
		//	temp_hei = temp_hei * percent;
		//
		//	cv::resize(ori_image, re_image, cv::Size(temp_wid, temp_hei), 0, 0, INTER_AREA);
		//	return re_image;
		//}
		return ori_image;
	}
	else if (base < 0) {	//Ȯ��

		temp_hei = hei * percent;
		temp_wid = base_width;

		if (percent > 2.5) {
			percent = 2.5;
			temp_hei = hei * percent;
			temp_wid = wid * percent;
		}
		//clock_t start = clock();

		cv::resize(ori_image, re_image, cv::Size((int)temp_wid+0.5, (int)temp_hei+0.5), 0, 0, INTER_CUBIC);				//3�� ���׽� ��ü ������		//0~1ms
		//cv::resize(ori_image, re_image, cv::Size(temp_wid, temp_hei), 0, 0, INTER_LINEAR);			//�缱�� ������  >>  Bilinear_Interpolation	//1~3ms
		//clock_t end = clock();

		//std::cout << "INTER_LINEAR : " << end - start << std::endl;
		//ShowImage(re_image);
		//Save2png(re_image, "INTER_CUBIC4");

		return re_image;
	}

	return ori_image;
}

//�׷��� �̹��� ���� >> BGRA 4ä�ο��� 1ä�η� ����Ǳ� ������ �ٽ� 4ä�� BGRA�� ���� �۾� ����.  >> �׼������� 1ä�η� ���̹����� ������
Mat ImageClass::GrayScale(Mat ori_image) {

	Mat GrayImage;

	GrayImage = Mat(ori_image.size(), CV_8UC4);
	cvtColor(ori_image, GrayImage, COLOR_BGRA2GRAY);		//1~2ms
	//ShowImage(GrayImage);

	//GrayImage = C_Canny(GrayImage);
	//std::vector<std::vector<cv::Point>> contours;
	//cv::findContours(GrayImage,
	//	contours,    // �ܰ��� ���� 
	//	RETR_EXTERNAL,  // �ܺ� �ܰ��� �˻�
	//	CHAIN_APPROX_NONE); // �� �ܰ����� ��� ȭ�� Ž��
	//ShowImage(GrayImage);

	//clock_t start = clock();
	Reverse_check(GrayImage);			// 1ms ����

	//clock_t end = clock();
	//std::cout << "RecerseCheck : " << end - start << std::endl;

	//cvtColor(GrayImage, ori_image, COLOR_GRAY2BGRA);			//4ä�η� ������ �׼����� 3~4ms �� �ɸ�
	//Save2png(GrayImage, "Lastest_PNG");
	return GrayImage;//ThreshImage�� �ٽ� 4ä�η� �����ϴ� �������� ori_image�� dst�� ��������.
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
	
	//adaptiveThreshold(image, // �Է¿��� 
	//	binaryAdaptive, // ����ȭ ��� ���� 
	//	255, // �ִ� ȭ�� �� 
	//	ADAPTIVE_THRESH_MEAN_C, // Adaptive �Լ�		//����ġ�� ����   //ADAPTIVE_THRESH_GAUSSIAN_C ����ġ�� ��
	//	THRESH_BINARY, // ����ȭ Ÿ�� 
	//	blockSize, // �̿�ũ�� 
	//	threshold); // threshold used

	if (Reverse_Color == false) {		//Locally adaptive thresholding
		adaptiveThreshold(ori_image, ThreshImage, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, 11, 13);			//11�� ������ ������.
		//ShowImage(ThreshImage);
		//(ori_image, ThreshImage, 127, 255, THRESH_BINARY);
		//Save2png(ThreshImage, "adaptiveThreshold_13");
		return ThreshImage;
	}
	else 
		return ori_image;

	//fix_image = Gaussian_Blur(ThreshImage, 3, 3);
	//Save2png(fix_image, "Thresh_11_5_blur");
	//adaptiveThreshold(ori_image, ThreshImage, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, 21, 5);
	//fix_image = Gaussian_Blur(ThreshImage, 3, 3);
	//Save2png(fix_image, "Thresh_21_5_blur");
	//adaptiveThreshold(ori_image, ThreshImage, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, 31, 5);
	//fix_image = Gaussian_Blur(ThreshImage, 3, 3);
	//Save2png(fix_image, "Thresh_31_5_blur");

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
	
	if (ori_image.rows > 500 || ori_image.cols > 500)
		return ori_image;

	GaussianBlur(ori_image, Gasu_image, Size(3, 3), BORDER_DEFAULT);

	//�׽�Ʈ ��� ����þ� ���� ���� ���� �νķ��� ������ �������� ������ �ѱ� �ν��ϱ⿡�� ���� ������.

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

	//ShowImage(Gasu_image);
	//Save2png(Gasu_image, "Lastest_PNG");

	return Gasu_image;
}
