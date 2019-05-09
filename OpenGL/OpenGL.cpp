// OpenGL.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <Windows.h>
#include "AgentClass.h"


int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)		//Instruments
//int main()
{

	//HWND hwnd = FindWindow(L"Shell_TrayWnd", 0); // taskbar의 핸들을 
	//HWND hDesktop = ::FindWindowEx(::FindWindowEx(::FindWindow(L"ProgMan", NULL), NULL, L"SHELLDLL_DefView", NULL), NULL, L"SysListView32", NULL);
	HWND Windowhwnd = GetDesktopWindow();

	AllocConsole();
	FILE* cp;
	freopen_s(&cp, "CONOUT$", "wt", stdout);

	AgentClass *AG = new AgentClass(Windowhwnd);

	FreeConsole();
	return 0;

}

