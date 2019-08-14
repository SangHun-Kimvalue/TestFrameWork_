#pragma once
#include <Windows.h>
#include <time.h>
#include <iostream>
#include <string>
#include "parent.hpp"

class GDICaptureClass
{
public:
	GDICaptureClass();
	GDICaptureClass(HWND hwnd);
	~GDICaptureClass();

	bool Cap_Release();
	bool Cap_Init();
	BYTE *src;
	HWND m_hWndCopy;
	int nWidth;
	int nHeight;
	BITMAPINFOHEADER bmpInfoHeader;
	DWORD b_size;
	HBITMAP hBitmap;
	bool RGBSaveBMP(BYTE *image);


	bool Get_Monitors();
	bool GetMouse();
	void Roop();
	int CalFPS();
	bool GetScreen();
	bool DeleteDCA();
private:

	HDC HDCC;
	HDC hCaptureDC;
	
	HGDIOBJ hOld;

	BITMAPINFO MyBMInfo;
	
	DISPLAY_DEVICEW Dis_info;
	
	CURSORINFO cursor;
	POINT point;
	ICONINFOEXW info;
	BITMAP bmpCursor;

	RECT ImageRect = { 0 , 0 , 0 , 0 };

	


	const int select;

	int nposx;
	int nposy;

};

