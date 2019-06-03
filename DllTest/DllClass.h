#pragma once

#include "stdafx.h"
#include "TesseractClass.h"
#include "ImageClass.h"
#include "TextMatchClass.h"

//#define _API __declspec(dllexport)

#ifdef _EXPORTS
#define _API __declspec(dllexport)
#else
#define _API __declspec(dllexport)
#endif

enum { USEMEMORY, USEFILE };

class DllClass : public BaseAlertModule
{
public:
	DllClass();
	~DllClass();

	virtual void print();
	virtual void test();

	virtual bool InitModule(ModuleInfo info, RECT* displayrect);
	virtual bool UpdateModule(ModuleInfo info);
	// for debug
	virtual void PrintModuleInfo();

	virtual bool GetModuleStatus();
	virtual std::string GetGUID();
	virtual std::string GetModuleName();
	virtual std::string GetModuleDesc();
	virtual int GetMonitorIndex();
	virtual float GetThreshold();
	virtual std::string GetFormula();
	virtual std::string GetModuleConfig();
	virtual std::wstring GetModuleType();
	virtual int GetModuleSenderInfo(ModuleSenderInfo &info);

	// 업데이트 되거나 변경될때 강제 종료를 위함.
	virtual void StopModule();
	virtual double ProcessAnalyze(std::shared_ptr<unsigned char[]> img);
	// 이벤트 발생 이미지를 저장한다.
	virtual int SaveImage(std::string saveFoler, std::string timestamp);

private:

	GDICaptureClass* Capturer;
	ImageClass* ImageCV;
	TesseractClass* Tesseract;
	TextMatchClass* Match;
	HWND m_hwnd;

	std::string Base_String;
	int Base_Num;
	std::string String_Type;

};

extern "C" _API DllClass* _GetInstance();



