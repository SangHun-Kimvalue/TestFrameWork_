#include "OCRClass.h"


OCRClass::OCRClass(GDICaptureClass* Cap)
{
}

OCRClass::OCRClass(GDICaptureClass* Cap, int Select) : m_Cap(Cap){

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

INT OCRClass::GetEncoderClsid(const WCHAR* format, CLSID* pClsid) {

	UINT  num = 0;          // number of image encoders
	UINT  size = 0;         // size of the image encoder array in bytes

	ImageCodecInfo* pImageCodecInfo = NULL;

	GetImageEncodersSize(&num, &size);
	if (size == 0)
		return -1;  // Failure

	pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
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

int OCRClass::converbmptopng() {

	// Initialize GDI+.
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	CLSID   encoderClsid;
	Status  stat;
	Image*   image = new Image(L"capture_0.bmp");

	// Get the CLSID of the PNG encoder.
	GetEncoderClsid(L"image/png", &encoderClsid);

	stat = image->Save(L"Bird.png", &encoderClsid, NULL);

	if (stat == Ok)
		printf("Bird.png was saved successfully\n");
	else
		printf("Failure: stat = %d\n", stat);

	delete image;
	GdiplusShutdown(gdiplusToken);

	return 0;
}

bool OCRClass::Test() {

	datapath = DATAPATH;

#ifdef PIXTEST
	struct Pix
	{
		l_uint32             w;         /*!< width in pixels                   */
		l_uint32             h;         /*!< height in pixels                  */
		l_uint32             d;         /*!< depth in bits (bpp)               */
		l_uint32             spp;       /*!< number of samples per pixel       */
		l_uint32             wpl;       /*!< 32-bit words/line                 */
		l_uint32             refcount;  /*!< reference count (1 if no clones)  */
		l_int32              xres;      /*!< image res (ppi) in x direction    */
										/*!< (use 0 if unknown)                */
		l_int32              yres;      /*!< image res (ppi) in y direction    */
										/*!< (use 0 if unknown)                */
		l_int32              informat;  /*!< input file format, IFF_*          */
		l_int32              special;   /*!< special instructions for I/O, etc */
		char                *text;      /*!< text string associated with pix   */
		struct PixColormap  *colormap;  /*!< colormap (may be null)            */
		l_uint32            *data;      /*!< the image data                    */
	};
	typedef struct Pix PIX;
#endif

	Init();

	//converbmptopng();
	//image = pixRead("Bird.png");
	//if (image == NULL) {
	//	std::cerr << "Not Read. Failed." << std::endl;
	//	api->End();
	//	pixDestroy(&image);
	//	system("pause");
	//	FreeConsole();
	//	return false;
	//}

	image = pixCreate(m_Cap->nWidth, m_Cap->nHeight, 32);
	pixSetData(image, (l_uint32*)m_Cap->src);
	//pixSetSpp(m_Test, 4);
	//pixSetXRes(m_Test, 96);
	//pixSetYRes(m_Test, 96);
	//pixSetInputFormat(m_Test, 0);
	//pixEndianByteSwap(m_Test);

	api->SetImage(image);

	outText = api->GetUTF8Text();
	std::string temp = UniToANSI(outText);
	std::cout << temp << std::endl;

	//delete m_Test;

	return true;
}

bool OCRClass::Init() {

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

	//std::cout << api->Version() << std::endl << "Tesseract Init Success" << std::endl;

	//if (res = api->Init(datapath.c_str(), "eng+kor", tesseract::OEM_DEFAULT)) {
	if (res = api->Init(datapath.c_str(), "eng+kor", tesseract::OEM_DEFAULT)) {			//차이가 얼마나 나는지		트레이닝 데이터는 어디서?
		fprintf(stderr, "Could not initialize tesseract.\n");
		return false;
		//exit(1);
	}

	return true;
}

void OCRClass::Release() {

	api->End();
	//pixDestroy(&image);				//??왜

}

bool OCRClass::Open() {

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

std::string OCRClass::UniToANSI(char* outText) {

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

void OCRClass::Process() {

	api->SetImage(image);
	//api->SetImage(m_image->GetPixels(), m_image->GetWidth(), m_image->GetHeight(), m_image->GetBytesPerPixel(), m_image->GetBytesPerScanLine());
	// Get OCR result
	outText = api->GetUTF8Text();
	OutPutstr = UniToANSI(outText);

	std::cout << OutPutstr << std::endl;

}


OCRClass::~OCRClass()
{
	Release();
}
