#include "OCRClass.h"


enum {USEMEMORY, USEFILE};

OCRClass::OCRClass(HWND hwnd) : m_hwnd(hwnd)
{
	std::string Test_String = "!@ #$ %^& *()_+=`   ~12/*-+3  ";


	Capturer = new GDICaptureClass(m_hwnd);
	Tesseract = new TesseractClass(Test_String, TNULL);	
	//Tesseract = new TesseractClass(USEFILE, ImageCV->nWidth, ImageCV->nHeight, ImageCV->src, Test_String);		
	//1 : from file // 0 : from memory  // non : not thing
	
	ImageCV = new ImageClass(Capturer->nWidth, Capturer->nHeight, Capturer->src, Tesseract->String_Type, Tesseract->Base_length);
	
	//Match = new TextMatchClass();

}

OCRClass::~OCRClass()
{

	delete Match;
	delete Capturer;
	delete Tesseract;
}
