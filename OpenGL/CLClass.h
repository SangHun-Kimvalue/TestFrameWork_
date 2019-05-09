#pragma once


//#include <glad/glad.h>
#include <glew.h>
#include <glfw3.h>
#include <linmath.h>
#include <wglew.h>

#include <Windows.h>
//#include <string>
//#include <gdiplus.h>
#include <stdlib.h>
#include <stdio.h>
#include <atlimage.h>
#include <wingdi.h>
#include <conio.h>
#include <time.h>
#include <iostream>
#include <thread>
//#include <list>

class CLClass
{
public:
	CLClass();
	CLClass(HWND hwnd);
	~CLClass();
	
	bool Roop();

private:
	HWND m_hWndCopy;

	
	bool RGBSaveBMP();
	bool RGBSaveBMP(BYTE *image);

	bool save_screenshot(std::string filename, int w, int h);
	void Capture_CImage_old();
	void Capture_old();

	int CalFPS();
	int GetMonitors();
	bool GetAdapter();
	void DumpDevice(const DISPLAY_DEVICE& dd, size_t nSpaceCount);

	bool Initialize();
	bool Initialize(HDC HDCC, HGLRC DC);

	bool Cap_Release();
	bool Cap_Init();
	int Test_i();
	int Test_j();

	bool wglInit();

	int nWidth;
	int nHeight;

	HDC HDCC;
	HDC hCaptureDC;
	HBITMAP hBitmap;
	HGDIOBJ hOld;

	BITMAPINFO MyBMInfo;
	BITMAPINFOHEADER bmpInfoHeader;
	DWORD b_size;
	BYTE *src;


	HGLRC DC;
	GLFWmonitor ** Monitor;
	GLFWwindow* GLwindow;
	GLenum doubleBuffer;
	GLubyte ubImage[65536];
};
