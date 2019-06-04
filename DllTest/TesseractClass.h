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
#define IMAGEPATH "..\\Libs\\Test_Image\\";

enum TextType { TNULL, ENG, KOR, NUM, SPACE, SPEC };

class TesseractClass
{
public:
	TesseractClass();
	TesseractClass(std::string Base_string, int wid, int hei, BYTE* src);
	TesseractClass(std::string Base_string);
	TesseractClass(int Select, int Iwidth, int Iheight, BYTE* Isrc, std::string Base_string, int InputType);
	~TesseractClass();

	bool Test(int wid, int hei, BYTE* src);
	bool Init(std::string InputType);
	std::string GetTextUTF8(int wid, int hei, unsigned char* src, size_t step);

	TextType String_Type;
	int Base_length;

private:
	
	bool Open();
	void Release();
	std::string UniToANSI(char* outText);


	tesseract::TessBaseAPI *api;
	Pix *image;
	int res = 0;

	std::string OutPutstr;

	std::string datapath;
	std::string imagepath;
	std::string imagename;
	std::string bmpimagename;
	std::string hangulname;

	const std::string Base_String;
	int Base_Num;

	TextType FindTextType(std::string Base_String, int Base_Type[256]);
	int FindEachText(std::string Base_String, std::string InputType);
	INT GetEncoderClsid(const WCHAR* format, CLSID* pClsid);
	int converbmptopng();

};