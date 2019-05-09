#include "DllClass.h"
#include <iostream>


extern "C" {
	_API DllClass* _GetInstance() {
		return new DllClass;
	}
}

DllClass::DllClass()
{
	std::cout << "Call Instance Seuccess" << std::endl;
	//print();
	
	//this->~DllClass();
}

DllClass::~DllClass()
{
	std::cout << "Call Instance delete" << std::endl;
}

void DllClass::print() {

	std::cout << "Print Call" << std::endl;

	return ;
}

void DllClass::test() {

	return;
}


bool DllClass::InitModule(ModuleInfo info, RECT* displayrect) {

	return true;
}

bool DllClass::UpdateModule(ModuleInfo info) {
	return true;
}

// for debug
void DllClass::PrintModuleInfo() {

	std::cout << "MpduleInfo" << std::endl;

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

double DllClass::ProcessAnalyze(std::shared_ptr<unsigned char[]> img) {
	
	return 0;
}

// 이벤트 발생 이미지를 저장한다.
int DllClass::SaveImage(std::string saveFoler, std::string timestamp) {
	
	return 0;
}
