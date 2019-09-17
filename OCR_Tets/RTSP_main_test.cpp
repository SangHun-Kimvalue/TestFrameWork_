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
#include "RTSP_Manager_Interface.h"

typedef RTSP_Manager* (*TestClass)();

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow) {

	//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetBreakAlloc(323);
	//_CrtSetBreakAlloc(217);
	//_CrtSetBreakAlloc(189);
	//_CrtSetBreakAlloc(188);
	//_CrtSetBreakAlloc(179);

	AllocConsole();

	FILE* cp;
	freopen_s(&cp, "CONOUT$", "wt", stdout);

	RTSP_Manager* dllclass = Create_Manager();
	
	Initialize(dllclass);
	Run(dllclass);
	std::cout << Get_URL(dllclass) << std::endl;
	std::cout << Get_Stream_Name(dllclass) << std::endl;
	
	system("pause");

	DeleteManager(dllclass);
	delete dllclass;
	
	fclose(cp);
	FreeConsole();

	return 0;
}
