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

	c_x = 0; 	c_y = 0;  	c_wid = 100; 	c_hei = 400;		//wid, hei 원본 사이즈 넘으면 안됨 주의.

	if (c_x + c_wid > nWidth) 
		std::cerr << "넓이 오류" << std::endl;
	else if(c_y + c_hei > nHeight)
		std::cerr << "높이 오류" << std::endl;

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
	else if (base > 0) {			//축소
		if (c_wid < base) {
			//이미지가 좌우로 작아지게 되면 더이상 검출이 불가능하다고 판단해 그대로 사용한다.
			return fix_image;
		}
		else if (c_wid > base) {
			cv::resize(fix_image, re_image, cv::Size(c_wid - base, base_height), 0, 0, INTER_AREA);		//이미지 데시메이션 (보간법의 반대 LowPass 필터도 사용됨)에 선호되는 플레그	
			// 높이와 최적 높이의 차이만큼 넓이도 같이 줄임.
			return re_image;
		}
	}
	else if (base < 0) {			//확대
		cv::resize(fix_image, re_image, cv::Size(c_wid - base, base_height), 0, 0, INTER_LINEAR);			//양선형 보간법  >>  Bilinear_Interpolation 
		return re_image;
	}

	return re_image;
}

Mat ImageClass::Refactoring(Mat fix_image) {


	Rect rect(c_x, c_y, c_wid, c_hei);		// x,y ,wid, hei

	fix_image = ori_image(rect);			//rect 만큼 crop
	ShowImage(fix_image);

	
	fix_image = Resize(fix_image);
	ShowImage(fix_image);

	//std::cout << "현재 길이 및 높이" << c_wid - base << " " << base_height << std::endl;

	//보간 추가

	fix_image = Gaussian_Blur(fix_image, 3, 3);
	ShowImage(fix_image);

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

