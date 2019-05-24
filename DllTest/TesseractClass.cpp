#include "TesseractClass.h"


TesseractClass::TesseractClass(std::string Base_String)
{
	Base_Type[256] = { TNULL };
	Base_length = FindEachText(Base_String);
	FindTextType(Base_String);
}

TesseractClass::TesseractClass(int Select, int Iwidth, int Iheight, BYTE* Isrc) : Imagewidth(Iwidth), Imageheight(Iheight), src(Isrc){

	Base_Type[256] = { TNULL };
	Base_length = 0;

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

TesseractClass::~TesseractClass()
{
	//Release();
}

int TesseractClass::FindEachText(std::string Base_String) {

	Base_length = Base_String.length();

	int point = -1;
	bool detec_han = 0;

	for (int i = 0; i < Base_length; i++) {		//타입 검출
		point++;
		if (i > 2 && Base_Type[point - 2] == TNULL && Base_Type[point - 1] == TNULL)
			break;
		if (0 >= Base_String.at(i) || 127 < Base_String.at(i)) {		//한글
			if (detec_han) {
				point--;
				detec_han = false;
				continue;
			}
			else {
				detec_han = true;
				Base_Type[point] = KOR;
				continue;
			}
		}

		//if (65 <= Base_String.at(i) && Base_String.at(i) <= 90 || (97 <= Base_String.at(i) && Base_String.at(i) <= 122)) {		//영문  한글일 경우 죽음
		else if (isalpha(Base_String.at(i)) != 0) {		//영문
			Base_Type[point] = ENG;
			continue;
		}

		//else if (48 <= Base_String.at(i) || 57 >= Base_String.at(i)) {		//숫자			//공백 검출 x 
		else if (isdigit(Base_String.at(i)) != 0) {		//숫자
			Base_Type[point] = NUM;
			continue;
		}
		else
			Base_Type[point] = TNULL;
	}

	return Base_length;
}

std::string TesseractClass::FindTextType(std::string Base_String) {

	/*while(1) {						//공백 제거 알고리즘

		size_t temp = Base_String.find(" ", 0);
		if (temp == std::string::npos)
			break;

		Base_String.erase(temp, 1);
	}*/

	int Engchecker = 0;
	int Korchecker = 0;
	int Numchecker = 0;

	for (int i = 0; i < Base_length; i++) {
		switch (Base_Type[i])
		{
		case TNULL:
			break;
		case KOR:
			Korchecker++;
			break;
		case ENG:
			Engchecker++;
			break;
		case NUM:
			Numchecker++;
			break;
		default:
			break;
		}

	}



	return Base_String;

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

