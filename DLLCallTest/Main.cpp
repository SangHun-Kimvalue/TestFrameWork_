#include <Windows.h>
#include <iostream>
#include <string>
#include <parent.hpp>

//class DllClass;

typedef parentclass* (*TestClass)();
TestClass pptr;

#define _API extern "C" __declspec(dllexport)


int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow) {

	AllocConsole();
	FILE* cp;
	freopen_s(&cp, "CONOUT$", "wt", stdout);
	
	HMODULE hdll = LoadLibrary(L"DllTest.dll");
	//HINSTANCE hdll = LoadLibrary(L"DllTest.dll");
	if (hdll == NULL) {
		std::cerr << "FIle Not Found" << std::endl;
		return -1;
	}

	pptr = (TestClass)::GetProcAddress(hdll, "_GetInstance");
	//DllClass* GetInstance = (DllClass*)GetProcAddress(hdll, "_GetInstance");
	if (pptr == NULL) {
		std::cerr << "File Not Found" << std::endl;
		FreeLibrary(hdll);
		return -1;
	}

	auto res2 = pptr();
	//auto res = GetInstance;
	//unsigned long res = pptr(1, 2);
	res2->print();
	res2->test();

	std::cout << res2 << std::endl;
	printf("%d", 10);
	getchar();
	Sleep(1000);

	//fibonacci_init();
	FreeLibrary(hdll);
	FreeConsole();
	//ptr = (unsigned long, unsigned long )GetProcAddress(hdll, "fibonacci_init");



	return 0;
}