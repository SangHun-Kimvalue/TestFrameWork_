#include "TesseractClass.h"

TesseractClass::TesseractClass(): Base_String("") {}

TesseractClass::TesseractClass(std::string Base_string, int Base_Num)
	: Base_String(Base_string), String_Type(TNULL) 
{
}

TesseractClass::TesseractClass(std::string Base_string, int wid, int hei, BYTE* src)
	: Base_String(Base_string), String_Type(TNULL)						//���� ���̽� �ѹ��� ��Ʈ�� ������ ���� ����ؾߵǴµ� ��Ʈ���� ����������.
{
	//Init(InputType);
	//Init();
	//Test(wid, hei, src);
}

TesseractClass::TesseractClass(int Select, int Iwidth, int Iheight, BYTE* Isrc, std::string Base_String, int InputType)
	: Base_String(Base_String), String_Type((TextType)InputType) {

}

TesseractClass::~TesseractClass()
{
	Release();
}

//Base_String ���̸� ��ȯ
int TesseractClass::FindEachText(std::string Base_String, std::string InputType, int Base_Num) {

	
	//int *Base_Type = (int *)malloc(sizeof(int)*Base_length);

	int point = -1;
	bool detec_han = 0;

	if (strstr(InputType.c_str(), "NUM")) {
		//for (int i = 0; i < Base_length; i++)
		//	Base_Type[i] == NUM;
		Base_length = std::to_string(Base_Num).length();
		String_Type = NUM;
		return Base_length;
	}
	else {
		
		Base_length = Base_String.length();

		int* Base_Type = new int[Base_length];

		for (int i = 0; i < Base_length; i++) {		//Ÿ�� ����
			point++;
			//if (i > 2 && Base_Type[point - 2] == TNULL && Base_Type[point - 1] == TNULL)
			//	break;
			if (0 >= Base_String.at(i) || 127 < Base_String.at(i)) {		//�ѱ�
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

			//if (65 <= Base_String.at(i) && Base_String.at(i) <= 90 || (97 <= Base_String.at(i) && Base_String.at(i) <= 122)) {		//����  �ѱ��� ��� ����
			else if (isalpha(Base_String.at(i)) != 0) {		//����
				Base_Type[point] = ENG;
				continue;
			}

			//else if (48 <= Base_String.at(i) || 57 >= Base_String.at(i)) {		//����			//���� ���� x 
			else if (isdigit(Base_String.at(i)) != 0) {		//����
				Base_Type[point] = NUM;
				continue;
			}

			//else if (isspace(Base_String.at(i)) != 0) {
			//	Base_Type[point] = SPACE;
			//	continue;
			//}
			//else if (ispunct(Base_String.at(i)) != 0) {
			//	Base_Type[point] = SPEC;
			//	continue;
			//}

			else
				Base_Type[point] = TNULL;
		}

		String_Type = FindTextType(Base_Type);

		delete Base_Type;
	}

	return Base_length;
}

//Base_String Ÿ���� ��ȯ
TextType TesseractClass::FindTextType(int Base_Type[]) {

	for (int i = 0; i < Base_length; i++) {
		switch (Base_Type[i])
		{
		case SPACE :
		case SPEC :
		case NUM :
		case TNULL :
		default :
			if (i == Base_length - 1 && String_Type == TNULL) {
				return NUM;
			}
			continue;
		//case NUM :
		//	Numchecker = true;
		//	if (i == Base_length - 1 && (String_Type == TNULL || String_Type == NUM)) {
		//		Base_Num = atoi(Base_String.c_str());
		//		return NUM;
		//	}
		//	break;
		case KOR :
			return KOR;
			
		case ENG :
			return ENG;
		}
	}
	
	return TNULL;

}

//�׽�Ʈ ȣ���
bool TesseractClass::Test(int wid, int hei, BYTE* src) {

	char *outText;
	//converbmptopng();
	//image = pixRead("Bird.png");

	image = pixCreate(wid, hei, 32);
	pixSetData(image, (l_uint32*)src);

	api->SetImage(image);

	outText = api->GetUTF8Text();
	std::string temp = UniToANSI(outText);
	std::cout << temp << std::endl;

	//delete m_Test;

	return true;
}

std::string TesseractClass::GetTextUTF8(int wid, int hei, unsigned char* src, int chanel, size_t step) {

	char *outText;
	//image = pixCreate(wid, hei, 32);
	//pixSetData(image, (l_uint32*)src);

	api->SetImage((unsigned char*)src, wid, hei, chanel, step);		//src, wid, hei, channels(), step1()

	outText = api->GetUTF8Text();
	
	std::string temp = UniToANSI(outText);
	std::cout << temp << std::endl;

	//pixDestroy(&image);

	return temp;
}

bool TesseractClass::Init(std::string Base_String, std::string InputType, int Base_Num) {

	api = new tesseract::TessBaseAPI();

	datapath = DATAPATH;
	TCHAR sPath[MAX_PATH] = { 0, };

	::GetCurrentDirectory(MAX_PATH, sPath);
	std::wstring temp = sPath;
	std::string temp2;
	temp2.assign(temp.begin(), temp.end());

	datapath = temp2 + datapath;

#ifdef _DEBUG
	datapath = DATAPATH;
#endif

	Base_length = FindEachText(Base_String, InputType, Base_Num);

	std::string Init_Type = "";
	if (String_Type == KOR) {
		Init_Type = "kor";
	}
	else if (String_Type == ENG || String_Type == NUM) {
		Init_Type = "eng";
	}
	//else if (DetectKor == true && DetectEng == true) {
	//	std::cout << "�ѱ� ���� ȥ�� ����� �����Ǿ����ϴ�. �νķ� ���ϰ� ����˴ϴ�." << std::endl;
	//	Init_Type = "eng+kor";			//�Ͼ���� �ȵǴ� ���. ���� ó��
	//}
	else {
		std::cout << "Could not find String type. Set type to default eng" << std::endl;
		Init_Type = "eng";
	}

	//if (res = api->Init(datapath.c_str(), "eng+kor", tesseract::OEM_DEFAULT)) {
	if (bool res = api->Init(datapath.c_str(), Init_Type.c_str(), tesseract::OEM_LSTM_ONLY)) {			//���̰� �󸶳� ������
		fprintf(stderr, "Could not initialize tesseract.\n");
		std::cout << "Could not initialize tesseract tessdata path." << std::endl;
		return false;

	}

	return true;
}

void TesseractClass::Release() {

	if(api != NULL)
		api->End();

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

	// bstrWide �迭 ���� Lenth�� �о� �´�.
	nLength = MultiByteToWideChar(CP_UTF8, 0, outText, lstrlenA(outText) + 1, NULL, NULL);

	// bstrWide �޸� ����
	bstrWide = SysAllocStringLen(NULL, nLength);

	//UTF-8�� ����
	MultiByteToWideChar(CP_UTF8, 0, outText, lstrlenA(outText) + 1, bstrWide, nLength);

	// char �迭 ������ Lenth�� �о� �´�.
	nLength = WideCharToMultiByte(CP_ACP, 0, bstrWide, -1, NULL, 0, NULL, NULL);

	// pszAnsi �迭 ����
	pszAnsi = new char[nLength];

	// char ��ȯ
	WideCharToMultiByte(CP_ACP, 0, bstrWide, -1, pszAnsi, nLength, NULL, NULL);

	// bstrWide �޸� ����
	SysFreeString(bstrWide);

	std::string temp = pszAnsi;

	delete pszAnsi;

	return temp;
}
