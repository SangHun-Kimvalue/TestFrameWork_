#include "OCRClass.h"


enum {USEMEMORY, USEFILE};

OCRClass::OCRClass(HWND hwnd) : m_hwnd(hwnd)
{
	std::string Test_String = "Improve";


	Capturer = new GDICaptureClass(m_hwnd);
		
	Tesseract = new TesseractClass(Test_String, TNULL, Capturer->nWidth, Capturer->nHeight, Capturer->src);
	//Tesseract = new TesseractClass(USEFILE, Capturer->nWidth, Capturer->nHeight, Capturer->src, Test_String, ENG);
	//1 : from file // 0 : from memory  // non : not thing
	
	ImageCV = new ImageClass(Capturer->nWidth, Capturer->nHeight, Capturer->src, Tesseract->String_Type, Tesseract->Base_length);
	//ImageCV = new ImageClass(Capturer);
	
	//Capturer->RGBSaveBMP(Capturer->src);
	//Match = new TextMatchClass();

	Tesseract->Test(ImageCV->c_wid, ImageCV->c_hei, ImageCV->src);

}

OCRClass::~OCRClass()
{

	delete Match;
	delete Capturer;
	delete Tesseract;
}
