#include <baseapi.h>
#include <allheaders.h>
#include <Windows.h>

//int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow) {
//
//	char *outText;
//
//	tesseract::TessBaseAPI *api = new tesseract::TessBaseAPI();
//	// Initialize tesseract-ocr with English, without specifying tessdata path
//	if (api->Init("..\\Libs\\tesseract\\tesseract\\tessdata", "eng")) {
//		fprintf(stderr, "Could not initialize tesseract.\n");
//		exit(1);
//	}
//
//	// Open input image with leptonica library
//	Pix *image = pixRead("..\\Libs\\tesseract\\OCR_Test_Image\\or_orig.png");
//	api->SetImage(image);
//	// Get OCR result
//	outText = api->GetUTF8Text();
//	printf("OCR output:\n%s", outText);
//
//	// Destroy used object and release memory
//	api->End();
//	delete[] outText;
//	pixDestroy(&image);
//
//	return 0;
//}

#include <allheaders.h>
#include <baseapi.h>
#include <renderer.h>
#include <Windows.h>
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
	const char* input_image = "..\\Libs\\tesseract\\OCR_Test_Image\\ocr_orig.png";
	const char* output_base = "my_first_tesseract_pdf";
	const char* datapath = "..\\Libs\\tesseract\\tesseract\\tessdata";
	int timeout_ms = 5000;
	const char* retry_config = nullptr;
	bool textonly = false;
	int jpg_quality = 92;

	tesseract::TessBaseAPI *api = new tesseract::TessBaseAPI();
	if (api->Init(datapath, "eng")) {
		fprintf(stderr, "Could not initialize tesseract.\n");
		exit(1);
	}

	tesseract::TessPDFRenderer *renderer = new tesseract::TessPDFRenderer(
		output_base, api->GetDatapath(), textonly);

	bool succeed = api->ProcessPages(input_image, retry_config, timeout_ms, renderer);
	if (!succeed) {
		fprintf(stderr, "Error during processing.\n");
		return EXIT_FAILURE;
	}
	api->End();
	return EXIT_SUCCESS;
}