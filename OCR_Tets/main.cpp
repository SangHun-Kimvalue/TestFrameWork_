#include <Windows.h>
#include "parent.hpp"

typedef BaseAlertModule* (*TestClass)();

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow) {

	//HRESULT rs;

	AllocConsole();
	FILE* cp;
	freopen_s(&cp, "CONOUT$", "wt", stdout);

	TestClass fptr;
	HMODULE hDLL;

	hDLL = LoadLibrary(L"DllTest.dll");
	if (hDLL == NULL) {
		std::cerr << "FIle Not Found" << std::endl;
		return -1;
	}

	fptr = (TestClass)::GetProcAddress(hDLL, "_GetInstance");
	//DllClass* GetInstance = (DllClass*)GetProcAddress(hdll, "_GetInstance");
	if (fptr == NULL) {
		std::cerr << "File Not Found" << std::endl;
		FreeLibrary(hDLL);
		return -1;
	}

	BaseAlertModule* dllclass = fptr();

	dllclass->PrintModuleInfo();


	delete dllclass;


	system("pause");
	FreeLibrary(hDLL);
	FreeConsole();

	return 0;
}

//int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow) {
//
//	AllocConsole();
//
//	FILE* cp;
//	freopen_s(&cp, "CONOUT$", "wt", stdout);
//	HWND hwnd = GetDesktopWindow();
//
//
//	OCRClass *OCR = new OCRClass(hwnd);
//
//
//	system("pause");
//	FreeConsole();
//
//
//
//	delete OCR;
//	return 0;
//}
