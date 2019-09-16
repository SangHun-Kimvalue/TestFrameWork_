
#include <Windows.h>
#include "RTSP_Manager_Interface.h"

typedef RTSP_Manager* (*TestClass)();

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow) {

	//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetBreakAlloc(171);

	AllocConsole();
	HWND m_Console = GetConsoleWindow();
	//SetWindowPos(m_Console, 0, 800, 400, 0, 0, SWP_NOSIZE);

	FILE* cp;
	freopen_s(&cp, "CONOUT$", "wt", stdout);

	TestClass fptr;
	HMODULE hDLL;

	//hDLL = LoadLibrary(L"RTSP_Manager.dll");
	////hDLL = LoadLibrary("DllTest.dll");
	//if (hDLL == NULL) {
	//	std::cerr << "FIle Not Found" << std::endl;
	//	return -1;
	//}
	//
	//fptr = (TestClass)::GetProcAddress(hDLL, "GetInstance");
	////DllClass* GetInstance = (DllClass*)GetProcAddress(hdll, "_GetInstance");
	//if (fptr == NULL) {
	//	std::cerr << "File Not Found" << std::endl;
	//	FreeLibrary(hDLL);
	//	return -1;
	//
	//RTSP_Manager* dllclass = fptr();

	RTSP_Manager* dllclass = Create_Manager();
	
	Initialize(dllclass);
	Run(dllclass);
	//std::cout << Get_URL(dllclass) << std::endl;
	//std::cout << Get_Stream_Name(dllclass) << std::endl;
	
	system("pause");

	DeleteManager(dllclass);
	delete dllclass;

	fclose(cp);
	//FreeLibrary(hDLL);
	FreeConsole();
	//DestroyWindow(m_Console);

	return 0;
}
