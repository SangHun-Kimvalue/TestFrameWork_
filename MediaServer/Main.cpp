#include "MediaServer.h"

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow) {

	int showdebugmsg = 0;

	FILE* cp = nullptr;

#ifdef _DEBUG
	showdebugmsg = 1;
#endif


	if (showdebugmsg == 1)
	{
		AllocConsole();
		freopen_s(&cp, "CONOUT$", "wt", stdout);
	}


	if (showdebugmsg == 1)
	{
		fclose(cp);
		FreeConsole();
	}

	return 0;
}

