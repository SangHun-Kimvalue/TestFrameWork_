#pragma once

#include "stdafx.h"
#include "GDICaptureClass.h"
#include <baseapi.h>
#include <allheaders.h>
#include <comutil.h>
#include <leptwin.h>
#include <gdiplus.h>

using namespace Gdiplus;
#pragma comment(lib, "gdiplus.lib")

//Ini ���Ϸ� �ҷ����� �߰�		Ŭ������ �ϳ� ����� ���ҵ�
#define DATAPATH "..\\Libs\\tesseract\\tesseract\\tessdata";
#define IMAGEPATH "..\\Libs\\OCR_Test_Image\\";

class OCRClass
{
public:
	OCRClass(GDICaptureClass* Cap);
	OCRClass(GDICaptureClass *Cap, int Select);
	~OCRClass();

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

	//Imageclass* m_image;
	GDICaptureClass* m_Cap;

	std::string datapath;
	std::string imagepath;
	std::string imagename;
	std::string bmpimagename;
	std::string hangulname;

	INT GetEncoderClsid(const WCHAR* format, CLSID* pClsid);
	int converbmptopng();

};
