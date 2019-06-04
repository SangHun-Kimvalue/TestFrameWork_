#include "GDICaptureClass.h"

GDICaptureClass::GDICaptureClass() : select(3)
{
}

GDICaptureClass::GDICaptureClass(HWND hwnd) : m_hWndCopy(hwnd), select(1)
{

	nWidth = 1920;
	nHeight = 1080;
	//nposx = 0;
	//nposy = 0;

	Get_Monitors();
	GetClientRect(m_hWndCopy, &ImageRect);									//윈도우 핸들 추가

	//이미지 영역 좌표를 이용하여 실제 이미지의 사이즈를 계산
	//nWidth = ImageRect.right - ImageRect.left;							  //윈도우 버전의 경우 사이즈 변경이 되므로 그때그때 조사
	//nHeight = ImageRect.bottom - ImageRect.top;

	//nWidth = nWidth * -1;
	//nHeight = nHeight *2;

	bool res;
	res = Cap_Init();
	if (!res)
		return;

	GetScreen();
	//Roop();

}

GDICaptureClass::~GDICaptureClass()
{
	Cap_Release();
	DeleteDCA();
}

bool GDICaptureClass::Cap_Release() {

	//if (src != NULL) {
	//	free(src);
	//}

	return true;
}

bool GDICaptureClass::DeleteDCA() {

	ReleaseDC(m_hWndCopy, HDCC);
	DeleteDC(hCaptureDC);
	DeleteObject(hBitmap);

	return true;
}

bool GDICaptureClass::Cap_Init() {

	MyBMInfo = { 0 };
	MyBMInfo.bmiHeader.biSize = sizeof(MyBMInfo.bmiHeader);

	bmpInfoHeader = { sizeof(BITMAPINFOHEADER) };
	bmpInfoHeader.biWidth = nWidth;
	bmpInfoHeader.biHeight = nHeight * -1;		//메모리 직접 pix 등록은 음수로 해줘야 안뒤집어짐.
	//bmpInfoHeader.biHeight = nHeight;
	bmpInfoHeader.biPlanes = 1;
	bmpInfoHeader.biBitCount = 32;

	b_size = ((nWidth * bmpInfoHeader.biBitCount + 31) / 32) * 4 * nHeight;

	hCaptureDC = CreateCompatibleDC(HDCC);									//DC생성
	hBitmap = CreateCompatibleBitmap(HDCC, nWidth, nHeight);

	return true;
}


bool GDICaptureClass::Get_Monitors() {

	const int count = GetSystemMetrics(SM_CMONITORS);

	//LPMONITORINFO name;
	//GetMonitorInfoW( , name);
	//
	//
	//sEnumInfo info;
	//info.iIndex = count;
	//info.hMonitor = NULL;
	//
	//EnumDisplayMonitors(NULL, NULL, GetMonitorByIndex, (LPARAM)&info);
	//
	//
	//int screenx = GetSystemMetrics(SM_XVIRTUALSCREEN);
	//int screeny = GetSystemMetrics(SM_YVIRTUALSCREEN);
	//int screenw = GetSystemMetrics(SM_CXVIRTUALSCREEN);
	//int screenh = GetSystemMetrics(SM_CYVIRTUALSCREEN);
	//
	//HDC dc = CreateDC((L"Display"), NULL, NULL, NULL);

	//MonitorCount();
	//Dis_info = { 0 };
	//Dis_info.cb = sizeof(DISPLAY_DEVICEW);
	//EnumDisplayDevicesW(NULL, 0, &Dis_info, EDD_GET_DEVICE_INTERFACE_NAME);

	HDCC = CreateDC(TEXT("Display"), NULL, NULL, NULL);
	//HDCC = CreateDC(Dis_info.DeviceName, NULL, NULL, NULL);

	DEVMODEW dev;

	std::string str = "\\\\.\\Display" + std::to_string(select);
	std::wstring temp;
	temp.assign(str.begin(), str.end());

	EnumDisplaySettingsW(temp.c_str(), ENUM_CURRENT_SETTINGS, &dev);
	//printf("Display%d : (%d * %d) (%d, %d)\n", select, dev.dmPelsWidth, dev.dmPelsHeight, dev.dmPosition.x, dev.dmPosition.y);

	//nWidth = dev.dmPelsWidth;
	//nHeight = dev.dmPelsHeight;
	nposx = dev.dmPosition.x;
	nposy = dev.dmPosition.y;

	//EnumDisplaySettingsW(L"\\\\.\\Display2", ENUM_CURRENT_SETTINGS, &dev);
	//printf("Display%d (%d * %d) (%d, %d)\n", 2, dev.dmPelsWidth, dev.dmPelsHeight, dev.dmPosition.x, dev.dmPosition.y);
	//
	//EnumDisplaySettingsW(L"\\\\.\\Display3", ENUM_CURRENT_SETTINGS, &dev);
	//printf("Display%d (%d * %d) (%d, %d)\n", 3, dev.dmPelsWidth, dev.dmPelsHeight, dev.dmPosition.x, dev.dmPosition.y);


	return true;
}


bool GDICaptureClass::GetMouse() {

	int colorcheck = GetSystemMetrics(SM_SAMEDISPLAYFORMAT);

	cursor = { sizeof(cursor) };					//매번 커서 데이터를 가져와야 아이콘 모양을 변경할 수 있음.
	bool check = ::GetCursorInfo(&cursor);
	int count = ShowCursor(TRUE);

	info = { sizeof(info) };
	::GetIconInfoExW(cursor.hCursor, &info);

	//bmpCursor = { 0 };
	//GetObject(info.hbmColor, sizeof(bmpCursor), &bmpCursor);
	GetCursorPos(&point);
	//GetPhysicalCursorPos(&point);

	if (point.x > nWidth) {
		point.x = point.x - nWidth;
	}
	else if (point.x < 0) {
		point.x = nWidth + point.x;
	}
	if (point.y > nHeight) {
		point.y = point.y - nHeight;
	}
	else if (point.y < 0) {
		point.y = nHeight + point.y;
	}
	cursor.ptScreenPos.x = point.x;
	cursor.ptScreenPos.y = point.y;

	bool res = ::DrawIconEx(hCaptureDC, point.x, point.y, cursor.hCursor, 0, 0, 0, NULL, DI_NORMAL);

	if (res == true)
		return true;
	else
		return false;
}

bool GDICaptureClass::GetScreen() {

	//clock_t start_t = clock();
	hOld = SelectObject(hCaptureDC, hBitmap);
	BitBlt(hCaptureDC, 0, 0, nWidth, nHeight, HDCC, nposx, nposy, SRCCOPY);				//Bitblt으로 이미지를 DC에 복사 한 후 바로 
					//des,							src
	//clock_t end_t = clock();
	//printf("BitBlt time = %d \n", end_t - start_t);
	GetMouse();

	//SelectObject(hCaptureDC, hOld);

	src = (BYTE*)malloc(b_size);
	//clock_t start_t1 = clock();
	if (GetDIBits(hCaptureDC, hBitmap, 0, nHeight, src, (BITMAPINFO*)&bmpInfoHeader, DIB_RGB_COLORS)) {
		//clock_t end_t1 = clock();
		//printf("DIBit time = %d \n", end_t1 - start_t1);
		//if (RGBSaveBMP(src) == true) 
			//OutputDebugStringA("success\n");
		//Cap_Release();
		//free(src);
	}

	return true;

}

void GDICaptureClass::Roop() {

	while (1)
	{
		CalFPS();
		Sleep(1);

		//clock_t start_t = clock();
		GetScreen();
		//clock_t end_t = clock();
		//printf("BitBlt time = %d \n", end_t - start_t);

	}
}

int GDICaptureClass::CalFPS() {

	static int numOfFrames;
	static clock_t oldt, nowt;

	nowt = clock();
	numOfFrames++;
	if ((nowt - oldt) / 1000 >= 1.0) {

		printf("%d fps \n", numOfFrames);
		numOfFrames = 0;
		oldt = nowt;
	}

	return 0;
	//return numOfFrames;
}

bool GDICaptureClass::RGBSaveBMP(BYTE *input) {

	BITMAPFILEHEADER bf;
	BITMAPINFOHEADER bi;

	BYTE *image = input;
	//unsigned char *image = (unsigned char*)malloc(sizeof(unsigned char)*nWidth*nHeight * 3);
	//memcpy();
	FILE *file;
	char title[1024];

	sprintf_s(title, "capture_%d.bmp", 1);
	fopen_s(&file, title, "wb");

	if (image != NULL)
	{
		if (file != NULL)
		{
			memset(&bf, 0, sizeof(bf));
			memset(&bi, 0, sizeof(bi));

			bf.bfType = 'MB';
			bf.bfSize = sizeof(bf) + sizeof(bi) + nWidth * nHeight * 4;
			bf.bfOffBits = sizeof(bf) + sizeof(bi);
			bi.biSize = sizeof(bi);
			bi.biWidth = nWidth;
			bi.biHeight = nHeight;
			bi.biPlanes = 1;
			bi.biBitCount = 32;
			bi.biSizeImage = nWidth * nHeight * 4;

			fwrite(&bf, sizeof(bf), 1, file);
			fwrite(&bi, sizeof(bi), 1, file);
			fwrite(image, sizeof(unsigned char), nHeight*nWidth * 4, file);

			fclose(file);
		}
	}
	return true;
}
