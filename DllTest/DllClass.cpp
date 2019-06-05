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

	//rect.left = 44;
	//rect.top = 0;
	//rect.right = 150;
	//rect.bottom = 40;
	//RECT* displayrect = &rect;
	////rect(44, 0, 150, 40);

	rect.left = 213;
	rect.top = 880;
	rect.right = rect.left + 330;
	rect.bottom = rect.top + 95;
	//rect.right = 200;
	//rect.bottom = 200;
	//rect.right = 50;
	//rect.bottom = 200;
	RECT* displayrect = &rect;

	m_stopThreadAnalyze = false;

	InitModule(info, displayrect);

	ProcessAnalyze(img);


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
	//std::cout << "Call Instance delete" << std::endl;

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
	Base_String = "위험";
	Base_Num = 52;
	String_Type = "NUM";		//임시 타입 변수
	Type = 1;
	Consistent = true;

	Capturer = new GDICaptureClass(m_hwnd);

	unsigned char* temp = (unsigned char*)Capturer->src;						//테스트용
	img = std::shared_ptr<unsigned char[]>(temp);

	ImageCV = new ImageClass(4);
	Tesseract = new TesseractClass(Base_String);
	//Tesseract = new TesseractClass(Base_String, Capturer->nWidth, Capturer->nHeight, Capturer->src);

	Match = new TextMatchClass(Base_String, Type, Base_Num, Formula, Consistent);
	//(std::string find_string, int type, int Base_Num, std::string fomula, bool Consistent)

	ImageCV->CV_Init(Capturer->nWidth, Capturer->nHeight, displayrect->left, displayrect->top,
		displayrect->right - displayrect->left, displayrect->bottom - displayrect->top, img.get());
	Tesseract->Init(String_Type);


	String_Type_Num = (int)Tesseract->String_Type;
	//Tesseract->Test(ImageCV->c_wid, ImageCV->c_hei, ImageCV->src);
	//Match = new TextMatchClass("qwertyuiopasdfghjklzxcvbnm", "zxcv", (int)Tesseract->String_Type, 3, "EQUAL");

	return true;
}


size_t DllClass::PreImageProcess(int String_Type, int String_length) {

	ImageCV->fix_image = ImageCV->Crop(ImageCV->ori_image);

	ImageCV->fix_image = ImageCV->Resize(ImageCV->fix_image, String_Type, String_length);

	ImageCV->fix_image = ImageCV->GrayScale(ImageCV->fix_image);
	ImageCV->ShowImage(ImageCV->fix_image);

	ImageCV->fix_image = ImageCV->Gaussian_Blur(ImageCV->fix_image);
	
	//int temp2 = ImageCV->fix_image.channels();

	data = ImageCV->fix_image.data;

	return ImageCV->fix_image.step1();
}

double DllClass::ProcessAnalyze(std::shared_ptr<unsigned char[]> img) {

	//std::shared_ptr<unsigned char[]> temp = PreImageProcess((int)(Tesseract->String_Type), Tesseract->Base_length);
	
	size_t Image_step = PreImageProcess((int)(Tesseract->String_Type), Tesseract->Base_length);
	
	std::string OutText = GetText(ImageCV->c_wid, ImageCV->c_hei, data, Image_step);

	bool res = CompareText(OutText, Base_Num);

	std::cout << res << std::endl;

	ImageCV->Release();

	if (res)
		return 1;
	else
		return 0;
}

bool DllClass::CompareText(std::string OutText, int Base_Num) {

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
		res = Match->Find_Scope(Base_Num);
		break;
	default:
		std::cout << "Text_Match_Class : 알수없는 형태가 들어옴" << std::endl;
		break;
	}

	return res;
}

std::string DllClass::GetText(int wid, int hei, unsigned char* src, size_t Image_step) {

	return Tesseract->GetTextUTF8(wid, hei, src, Image_step);
}


void DllClass::print() {

	std::cout << "Print Call" << std::endl;

	return ;
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

	return 0;
}

std::string DllClass::GetFormula() {
	std::string temp = "";

	return temp;
}

std::string DllClass::GetModuleConfig() {
	std::string temp = "";

	return temp;
}

std::wstring DllClass::GetModuleType() {
	std::wstring temp = L"";

	return temp;
}

int DllClass::GetModuleSenderInfo(ModuleSenderInfo &info) {
	
	return 0;
}

// 업데이트 되거나 변경될때 강제 종료를 위함.
void DllClass::StopModule() {

}

// 이벤트 발생 이미지를 저장한다.
int DllClass::SaveImage(std::string saveFoler, std::string timestamp) {
	
	return 0;
}
