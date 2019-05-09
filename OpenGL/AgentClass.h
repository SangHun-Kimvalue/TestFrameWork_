#pragma once
#include <Windows.h>
#include <time.h>
#include <iostream>
#include <string>


class AgentClass
{
public:
	AgentClass();
	AgentClass(HWND hwnd);
	~AgentClass();

	bool Cap_Release();
	bool Cap_Init();

private:

	HWND m_hWndCopy;
	HDC HDCC;
	HDC hCaptureDC;
	HBITMAP hBitmap;
	HGDIOBJ hOld;

	BITMAPINFO MyBMInfo;
	BITMAPINFOHEADER bmpInfoHeader;
	DISPLAY_DEVICEW Dis_info;
	DWORD b_size;
	BYTE *src;
	CURSORINFO cursor;
	POINT point;
	ICONINFOEXW info;
	BITMAP bmpCursor;

	RECT ImageRect = { 0 , 0 , 0 , 0 };

	bool Get_Monitors();
	bool GetMouse();
	void Roop();
	int CalFPS();
	bool GetScreen();
	bool DeleteDCA();
	bool RGBSaveBMP(BYTE *image);

	const int select;
	int nWidth;
	int nHeight;
	int nposx;
	int nposy;

};

