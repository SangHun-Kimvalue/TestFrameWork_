
#include "OCRClass.h"

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

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow) {

	AllocConsole();

	FILE* cp;
	freopen_s(&cp, "CONOUT$", "wt", stdout);
	HWND hwnd = GetDesktopWindow();

	GDICaptureClass* Capturer = new GDICaptureClass(hwnd);
	OCRClass* OCR = new OCRClass(Capturer, 0);

	//OCR->Test();

	system("pause");
	FreeConsole();


	delete OCR;
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