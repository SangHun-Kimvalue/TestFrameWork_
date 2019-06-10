#include "DllClass.h"
#include <iostream>


extern "C" {
	_API DllClass* _GetInstance() {
		return new DllClass;
	}
}

DllClass::DllClass()
{

	m_hwnd = GetDesktopWindow();

	ModuleInfo info;

	RECT rect;

	rect.left = 44;
	rect.top = 0;
	rect.right = 150;
	rect.bottom = 40;
	RECT* displayrect = &rect;
	//rect(44, 0, 150, 40);

	m_stopThreadAnalyze = false;

	InitModule(info, displayrect);

	while (1) {
		//clock_t start = clock();

		Capturer->GetScreen();					//30~33
		unsigned char* temp = (unsigned char*)Capturer->src;						//테스트용
		img = std::shared_ptr<unsigned char[]>(temp);

		clock_t start = clock();

		ProcessAnalyze(img);

		clock_t end = clock();
		std::cout << "ProcessAnalyze : " << end - start << std::endl;
		//clock_t end = clock();

		//std::cout << "ImageProcessTime : " << end - start << std::endl;

		Sleep(1);
	}

	/*std::string Test_String = "Improve";

	Capturer = new GDICaptureClass(m_hwnd);

	Tesseract = new TesseractClass(Test_String, "", Capturer->nWidth, Capturer->nHeight, Capturer->src);
	//Tesseract = new TesseractClass(USEFILE, Capturer->nWidth, Capturer->nHeight, Capturer->src, Test_String, ENG);
	//1 : from file // 0 : from memory  // non : not thing

	//ImageCV = new ImageClass();
	//int wid, int hei, BYTE* src, int* String_Type, int Base_length
	ImageCV = new ImageClass(Capturer->nWidth, Capturer->nHeight, Capturer->src, (int)Tesseract->String_Type, Tesseract->Base_length);
	//ImageCV->CV_Init(Capturer->nWidth, Capturer->nHeight, 44, 0, 150, 40);	

	Tesseract->Test(ImageCV->c_wid, ImageCV->c_hei, ImageCV->src);
	Tesseract->String_Type;

	//Match = new TextMatchClass("asdf", "df", (INTYPE)Tesseract->String_Type, 0);
	//std::string input_string, std::string find_string, int type, int threshold, std::string fomula
	
	Match = new TextMatchClass("qwertyuiopasdfghjklzxcvbnm", "zxcv", (int)Tesseract->String_Type, 3, "EQUAL");*/

}

DllClass::~DllClass()
{
	//if (data != nullptr)
	//{
	//	delete[] data;
	//}
	
	//_CrtDumpMemoryLeaks();

	delete Match;
	delete Capturer;
	delete Tesseract;
}

bool DllClass::InitModule(ModuleInfo info, RECT* displayrect) {

	Formula = "EQUAL";
	Base_String = "How";
	Base_Num = 50;
	String_Type = "STR";		//임시 타입 변수		//NUM or STR
	std::string NUMTYPE = "NUM";

	if (NUMTYPE != String_Type)
		Base_Num = 0;
	else
		Base_String = "";

	Capturer = new GDICaptureClass(m_hwnd);

	unsigned char* temp = (unsigned char*)Capturer->src;						//테스트용
	img = std::shared_ptr<unsigned char[]>(temp);

	ImageCV = new ImageClass();
	Tesseract = new TesseractClass();
	//Tesseract = new TesseractClass(Base_String, Capturer->nWidth, Capturer->nHeight, Capturer->src);
	
	Match = new TextMatchClass(Base_String, Base_Num, Formula);

	ImageCV->CV_Init(Capturer->nWidth, Capturer->nHeight, displayrect->left, displayrect->top,
		displayrect->right - displayrect->left, displayrect->bottom - displayrect->top);
	Tesseract->Init(Base_String, String_Type, Base_Num);

	//(std::string find_string, int Base_Num, std::string fomula)

	String_Type_Num = (int)Tesseract->String_Type;
	//Tesseract->Test(ImageCV->c_wid, ImageCV->c_hei, ImageCV->src);
	//Match = new TextMatchClass("qwertyuiopasdfghjklzxcvbnm", "zxcv", (int)Tesseract->String_Type, 3, "EQUAL");

	return true;
}

void DllClass::PreImageProcess(int String_Type, int String_length) {

	ImageCV->Create_Mat(Capturer->nWidth, Capturer->nHeight, img.get());

	ImageCV->fix_image = ImageCV->Crop(ImageCV->ori_image);					//0ms
	//ImageCV->ShowImage(ImageCV->fix_image);
	ImageCV->fix_image = ImageCV->Resize(ImageCV->fix_image, String_Type, String_length);		//1~2ms
	//ImageCV->ShowImage(ImageCV->fix_image);
	
	if (String_Type_Num != (int)KOR) {
		ImageCV->fix_image = ImageCV->GrayScale(ImageCV->fix_image);				//4~5ms
		//ImageCV->ShowImage(ImageCV->fix_image);
	}

	ImageCV->fix_image = ImageCV->Gaussian_Blur(ImageCV->fix_image);				//1~2ms
	//ImageCV->ShowImage(ImageCV->fix_image);

	Iinfo.data = ImageCV->fix_image.data;
	Iinfo.step = ImageCV->fix_image.step1();
	Iinfo.channel = ImageCV->fix_image.step.buf[1];

	return ;
}

double DllClass::ProcessAnalyze(std::shared_ptr<unsigned char[]> img) {

	//std::shared_ptr<unsigned char[]> temp = PreImageProcess((int)(Tesseract->String_Type), Tesseract->Base_length);
	//clock_t start = clock();
	PreImageProcess((int)(Tesseract->String_Type), Tesseract->Base_length);		// 5~6 ms
	
	std::string OutText = GetText(ImageCV->fix_image.cols, ImageCV->fix_image.rows, Iinfo.data, Iinfo.channel, Iinfo.step);

	bool Detect = CompareText(OutText);
	
	std::cout << Detect << std::endl;

	ImageCV->Release();
	//clock_t end = clock();
	//std::cout << "ImageProcessTime : " << end - start << std::endl;

	if (Detect)
		return 1;
	else
		return 0;
}

bool DllClass::CompareText(std::string OutText) {

	bool res = false;

	switch (String_Type_Num) 
	{
	case 1:		//ENG
		res = Match->Find_Base_String(OutText);
		break;
	case 2:		//KOR
		res = Match->Han_Delete(OutText);
		break;
	case 3:		//NUM
		res = Match->Find_Scope(OutText);
		break;
	default:
		std::cerr << "Text_Match_Class : 알수없는 형태가 들어옴" << std::endl;
		break;
	}

	return res;
}

std::string DllClass::GetText(int wid, int hei, unsigned char* src, int chanel, size_t Image_step) {

	return Tesseract->GetTextUTF8(wid, hei, src, chanel, Image_step);
}

bool DllClass::UpdateModule(ModuleInfo info) {
	return true;
}

// for debug
void DllClass::PrintModuleInfo() {

	//std::cout << "MpduleInfo" << std::endl;

}

bool DllClass::GetModuleStatus() {
	return true;
}

std::string DllClass::GetGUID() {

	std::string temp = "";

	return temp = "";
}

std::string DllClass::GetModuleName() {
	std::string temp;

	return temp;
}

std::string DllClass::GetModuleDesc() {
	std::string temp ="";

	return temp;
}

int DllClass::GetMonitorIndex() {

	return 0;
}

float DllClass::GetThreshold() {

	return  Match->Base_Num;
}

std::string DllClass::GetFormula() {

	return Match->StringFomula;
}

std::string DllClass::GetModuleConfig() {
	std::string temp = "";

	return temp;
}

std::wstring DllClass::GetModuleType() {
	
	std::wstring temp = L"OCR";

	return temp;
}

int DllClass::GetModuleSenderInfo(ModuleSenderInfo &info) {
	
	return 0;
}

// 업데이트 되거나 변경될때 강제 종료를 위함.
void DllClass::StopModule() {
	m_stopThreadAnalyze = true;
}

// 이벤트 발생 이미지를 저장한다.
int DllClass::SaveImage(std::string saveFoler, std::string timestamp) {
	


	return 0;
}
