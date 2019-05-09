#include "OCRClass.h"
#include "stdafx.h"
#include <parent.hpp>
#include <Windows.h>
#include <iostream>


//Ini 파일로 불러오기 추가		클래스로 하나 만들면 편할듯
#define DATAPATH "..\\Libs\\tesseract\\tesseract\\tessdata";
#define IMAGEPATH "..\\Libs\\OCR_Test_Image\\";

//typedef BaseAlertModule* (*TestClass)();
//
//int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow) {
//
//	//HRESULT rs;
//
//	AllocConsole();
//	FILE* cp;
//	freopen_s(&cp, "CONOUT$", "wt", stdout);
//
//	TestClass fptr;
//	HMODULE hDLL;
//
//	hDLL = LoadLibrary(L"DllTest.dll");
//	if (hDLL == NULL) {
//		std::cerr << "FIle Not Found" << std::endl;
//		return -1;
//	}
//
//	fptr = (TestClass)::GetProcAddress(hDLL, "_GetInstance");
//	//DllClass* GetInstance = (DllClass*)GetProcAddress(hdll, "_GetInstance");
//	if (fptr == NULL) {
//		std::cerr << "File Not Found" << std::endl;
//		FreeLibrary(hDLL);
//		return -1;
//	}
//
//	BaseAlertModule* dllclass = fptr();
//
//	dllclass->PrintModuleInfo();
//
//
//	delete dllclass;
//
//	FreeLibrary(hDLL);
//	FreeConsole();
//
//	return 0;
//}

#include <baseapi.h>
#include <allheaders.h>
#include <Windows.h>
#include <renderer.h>

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow) {

	char *outText;
	int res = 0;
	AllocConsole();
	FILE* cp;
	freopen_s(&cp, "CONOUT$", "wt", stdout);

	tesseract::TessBaseAPI *api = new tesseract::TessBaseAPI();
	// Initialize tesseract-ocr with English, without specifying tessdata path
	std::string datapath = DATAPATH;
	std::string imagepath = IMAGEPATH;
	std::string imagename = "ocr.jpeg";
	imagepath = imagepath + imagename;

	std::cout << api->Version() << std::endl;
	//api->SetVariable();

	//if (res = api->Init(datapath.c_str(), "eng+kor", tesseract::OEM_DEFAULT)) {
	if (res = api->Init(datapath.c_str(), "eng", tesseract::OEM_DEFAULT)) {
		fprintf(stderr, "Could not initialize tesseract.\n");
		exit(1);
	}

	// Open input image with leptonica library
	Pix *image = pixRead(imagepath.c_str());
	if (image == NULL) {
		std::cerr << "Not Read. Failed." << std::endl;
		api->End();
		pixDestroy(&image);
		system("pause");
		FreeConsole();
		return -1;
	}
	//bool succeed = api->ProcessPages(input_image, retry_config, timeout_ms, renderer);
	api->SetImage(image);
	// Get OCR result
	outText = api->GetUTF8Text();
	printf("OCR output:\n%s", outText);

	
	pixDestroy(&image);

	FreeConsole();

	return 0;
}

//#include <allheaders.h>
//#include <baseapi.h>
//#include <renderer.h>
//#include <Windows.h>
//int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
//{
//	const char* input_image = "..\\Libs\\tesseract\\OCR_Test_Image\\ocr_orig.png";
//	const char* output_base = "my_first_tesseract_pdf";
//	const char* datapath = "..\\Libs\\tesseract\\tesseract\\tessdata";
//	int timeout_ms = 5000;
//	const char* retry_config = nullptr;
//	bool textonly = false;
//	int jpg_quality = 92;
//
//	tesseract::TessBaseAPI *api = new tesseract::TessBaseAPI();
//	if (api->Init(datapath, "eng")) {
//		fprintf(stderr, "Could not initialize tesseract.\n");
//		exit(1);
//	}
//
//	tesseract::TessPDFRenderer *renderer = new tesseract::TessPDFRenderer(
//		output_base, api->GetDatapath(), textonly);
//
//	bool succeed = api->ProcessPages(input_image, retry_config, timeout_ms, renderer);
//	if (!succeed) {
//		fprintf(stderr, "Error during processing.\n");
//		return EXIT_FAILURE;
//	}
//	api->End();
//	return EXIT_SUCCESS;
//}