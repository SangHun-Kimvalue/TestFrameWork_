// MemoryDetect.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#pragma once

#define _CRTDBG_MAP_ALLOC  
#include <crtdbg.h> 
#include <stdlib.h> 
#include <Windows.h>

#ifdef _DEBUG
#define new new( _NORMAL_BLOCK , __FILE__ , __LINE__ )
// Replace _NORMAL_BLOCK with _CLIENT_BLOCK if you want the
// allocations to be of _CLIENT_BLOCK type
#else
#define DBG_NEW new
#endif

#include "DllClass.h"
//#include "parent.hpp"


//typedef BaseAlertModule* (*TestClass)();

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow) {

	//HRESULT rs;

	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetBreakAlloc(222);
	//{, , msvcr71d.dll}_CrtSetBreakAlloc(167);

	//AllocConsole();
	//HWND m_Console = GetConsoleWindow();
	//SetWindowPos(m_Console, 0, 800, 400, 0, 0, SWP_NOSIZE);
	//
	//FILE* cp;
	//freopen_s(&cp, "CONOUT$", "wt", stdout);


	DllClass * Detect = new DllClass;
	//int* temp = new int;

	//TestClass fptr;
	//HMODULE hDLL;

	//hDLL = LoadLibrary(L"DllTest.dll");
	//if (hDLL == NULL) {
	//	std::cerr << "FIle Not Found" << std::endl;
	//	return -1;
	//}

	//fptr = (TestClass)::GetProcAddress(hDLL, "_GetInstance");
	////DllClass* GetInstance = (DllClass*)GetProcAddress(hdll, "_GetInstance");
	//if (fptr == NULL) {
	//	std::cerr << "File Not Found" << std::endl;
	//	FreeLibrary(hDLL);
	//	return -1;
	//}

	//BaseAlertModule* dllclass = fptr();

	//system("pause");

	//delete dllclass;

	delete Detect;

	//fclose(cp);
	//FreeLibrary(hDLL);
	//FreeConsole();
	//DestroyWindow(m_Console);

	_CrtDumpMemoryLeaks();

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
