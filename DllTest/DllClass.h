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

	// ������Ʈ �ǰų� ����ɶ� ���� ���Ḧ ����.
	virtual void StopModule();
	virtual double ProcessAnalyze(std::shared_ptr<unsigned char[]> img);
	// �̺�Ʈ �߻� �̹����� �����Ѵ�.
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



