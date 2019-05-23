#pragma once

#include "stdafx.h"
#include "TesseractClass.h"
#include "ImageClass.h"
#include "TextMatchClass.h"

class OCRClass
{
public:
	OCRClass(HWND hwnd);
	~OCRClass();

	bool Test();


private:


	GDICaptureClass* Capturer;
	ImageClass* ImageCV;
	TesseractClass* Tesseract;	
	TextMatchClass* Match;

	HWND m_hwnd;

};
