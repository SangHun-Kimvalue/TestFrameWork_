
//#ifdef _DEBUG
//#define _CRTDBG_MAP_ALLOC  
//#include <stdlib.h>  
//#include <crtdbg.h> 
//
//#define DEBUG_NEW new(_CLIENT_BLOCK, __FILE__, __LINE__ )
//// Replace _NORMAL_BLOCK with _CLIENT_BLOCK if you want the
//// allocations to be of _CLIENT_BLOCK type
//#else
//#define DBG_NEW new
//#endif

#include <Windows.h>
#include "parent.hpp"

typedef BaseAlertModule* (*TestClass)();

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow) {

	//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetBreakAlloc(171);

	AllocConsole();
	HWND m_Console = GetConsoleWindow();
	SetWindowPos(m_Console, 0, 800, 400, 0, 0, SWP_NOSIZE);

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

	system("pause");

	delete dllclass;

	fclose(cp);
	FreeLibrary(hDLL);
	FreeConsole();
	DestroyWindow(m_Console);

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
