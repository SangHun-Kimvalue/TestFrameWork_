#include "OCRClass.h"


enum {UMEMORY, UFILE};

OCRClass::OCRClass(HWND hwnd) : m_hwnd(hwnd)
{

	Capturer = new GDICaptureClass(m_hwnd);
	//ImageCV = new ImageClass(Capturer->nWidth, Capturer->nHeight, Capturer->src);
	Tesseract = new TesseractClass("Test 123 가나다");		//1 : from file // 0 : from memory  // non : not thing
	//Tesseract = new TesseractClass(UFILE, ImageCV->nWidth, ImageCV->nHeight, ImageCV->src);		//1 : from file // 0 : from memory  // non : not thing
	//Match = new TextMatchClass();

}

OCRClass::~OCRClass()
{

	delete Match;
	delete Capturer;
	delete Tesseract;
}
