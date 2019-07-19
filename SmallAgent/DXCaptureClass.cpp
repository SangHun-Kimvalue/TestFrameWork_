// DXCapClassapture.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "pch.h"
#include "DXCaptureClass.h"

DXCapClass::DXCapClass() {

	HRESULT hr;

	InitCap();

}

DXCapClass::~DXCapClass() {

	Release();
}

void DXCapClass::InitCap() {

	HRESULT hr = E_FAIL;

	int Monitor_index = 1;

	//GetAdapter();

	hr = CreateDevice();
	Check(hr,"CreateDevice");
	hr = InitResource(Monitor_index);
	Check(hr, "InitResource");

	return;
}

HRESULT DXCapClass::GetAdapter() {

	IDXGIAdapter * pAdapter;

	IDXGIFactory* pFactory = NULL;


	// Create a DXGIFactory object.
	if (FAILED(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&pFactory))) {
		return E_FAIL;
	}

	for (UINT i = 0;
		pFactory->EnumAdapters(i, &pAdapter) != DXGI_ERROR_NOT_FOUND;
		++i) {
		//AdapterList.push_back(pAdapter);
	}

	IDXGIOutput *ppOutput;
	std::vector<IDXGIOutput*> OutputList;
	//for (UINT i = 0; AdapterList.at(0)->EnumOutputs(i, &ppOutput) != DXGI_ERROR_NOT_FOUND; ++i) {
	//	OutputList.push_back(ppOutput);
	//}

	if (pFactory){
		pFactory->Release();
	}

	return S_OK;
}

HRESULT DXCapClass::CreateDevice() {

	HRESULT hr = E_FAIL;
	D3D_FEATURE_LEVEL lFeatureLevel;

	D3D_DRIVER_TYPE gDriverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE
	};
	UINT gNumDriverTypes = ARRAYSIZE(gDriverTypes);

	// Feature levels supported
	D3D_FEATURE_LEVEL gFeatureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_1
	};

	UINT gNumFeatureLevels = ARRAYSIZE(gFeatureLevels);


	//IDXGIAdapter* tempAdap = AdapterList.at(Monitor_index);

	for (UINT DriverTypeIndex = 0; DriverTypeIndex < gNumDriverTypes; ++DriverTypeIndex)
	{

		hr = D3D11CreateDevice(
			nullptr,
			gDriverTypes[DriverTypeIndex],
			nullptr,
			0,
			gFeatureLevels,
			gNumFeatureLevels,
			D3D11_SDK_VERSION,
			&lDevice,
			&lFeatureLevel,
			&lImmediateContext);

		if (SUCCEEDED(hr))
		{
			// Device creation success, no need to loop anymore
			break;
		}

		lDevice->Release();

		lImmediateContext->Release();
	}

	if (FAILED(hr))
		return hr;

	Sleep(100);

	if (lDevice == nullptr)
		return hr;

	Check(hr, "D3D11CreateDevice");

	// Get DXGI device
	IDXGIDevice* lDxgiDevice;

	hr = lDevice->QueryInterface(IID_PPV_ARGS(&lDxgiDevice));

	if (FAILED(hr))
		return hr;

	// Get DXGI adapter

	hr = lDxgiDevice->GetParent(
		__uuidof(IDXGIAdapter),
		reinterpret_cast<void**>(&lDxgiAdapter));

	if (FAILED(hr))
		return hr;

	lDxgiDevice->Release();

	//AdapterList.push_back(lDxgiAdapter);

	return hr;
}

HRESULT DXCapClass::InitResource(int index) {

	HRESULT hr = E_FAIL;

	UINT Output = index;

	IDXGIOutput* lDxgiOutput;
	IDXGIAdapter* Adaptertemp = lDxgiAdapter;
	hr = Adaptertemp->EnumOutputs(Output, &lDxgiOutput);
	Check(hr, "EnumOutputs");
	if (Adaptertemp)
		Adaptertemp->Release();

	hr = lDxgiOutput->GetDesc(&lOutputDesc);
	Check(hr, "lDxgiOutput->GetDesc");

	IDXGIOutput1* Outputtemp;
	hr = lDxgiOutput->QueryInterface(IID_PPV_ARGS(&Outputtemp));
	Check(hr, "lDxgiOutput->QueryInterface");
	if (lDxgiOutput)
		lDxgiOutput->Release();

	hr = Outputtemp->DuplicateOutput(lDevice, &lDeskDupl);
	Check(hr, "Outputtemp->DuplicateOutput");
	if (Outputtemp) {
		Outputtemp->Release();
		Outputtemp = NULL;
	}

	lDeskDupl->GetDesc(&lOutputDuplDesc);
	
	D3D11_TEXTURE2D_DESC Due_desc;

	Due_desc.Width = lOutputDuplDesc.ModeDesc.Width;
	Due_desc.Height = lOutputDuplDesc.ModeDesc.Height;
	Due_desc.Format = lOutputDuplDesc.ModeDesc.Format;		//DXGI_FORMAT_R8G8B8A8_UNORM
	Due_desc.ArraySize = 1;
	Due_desc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_RENDER_TARGET;
	Due_desc.SampleDesc.Count = 1;
	Due_desc.SampleDesc.Quality = 0;
	Due_desc.Usage = D3D11_USAGE_DEFAULT;
	Due_desc.MipLevels = 1;
	Due_desc.CPUAccessFlags = 0;
	Due_desc.MiscFlags = D3D11_RESOURCE_MISC_GDI_COMPATIBLE;

	hr = lDevice->CreateTexture2D(&Due_desc, NULL, &lGDIImage);
	Check(hr, "lDevice->CreateTexture2D(&Due_desc, NULL, &lGDIImage);");
	if (lGDIImage == nullptr)
		return E_FAIL;

	D3D11_TEXTURE2D_DESC CPUdesc;

	CPUdesc.Width = lOutputDuplDesc.ModeDesc.Width;
	CPUdesc.Height = lOutputDuplDesc.ModeDesc.Height;
	CPUdesc.MipLevels = 1;
	CPUdesc.Format = lOutputDuplDesc.ModeDesc.Format;
	CPUdesc.ArraySize = 1;
	CPUdesc.SampleDesc.Count = 1;
	CPUdesc.SampleDesc.Quality = 0;
	CPUdesc.Usage = D3D11_USAGE_STAGING;
	CPUdesc.BindFlags = 0;
	CPUdesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
	CPUdesc.MiscFlags = 0;

	hr = lDevice->CreateTexture2D(&CPUdesc, NULL, &lDestImage);
	Check(hr, "lDevice->CreateTexture2D(&CPUdesc, NULL, &lDestImage);");
	if (lDestImage == nullptr)
		return E_FAIL;

	return hr;
}

void DXCapClass::Release() {


	if (lDevice) {
		lDevice->Release();
		lDevice = NULL;
	}

	if (lImmediateContext) {
		lImmediateContext->Release();
		lImmediateContext = NULL;
	}

	if (lDeskDupl) {
		lDeskDupl->Release();
		lDeskDupl = NULL;
	}

	if (lAcquiredDesktopImage) {
		lAcquiredDesktopImage->Release();
		lAcquiredDesktopImage = NULL;
	}

	if (lGDIImage) {
		lGDIImage->Release();
		lGDIImage = NULL;
	}

	if (lDestImage) {
		lDestImage->Release();
		lDestImage = NULL;
	}

	//if (AdapterList.empty() == false) {
	//	for(int i = 0 ; i < AdapterList.size(); i++)
	//		AdapterList.at(i)->Release();
	//	AdapterList.clear();
	//}

	return;
}

HRESULT DXCapClass::Capture(std::shared_ptr<BYTE> I_data) {

	HRESULT hr = S_OK;

	IDXGIResource* DesktopResource = NULL;
	DXGI_OUTDUPL_FRAME_INFO FrameInfo;

	int lTryCount = 4;
	do
	{
		Sleep(100);
		//Get new frame
		hr = lDeskDupl->AcquireNextFrame(500, &FrameInfo, &DesktopResource);

		if (SUCCEEDED(hr))
			break;

		if (hr == DXGI_ERROR_WAIT_TIMEOUT)
		{
			continue;
		}

		else if (FAILED(hr))
			break;

	} while (--lTryCount > 0);

	// If still holding old frame, destroy it
	if (lAcquiredDesktopImage)
	{
		lAcquiredDesktopImage->Release();
		lAcquiredDesktopImage = NULL;
	}

	// QI for IDXGIResource
	hr = DesktopResource->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void **>(&lAcquiredDesktopImage));
	DesktopResource->Release();
	DesktopResource = NULL;
	if (FAILED(hr))
	{
		std::cerr << "DesktopResource->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void **>(&lAcquiredDesktopImage));" << std::endl;
		return hr;
	}

	if (lAcquiredDesktopImage == nullptr) {
		std::cerr << "lAcquiredDesktopImage Capture Error" << std::endl;
		return E_FAIL;
	}

	lImmediateContext->CopyResource(lGDIImage, lAcquiredDesktopImage);
	
	IDXGISurface1* lIDXGISurface1;
	
	hr = lGDIImage->QueryInterface(IID_PPV_ARGS(&lIDXGISurface1)); 
	if (FAILED(hr))
	{
		std::cerr << "QueryInterface(IID_PPV_ARGS(&lIDXGISurface1));" << std::endl;
		return hr;
	}

	CURSORINFO lCursorInfo = { 0 };
	lCursorInfo.cbSize = sizeof(lCursorInfo);

	auto lBoolres = GetCursorInfo(&lCursorInfo);
	if (lBoolres == TRUE)
	{
		if (lCursorInfo.flags == CURSOR_SHOWING)
		{
			auto lCursorPosition = lCursorInfo.ptScreenPos;

			auto lCursorSize = lCursorInfo.cbSize;

			HDC  lHDC;

			lIDXGISurface1->GetDC(FALSE, &lHDC);

			DrawIconEx(
				lHDC,
				lCursorPosition.x,
				lCursorPosition.y,
				lCursorInfo.hCursor,
				0,
				0,
				0,
				0,
				DI_NORMAL | DI_DEFAULTSIZE);

			lIDXGISurface1->ReleaseDC(nullptr);
		}

	}
	lIDXGISurface1->Release();

	// Copy image into CPU access texture
	lImmediateContext->CopyResource(lDestImage, lGDIImage);

	D3D11_MAPPED_SUBRESOURCE resource;
	lImmediateContext->Map(lDestImage, 0, D3D11_MAP_READ_WRITE, 0, &resource);

	BYTE* sptr = reinterpret_cast<BYTE*>(resource.pData);
	memcpy(I_data.get(), sptr, resource.RowPitch * lOutputDuplDesc.ModeDesc.Height);

	lImmediateContext->Unmap(lDestImage, 0);
	lDeskDupl->ReleaseFrame();

	Savetobmp(lOutputDuplDesc.ModeDesc.Width, lOutputDuplDesc.ModeDesc.Height, I_data.get());

	return hr;

}

bool DXCapClass::Savetobmp(int wid, int hei, unsigned char * sptr) {

	BITMAPINFO	lBmpInfo;

	// BMP 32 bpp

	ZeroMemory(&lBmpInfo, sizeof(BITMAPINFO));

	lBmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);

	lBmpInfo.bmiHeader.biBitCount = 32;

	lBmpInfo.bmiHeader.biCompression = BI_RGB;

	lBmpInfo.bmiHeader.biWidth = wid;

	lBmpInfo.bmiHeader.biHeight = hei;

	lBmpInfo.bmiHeader.biPlanes = 1;

	lBmpInfo.bmiHeader.biSizeImage = wid * hei * 4;

	UINT lBmpRowPitch = wid * 4;
	std::unique_ptr<BYTE> pBuf(new BYTE[lBmpInfo.bmiHeader.biSizeImage]);
	BYTE* dptr = pBuf.get() + lBmpInfo.bmiHeader.biSizeImage - lBmpRowPitch;



	UINT lRowPitch = std::min<UINT>(lBmpRowPitch, wid * 4);


	for (size_t h = 0; h < hei; ++h)
	{

		memcpy_s(dptr, lBmpRowPitch, sptr, lRowPitch);
		sptr += wid * 4;
		dptr -= lBmpRowPitch;
	}

	// Save bitmap buffer into the file ScreenShot.bmp

	//WCHAR lMyDocPath[MAX_PATH];

	//hr = SHGetFolderPath(nullptr, CSIDL_PERSONAL, nullptr, SHGFP_TYPE_CURRENT, lMyDocPath);

	std::wstring lMyDocPath = L"D:\\Visual_Local_Repo\\SmallAgent";

	std::wstring lFilePath = std::wstring(lMyDocPath) + L"\\ScreenShot.bmp";

	FILE* lfile = nullptr;

	auto lerr = _wfopen_s(&lfile, lFilePath.c_str(), L"wb");

	if (lerr != 0)
		return E_FAIL;

	if (lfile != nullptr)
	{

		BITMAPFILEHEADER	bmpFileHeader;

		bmpFileHeader.bfReserved1 = 0;
		bmpFileHeader.bfReserved2 = 0;
		bmpFileHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + lBmpInfo.bmiHeader.biSizeImage;
		bmpFileHeader.bfType = 'MB';
		bmpFileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

		fwrite(&bmpFileHeader, sizeof(BITMAPFILEHEADER), 1, lfile);
		fwrite(&lBmpInfo.bmiHeader, sizeof(BITMAPINFOHEADER), 1, lfile);
		fwrite(pBuf.get(), lBmpInfo.bmiHeader.biSizeImage, 1, lfile);

		fclose(lfile);

		//ShellExecute(0, 0, lFilePath.c_str(), 0, 0, SW_SHOW);

		//lresult = 0;
		std::string temp;
		temp.assign(lFilePath.begin(), lFilePath.end());
		std::cout << "Save File to bmp : \t" << temp.c_str() << std::endl;
	}
	return true;
}