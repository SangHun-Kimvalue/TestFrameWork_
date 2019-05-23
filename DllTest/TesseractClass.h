#pragma once

#include <comutil.h>
#include <leptwin.h>
#include <gdiplus.h>
#include "stdafx.h"
#include <baseapi.h>
#include <allheaders.h>
#include <iostream>

using namespace Gdiplus;
#pragma comment(lib, "gdiplus.lib")

//Ini 파일로 불러오기 추가		클래스로 하나 만들면 편할듯
#define DATAPATH "\\tessdata";
#ifdef _DEBUG
#define DATAPATH "..\\Libs\\tesseract\\tesseract\\tessdata\\tessdata"
#endif
#define IMAGEPATH "..\\Libs\\OCR_Test_Image\\";

class TesseractClass
{
public:
	TesseractClass();
	TesseractClass(int Select, int Iwidth, int Iheight, BYTE* Isrc);
	~TesseractClass();

	bool Test();


private:

	bool Init();
	void Process();
	bool Open();
	void Release();
	std::string UniToANSI(char* outText);


	tesseract::TessBaseAPI *api;
	Pix *image;
	int res = 0;
	char *outText;
	std::string OutPutstr;

	int Imagewidth;
	int Imageheight;
	BYTE* src;

	std::string datapath;
	std::string imagepath;
	std::string imagename;
	std::string bmpimagename;
	std::string hangulname;

	INT GetEncoderClsid(const WCHAR* format, CLSID* pClsid);
	int converbmptopng();

};
