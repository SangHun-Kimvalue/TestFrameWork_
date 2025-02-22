#pragma once

#include "TesseractClass.h"
#include "ImageClass.h"
#include "TextMatchClass.h"

//#define _API __declspec(dllexport)

#ifdef _EXPORTS
#define _API __declspec(dllexport)
#else
#define _API __declspec(dllexport)
#endif

class DllClass : public BaseAlertModule
{
public:
	DllClass();
	~DllClass();

	virtual void print();

	bool InitModule(ModuleInfo info, RECT* displayrect);
	bool UpdateModule(ModuleInfo info);
	// for debug
	void PrintModuleInfo();

	bool GetModuleStatus();
	std::string GetGUID();
	std::string GetModuleName();
	std::string GetModuleDesc();
	int GetMonitorIndex();
	float GetThreshold();
	std::string GetFormula();
	std::string GetModuleConfig();
	std::wstring GetModuleType();
	int GetModuleSenderInfo(ModuleSenderInfo &info);

	// 업데이트 되거나 변경될때 강제 종료를 위함.
	void StopModule();
	double ProcessAnalyze(std::shared_ptr<unsigned char[]> img);
	// 이벤트 발생 이미지를 저장한다.
	int SaveImage(std::string saveFoler, std::string timestamp);

private:

	GDICaptureClass* Capturer;
	ImageClass* ImageCV;
	TesseractClass* Tesseract;
	TextMatchClass* Match;
	HWND m_hwnd;

	unsigned char* PreImageProcess(int String_Type, int String_length);
	std::string GetText(int wid, int hei, unsigned char* src);
	bool CompareText(std::string OutText, int Base_Num);

	std::string Base_String;
	std::string String_Type;
	std::string Formula;

	int Base_Num;
	int String_Type_Num;
	int Type;
	bool Consistent;

	std::shared_ptr<unsigned char[]> img;
	unsigned char* data;

	bool			m_stopThreadAnalyze;

};




