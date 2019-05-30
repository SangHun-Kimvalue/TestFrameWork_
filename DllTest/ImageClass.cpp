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

	base_height = 75;				// 사용자가 평균적으로 지정한 영역내에서 글자의 높이 비율을 50 ~ 70% 로 분포 되어 있고 
									// 글자의 크기를 35와 비슷한 크기로 만들기 위해서는 영역 높이를 75로 변경해야함.
	base_width = 100;

	// 1. 높이와 너비가 같지 않을 때 그림이 겹침(linesize 가 문제 인듯)
	// 2. 그레이 이미지만 하면 테서렉에서 인식을 못함	>> 채널수 이상한듯(1개가 정상)(BGRA >> GRAY)(채널)		//tesseract 에서 1채널로 설정해줄수없나?
	// 2.2 그레이 이미지까지는 패스  >> 스레쉬 홀드에서 1채널 이미지를 다시 4채널로 만들어줘야함.

	ori_image = CV_Ini_t(44, 0, 150, 40);			//높이 넓이 다르면 깨짐
	ShowImage(ori_image);
	//bitwise_not(ori_image, ori_image);			//픽셀 반전

	fix_image = Refactoring(ori_image);	

	fix_image = GrayScale(fix_image);				//그레이 이미지하면 src가 이상해짐.(pixel 채널때문인듯.)  해결
	
	fix_image = Thresholding(fix_image);
	ShowImage(fix_image);
	
	fix_image = Gaussian_Blur(fix_image, 3, 3);
	ShowImage(fix_image);

	//Canny(fix_image, fix_image, 50, 200);           // 외곽선 추출도 해보고.

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

	//ori_image = imread(Ipath.c_str(), IMREAD_COLOR);			//BGR로 들어옴(파일 로드)
	
	return ori_image;
}

Mat ImageClass::CV_Init() {

	c_x = 0; 	c_y = 0;  	c_wid = 500; 	c_hei = 500;		//wid, hei 원본 사이즈 넘으면 안됨 주의.

	if (c_x + c_wid > nWidth) 
		std::cerr << "넓이 오류" << std::endl;
	else if(c_y + c_hei > nHeight)
		std::cerr << "높이 오류" << std::endl;

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

	//std::cout << "현재 길이 및 높이" << c_wid << " " << c_hei << std::endl;

	//CV_EXPORTS_W void resize(InputArray src, OutputArray dst,
	//	Size dsize, double fx = 0, double fy = 0,
	//	int interpolation = INTER_LINEAR);
	//cv::resize(fix_image, fix_image, cv::Size(wid, hei), 0, 0, INTER_LINEAR);
	//이미지를 축소하려면 일반적으로 INTER_AREA 보간으로 괜찮게 보이지만 이미지를 확대하려면 일반적으로 
	//INTER_LINEAR (가장 빠르지 만 적당히 괜찮아 보입니다) 또는 INTER_CUBIC (느림)로 가장 잘 보입니다.

	base_width = base_height;

	int base = c_wid - base_width;

	float percent = (float)c_wid - (float)base / (float)c_wid;
	int temp_hei = c_hei * percent;
	int temp_wid = c_wid - base;

	if (base == 0) {					//절대적인 값 차이가 아니라 퍼센테이지로 높이와 넓이를 계산하면 좋을듯??
		return fix_image;
	}
	else if (base > 0) {				//축소
		
		//이미지가 기준치 보다 더 작아지게 안됨으로 기준 수치를 한번 더 더해 사용한다.
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
			//이미지 데시메이션 (보간법의 반대되는 유사용어 LowPass 필터도 사용됨)에 선호되는 플레그		//INTER_AREA >> Bilinear_Interpolation에 자세히
			cv::resize(fix_image, re_image, cv::Size(temp_wid, temp_hei), 0, 0, INTER_AREA);
			c_wid = temp_wid;
			c_hei = temp_hei;

			return re_image;
		}
	}
	else if (base < 0) {			//확대

		cv::resize(fix_image, re_image, cv::Size(temp_wid, temp_hei), 0, 0, INTER_LINEAR);			//양선형 보간법  >>  Bilinear_Interpolation 
		c_wid = temp_wid;
		c_hei = temp_hei;

		return re_image;
	}
	return re_image;
}

Mat ImageClass::Resize_String(Mat ori_image) {
	
	Mat re_image;

	//std::cout << "현재 길이 및 높이" << c_wid << " " << c_hei << std::endl;

	//CV_EXPORTS_W void resize(InputArray src, OutputArray dst,
	//	Size dsize, double fx = 0, double fy = 0,
	//	int interpolation = INTER_LINEAR);
	//cv::resize(fix_image, fix_image, cv::Size(wid, hei), 0, 0, INTER_LINEAR);
	//이미지를 축소하려면 일반적으로 INTER_AREA 보간으로 괜찮게 보이지만 이미지를 확대하려면 일반적으로 
	//INTER_LINEAR (가장 빠르지 만 적당히 괜찮아 보입니다) 또는 INTER_CUBIC (느림)로 가장 잘 보입니다.
	
	int base = c_hei - base_height;
	float percent = ((float)c_hei - (float)base) / (float)c_hei;

	int temp_wid = c_wid * percent;
	int temp_hei = c_hei - base;

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
			c_wid = temp_wid;
			c_hei = temp_hei;
			return re_image;
		}
		else {

			//이미지 데시메이션 (보간법의 반대되는 유사용어 LowPass 필터도 사용됨)에 선호되는 플레그		//INTER_AREA >> Bilinear_Interpolation에 자세히
			cv::resize(ori_image, re_image, cv::Size(temp_wid, temp_hei), 0, 0, INTER_AREA);
			c_wid = temp_wid;
			c_hei = temp_hei;

			return re_image;
		}
	}
	else if (base < 0) {			//확대

		cv::resize(ori_image, re_image, cv::Size(temp_wid, temp_hei), 0, 0, INTER_LINEAR);			//양선형 보간법  >>  Bilinear_Interpolation 
		c_wid = temp_wid;
		c_hei = temp_hei;

		return re_image;
	}

	/*else if (base > 0) {				//축소
		if (c_wid <= base) {
			//이미지가 좌우로 작아지게 되면 더이상 검출이 불가능하다고 판단해 그대로 사용한다.
			return fix_image;
		}
		else if (c_wid > base) {
			if(c_wid - base < c_wid)
				c_wid = c_wid - base;
			cv::resize(fix_image, re_image, cv::Size(c_wid, base_height), 0, 0, INTER_AREA);		//이미지 데시메이션 (보간법의 반대되는 유사용어 LowPass 필터도 사용됨)에 선호되는 플레그	
			// 높이와 최적 높이의 차이만큼 넓이도 같이 줄임.
			return re_image;
		}
	}

	else if (base < 0) {			//확대
		cv::resize(fix_image, re_image, cv::Size(c_wid + base, c_hei + base), 0, 0, INTER_LINEAR);			//양선형 보간법  >>  Bilinear_Interpolation 
		c_wid = c_wid + base;
		c_hei = c_hei + base;
		return re_image;
	}*/

	return re_image;
}

Mat ImageClass::Refactoring(Mat ori_image) {

	Rect rect(c_x, c_y, c_wid, c_hei);							// x, y ,wid, hei

	(ori_image(rect)).copyTo(fix_image);						//rect 만큼 crop
	//fix_image = ori_image(rect);			//rect 만큼 crop

	if(String_Type == 4)
		fix_image = Resize_Num(fix_image);
	else 
		fix_image = Resize_String(fix_image);

	//std::cout << "현재 길이 및 높이" << c_wid - base << " " << base_height << std::endl;


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

	//src	소스 8 비트 단일 채널 이미지.
	//DST	src와 같은 크기 및 동일한 유형의 대상 이미지.
	//maxValue	조건이 만족되는 픽셀에 할당 된 0이 아닌 값
	//적응 형 방법	적응 형 thresholding 알고리즘을 사용합니다.	//	ADAPTIVE_THRESH_MEAN_C / ADAPTIVE_THRESH_GAUSSIAN_C
	//임계 값 유형	thresholding 타입은 THRESH_BINARY 또는 THRESH_BINARY_INV가 아니면 안된다 .
	//blockSize	픽셀의 임계 값을 계산하는 데 사용되는 픽셀 인접 영역의 크기 : 3, 5, 7 등.
	//기음	상수는 평균 또는 가중 평균에서 뺍니다(아래 세부 정보 참조).일반적으로 양수이지만 0 또는 음수 일 수 있습니다.

	//threshold(fix_image, ThreshImage, 127, 255, THRESH_BINARY);
	adaptiveThreshold(fix_image, ThreshImage, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, 5, 10);
	cvtColor(ThreshImage, fix_image, COLOR_GRAY2BGRA);


	return ThreshImage;
}

Mat ImageClass::Bilinear_Interpolation(Mat fix_image){
	

	//INTER_AREA에 대해.
	//INTER_AREA 플레그는 이미지 확대에 경우 INTER_LINEAR를 호출한다.(계수가 약간 다름)
	//축소의 경우 wid와 hei 이 2의 배수이고, 비트 채널수가 2가 아니면 INTER_AREA를 사용함. 그외의 경우에는 INTER_LINEAR_EXACT 이나 INTER_LINEAR를 사용
	//축소하는 경우에만 사용되고 영역의 모든 합의 평균과 비슷함.(커널에서 합하고 나눔 Not Mask)
	//https://medium.com/@wenrudong/what-is-opencvs-inter-area-actually-doing-282a626a09b3

	//값을 아는 두점 P,Q 사이의 값 R을 유츄할때, 세 점의 값이 선형적인 관계를 갖는다고 사정하고 P,Q의 값과 거리비를 이용해 R을 추정한다.
	//R = P * (dx) + Q * (1-dx)		을 이용하여 x 축과 y으로 두번 값을 유추해 면적에 따른 가중치를 적용한다.
	//R = A * (dx)(dy) + B * (1-dx)(dy) + C * (dx)(1-dy) + D * (1-dx)(1-dy)

	//cv::resize(fix_image, fix_image, cv::Size(c_wid - base, base_height), 0, 0, INTER_LINEAR); 에서 이미 적용 되어있음.


	return fix_image;

}

Mat ImageClass::Gaussian_Blur(Mat fix_image, int sigmaX, int sigmaY) {			//시그마가 0이면 자동으로 계산됨(MASK 자동계산)

	Mat Gasu_image;

	//void GaussianBlur(InputArray src, OutputArray dst, Size ksize, double sigmaX, double sigmaY=0, int borderType=BORDER_DEFAULT )
	// GaussianBlur( src, dst, Size( i, i ), 0, 0 );				//거리에 따른 가중치 마스크값
	GaussianBlur(fix_image, Gasu_image, Size(sigmaX, sigmaX), 1.5);
	
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

	return Gasu_image;
}

BYTE* ImageClass::Mat2Byte(Mat input_image, int index, int depth) {				//수정

	int size = input_image.rows * input_image.cols;
	
	//std::memcpy(src, fix_image.data, size * sizeof(BYTE));
	//imencode(".png", fix_image, src, src);

	this->src = (BYTE*)input_image.data;
	//RGBSaveBMP(this->src, input_image.rows, input_image.cols, index, depth);

	return src;
}

BYTE* ImageClass::Mat2Byte(Mat input_image, int index, int depth, int save) {				//수정

	int size = input_image.rows * input_image.cols;

	//std::memcpy(src, fix_image.data, size * sizeof(BYTE));
	//imencode(".png", fix_image, src, src);

	this->src = (BYTE*)input_image.data;
	RGBSaveBMP(this->src, 1000, 1000, index, depth);

	return src;
}