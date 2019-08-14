////////////////////////////////////////////////////////////////////////////////
// Filename: systemclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "systemclass.h"

TCHAR g_szClassName[] = TEXT("ConsoleWindow");
TCHAR g_szWindowName[] = TEXT("ConsoleWindow Example");
bool Show_Debug = true;

SystemClass::SystemClass(FramequeueClass* src_queue, DecoderType Type) : selectdecode(Type), m_queue(src_queue), Pend(false), Render_FPS(0), timecheck(0), m_Show_Debug(true) {
	m_keys[256] = 0;
	m_D3D = new D3DClass();
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam)
{
	int len = 0;
	static char str[256];
	static string userinput;
	LPCWSTR ff_name = 0;
	FILE *stream;
	wstring wstr = L"";
	switch (umessage) {
		// Check if the window is being destroyed.
	case WM_CREATE:
		AllocConsole();
		SetConsoleTitle(TEXT("Debug"));
		//GetConsoleOriginalTitle(lpConsoleTitle, IN DWORD nSize);
		SetConsoleCP(GetConsoleCP());
		SetConsoleOutputCP(GetConsoleOutputCP());
		freopen_s(&stream,("CONOUT$"), ("w"), stdout);
		freopen_s(&stream,("CONIN$"), ("r"), stdin);
		freopen_s(&stream,("CONERR$"), ("w"), stderr);
		//_tsetlocale(LC_ALL, _T(""));
		return 0;
	case WM_DESTROY:
	{
		FreeConsole();
		PostQuitMessage(0);
		return 0;
	}
	case WM_CHAR:
	{
		if(wparam == 9){
			if (Show_Debug == true)
				Show_Debug = false;
			else
				Show_Debug = true;
			return 0;
		}
	}

	//case WM_CHAR:
	//{
	//	if ((int)strlen(str) >= 0) {
	//		len = (int)strlen(str);
	//		str[len] = (char)wparam;
	//		str[len + 1] = 0;
	//		userinput = str;
	//		if (userinput.at(len) == 8 ) {
	//			if (userinput.at(0) == '\b') {
	//				userinput = "";
	//				return 0;
	//			}
	//			userinput.pop_back();
	//			userinput.pop_back();
	//			strcpy_s(str, userinput.c_str());
	//			return 0;
	//		}
	//		if ((userinput.at(len) != 13) && check < 2) {
	//			userinput = str;					//스트링으로 받고
	//			//wstr = wstring(userinput.begin(), userinput.end()).c_str();//스트링 w스트링변환
	//			//ff_name = wstr.c_str();
	//			//MessageBox(hwnd, ff_name, ff_name, MB_OK);
	//			check = 1;
	//			return 0;
	//		}
	//		else if ((userinput.at(len) == 13) && check == 1) {
	//			//MessageBox(hwnd, L"출력합니다.", L"완료", MB_OK);
	//			userinput = str;					//스트링으로 받고
	//			userinput.pop_back();
	//			while (1) {
	//				int location = userinput.find('\b');
	//				if (location == -1)
	//					break;
	//				userinput.erase(0, 1);
	//			}
	//			wstr = wstring(userinput.begin(), userinput.end()).c_str();//스트링 w스트링변환
	//			ff_name = wstr.c_str();
	//			//MessageBox(hwnd, ff_name, L"경로", MB_OK);
	//			check = 2;
	//			file_t = userinput;
	//			return 0;
	//		}
	//	}
	//	return 0;
	//}
	// Check if the window is being closed.
	case WM_CLOSE:
	{
		PostQuitMessage(0);
		return 0;
	}
	// All other messages pass to the message handler in the system class.
	default:
	{
		return ApplicationHandle->MessageHandler(hwnd, umessage, wparam, lparam);
	}
	}
}

int SystemClass::userinput(string &filename) {
	MSG msg;
	bool done = false;
	ZeroMemory(&msg, sizeof(MSG));

	while (!done) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (msg.message == WM_QUIT)
			done = true;
		else {
			if (IsKeyDown(VK_ESCAPE))
			{
				Initializeinput();
				done = true;

			}
		}
	}

	return 0;
}

LRESULT CALLBACK SystemClass::MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
	switch (umsg)
	{
		// Check if a key has been pressed on the keyboard.
	case WM_KEYDOWN:
	{
		// If a key is pressed send it to the input object so it can record that state.
		this->KeyDown((unsigned int)wparam);
		return 0;
	}

	// Check if a key has been released on the keyboard.
	case WM_KEYUP:
	{
		// If a key is released then send it to the input object so it can unset the state for that key.
		this->KeyUp((unsigned int)wparam);
		return 0;
	}

	// Any other messages send to the default message handler as our application won't make use of them.
	default:
	{
		return DefWindowProc(hwnd, umsg, wparam, lparam);
	}
	}
}

void SystemClass::Initializeinput() {
	// Initialize all the keys to being released and not pressed.
	for (auto i = 0; i < 256; i++) {
		m_keys[i] = false;
	}
	return;
}

void SystemClass::KeyDown(unsigned int input) {
	// If a key is pressed then save that state in the key array.
	m_keys[input] = true;
	return;
}

void SystemClass::KeyUp(unsigned int input) {
	// If a key is released then clear that state in the key array.
	m_keys[input] = false;
	return;
}

bool SystemClass::IsKeyDown(unsigned int key) {
	// Return what state the key is in (pressed/not pressed).
	return m_keys[key];
}

HWND SystemClass::InitializeWindows()
{
	WNDCLASSEX wc;
	int posX, posY;
	int screenWidth, screenHeight = 0;

	// 외부 포인터를 여기로 설정
	ApplicationHandle = this;
	// 지금 어플의 인스턴스를 설정함
	m_hinstance = GetModuleHandle(NULL);
	// Give the application a name.
	m_applicationName = L"Hello~Hun";

	// Setup the windows class with default settings.
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = m_hinstance;
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hIconSm = wc.hIcon;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = m_applicationName;
	wc.cbSize = sizeof(WNDCLASSEX);

	// Register the window class.
	RegisterClassEx(&wc);

	// Determine the resolution of the clients desktop screen.
	RECT desktop;								//pixel per inch		//해상도 == 픽셀 밀도 단위
	SetProcessDPIAware();						//Dot per inch
	m_hwnd = GetDesktopWindow();				//dpi비율을 계산해 윈도우 해상도를 가져온다
	GetWindowRect(m_hwnd, &desktop);
	screenWidth = desktop.right;	screenHeight = desktop.bottom;
	//screenWidth = (int)(GetSystemMetricsForDpi(SM_CXSCREEN, GetDpiForWindow(m_hwnd)));// *1.25);			//DPI(텍스트, 앱 및 기타 항목의 크기 변경 비율에 따라 달라짐)
	//screenHeight = (int)(GetSystemMetricsForDpi(SM_CYSCREEN, GetDpiForWindow(m_hwnd)));// *1.25);	

	// 화면 중앙에 위치
	posX = (GetSystemMetrics(SM_CXSCREEN) - screenWidth) / 2;
	posY = (GetSystemMetrics(SM_CYSCREEN) - screenHeight) / 2;

	// 설정값으로 윈도우를 만들고 핸들을 연결
	m_hwnd = CreateWindowEx(WS_EX_APPWINDOW, m_applicationName, m_applicationName,
		WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP,
		posX, posY, screenWidth, screenHeight, NULL, NULL, m_hinstance, NULL);

	// 윈도우에 포커싱을 줌
	ShowWindow(m_hwnd, SW_SHOW);
	SetForegroundWindow(m_hwnd);
	SetFocus(m_hwnd);

	// Hide the mouse cursor.
	ShowCursor(true);
	m_D3D->d3d_hwnd = m_hwnd;
	timer_set();

	return m_hwnd;
}

bool SystemClass::Close() {

	m_D3D->Shutdown();

	delete m_D3D;
	m_D3D = NULL;

	// Show the mouse cursor.
	ShowCursor(true);

	// Fix the display settings if leaving full screen mode.
	if (FULL_SCREEN)
		ChangeDisplaySettings(NULL, 0);

	// Remove the window.
	DestroyWindow(m_hwnd);
	m_hwnd = NULL;

	// Remove the application instance.
	UnregisterClass(m_applicationName, m_hinstance);
	m_hinstance = NULL;

	// Release the pointer to this class.
	ApplicationHandle = NULL;
	
	return true;
}

void SystemClass::DeleteFrame(AVFrame* temp) {
	if (temp != NULL) {

		av_frame_unref(temp);
		av_frame_free(&temp);

		//av_free(&temp);		//이거 함부러 쓰는거 아님.
		//av_freep(&temp->data[0]);
	}
	return ;
}

void SystemClass::Run() {
	MSG msg;	bool done = false;	
	bool Debug_change_check = false;
	int PoPCount = 0;	
	float sleeptime = 0;	float RealFPStemp = 0;	float RenderCycle = 0;
	AVFrame* VFtemp = NULL;
	clock_t Render_start, Render_end;
	Render_start = 0; Render_end = 0;
	ZeroMemory(&msg, sizeof(MSG));

	while (!done) {
		if (Show_Debug == true && Debug_change_check == false) {
			Debug_change_check = m_D3D->Debug_position(Show_Debug);
			m_Show_Debug = Show_Debug;
		}
		else if (Show_Debug == false && Debug_change_check == true) {
			Debug_change_check = m_D3D->Debug_position(Show_Debug);
			m_Show_Debug = Show_Debug;
		}

		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (msg.message == WM_QUIT)
			done = true;
		else {
			if (IsKeyDown(VK_ESCAPE)) {
				Initializeinput();
				done = true;
			}
		}
		while (m_queue->q_PushCount == 0) {
			if (m_queue->push_end == true)
				return;
			Sleep(10);
			m_D3D->Megabitrate = m_queue->CallBitrate();
		}

		if (m_queue->VFrame->empty() == false) {
			DeleteFrame(VFtemp);
			VFtemp = m_queue->PoPFrame();
			Render(VFtemp);
			PoPCount++;
			Render_end = clock();												//이 순서 중요
			RenderCycle = (float)(Render_end - Render_start);
			Render_start = clock();
			Sleep_time_control(RenderCycle, RealFPStemp, sleeptime);			//렌더 사이클을 가지고 Sleep 양을 조절하기때문에 clock위치 중요.
			m_D3D->Megabitrate = m_queue->CallBitrate();															//클락 스타트가 이거 아래로 내려오면 x	(sleep 시간을 제외하고 계산해버림)
		}
		else {
			printf("\n\n ----------Render Delayed----------- \n\n");
			Sleep(50);
			Render_end = clock();
			RenderCycle = (float)(Render_end - Render_start);
			Render_start = clock();
			if (m_queue->push_end == true && PoPCount == m_queue->q_PushCount) {
				DeleteFrame(VFtemp);
				break;
			}
		}
	}
	m_queue->CallEnd(true);
	return ;
}

void SystemClass::Render(AVFrame* VFtemp) {

	m_D3D->queuesize = m_queue->VFrame->size();
	if (selectdecode == CPU) {
		if (m_D3D->Update_Tex_YUV420P(VFtemp) == true)
			Render_FPS = m_D3D->Render(m_Show_Debug);
	}
	else {
		if(m_D3D->Update_Tex_NV12(VFtemp) == true) 
		//if (m_D3D->Update_Tex_RGB(VFtemp) == true)
			Render_FPS = m_D3D->Render(m_Show_Debug);
	}
		
	return;
}

void SystemClass::Sleep_time_control(float RenderCycle, float &RealFPStemp, float &sleeptime) {
	
	static int av_count;	static float av_sleeptime;

	if (RealFPStemp == 0) {
		RealFPStemp = m_queue->CallFPS();
		m_D3D->VideoFPS = RealFPStemp;
		sleeptime = 1 / RealFPStemp * 1000;
	}

	float temp = 1 / RealFPStemp * 1000;
	float Control_value = (1 / RealFPStemp) / 2;		//기존 Sleep 만큼씩 증감

	av_count++;
	av_sleeptime += RenderCycle;
	if (av_count == 3) {
		av_count = 0;
		av_sleeptime = av_sleeptime / 4;
		float a = round(av_sleeptime * 10) / 10;
		float b = round(temp * 10) / 10;
		//float dif = (int)round(av_sleeptime) - (int)round(temp);
		float dif = a - b;
		if (dif < 0) 
			sleeptime += Control_value / 2;
		else if	(dif == 0) 
			{}
		else		
			sleeptime -= Control_value / 2;
	}

	float diff = Render_FPS - round(RealFPStemp);
	if (diff == 0) {}
	else if (diff > 0) {
		sleeptime += Control_value / 2;
		if (diff > 4)
			sleeptime += Control_value / 2;
		if (diff > 8)
			sleeptime += Control_value * 8;
	}
	else {
		sleeptime -= Control_value / 2;
		if (diff < -4)
			sleeptime -= Control_value / 2;
		if (diff < -8)
			sleeptime -= Control_value * 8;
	}

	m_queue->q_sleeptime = sleeptime;
	m_D3D->sleeptime = sleeptime;
	m_D3D->RenderCycle = RenderCycle;

	//printf("Rendercycle : %.0f\n\n", RenderCycle);
	if (sleeptime < 1)
		sleeptime = 1;
	Sleep(sleeptime);

	return;
}

bool SystemClass::timer_set() {

	QueryPerformanceFrequency((LARGE_INTEGER*)&m_frequency);
	if (m_frequency == 0) {
		return false;
	}
	m_ticksPerMs = (float)(m_frequency / 1000);
	QueryPerformanceCounter((LARGE_INTEGER*)&m_startTime);

	return true;
}

bool SystemClass::ClearQueue() {		//혹시남아있는 큐의 잔여데이터를 제거
	if (m_queue->VFrame->size()) {
		while (m_queue->VFrame->size() > 0) {
			av_freep(&m_queue->VFrame->front());
			av_free(m_queue->VFrame->front());
			m_queue->VFrame->pop();
		}
	}

	return true;
}

bool SystemClass::Initialize3D() {
	
	bool result = 0;

	result = m_D3D->InitializeBuffers3D(m_hwnd, selectdecode);
	if (!result) {
		//m_D3D->log->LogFile("Could not initialize the texture shader object.");
		return false;
	}
	return true;
}
