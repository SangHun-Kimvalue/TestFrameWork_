#include "OCRClass.h"


enum {USEMEMORY, USEFILE};

OCRClass::OCRClass(HWND hwnd) : m_hwnd(hwnd)
{
	std::string Test_String = "!@ #$ %^& *()_+=`   ~12/*-+3  ";


	Capturer = new GDICaptureClass(m_hwnd);
	//ImageCV = new ImageClass(Capturer->nWidth, Capturer->nHeight, Capturer->src);
	Tesseract = new TesseractClass(Test_String);
	//Tesseract = new TesseractClass(USEFILE, ImageCV->nWidth, ImageCV->nHeight, ImageCV->src, Test_String);		
	//1 : from file // 0 : from memory  // non : not thing
	//Match = new TextMatchClass();

}

OCRClass::~OCRClass()
{

	delete Match;
	delete Capturer;
	delete Tesseract;
}
