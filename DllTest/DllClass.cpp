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
	
	std::string Test_String = "Improve";

	Capturer = new GDICaptureClass(m_hwnd);

	Tesseract = new TesseractClass(Test_String, TNULL, Capturer->nWidth, Capturer->nHeight, Capturer->src);
	//Tesseract = new TesseractClass(USEFILE, Capturer->nWidth, Capturer->nHeight, Capturer->src, Test_String, ENG);
	//1 : from file // 0 : from memory  // non : not thing

	//ImageCV = new ImageClass();
	ImageCV = new ImageClass(Capturer->nWidth, Capturer->nHeight, Capturer->src, Tesseract->String_Type, Tesseract->Base_length);
	//ImageCV->CV_Init(Capturer->nWidth, Capturer->nHeight, 44, 0, 150, 40);			//���� ���� �ٸ��� ����


	Tesseract->Test(ImageCV->c_wid, ImageCV->c_hei, ImageCV->src);
	Tesseract->String_Type;

	//Match = new TextMatchClass("asdf", "df", (INTYPE)Tesseract->String_Type, 0);
	//std::string input_string, std::string find_string, int type, int threshold, std::string fomula
	Match = new TextMatchClass("qwertyuiopasdfghjklzxcvbnm", "zxcv", 1, 3, "EQUAL");


}

DllClass::~DllClass()
{
	//std::cout << "Call Instance delete" << std::endl;
	delete Match;
	delete Capturer;
	delete Tesseract;
}

void DllClass::print() {

	std::cout << "Print Call" << std::endl;

	return ;
}

void DllClass::test() {

	return;
}


bool DllClass::InitModule(ModuleInfo info, RECT* displayrect) {

	//std::string Test_String = "Improve";

	//Capturer = new GDICaptureClass(m_hwnd);
	//
	//Tesseract = new TesseractClass(Test_String, TNULL, Capturer->nWidth, Capturer->nHeight, Capturer->src);
	////Tesseract = new TesseractClass(USEFILE, Capturer->nWidth, Capturer->nHeight, Capturer->src, Test_String, ENG);
	////1 : from file // 0 : from memory  // non : not thing
	//
	//ImageCV = new ImageClass();
	////ImageCV = new ImageClass(Capturer->nWidth, Capturer->nHeight, Capturer->src, Tesseract->String_Type, Tesseract->Base_length);
	//ImageCV->CV_Init(Capturer->nWidth, Capturer->nHeight, 44, 0, 150, 40);			//���� ���� �ٸ��� ����
	//
	//
	//Tesseract->Test(ImageCV->c_wid, ImageCV->c_hei, ImageCV->src);
	//Tesseract->String_Type;
	//
	//Match = new TextMatchClass("asdf", (INTYPE)Tesseract->String_Type);

	return true;
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

// ������Ʈ �ǰų� ����ɶ� ���� ���Ḧ ����.
void DllClass::StopModule() {

}

double DllClass::ProcessAnalyze(std::shared_ptr<unsigned char[]> img) {
	
	//ImageCV->Process(1);
	//Tesseract->Test(ImageCV->c_wid, ImageCV->c_hei, ImageCV->src);
	//
	//
	//if (Tesseract->String_Type) {
	//
	//
	//}
	//


	return 0;
}

// �̺�Ʈ �߻� �̹����� �����Ѵ�.
int DllClass::SaveImage(std::string saveFoler, std::string timestamp) {
	
	return 0;
}
