#include "pch.h"
#include "CLClass.h"

static void errorCallback(int errorCode, const char* errorDescription);
static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

static int i = 0;
//HMODULE gl_module = LoadLibrary(TEXT("opengl32.dll"));

/* A procedure for getting OpenGL functions and OpenGL or WGL extensions.
   When looking for OpenGL 1.2 and above, or extensions, it uses wglGetProcAddress,
   otherwise it falls back to GetProcAddress. */
//void* get_proc(const char *proc_name)
//{
//	void *proc = (void*)wglGetProcAddress(proc_name);
//	if (!proc) proc = (void*)GetProcAddress(gl_module, proc_name);
//
//	return proc;
//}

CLClass::CLClass()
{
}

CLClass::CLClass(HWND hwnd) : m_hWndCopy(hwnd)
{
	RECT ImageRect = { 0 , 0 , 0 , 0 };
	GetClientRect(m_hWndCopy, &ImageRect);			//윈도우 핸들 추가

	//이미지 영역 좌표를 이용하여 실제 이미지의 사이즈를 계산
	nWidth = ImageRect.right - ImageRect.left;     //윈도우 버전의 경우 사이즈 변경이 되므로 그때그때 조사
	nHeight = ImageRect.bottom - ImageRect.top;

	bool res;
	res = Cap_Init();

	if (!res)
		return;

	Roop();

}

CLClass::~CLClass()
{
	ReleaseDC(m_hWndCopy, HDCC);
	//glfwDestroyWindow(GLwindow);
}

//HWND hnd, hnda;
//TCanvas *scr;
//TBitmap *bmp;
//int x0, y0, xs, ys;
//
//void __delete()
//{
//#ifdef _mmap_h
//	mmap_del('scrc', scr);
//	mmap_del('scrc', bmp);
//#endif
//	if (scr) delete scr; scr = NULL;
//	if (bmp) delete bmp; bmp = NULL;
//}
//
//void capture()
//{
//	if (scr == NULL) return;
//	if (bmp == NULL) return;
//	bmp->Canvas->CopyRect(Rect(0, 0, xs, ys), scr, TRect(x0, y0, x0 + xs, y0 + ys));
//}
//
//void init(HWND _hnd = NULL)
//{
//	RECT r;
//	if (scr == NULL) return;
//	if (bmp == NULL) return;
//	bmp->SetSize(1, 1);
//	if (!IsWindow(_hnd)) _hnd = hnd;
//	scr->Handle = GetDC(_hnd);
//	hnda = _hnd;
//	resize();
//}
//
//void resize()
//{
//	if (!IsWindow(hnda)) return;
//	RECT r;
//	//      GetWindowRect(hnda,&r);
//	GetClientRect(hnda, &r);
//	x0 = r.left; xs = r.right - x0;
//	y0 = r.top; ys = r.bottom - y0;
//	bmp->SetSize(xs, ys);
//	xs = bmp->Width;
//	ys = bmp->Height;
//}
//
//bool CLClass::Testinit() {
//
//		hnd = NULL;
//		hnda = NULL;
//		scr = new TCanvas();
//		bmp = new Graphics::TBitmap;
//#ifdef _mmap_h
//		mmap_new('scrc', scr, sizeof(TCanvas()));
//		mmap_new('scrc', bmp, sizeof(Graphics::TBitmap));
//#endif
//		if (bmp)
//		{
//			bmp->HandleType = bmDIB;
//			bmp->PixelFormat = pf32bit;
//		}
//		x0 = 0; y0 = 0; xs = 1; ys = 1;
//		hnd = GetDesktopWindow();
//
//	return true;
//}

bool CLClass::wglInit() {			//Asynchronous read-back

	if (!glfwInit()) {
		std::cerr << "Error: GLFW 초기화 실패" << std::endl;
		exit(EXIT_FAILURE);
	}
	glGenFramebuffers
		= (PFNGLGENFRAMEBUFFERSPROC)wglGetProcAddress(
			"glGenFramebuffers");

	//HDC TDC = CreateCompatibleDC(HDCC);
	HDCC = GetDC(m_hWndCopy);
	DC = wglCreateContext(HDCC);
	wglMakeCurrent(HDCC, DC);

	glewExperimental = GL_TRUE;											//GLEW에서 이용가능한 모든 OpenGL extension에 대한 정보를 가져올 수 있도록 설정
	GLenum errorCode = glewInit();
	if (GLEW_OK != errorCode) {

		std::cerr << "Error: GLEW 초기화 실패 - " << glewGetErrorString(errorCode) << std::endl;
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	if (!GLEW_VERSION_3_3) {

		std::cerr << "OpenGL 3.3 API is not available." << std::endl;

		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	
	//Initialize();

	GLuint pbo;
	glGenBuffersARB(1, &pbo);
	glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, pbo);
	glBufferDataARB(GL_PIXEL_PACK_BUFFER_ARB, (nWidth * 3 + 3) / 4 * 4 * nHeight, NULL, GL_STREAM_READ);

	glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, pbo);
	glReadBuffer(GL_FRONT);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glReadPixels(0, 0, nWidth, nHeight, GL_BGR_EXT, GL_UNSIGNED_BYTE, 0);
	BYTE* data = (BYTE*)glMapBufferARB(GL_PIXEL_PACK_BUFFER_ARB, GL_READ_ONLY_ARB);
	if (data)	{
		glUnmapBufferARB(GL_PIXEL_PACK_BUFFER_ARB);
	}

	glBindFramebuffer(GL_PIXEL_PACK_BUFFER_ARB, 0);

	SwapBuffers(HDCC);

	glDeleteBuffers(1, &pbo);
	wglMakeCurrent(HDCC, NULL);
	wglDeleteContext(DC);

	//Initialize();
	//wglMakeCurrent(HDCC, DC);
	//
	//unsigned char *image = (unsigned char*)malloc(sizeof(unsigned char)* nWidth * nHeight * 3);
	//unsigned int pbo_userImage;						//수정
	//glGenBuffersARB(1, &pbo_userImage);
	//glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, pbo_userImage);
	//glReadBuffer(GL_BACK);
	//
	////glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo_userImage);
	//unsigned char* ptr = static_cast<unsigned char*>(glMapBuffer(GL_PIXEL_PACK_BUFFER_ARB, GL_READ_ONLY_ARB));
	//glReadPixels(0, 0, nWidth, nHeight, GL_BGR, GL_UNSIGNED_BYTE, ptr);
	//
	////std::copy(ptr, ptr + nWidth * nHeight * 3 * sizeof(unsigned char), image);
	//glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
	//glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

	//case WM_DESTROYED:   // Message when window is destroyed  
	//{
	//	HGLRC hRC        // rendering context handle  
	//		HDC   hDC;       // device context handle  
	//
	//		/* Release and free the device context and rendering context. */
	//	hDC = wglGetCurrentDC;
	//	hRC = wglGetCurrentContext;
	//
	//	wglMakeCurrent(NULL, NULL);
	//
	//	if (hRC)
	//		wglDeleteContext(hRC);
	//
	//	if (hDC)
	//		ReleaseDC(hWnd, hDC);
	//
	//	PostQuitMessage(0);
	//}
	//break;

	return true;
}

bool CLClass::Initialize() {

	glfwSetErrorCallback(errorCallback);

	if (!glfwInit()) {
		std::cerr << "Error: GLFW 초기화 실패" << std::endl;
		exit(EXIT_FAILURE);
	}
	glfwWindowHint(GLFW_DEPTH_BITS, 16);
	glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);						//OpenGL 버전  3.
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);						//OpenGL 버전  3.3
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);		//Core Profile(GLFW_OPENGL_CORE_PROFILE)	이전 버전과의 호환성을 고려하지 않고 deprecated(= 쓸모 없어지거나 대체되는 기능이 있어서 도태될 예정) 되지 않은 기능들만을 사용하여 코드 작성시 선택합니다.
																		//Compatibility Profile(GLFW_OPENGL_COMPAT_PROFILE) 이전 버전과의 호환성을 유지하기 위하여  deprecated된 기능도 같이  사용해서 코드 작성이 필요할 때 선택합니다.
																		//OpenGL 3.2 이하 버전 사용(GLFW_OPENGL_ANY_PROFILE)
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);				//지정한 OpenGL 버전에서 deprecated API는 제외시킵니다.
	//glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);							//사용자가 창 크기 변경
	glfwWindowHint(GLFW_SAMPLES, 4);									//안티 엘리어싱(멀티샘플링) 4xMSAA


	const int Monitor_count = GetMonitors();							//모니터 정보 로드			//Monitor 0>메인모니터, 1부터 서브(오른) 기준은 모름

	GLwindow = glfwCreateWindow(
		nWidth, // width
		nHeight, // height
		"OpenGL_Test", // window title
		NULL, NULL);
	if (!GLwindow) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glfwSwapInterval(1);												//V-Sync 설정
	//glfwShowWindow(GLwindow);


	if (glfwGetWindowAttrib(GLwindow, GLFW_TRANSPARENT_FRAMEBUFFER))
	{
		// 윈도우 프레임 버퍼는 현재 투명하다.
	}
	glfwSetWindowOpacity(GLwindow, 0.0f);

	auto Mode = glfwGetVideoMode(Monitor[0]);																//모드 지정
	//glfwSetWindowMonitor(GLwindow, NULL, 0, 0, Mode->width, Mode->height, Mode->refreshRate);		//전체화면 모드

	//glfwSetWindowPos(GLwindow, 0, 0);

	glfwMakeContextCurrent(GLwindow);									//GLEW를 사용하기 위해서는 항상 사용할 윈도우를 설정해줘야함. 사용할 윈도우가 바뀔떄마다 호출해야함. state machine이라서.
	glfwSetKeyCallback(GLwindow, keyCallback);
	//////////////////////////////////////////////////////////////////////GLEW Init
	glewExperimental = GL_TRUE;											//GLEW에서 이용가능한 모든 OpenGL extension에 대한 정보를 가져올 수 있도록 설정
	GLenum errorCode = glewInit();

	if (GLEW_OK != errorCode) {

		std::cerr << "Error: GLEW 초기화 실패 - " << glewGetErrorString(errorCode) << std::endl;
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	if (!GLEW_VERSION_3_3) {

		std::cerr << "OpenGL 3.3 API is not available." << std::endl;

		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	//////////////////////////////////////////////////////////////////////
	glViewport(0, 0, nWidth, nHeight);

	//std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl; //8
	//std::cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
	//std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
	//std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;

	//glBindFramebuffer(GL_FRAMEBUFFER, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE) {
		std::cerr << "Error: GLFW 초기화 실패" << std::endl;
	}

	return true;
}

bool CLClass::Initialize(HDC HDCC, HGLRC DC) {

	PIXELFORMATDESCRIPTOR pfd;

	glfwSetErrorCallback(errorCallback);

	if (!glfwInit()) {
		std::cerr << "Error: GLFW 초기화 실패" << std::endl;
		exit(EXIT_FAILURE);
	}

	memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cDepthBits = 32;
	pfd.iLayerType = PFD_MAIN_PLANE;

	int nPixelFormat = ChoosePixelFormat(HDCC, &pfd);
	SetPixelFormat(HDCC, nPixelFormat, &pfd);
	DescribePixelFormat(HDCC, nPixelFormat, sizeof(pfd), &pfd);
	wglMakeCurrent(NULL, NULL);

	//m_hRC = wglCreateContext(m_hDC);
	int attribs[] =
	{
		WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
		WGL_CONTEXT_MINOR_VERSION_ARB, 3,
		WGL_CONTEXT_FLAGS_ARB, 0,
		0
	};

	DC = wglCreateContext(HDCC);
	
	//glfwMakeContextCurrent(GLwindow);									//GLEW를 사용하기 위해서는 항상 사용할 윈도우를 설정해줘야함. 사용할 윈도우가 바뀔떄마다 호출해야함. state machine이라서.
	//glfwSetKeyCallback(GLwindow, keyCallback);
	bool res = wglMakeCurrent(HDCC, DC);									//GLEW를 사용하기 위해서는 항상 사용할 윈도우를 설정해줘야함. 사용할 윈도우가 바뀔떄마다 호출해야함. state machine이라서.
	
	glewExperimental = GL_TRUE;											//GLEW에서 이용가능한 모든 OpenGL extension에 대한 정보를 가져올 수 있도록 설정
	GLenum errorCode = glewInit();
	if (GLEW_OK != errorCode) {

		std::cerr << "Error: GLEW 초기화 실패 - " << glewGetErrorString(errorCode) << std::endl;
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	if (!GLEW_VERSION_3_3) {

		std::cerr << "OpenGL 3.3 API is not available." << std::endl;

		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	unsigned char *image = (unsigned char*)malloc(sizeof(unsigned char)*nWidth*nHeight * 3);
	unsigned int pbo_userImage;						//수정
	glGenFramebuffers(1, &pbo_userImage);

	glReadBuffer(GL_FRONT);
	glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo_userImage);
	glReadPixels(0, 0, nWidth, nHeight, GL_BGR, GL_UNSIGNED_BYTE, 0);
	unsigned char* ptr = static_cast<unsigned char*>(glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY));
	std::copy(ptr, ptr + nWidth * nHeight * 3 * sizeof(unsigned char), image);
	glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
	glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

	//glBindFramebuffer(GL_FRAMEBUFFER, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE) {
		std::cerr << "Error: GLFW 초기화 실패" << std::endl;
	}

	return true;
}

void CLClass::DumpDevice(const DISPLAY_DEVICE& dd, size_t nSpaceCount)
{
	printf("%*sDevice Name: %s\n", nSpaceCount, "", dd.DeviceName);
	printf("%*sDevice String: %s\n", nSpaceCount, "", dd.DeviceString);
	printf("%*sState Flags: %x\n", nSpaceCount, "", dd.StateFlags);
	printf("%*sDeviceID: %s\n", nSpaceCount, "", dd.DeviceID);
	printf("%*sDeviceKey: ...%s\n\n", nSpaceCount, "", dd.DeviceKey + 42);
}

bool CLClass::GetAdapter() {

	DISPLAY_DEVICE DumpD;
	DumpD.cb = sizeof(DISPLAY_DEVICE);

	DWORD deviceNum = 0;
	while (EnumDisplayDevices(NULL, deviceNum, &DumpD, 0)) {
		DumpDevice(DumpD, 0);
		DISPLAY_DEVICE newdd = { 0 };
		newdd.cb = sizeof(DISPLAY_DEVICE);
		DWORD monitorNum = 0;
		while (EnumDisplayDevices(DumpD.DeviceName, monitorNum, &newdd, 0))
		{
			DumpDevice(newdd, 4);
			monitorNum++;
		}
		puts("");
		deviceNum++;
	}

	return true;
}

//CIMAGE 굉장히 편함.
void CLClass::Capture_CImage_old() {

	clock_t start = clock();

	// 화면 전체를 캡쳐하기 위해서 Window DC 를 사용합니다. ::GetDC(NULL) 이라고 해도 결과는 동일합니다.
	HDC h_dc = ::GetWindowDC(NULL);

	// 캡쳐에 사용할 CImage 객체를 선언한다.
	CImage tips_image;

	// 수평 해상도를 얻는다.
	int cx = ::GetSystemMetrics(SM_CXSCREEN);

	// 수직 해상도를 얻는다.
	int cy = ::GetSystemMetrics(SM_CYSCREEN);

	// 화면의 색상 수를 얻는다.
	int color_depth = ::GetDeviceCaps(h_dc, BITSPIXEL);

	// 캡쳐에 사용할 이미지를 생성한다.
	tips_image.Create(cx, cy, color_depth, 0);

	// 화면 전체 이미지를 m_tips_image 객체에 복사한다. 
	::BitBlt(tips_image.GetDC(), 0, 0, cx, cy, h_dc, 0, 0, SRCCOPY);
	tips_image.BitBlt(h_dc, 0, 0);

	// 캡쳐한 이미지를 "test.png" 라는 이름으로 저장한다.
	//tips_image.Save(L"test.png", Gdiplus::ImageFormatPNG);



	// 화면 캡쳐에 사용했던 DC를 해제한다.
	::ReleaseDC(NULL, h_dc);
	tips_image.ReleaseDC();

	clock_t end = clock();

	//printf("time : %d \n", end - start);

}

bool CLClass::save_screenshot(std::string filename, int w, int h)
{

	glPixelStorei(GL_PACK_ALIGNMENT, 1);

	int nSize = w * h * 3;
	// First let's create our buffer, 3 channels per Pixel
	char* dataBuffer = (char*)malloc(nSize * sizeof(char));

	if (!dataBuffer) return false;

	// Let's fetch them from the backbuffer	
	// We request the pixels in GL_BGR format, thanks to Berzeger for the tip
	glReadPixels((GLint)0, (GLint)0,
		(GLint)w, (GLint)h,
		GL_BGR, GL_UNSIGNED_BYTE, dataBuffer);

	RGBSaveBMP();

	return true;
}

bool CLClass::Cap_Init() {

	HDCC = GetDC(m_hWndCopy);
	hCaptureDC = CreateCompatibleDC(HDCC);									//DC생성

	MyBMInfo = { 0 };
	MyBMInfo.bmiHeader.biSize = sizeof(MyBMInfo.bmiHeader);

	bmpInfoHeader = { sizeof(BITMAPINFOHEADER) };
	bmpInfoHeader.biWidth = nWidth;
	bmpInfoHeader.biHeight = nHeight;
	bmpInfoHeader.biPlanes = 1;
	bmpInfoHeader.biBitCount = 32;

	b_size = ((nWidth * bmpInfoHeader.biBitCount + 31) / 32) * 4 * nHeight;

	hBitmap = CreateCompatibleBitmap(HDCC, nWidth, nHeight);

	return true;
}

bool CLClass::Cap_Release() {

	if (src != NULL) {
		free(src);
	}
	//DeleteDC(hCaptureDC);
	//DeleteObject(hBitmap);
	//ReleaseDC(m_hWndCopy, HDCC);
	return true;
}

int CLClass::Test_j() {


	return 0;
}

int CLClass::Test_i() {

	//hBitmap = CreateCompatibleBitmap(HDCC, nWidth, nHeight);				//Bitmap 생성
	hOld = SelectObject(hCaptureDC, hBitmap);
	BitBlt(hCaptureDC, 0, 0, nWidth, nHeight, HDCC, 0, 0, SRCCOPY);

	SelectObject(hCaptureDC, hOld);
	src = (BYTE*)malloc(b_size);

	if (GetDIBits(HDCC, hBitmap, 0, nHeight, src, (BITMAPINFO*)&bmpInfoHeader, DIB_RGB_COLORS)) {
		//if (RGBSaveBMP(src) == true) 
		//	OutputDebugStringA("success\n");
		//Cap_Release();
		free(src);
	}
	//	//you can use bits here
	//	//
	//	//access bits from lower-left to upper-right corner
	//	//for (int row = 0; row < height; row++)
	//	//{
	//	//	for (int col = 0; col < width; col++)
	//	//	{
	//	//		//for 32 bit image only:
	//	//		int index = (row * width + col) * 4;
	//	//
	//	//		BYTE blue = bits[index + 0];
	//	//		BYTE green = bits[index + 1];
	//	//		BYTE red = bits[index + 2];
	//	//	}
	//	//}
	//	//
	//	////access bits from upper-left to lower-right corner
	//	//for (int y = 0; y < height; y++)
	//	//{
	//	//	for (int x = 0; x < width; x++)
	//	//	{
	//	//		int col = x;
	//	//		int row = height - y - 1;
	//	//		int index = (row * width + col) * 4;
	//	//
	//	//		BYTE b = bits[index + 0];
	//	//		BYTE g = bits[index + 1];
	//	//		BYTE r = bits[index + 2];
	//	//	}
	//	//}
	//else
	//	OutputDebugStringA("error\n");


	return 0;
}

int CLClass::GetMonitors() {
	
	int temp = 0;

	Monitor = glfwGetMonitors(&temp);
	if (temp == 0) {
		return false;
	}
	
	//Monitor = glfwGetWindowMonitor(GLwindow);

	return temp;
}

//void framebuffer_size_callback(GLFWwindow* window, int width, int height)
//{
//	glViewport(0, 0, width, height);
//}
//glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);  

bool CLClass::Roop() {
	
	static int count;

	//while (!glfwWindowShouldClose(GLwindow)) {
	while (1) {
		
		CalFPS();
		Sleep(1);

		//glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		//glClearColor(0.0f, 0.3f, 0.3f, 0.5f);
		//glClear(GL_COLOR_BUFFER_BIT);

		//std::string a = "asdf.bmp";
		//save_screenshot(a, nWidth, nHeight);

		//DrawInit();
		//bool res = RGBSaveBMP();
		//unsigned char *image = (unsigned char*)malloc(sizeof(unsigned char)*nWidth*nHeight * 3);
		//glReadPixels(0, 0, nWidth, nHeight, GL_BGR_EXT, GL_UNSIGNED_BYTE, image);
		//if (res == true) {						//39~40FPS 정도 나옴.  인코딩 제외. 파일 저장 제외.
		//	std::cout << count++ << std::endl;
		//}
		//free(image);
		//glfwSwapBuffers(GLwindow);
		//glfwPollEvents();
		
		//std::thread Th1 = std::thread(&CLClass::Test_j, this);
		//std::thread Th2 = std::thread(&CLClass::Test_i, this);
		//Test_j();
		Test_i();

		//ScreenCapture(0, 0, nWidth, nHeight);
		//Capture_CImage_old();
		//Capture_old();
	}
	//}

	return true;
}

int CLClass::CalFPS() {

	static int numOfFrames;
	static clock_t oldt, nowt;

	nowt = clock();
	//std::cout<< numOfFrames << std::endl;
	numOfFrames++;
	if ((nowt - oldt) / 1000 >= 1.0) {

		printf("%d fps \n", numOfFrames);
		numOfFrames = 0;
		oldt = nowt;
	}

	return 0;
	//return numOfFrames;
}

bool CLClass::RGBSaveBMP(BYTE *input) {

	BITMAPFILEHEADER bf;
	BITMAPINFOHEADER bi;

	BYTE *image = input;
	//unsigned char *image = (unsigned char*)malloc(sizeof(unsigned char)*nWidth*nHeight * 3);
	//memcpy();
	FILE *file;
	char title[1024];

	sprintf_s(title, "capture_%d.bmp", i++);
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
			bi.biSizeImage =nWidth * nHeight * 4;

			fwrite(&bf, sizeof(bf), 1, file);
			fwrite(&bi, sizeof(bi), 1, file);
			fwrite(image, sizeof(unsigned char), nHeight*nWidth * 4, file);

			fclose(file);
		}
		//free(image);
	}
	return true;
}

bool CLClass::RGBSaveBMP() {

	BITMAPFILEHEADER bf;
	BITMAPINFOHEADER bi;

	unsigned char *image = (unsigned char*)malloc(sizeof(unsigned char)*nWidth*nHeight * 3);
	FILE *file;
	fopen_s(&file, "capture.bmp", "wb");

	if (image != NULL)
	{
		if (file != NULL)
		{

			glReadPixels(0, 0, nWidth, nHeight, GL_BGR_EXT, GL_UNSIGNED_BYTE, image);

			memset(&bf, 0, sizeof(bf));
			memset(&bi, 0, sizeof(bi));

			bf.bfType = 'MB';
			bf.bfSize = sizeof(bf) + sizeof(bi) + nWidth * nHeight * 3;
			bf.bfOffBits = sizeof(bf) + sizeof(bi);
			bi.biSize = sizeof(bi);
			bi.biWidth = nWidth;
			bi.biHeight = nHeight;
			bi.biPlanes = 1;
			bi.biBitCount = 24;
			bi.biSizeImage = nWidth * nHeight * 3;

			fwrite(&bf, sizeof(bf), 1, file);
			fwrite(&bi, sizeof(bi), 1, file);
			fwrite(image, sizeof(unsigned char), nHeight*nWidth * 3, file);

			fclose(file);
		}
		free(image);
	}
	return true;
}

//추가
void CLClass::Capture_old() {

	//HDCC;
	//DC;

	HWND hWnd = GetDesktopWindow();
	HDC hdc = GetDC(hWnd);

	RECT rect;
	GetWindowRect(hWnd, &rect);

	int MAX_WIDTH = rect.right - rect.left;
	int MAX_HEIGHT = rect.bottom - rect.top;

	//cout << "MAX_WIDTH " << MAX_WIDTH << " MAX_HEIGHT " << MAX_HEIGHT << endl;

	HDC hdcTemp = CreateCompatibleDC(hdc);

	BITMAPINFO bitmap;
	bitmap.bmiHeader.biSize = sizeof(bitmap.bmiHeader);
	bitmap.bmiHeader.biWidth = MAX_WIDTH;
	bitmap.bmiHeader.biHeight = -MAX_HEIGHT;
	bitmap.bmiHeader.biPlanes = 1;
	bitmap.bmiHeader.biBitCount = 32;
	bitmap.bmiHeader.biCompression = BI_RGB;
	bitmap.bmiHeader.biSizeImage = 0;
	bitmap.bmiHeader.biClrUsed = 0;
	bitmap.bmiHeader.biClrImportant = 0;

	LPRGBQUAD bitPointer;
	HBITMAP hBitmap2 = CreateDIBSection(hdcTemp, &bitmap, DIB_RGB_COLORS, (void**)&bitPointer, 0, 0);

	HBITMAP hbmpOld = (HBITMAP)SelectObject(hdcTemp, hBitmap2);
	BitBlt(hdcTemp, 0, 0, MAX_WIDTH, MAX_HEIGHT, hdc, 0, 0, SRCCOPY);
	
	//pDC->SetStretchBltMode(HALFTONE);
	//::StretchDIBits(pDC->m_hDC, 0, 0, 640, 480, (BITMAPINFO*)&bmi, DIB_RGB_COLORS, SRCCOPY);

	//SelectObject(hdcTemp, hbmpOld);

}

static void errorCallback(int errorCode, const char* errorDescription){
	fprintf(stderr, "Error: %s\n", errorDescription);
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods){
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
}


