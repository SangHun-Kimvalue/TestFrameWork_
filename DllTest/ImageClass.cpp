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

	base_height = 75;				// 사용자가 평균적으로 지정한 영역내에서 글자의 높이 비율을 50 ~ 70% 로 분포 되어 있고 
									// 글자의 크기를 35와 비슷한 크기로 만들기 위해서는 영역 높이를 75로 변경해야함.
	base_width = 100;

	
	ori_image = CV_Init();
	//bitwise_not(ori_image, ori_image);

	ShowImage(ori_image, c_wid, c_hei);

	fix_image = ori_image.clone();
	//fix_image = Refactoring(ori_image);
	
	Rect rect(c_x, c_y, c_wid, c_hei);		// x,y ,wid, hei

	//fix_image

	(ori_image(rect)).copyTo(fix_image);			//rect 만큼 crop
	ShowImage(fix_image, c_wid, c_hei);

	//fix_image = GrayScale(ori_image);			//그레이 이미지하면 src가 이상해짐.(pixel 채널때문인듯.)
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

	c_x = 0; 	c_y = 0;  	c_wid = 400; 	c_hei = 400;		//wid, hei 원본 사이즈 넘으면 안됨 주의.

	if (c_x + c_wid > nWidth) 
		std::cerr << "넓이 오류" << std::endl;
	else if(c_y + c_hei > nHeight)
		std::cerr << "높이 오류" << std::endl;

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

	//std::cout << "현재 길이 및 높이" << c_wid << " " << c_hei << std::endl;

	//CV_EXPORTS_W void resize(InputArray src, OutputArray dst,
	//	Size dsize, double fx = 0, double fy = 0,
	//	int interpolation = INTER_LINEAR);
	//cv::resize(fix_image, fix_image, cv::Size(wid, hei), 0, 0, INTER_LINEAR);
	//이미지를 축소하려면 일반적으로 INTER_AREA 보간으로 괜찮게 보이지만 이미지를 확대하려면 일반적으로 
	//INTER_LINEAR (가장 빠르지 만 적당히 괜찮아 보입니다) 또는 INTER_CUBIC (느림)로 가장 잘 보입니다.

	base_width = base_height;

	int base = c_wid - base_width;

	if (base == 0) {					//절대적인 값 차이가 아니라 퍼센테이지로 높이와 넓이를 계산하면 좋을듯??
		return fix_image;
	}
	else if (base > 0) {				//축소
		
		float percent = (float)c_wid - (float)base / (float)c_wid;
		int temp_hei = c_hei * percent;
		int temp_wid = c_wid - base;
		
		//이미지가 기준치 보다 더 작아지게 되면 더이상 검출이 불가능하다고 판단해 그대로 사용한다.
		if ((temp_hei < base_height) || (temp_wid < base_width))
			return fix_image;

		else {
			//이미지 데시메이션 (보간법의 반대되는 유사용어 LowPass 필터도 사용됨)에 선호되는 플레그		//INTER_AREA >> Bilinear_Interpolation에 자세히
			cv::resize(fix_image, re_image, cv::Size(temp_wid, temp_hei), 0, 0, INTER_AREA);
			c_wid = temp_wid;
			c_hei = temp_hei;

			return re_image;
		}
	}
	else if (base < 0) {			//확대

		float percent = (float)c_wid - (float)base / (float)c_wid;
		int temp_hei = c_hei * percent;
		int temp_wid = c_wid - base;

		cv::resize(fix_image, re_image, cv::Size(temp_wid, temp_hei), 0, 0, INTER_LINEAR);			//양선형 보간법  >>  Bilinear_Interpolation 
		c_wid = temp_wid;
		c_hei = temp_hei;

		return re_image;
	}
	return re_image;
}

Mat ImageClass::Resize_String(Mat fix_image) {
	
	Mat re_image;

	//std::cout << "현재 길이 및 높이" << c_wid << " " << c_hei << std::endl;

	//CV_EXPORTS_W void resize(InputArray src, OutputArray dst,
	//	Size dsize, double fx = 0, double fy = 0,
	//	int interpolation = INTER_LINEAR);
	//cv::resize(fix_image, fix_image, cv::Size(wid, hei), 0, 0, INTER_LINEAR);
	//이미지를 축소하려면 일반적으로 INTER_AREA 보간으로 괜찮게 보이지만 이미지를 확대하려면 일반적으로 
	//INTER_LINEAR (가장 빠르지 만 적당히 괜찮아 보입니다) 또는 INTER_CUBIC (느림)로 가장 잘 보입니다.
	
	int base = c_hei - base_height;

	if (base == 0) {					//절대적인 값 차이가 아니라 퍼센테이지로 높이와 넓이를 계산하면 좋을듯??
		return fix_image;
	}
	else if (base > 0) {				//축소
		//이미지가 기준치 보다 더 작아지게 되면 더이상 검출이 불가능하다고 판단해 그대로 사용한다.
		if (c_hei - base < base_height || (c_wid - base < base_width))	
			return fix_image;
		else {
			float percent = (float)c_hei - (float)base / (float)c_hei;
			int temp_wid = c_wid * percent;
			int temp_hei = c_hei - base;

			//이미지 데시메이션 (보간법의 반대되는 유사용어 LowPass 필터도 사용됨)에 선호되는 플레그		//INTER_AREA >> Bilinear_Interpolation에 자세히
			cv::resize(fix_image, re_image, cv::Size(temp_wid, temp_hei), 0, 0, INTER_AREA);
			c_wid = temp_wid;
			c_hei = temp_hei;

			return re_image;
		}
	}
	else if (base < 0) {			//확대

		float percent = ((float)c_hei - (float)base) / (float)c_hei;

		int temp_wid = c_wid * percent;
		int temp_hei = c_hei - base;

		cv::resize(fix_image, re_image, cv::Size(temp_wid, temp_hei), 0, 0, INTER_LINEAR);			//양선형 보간법  >>  Bilinear_Interpolation 
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


	Rect rect(c_x, c_y, c_wid, c_hei);		// x, y ,wid, hei

	fix_image = ori_image(rect);			//rect 만큼 crop
	ShowImage(fix_image, c_wid, c_hei);

	if(String_Type == 4)
		fix_image = Resize_Num(fix_image);
	else 
		fix_image = Resize_String(fix_image);
	ShowImage(fix_image, c_wid, c_hei);

	//std::cout << "현재 길이 및 높이" << c_wid - base << " " << base_height << std::endl;

	//보간 추가

	//fix_image = Gaussian_Blur(fix_image, 3, 3);

	//Canny(fix_image, fix_image, 50, 200);           // 외곽선 추출도 해보고.

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

BYTE* ImageClass::Mat2Byte(Mat fix_image) {				//수정

	int size = fix_image.rows * fix_image.cols;

	src = /*(BYTE*)*/fix_image.data;

	//std::memcpy(src, fix_image.data, size * sizeof(BYTE));

	//imencode(".png", fix_image, src, src);


	return src;
}
