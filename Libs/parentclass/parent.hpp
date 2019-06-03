#pragma once


#include <Windows.h>
#include <iostream>
#include <memory>
#include <string>



enum ModuleType
{
	AREA, COLOR, OCR
};

struct ModuleInfo
{
	std::string guid;
	std::string name;
	std::string description;
	std::string createtime;
	std::string modifytime;
	bool enable;
	int monitorinx;
	std::string sendertype;
	std::string senderconfig;
	std::string moduletype;
	std::string moduleconfig;
};

struct ModuleSenderInfo
{
	std::string type;
	std::string url;
};


struct CapturedFrame
{
	unsigned char*	imgBuf;
	int				imgSize;
	std::string		time;
};

class __declspec(dllexport) BaseAlertModule{
public:
	virtual ~BaseAlertModule() {};

	virtual bool InitModule(ModuleInfo info, RECT* displayrect) = 0;
	virtual bool UpdateModule(ModuleInfo info) = 0;
	// for debug
	virtual void PrintModuleInfo() = 0;

	virtual bool GetModuleStatus() = 0;
	virtual std::string GetGUID() = 0;
	virtual std::string GetModuleName() = 0;
	virtual std::string GetModuleDesc() = 0;
	virtual int GetMonitorIndex() = 0;
	virtual float GetThreshold() = 0;
	virtual std::string GetFormula() = 0;
	virtual std::string GetModuleConfig() = 0;
	virtual std::wstring GetModuleType() = 0;
	virtual int GetModuleSenderInfo(ModuleSenderInfo &info) = 0;

	// 업데이트 되거나 변경될때 강제 종료를 위함.
	virtual void StopModule() = 0;
	virtual double ProcessAnalyze(std::shared_ptr<unsigned char[]> img) = 0;
	// 이벤트 발생 이미지를 저장한다.
	virtual int SaveImage(std::string saveFoler, std::string timestamp) = 0;
};
