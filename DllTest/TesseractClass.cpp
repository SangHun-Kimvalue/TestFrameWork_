#include "TesseractClass.h"


TesseractClass::TesseractClass()
{
}

TesseractClass::TesseractClass(int Select, int Iwidth, int Iheight, BYTE* Isrc) : Imagewidth(Iwidth), Imageheight(Iheight), src(Isrc){

	if (Select == 1) {
		if (Init() == true) {
			if (Open() == true)
				Process();
			else
				Release();
		}
	}
	else
		Test();

}

INT TesseractClass::GetEncoderClsid(const WCHAR* format, CLSID* pClsid) {

	UINT  num = 0;          // number of image encoders
	UINT  size = 0;         // size of the image encoder array in bytes

	Gdiplus::ImageCodecInfo* pImageCodecInfo = NULL;

	Gdiplus::GetImageEncodersSize(&num, &size);
	if (size == 0)
		return -1;  // Failure

	pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
	if (pImageCodecInfo == NULL)
		return -1;  // Failure

	GetImageEncoders(num, size, pImageCodecInfo);

	for (UINT j = 0; j < num; ++j)
	{
		if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;  // Success
		}
	}

	free(pImageCodecInfo);
	return -1;  // Failure

}

int TesseractClass::converbmptopng() {

	// Initialize GDI+.
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	CLSID   encoderClsid;
	Gdiplus::Status  stat;
	Gdiplus::Image*   image = new Gdiplus::Image(L"capture_0.bmp");

	// Get the CLSID of the PNG encoder.
	GetEncoderClsid(L"image/png", &encoderClsid);

	stat = image->Save(L"Bird.png", &encoderClsid, NULL);

	if (stat == Gdiplus::Ok)
		printf("Bird.png was saved successfully\n");
	else
		printf("Failure: stat = %d\n", stat);

	delete image;
	Gdiplus::GdiplusShutdown(gdiplusToken);

	return 0;
}

bool TesseractClass::Test() {

	Init();

	//converbmptopng();
	//image = pixRead("Bird.png");

	image = pixCreate(Imagewidth, Imageheight, 32);
	pixSetData(image, (l_uint32*)src);

	api->SetImage(image);

	outText = api->GetUTF8Text();
	std::string temp = UniToANSI(outText);
	std::cout << temp << std::endl;

	//delete m_Test;

	return true;
}

bool TesseractClass::Init() {

	api = new tesseract::TessBaseAPI();
	// Initialize tesseract-ocr with English, without specifying tessdata path
	datapath = DATAPATH;
	imagepath = IMAGEPATH;
	imagename = "WiDpa.jpeg";
	bmpimagename = "WiDpa.bmp";
	//hangulname = "Text_Color_Test.PNG";
	//hangulname = "eng+kor2.PNG";
	hangulname = "Noise_Test.PNG";
	imagepath = imagepath + hangulname;
	TCHAR sPath[MAX_PATH] = { 0, };

	::GetCurrentDirectory(MAX_PATH, sPath);
	std::wstring temp = sPath;
	std::string temp2;
	temp2.assign(temp.begin(), temp.end());

	datapath = temp2 + datapath;
	//std::cout << datapath << std::endl;

#ifdef _DEBUG
	datapath = DATAPATH;
#endif

	//if (res = api->Init(datapath.c_str(), "eng+kor", tesseract::OEM_DEFAULT)) {
	if (res = api->Init(datapath.c_str(), "eng+kor", tesseract::OEM_DEFAULT)) {			//차이가 얼마나 나는지		트레이닝 데이터는 어디서?
		fprintf(stderr, "Could not initialize tesseract.\n");
		std::cout << "Could not initialize tesseract tessdata path." << std::endl;
		return false;

	}

	return true;
}

void TesseractClass::Release() {

	api->End();
	//pixDestroy(&image);				//??왜

}

bool TesseractClass::Open() {

	// Open input image with leptonica library
	image = pixRead(imagepath.c_str());
	if (image == NULL) {
		std::cerr << "Not Read. Failed." << std::endl;
		api->End();
		pixDestroy(&image);
		system("pause");
		FreeConsole();
		return false;
	}
	else {

		return true;
	}

}

std::string TesseractClass::UniToANSI(char* outText) {

	BSTR    bstrWide;
	char*   pszAnsi;
	int     nLength;

	// bstrWide 배열 생성 Lenth를 읽어 온다.
	nLength = MultiByteToWideChar(CP_UTF8, 0, outText, lstrlenA(outText) + 1, NULL, NULL);

	// bstrWide 메모리 설정
	bstrWide = SysAllocStringLen(NULL, nLength);

	//UTF-8로 변경
	MultiByteToWideChar(CP_UTF8, 0, outText, lstrlenA(outText) + 1, bstrWide, nLength);

	// char 배열 생성전 Lenth를 읽어 온다.
	nLength = WideCharToMultiByte(CP_ACP, 0, bstrWide, -1, NULL, 0, NULL, NULL);

	// pszAnsi 배열 생성
	pszAnsi = new char[nLength];

	// char 변환
	WideCharToMultiByte(CP_ACP, 0, bstrWide, -1, pszAnsi, nLength, NULL, NULL);

	// bstrWide 메모리 해제
	SysFreeString(bstrWide);

	return pszAnsi;
}

void TesseractClass::Process() {

	api->SetImage(image);
	//api->SetImage(m_image->GetPixels(), m_image->GetWidth(), m_image->GetHeight(), m_image->GetBytesPerPixel(), m_image->GetBytesPerScanLine());
	// Get OCR result
	outText = api->GetUTF8Text();
	OutPutstr = UniToANSI(outText);

	std::cout << OutPutstr << std::endl;

}


TesseractClass::~TesseractClass()
{
	Release();
}
