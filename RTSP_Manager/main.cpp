#include <Windows.h>
#include "RTSP_Manager.h"
#include "RTSP_Manager_Interface.h"

int APIENTRY wWinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPTSTR    lpCmdLine,
	int       nCmdShow) {
	FILE* cp = nullptr;
	AllocConsole();
	freopen_s(&cp, "CONOUT$", "wt", stdout);


	RTSP_Manager* manager = NULL;

	manager = new RTSP_Manager();

	manager->Initialize();
	manager->Run();

}