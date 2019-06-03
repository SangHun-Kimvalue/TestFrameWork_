#include "TesseractClass.h"

TesseractClass::TesseractClass() {}

TesseractClass::TesseractClass(std::string Base_string, std::string InputType, int wid, int hei, BYTE* src)
	: Base_String(Base_string), String_Type(TNULL), Imagewidth(wid), Imageheight(hei), src(src)
{
	Base_Num = 0;
	Base_Type[256] = {};

	Base_length = FindEachText(Base_String);
	String_Type = FindTextType(Base_String);
	Init();

	//Init();
	//Test();
}

TesseractClass::TesseractClass(int Select, int Iwidth, int Iheight, BYTE* Isrc, std::string Base_String, int InputType)
	: Imagewidth(Iwidth), Imageheight(Iheight), src(Isrc), Base_String(Base_String), String_Type((TextType)InputType) {

	Base_Num = 0;
	Base_Type[256] = {};
	Base_length = 0;

	if (Select == 1) {
		if (Init() == true) {
			if (Open() == true)
				Process();
			else
				Release();
		}
	}
	else {
		Init();
		Test(Iwidth, Iheight, Isrc);
	}
}

TesseractClass::~TesseractClass()
{
	Release();
}

//Base_String 길이를 반환
int TesseractClass::FindEachText(std::string Base_String) {		

	Base_length = Base_String.length();

	int point = -1;
	bool detec_han = 0;

	for (int i = 0; i < Base_length; i++) {		//타입 검출
		point++;
		//if (i > 2 && Base_Type[point - 2] == TNULL && Base_Type[point - 1] == TNULL)
		//	break;
		if (0 >= Base_String.at(i) || 127 < Base_String.at(i)) {		//한글
			if (detec_han) {
				point--;
				detec_han = false;
				continue;
			}
			else {
				detec_han = true;
				Base_Type[point] = KOR;
				DetectKor = true;
				continue;
			}
		}

		//if (65 <= Base_String.at(i) && Base_String.at(i) <= 90 || (97 <= Base_String.at(i) && Base_String.at(i) <= 122)) {		//영문  한글일 경우 죽음
		else if (isalpha(Base_String.at(i)) != 0) {		//영문
			Base_Type[point] = ENG;
			DetectEng = true;
			continue;
		}

		//else if (48 <= Base_String.at(i) || 57 >= Base_String.at(i)) {		//숫자			//공백 검출 x 
		else if (isdigit(Base_String.at(i)) != 0) {		//숫자
			Base_Type[point] = NUM;
			continue;
		}

		else if (isspace(Base_String.at(i)) != 0) {
			Base_Type[point] = SPACE;
			continue;
		}
		
		else if (ispunct(Base_String.at(i)) != 0) {
			Base_Type[point] = SPEC;
			continue;
		}

		else
			Base_Type[point] = TNULL;
	}

	return Base_length;
}

//Base_String 타입을 반환
TextType TesseractClass::FindTextType(std::string Base_String) {

	bool Numchecker = 0;

	for (int i = 0; i < Base_length; i++) {
		switch (Base_Type[i])
		{
		case SPACE :
		case SPEC :
			if (i == Base_length - 1 && String_Type == TNULL) {
				String_Type = ENG;
				return String_Type;
			}
			continue;
		case NUM :
			Numchecker = true;
			if (i == Base_length - 1 && (String_Type == TNULL || String_Type == NUM)) {
				String_Type = NUM;
				Base_Num = atoi(Base_String.c_str());
				return NUM;
			}
			break;
		case KOR :
			String_Type = KOR;
			return String_Type;
		case ENG :
			String_Type = ENG;
			return String_Type;
		case TNULL:
		default:
			break;
		}
		if (String_Type != TNULL && Numchecker == false)
			break;
	}
	
	return String_Type;

}

//UTF 8 -> 16 변환
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

//테스트용 bmp to png
int TesseractClass::converbmptopng() {

	// Initialize GDI+.
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	CLSID   encoderClsid;
	Gdiplus::Status  stat;
	Gdiplus::Image*   image = new Gdiplus::Image(L"CCapture_1.bmp");

	// Get the CLSID of the PNG encoder.
	GetEncoderClsid(L"image/png", &encoderClsid);

	stat = image->Save(L"CCapture_1.png", &encoderClsid, NULL);

	if (stat == Gdiplus::Ok)
		printf( "" /*"Bird.png was saved successfully\n"*/);
	else
		printf("Failure: stat = %d\n", stat);

	delete image;
	Gdiplus::GdiplusShutdown(gdiplusToken);

	return 0;
}

//테스트 호출용
bool TesseractClass::Test(int wid, int hei, BYTE* src) {



	//converbmptopng();
	//image = pixRead("Bird.png");

	image = pixCreate(wid, hei, 32);
	pixSetData(image, (l_uint32*)src);
	//pixSetData

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


	std::string Init_Type = "";
	if (String_Type == KOR) {
		Init_Type = "kor";
	}
	else if (String_Type == ENG || String_Type == NUM) {
		Init_Type = "eng";
	}
	else if (DetectKor == true && DetectEng == true) {
		std::cout << "한글 영어 혼용 사용이 감지되었습니다. 인식률 저하가 우려됩니다." << std::endl;
		Init_Type = "eng+kor";			//일어나서는 안되는 경우. 에러 처리
	}
	else {
		std::cout << "Could find String type. Set type to default eng" << std::endl;
		Init_Type = "eng";
	}

	//if (res = api->Init(datapath.c_str(), "eng+kor", tesseract::OEM_DEFAULT)) {
	if (res = api->Init(datapath.c_str(), Init_Type.c_str(), tesseract::OEM_DEFAULT)) {			//차이가 얼마나 나는지
		fprintf(stderr, "Could not initialize tesseract.\n");
		std::cout << "Could not initialize tesseract tessdata path." << std::endl;
		return false;

	}

	return true;
}

void TesseractClass::Release() {

	if(api != NULL)
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

	outText = api->GetUTF8Text();
	OutPutstr = UniToANSI(outText);

	std::cout << OutPutstr << std::endl;

}

