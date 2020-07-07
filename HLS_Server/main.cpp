#include "HttpServer.h"
#include <Windows.h>

int APIENTRY wWinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPTSTR    lpCmdLine,
	int       nCmdShow)
{
#ifdef _DEBUG
	FILE* cp = nullptr;
	AllocConsole();
	freopen_s(&cp, "CONOUT$", "wt", stdout);
#endif

	HttpServer* m_HttpServer = new HttpServer();


	//FFSegmenter* FF = new FFSegmenter();
	//
	//FF->init();
	//FF->run();
	//FF->close();


#ifdef _DEBUG
	fclose(cp);
	FreeConsole();
#endif

	return (EXIT_SUCCESS);
}