#pragma once

#include "stdafx.h"
#include "TesseractClass.h"
#include "ImageClass.h"
#include "TextMatchClass.h"
#include <Windows.h>

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

	struct ImageInfo {
		size_t step;
		int channel;
		unsigned char* data;
	};

	ImageInfo Iinfo;

	GDICaptureClass* Capturer;
	ImageClass* ImageCV;
	TesseractClass* Tesseract;
	TextMatchClass* Match;
	HWND m_hwnd;

	void PreImageProcess(int String_length);
	std::string GetText(int wid, int hei, unsigned char* src, int chanel, size_t Image_step);
	bool CompareText(std::string OutText);

	std::string Base_String;
	std::string String_Type;
	std::string Formula;

	int Base_Num;
	int String_Type_Num;

	std::shared_ptr<unsigned char[]> img;

	bool			m_stopThreadAnalyze;

};

extern "C" _API DllClass* _GetInstance();



