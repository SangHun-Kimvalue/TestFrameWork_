#include "DllClass.h"
#include "msgcontrol.h"
#include <iostream>

using nlohmann::json;
using namespace ocrmodule;

DllClass::DllClass()
{
	m_hwnd = GetDesktopWindow();

	ModuleInfo info;

	RECT rect;
	//rect.left = 44;
	//rect.top = 0;
	//rect.right = rect.left + 150;
	//rect.bottom = rect.top + 40;

	//rect.left = 550;
	//rect.top = 380;
	//rect.right = rect.left + 500;
	//rect.bottom = rect.top + 700;

	rect.left = 300;
	rect.top = 230;
	rect.right = rect.left + 1200;
	rect.bottom = rect.top + 700;
	RECT* displayrect = &rect;

	//rect(44, 0, 150, 40);

	m_stopThreadAnalyze = false;

	InitModule(info, displayrect);

	while (!m_stopThreadAnalyze) {

		Capturer->GetScreen();					//30~33
		unsigned char* temp = (unsigned char*)Capturer->src;						//테스트용
		std::shared_ptr<unsigned char[]> img = std::shared_ptr<unsigned char[]>(temp);

		//clock_t start = clock();

		ProcessAnalyze(img);

		//clock_t end = clock();
		//std::cout << "ProcessAnalyze : " << end - start << std::endl;
	
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
	//	m_img_input.release();
	//	delete[] data;
	//}
	
	//_CrtDumpMemoryLeaks();

	delete ImageCV;
	delete Match;
	delete Capturer;
	delete Tesseract;
}

bool DllClass::InitModule(ModuleInfo info, RECT* displayrect) {

	//ocrmodule::ModuleConfig config = json::parse(info.moduleconfig.c_str());
	//ocrmodule::SenderConfig senderconfig = json::parse(info.senderconfig.c_str());

	this->m_DisplayWidth = displayrect->right - displayrect->left;
	this->m_DisplayHeight = displayrect->bottom - displayrect->top;

	//float startx = displayrect->left;								//(this->m_DisplayWidth / 100.f) * this->rect[0];			//모니터의 해상도에 맞게 재설정?
	//float starty = displayrect->top;								//(this->m_DisplayHeight / 100.f) * this->rect[1];
	//float fWidth = displayrect->right - displayrect->left;			//(this->m_DisplayWidth / 100.f) * this->rect[2];
	//float fHeight = displayrect->bottom - displayrect->top;			// (this->m_DisplayHeight / 100.f) * this->rect[3];
	
	float startx = 465;					//x좌표 시작 퍼센트
	float starty = 320;					//y좌표 시작 퍼센트
	float fWidth = 140;	
	float fHeight = 60;

	m_FocusArea.x = startx;
	m_FocusArea.y = starty;
	m_FocusArea.width = fWidth;
	m_FocusArea.height = fHeight;
	this->m_totalCount = m_FocusArea.width * m_FocusArea.height;

	bool res = false;
	formula = "EQUAL";
	Base_String = "bemwonyo";
	Base_Num = 5;
	moduletype = "STR";		//임시 타입 변수		//NUM or STR
	std::string NUMTYPE = "NUM";

	if (NUMTYPE != moduletype)
		Base_Num = 0;
	else
		Base_String = "";

	Capturer = new GDICaptureClass(m_hwnd);

	ImageCV = new ImageClass();
	ImageCV->ori_image;
	Tesseract = new TesseractClass();
	Match = new TextMatchClass(Base_String, Base_Num, formula);

	//oriwid, orihei, x, y, wid, hei(crop용 변수)
	//res = ImageCV->Init(Capturer->nWidth, Capturer->nHeight, 
	//	m_FocusArea.x, m_FocusArea.y,m_FocusArea.width, m_FocusArea.height);
  
	res = ImageCV->Init(Capturer->nWidth, Capturer->nHeight,
		displayrect->left, displayrect->top,displayrect->right - displayrect->left, displayrect->bottom - displayrect->top);
		//displayrect->left, displayrect->top,
		//displayrect->right - displayrect->left, displayrect->bottom - displayrect->top
	if(res)
		res = Tesseract->Init(Base_String, moduletype, Base_Num);

	String_Type_Num = (int)Tesseract->String_Type;

	//this->guid = info.guid;
	//this->name = info.name;
	//this->description = info.description;
	//this->createtime = info.createtime;
	//this->modifytime = info.modifytime;
	//this->enable = info.enable;
	//this->monitorinx = info.monitorinx;
	//this->sendertype = info.sendertype;
	//this->senderconfig = info.senderconfig;
	//this->url = senderconfig.url;
	//this->moduletype = info.moduletype;
	//this->rect = config.rect;
	//this->formula = config.formula;
	//this->threshold = config.threshold;

	return res;
}

void DllClass::PreImageProcess(int String_length, std::shared_ptr<unsigned char[]> img) {

	ImageCV->fix_image = ImageCV->Create_Mat(Capturer->nWidth, Capturer->nHeight, img.get());

	ImageCV->fix_image = ImageCV->Crop(ImageCV->fix_image);							//0ms
	//ImageCV->ShowImage(ImageCV->fix_image);
	ImageCV->fix_image = ImageCV->Resize(ImageCV->fix_image, String_length);		//1~2ms
	//ImageCV->ShowImage(ImageCV->fix_image);

	ImageCV->fix_image = ImageCV->GrayScale(ImageCV->fix_image);					//4~5ms
	//ImageCV->ShowImage(ImageCV->fix_image);
	ImageCV->fix_image = ImageCV->Thresholding(ImageCV->fix_image);					//1 ~ 6ms		테서렉에서 3ms정도 더 걸림
	//ImageCV->ShowImage(ImageCV->fix_image);

	//if (String_Type_Num != (int)KOR) {
	//ImageCV->fix_image = ImageCV->Thresholding(ImageCV->fix_image);				//2ms		테서렉에서 3ms정도 더 걸림
		//ImageCV->ShowImage(ImageCV->fix_image);
	//}
	//else {		//한글이면 블러만 함
	//	ImageCV->fix_image = ImageCV->Gaussian_Blur(ImageCV->fix_image);				//1~2ms
		//ImageCV->ShowImage(ImageCV->fix_image);
	//}

	Iinfo.data = ImageCV->fix_image.data;
	Iinfo.step = ImageCV->fix_image.step1();
	Iinfo.channel = ImageCV->fix_image.step.buf[1];

	return ;
}

double DllClass::ProcessAnalyze(std::shared_ptr<unsigned char[]> img) {

	//std::shared_ptr<unsigned char[]> temp = PreImageProcess((int)(Tesseract->String_Type), Tesseract->Base_length);

	PreImageProcess(Tesseract->Base_length, img);		// 5~6 ms
	//clock_t start = clock();
	std::string OutText = GetText(ImageCV->fix_image.cols, ImageCV->fix_image.rows, Iinfo.data, Iinfo.channel, Iinfo.step);

	bool Detect = CompareText(OutText);
	//clock_t end = clock();
	//std::cout << "GetText : " << end - start << std::endl;

	ImageCV->Release();

	if (Detect) {
		std::cout << "Detected! " <<Detect << std::endl;
		return 1;
	}
	else {
		std::cout << "Not Detected! " << Detect << std::endl;
		return 0;
	}
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

	ocrmodule::ModuleConfig config = json::parse(info.moduleconfig.c_str());
	ocrmodule::SenderConfig senderconfig = json::parse(info.senderconfig.c_str());

	if (this->enable)
		this->enable = false;

	bool isChangedArea = false;

	// 모듈 쓰레드 종료를 기다렸다가 종료되면 업데이트 한다.
	this->guid = info.guid;
	this->name = info.name;
	this->description = info.description;
	this->createtime = info.createtime;
	this->modifytime = info.modifytime;
	this->enable = info.enable;
	this->monitorinx = info.monitorinx;
	this->sendertype = info.sendertype;
	this->url = senderconfig.url;
	this->senderconfig = info.senderconfig;
	this->moduletype = info.moduletype;

	if (config.rect != this->rect)
		isChangedArea = true;

	if (isChangedArea) {
		float startx = (this->m_DisplayWidth / 100.f) * this->rect[0];
		float starty = (this->m_DisplayHeight / 100.f) * this->rect[1];
		float fWidth = (this->m_DisplayWidth / 100.f) * this->rect[2];
		float fHeight = (this->m_DisplayHeight / 100.f) * this->rect[3];
		m_FocusArea.x = startx;
		m_FocusArea.y = starty;
		m_FocusArea.width = fWidth;
		m_FocusArea.height = fHeight;
		this->m_totalCount = m_FocusArea.width * m_FocusArea.height;
	}

	this->rect = config.rect;
	this->formula = config.formula;
	this->threshold = config.threshold;

	m_moduleInfo = info;

	return true;
}

// for debug
void DllClass::PrintModuleInfo() {

	//std::cout << "MpduleInfo" << std::endl;

}

bool DllClass::GetModuleStatus() {
	return this->enable;
}

std::string DllClass::GetGUID() {
	return this->guid;
}

std::string DllClass::GetModuleName() {
	return this->name;
}

std::string DllClass::GetModuleDesc() {
	return this->description;
}

int DllClass::GetMonitorIndex() {

	return this->monitorinx;
}

float DllClass::GetThreshold() {

	return  Match->Base_Num;
}

std::string DllClass::GetFormula() {
	return Match->StringFomula;
}

std::string DllClass::GetModuleConfig()
{
	std::string moduleconfig = "";
	auto config = ocrmodule::SmartAlertModule();
	config.guid = m_moduleInfo.guid;
	config.name = m_moduleInfo.name;
	config.description = m_moduleInfo.description;
	config.createtime = m_moduleInfo.createtime;
	config.modifytime = m_moduleInfo.modifytime;
	config.enable = m_moduleInfo.enable;
	config.monitorinx = m_moduleInfo.monitorinx;
	config.sendertype = m_moduleInfo.sendertype;
	config.senderconfig = m_moduleInfo.senderconfig;
	config.moduletype = m_moduleInfo.moduletype;
	config.moduleconfig = m_moduleInfo.moduleconfig;

	json temp = config;
	moduleconfig = temp.dump();// (char*)temp.dump().c_str();
	return moduleconfig;
}

int DllClass::GetModuleConfig(ModuleSenderInfo &info) {

	info.type = this->m_moduleInfo.sendertype;
	info.url = this->url;
	return 0;

}

std::wstring DllClass::GetModuleType() {
	
	return L"ocr";
}

int DllClass::GetModuleSenderInfo(ModuleSenderInfo &info) {
	info.type = this->m_moduleInfo.sendertype;
	info.url = this->url;
	return 0;
}

// 업데이트 되거나 변경될때 강제 종료를 위함.
void DllClass::StopModule() {
	m_stopThreadAnalyze = true;
}

// 이벤트 발생 이미지를 저장한다.
int DllClass::SaveImage(std::string saveFoler, std::string timestamp) {
	
	char file[256];
	sprintf_s(file, "%s\\%s-%s.jpg", saveFoler.c_str(), this->m_moduleInfo.name.c_str(), timestamp.c_str());

	rectangle(m_img_input, Point2f(m_FocusArea.x, m_FocusArea.y), Point2f(m_FocusArea.x + m_FocusArea.width, m_FocusArea.y + m_FocusArea.height),
		Scalar(0, 255, 0), 5);

	imwrite(file, m_img_input);

	return 0;
}
