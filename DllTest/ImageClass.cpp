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

	//base_height = 150;											// 사용자가 평균적으로 지정한 영역내에서 글자의 높이 비율을 50 ~ 70% 로 분포 되어 있고 
	//base_width = 200;												// 글자의 크기를 35와 비슷한 크기로 만들기 위해서는 영역 높이를 75로 변경해야함.
	best_font_size = 75;

	c_x = x; 	c_y = y;  	c_wid = wid; 	c_hei = hei;		//wid, hei 원본 사이즈 넘으면 안됨 주의.
												
	if (c_x + c_wid > ori_wid) {
		std::cerr << "넓이 오류" << std::endl;
		return false;
	}
	else if (c_y + c_hei > ori_hei) {
		std::cerr << "높이 오류" << std::endl;
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

//이미지를 보여주는 디버그용 
void ImageClass::ShowImage(Mat Image) {

	//imshow("sub", cropimage);				//Show image
	namedWindow("main", WINDOW_AUTOSIZE);					// Create a window
	//namedWindow("main", WINDOW_NORMAL);					// Create a window
	imshow("main", Image);				// Show our image inside the created window.
	waitKey(0);								// Wait for any keystroke in the window

	destroyWindow("main");					//destroy the created window
}

//String_Type이 Num일때 넓이를 중심으로 변환			//폐기
Mat ImageClass::Resize_Num(Mat ori_image) {

	Mat re_image;

	//CV_EXPORTS_W void resize(InputArray src, OutputArray dst,
	//	Size dsize, double fx = 0, double fy = 0,
	//	int interpolation = INTER_LINEAR);
	//cv::resize(fix_image, fix_image, cv::Size(wid, hei), 0, 0, INTER_LINEAR);
	//이미지를 축소하려면 일반적으로 INTER_AREA 보간으로 괜찮게 보이지만 이미지를 확대하려면 일반적으로 
	//INTER_LINEAR (가장 빠르지 만 적당히 괜찮아 보입니다) 또는 INTER_CUBIC (느림)로 가장 잘 보입니다.

	base_width = base_height;

	int base = fix_image.cols - base_width;

	float percent = ((float)fix_image.cols - (float)base) / (float)fix_image.cols;
	int temp_hei = fix_image.rows * percent;
	int temp_wid = fix_image.cols - base;

	if (base == 0) {					//절대적인 값 차이가 아니라 퍼센테이지로 높이와 넓이를 계산하면 좋을듯??
		return ori_image;
	}
	else if (base > 0) {				//축소
		
		//이미지가 기준치 보다 더 작아지게 안됨으로 기준 수치를 한번 더 더해 사용한다.
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
			//이미지 데시메이션 (보간법의 반대되는 유사용어 LowPass 필터도 사용됨)에 선호되는 플레그		//INTER_AREA >> Bilinear_Interpolation에 자세히
			cv::resize(ori_image, re_image, cv::Size(temp_wid, temp_hei), 0, 0, INTER_AREA);
			//c_wid = temp_wid;
			//c_hei = temp_hei;

			return re_image;
		}
	}
	else if (base < 0) {			//확대

		cv::resize(ori_image, re_image, cv::Size(temp_wid, temp_hei), 0, 0, INTER_LINEAR);			//양선형 보간법  >>  Bilinear_Interpolation 
		//c_wid = temp_wid;
		//c_hei = temp_hei;

		return re_image;
	}
	return re_image;
}

//String_Type이 문자일때 높이를 중심으로 변환		//폐기	//INTER_AREA >> Bilinear_Interpolation에 자세히			//양선형 보간법  >>  Bilinear_Interpolation 
Mat ImageClass::Resize_String(Mat ori_image, int String_length) {		//문장이 아닌 단어에 초점을 맞춤.	//버그의 여지가 쬐큼 있긴있음.		>> 모든 경우의 수를 생각해봐야함/
	
	Mat re_image;
	int temp_wid;
	int temp_hei;
	base_width = String_length * 37;		//어자피 베이스 문자의 길이보다는 크게 조절해야함(그 보다 작으면 추출이 힘듬) (37포인트가 제일 이상적인 길이)

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
	else if (base > 0) {				//축소
		//이미지가 기준치 보다 더 작아지게 안됨		//너무 줄이면 글씨가 아예 짜부됨.( 사용자가 넓은 영역 설정을 피해야함.)
		if (temp_hei > base_height && (temp_wid > base_width) && percent > 0.5f) {
			//temp_hei = temp_hei + base_height;
			//temp_wid = temp_wid + base_width;
			//이미지 데시메이션 (보간법의 반대되는 유사용어 LowPass 필터도 사용됨)에 선호되는 플레그		//INTER_AREA >> Bilinear_Interpolation에 자세히
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
	else if (base < 0) {			//확대

		cv::resize(ori_image, re_image, cv::Size(temp_wid, temp_hei), 0, 0, INTER_LINEAR);			//양선형 보간법  >>  Bilinear_Interpolation 

		return re_image;
	}

	return re_image;
}

//인식률이 너무 않좋아서 폐기.		윤곽선 추출
Mat ImageClass::C_Canny(Mat ori_image) {

	Mat canny_image;
	int lowThreshold = 50;
	int highThreshold = 150;

	//그레이, 가우시안, 소벨 커널로 수평 수직 엣지 검출, 엣지 양옆의 값으로 엣지 검출, 이진화   순서(캐니 엣지 디텍트 이론)
	//void cv::Canny(InputArray image, OutputArray edges, double threshold1, double threshold2, int apertureSize = 3, bool 	L2gradient = false)
	Canny(ori_image, canny_image, lowThreshold, highThreshold);			//낮은 임계값, 높은 임계값은 1 : 2 나 1 : 3 정도가 좋음.
	ShowImage(canny_image);

	return canny_image;
}

//이미지 자르기
Mat ImageClass::Crop(Mat ori_image) {

	Mat crop_image;
	Rect rect(c_x, c_y, c_wid, c_hei);							// x, y ,wid, hei

	(ori_image(rect)).copyTo(crop_image);						//rect 만큼 crop
	//fix_image = ori_image(rect);			//rect 만큼 crop

	return crop_image;
}

//이미지 사이즈 변경		//Base_String의 길이에 최적화 하여 변경함 	//INTER_AREA >> Bilinear_Interpolation에 자세히			//양선형 보간법  >>  Bilinear_Interpolation 
Mat ImageClass::Resize(Mat ori_image, int String_length) {
	
	Mat re_image;
	int wid = ori_image.cols;		int hei = ori_image.rows; 
	float temp_wid = 0;				float temp_hei = 0;
	int base = 0;					float percent = 0;

	if (String_length < 3)					//wid가 300정도는 되야 Tesseract 에서 최소한의 부하가 걸림
		String_length = 3;
	base_width = String_length * best_font_size;		//어자피 베이스 문자의 길이보다는 크게 조절해야함(그 보다 작으면 추출이 힘듬) (37포인트가 제일 이상적인 길이)

	base = wid - base_width;
	percent = ((float)wid - (float)base) / (float)wid;

	if(base >= 0){
	//if (base == 0)
	//		return ori_image;
	//else if (base > 0) {	//축소
		//temp_hei = hei * percent;
		//temp_wid = base_width;
		//if (temp_hei >= base_height) {
		//	//이미지 데시메이션 (보간법의 반대되는 유사용어 LowPass 필터도 사용됨)에 선호되는 플레그		//INTER_AREA >> Bilinear_Interpolation에 자세히
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
	else if (base < 0) {	//확대

		temp_hei = hei * percent;
		temp_wid = base_width;

		if (percent > 2.5) {
			percent = 2.5;
			temp_hei = hei * percent;
			temp_wid = wid * percent;
		}
		//clock_t start = clock();

		cv::resize(ori_image, re_image, cv::Size((int)temp_wid+0.5, (int)temp_hei+0.5), 0, 0, INTER_CUBIC);				//3차 다항식 입체 보간법		//0~1ms
		//cv::resize(ori_image, re_image, cv::Size(temp_wid, temp_hei), 0, 0, INTER_LINEAR);			//양선형 보간법  >>  Bilinear_Interpolation	//1~3ms
		//clock_t end = clock();

		//std::cout << "INTER_LINEAR : " << end - start << std::endl;
		//ShowImage(re_image);
		//Save2png(re_image, "INTER_CUBIC4");

		return re_image;
	}

	return ori_image;
}

//그레이 이미지 변경 >> BGRA 4채널에서 1채널로 변경되기 때문에 다시 4채널 BGRA로 변경 작업 포함.  >> 테세렉에서 1채널로 셋이미지로 변경함
Mat ImageClass::GrayScale(Mat ori_image) {

	Mat GrayImage;

	GrayImage = Mat(ori_image.size(), CV_8UC4);
	cvtColor(ori_image, GrayImage, COLOR_BGRA2GRAY);		//1~2ms
	//ShowImage(GrayImage);

	//GrayImage = C_Canny(GrayImage);
	//std::vector<std::vector<cv::Point>> contours;
	//cv::findContours(GrayImage,
	//	contours,    // 외곽선 벡터 
	//	RETR_EXTERNAL,  // 외부 외곽선 검색
	//	CHAIN_APPROX_NONE); // 각 외곽선의 모든 화소 탐색
	//ShowImage(GrayImage);

	//clock_t start = clock();
	Reverse_check(GrayImage);			// 1ms 이하

	//clock_t end = clock();
	//std::cout << "RecerseCheck : " << end - start << std::endl;

	//cvtColor(GrayImage, ori_image, COLOR_GRAY2BGRA);			//4채널로 보내면 테서렉이 3~4ms 더 걸림
	//Save2png(GrayImage, "Lastest_PNG");
	return GrayImage;//ThreshImage를 다시 4채널로 변경하는 과정에서 ori_image를 dst로 설정했음.
}

// 1채널 그레이 이미지만 매개변수로 받아들임
Mat ImageClass::Thresholding(Mat ori_image) {

	Mat ThreshImage;			//매개변수 src에 반드시 그레이 이미지가와야함 안그러면 터짐.

	//src	소스 8 비트 단일 채널 이미지.
	//DST	src와 같은 크기 및 동일한 유형의 대상 이미지.
	//maxValue	조건이 만족되는 픽셀에 할당 된 0이 아닌 값
	//적응 형 방법	적응 형 thresholding 알고리즘을 사용합니다.	//	ADAPTIVE_THRESH_MEAN_C / ADAPTIVE_THRESH_GAUSSIAN_C
	//임계 값 유형	thresholding 타입은 THRESH_BINARY 또는 THRESH_BINARY_INV가 아니면 안된다 .
	//blockSize	픽셀의 임계 값을 계산하는 데 사용되는 픽셀 인접 영역의 크기 : 3, 5, 7 등.
	//기음	상수는 평균 또는 가중 평균에서 뺍니다(아래 세부 정보 참조).일반적으로 양수이지만 0 또는 음수 일 수 있습니다.
	
	//adaptiveThreshold(image, // 입력영상 
	//	binaryAdaptive, // 이진화 결과 영상 
	//	255, // 최대 화소 값 
	//	ADAPTIVE_THRESH_MEAN_C, // Adaptive 함수		//가중치를 안줌   //ADAPTIVE_THRESH_GAUSSIAN_C 가중치를 줌
	//	THRESH_BINARY, // 이진화 타입 
	//	blockSize, // 이웃크기 
	//	threshold); // threshold used
	
	if (Reverse_Color == false) {		//Locally adaptive thresholding
		//clock_t start = clock();
		adaptiveThreshold(ori_image, ThreshImage, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, 11, 13);			//11이 적당해 보이임.
		fix_image = Gaussian_Blur(ThreshImage);				//1~2ms
		//ShowImage(ThreshImage);
		//clock_t end = clock();
		//std::cout << "adaptiveThreshold : " << end - start << std::endl;
		return ThreshImage;
	}
	else {
		//clock_t start = clock();					//오츠 속도가 월등히 빠름.
		//임의의 값으로 블랙이 될 픽셀의 값들의 평균을 구해 분산을 구하고 임시 임계값을 
		//다른 임의의 값에 대한 임계값과 비교해 분산이 더 적은 값을 임계값으로 설정해 이진화 
		//다른 클래스와의 분산은 크게, 클래스 내부의 분산은 최소가 되는 값으로.
		//두 클래스의 분산차이가 가장 큰 값을 설정.
		ThreshImage = Gaussian_Blur(ori_image);
		threshold(ThreshImage, ThreshImage, 0, 255, THRESH_OTSU);				
		//ShowImage(ThreshImage);
		//clock_t end = clock();
		//std::cout << "THRESH_OTSU : " << end - start << std::endl;
		return ThreshImage;
	}
		
	//fix_image = Gaussian_Blur(ThreshImage, 3, 3);
	//Save2png(fix_image, "Thresh_11_5_blur");
	//adaptiveThreshold(ori_image, ThreshImage, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, 21, 5);
	//fix_image = Gaussian_Blur(ThreshImage, 3, 3);
	//Save2png(fix_image, "Thresh_21_5_blur");
	//adaptiveThreshold(ori_image, ThreshImage, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, 31, 5);
	//fix_image = Gaussian_Blur(ThreshImage, 3, 3);
	//Save2png(fix_image, "Thresh_31_5_blur");

}

//설명용..   양선형 보간법이 이미 Resize에서 적용되어 있어 필요가 없어짐.
Mat ImageClass::Bilinear_Interpolation(Mat ori_image){
	

	//INTER_AREA에 대해.
	//INTER_AREA 플레그는 이미지 확대에 경우 INTER_LINEAR를 호출한다.(계수가 약간 다름)
	//축소의 경우 wid와 hei 이 2의 배수이고, 비트 채널수가 2가 아니면 INTER_AREA를 사용함. 그외의 경우에는 INTER_LINEAR_EXACT 이나 INTER_LINEAR를 사용
	//축소하는 경우에만 사용되고 영역의 모든 합의 평균과 비슷함.(커널에서 합하고 나눔 Not Mask)
	//https://medium.com/@wenrudong/what-is-opencvs-inter-area-actually-doing-282a626a09b3

	//값을 아는 두점 P,Q 사이의 값 R을 유츄할때, 세 점의 값이 선형적인 관계를 갖는다고 사정하고 P,Q의 값과 거리비를 이용해 R을 추정한다.
	//R = P * (dx) + Q * (1-dx)		을 이용하여 x 축과 y으로 두번 값을 유추해 면적에 따른 가중치를 적용한다.
	//R = A * (dx)(dy) + B * (1-dx)(dy) + C * (dx)(1-dy) + D * (1-dx)(1-dy)

	//cv::resize(fix_image, fix_image, cv::Size(c_wid - base, base_height), 0, 0, INTER_LINEAR); 에서 이미 적용 되어있음.


	return ori_image;

}

//가우시안 흐림 효과
Mat ImageClass::Gaussian_Blur(Mat ori_image) {			//시그마가 0이면 자동으로 계산됨(MASK 자동계산)

	Mat Gasu_image;
	//void GaussianBlur(InputArray src, OutputArray dst, double sigmaX, double sigmaY=0, int borderType=BORDER_DEFAULT )
	// GaussianBlur( src, dst, Size( i, i ), 0, 0 );				//거리에 따른 가중치 마스크값		, BorderType은 크게 영향이 없는듯함.
	
	if (ori_image.rows > 500 || ori_image.cols > 500)
		return ori_image;

	GaussianBlur(ori_image, Gasu_image, Size(3, 3), BORDER_DEFAULT);

	//테스트 결과 가우시안 블러가 들어가면 영문 인식률이 현저히 떨어져서 오히려 한글 인식하기에는 더욱 용이함.

	//엣지 보존 스무딩		거리 및 픽셀의 밝기차에 따른 가중치 마스크값
	//void bilateralFilter(InputArray src, OutputArray dst, int d, double sigmaColor, double sigmaSpace, int borderType=BORDER_DEFAULT )
	//src: 소스 이미지
	//dst: 목표 이미지, 즉 처리된 이미지를 담을 변수
	//d: Diameter of each pixel neighborhood that is used during filtering. 
	//sigmaColor: Filter sigma in the color space. A larger value of the parameter means that farther colors			//빅셀 밝기차 영역
	//			within the pixel neighborhood will be mixed together, resulting in larger areas of semi-equal color.
	//sigmaSpace: Filter sigma in the coordinate space. A larger value of the parameter means that					//거리차 영역
	//			farther pixels will influence each other as long as their colors are close enough. 
	//Mat Bilateral;
	//Bilateral.create(fix_image, CV_8UC3);
	//bilateralFilter(fix_image, Bilateral, 3, 15, 15);			//CV_8UC1 이나 3을 써야되는데 4를 쓰고있어서 쓰려면 변환이 필요.

	//ShowImage(Gasu_image);
	//Save2png(Gasu_image, "Lastest_PNG");

	return Gasu_image;
}
