// DXCapture.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "pch.h"
#include "DXCapture.h"

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

DXC::DXC() {
	InitCap();

}

DXC::~DXC() {

	Release();
}

void Check(HRESULT hr) {

	if (FAILED(hr))
	{
		std::cerr << "Create Device Error" << std::endl;
		return;
	}
}

void DXC::InitCap() {

	HRESULT hr = E_FAIL;

	int Monitor_index = 0;

	std::vector<IDXGIAdapter*> AdapterList = GetAdapter();
	hr = CreateDevice(AdapterList, Monitor_index);
	Check(hr);
	hr = InitResource(AdapterList, Monitor_index);
	Check(hr);
	hr = Capture();
	Check(hr);

	return;
}

std::vector<IDXGIAdapter*> DXC::GetAdapter() {

	IDXGIAdapter * pAdapter;
	std::vector <IDXGIAdapter*> vAdapters;
	IDXGIFactory* pFactory = NULL;

	// Create a DXGIFactory object.
	if (FAILED(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&pFactory))){
		return vAdapters;
	}

	for (UINT i = 0;
		pFactory->EnumAdapters(i, &pAdapter) != DXGI_ERROR_NOT_FOUND;
		++i){
		vAdapters.push_back(pAdapter);
	}

	if (pFactory){
		pFactory->Release();
	}

	return vAdapters;
}

HRESULT DXC::InitResource(std::vector<IDXGIAdapter*> AdapterList, int Monitor_index) {

	HRESULT hr = E_FAIL;
	//IDXGIDevice* lDxgiDevice;
	//hr = lDevice->QueryInterface(IID_PPV_ARGS(&lDxgiDevice));
	//Check(hr);

	IDXGIOutput* lDxgiOutput;
	IDXGIAdapter* Adaptertemp = AdapterList.at(Monitor_index);
	hr = Adaptertemp->EnumOutputs(0, &lDxgiOutput);
	Check(hr);
	if (Adaptertemp)
		Adaptertemp->Release();

	//hr = lDxgiOutput->GetDesc(&lOutputDesc);
	//Check(hr);

	IDXGIOutput1* Outputtemp;
	hr = lDxgiOutput->QueryInterface(IID_PPV_ARGS(&Outputtemp));
	Check(hr);
	if (lDxgiOutput)
		lDxgiOutput->Release();

	hr = Outputtemp->DuplicateOutput(lDevice, &lDeskDupl);
	Check(hr);
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
	Due_desc.BindFlags = D3D11_BIND_RENDER_TARGET;
	Due_desc.SampleDesc.Count = 1;
	Due_desc.SampleDesc.Quality = 0;
	Due_desc.Usage = D3D11_USAGE_DEFAULT;
	Due_desc.MipLevels = 1;
	Due_desc.CPUAccessFlags = 0;
	Due_desc.MiscFlags = D3D11_RESOURCE_MISC_GDI_COMPATIBLE;

	hr = lDevice->CreateTexture2D(&Due_desc, NULL, &lGDIImage);
	Check(hr);
	if (lGDIImage == nullptr)
		return E_FAIL;

	D3D11_TEXTURE2D_DESC CPUdesc;

	CPUdesc.Width = lOutputDuplDesc.ModeDesc.Width;
	CPUdesc.Height = lOutputDuplDesc.ModeDesc.Height;
	CPUdesc.MipLevels = 1;
	CPUdesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	CPUdesc.ArraySize = 1;
	CPUdesc.SampleDesc.Count = 1;
	CPUdesc.SampleDesc.Quality = 0;
	CPUdesc.Usage = D3D11_USAGE_STAGING;
	CPUdesc.BindFlags = 0;
	CPUdesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
	CPUdesc.MiscFlags = 0;

	hr = lDevice->CreateTexture2D(&CPUdesc, NULL, &lDestImage);
	Check(hr);
	if (lDestImage == nullptr)
		return E_FAIL;

	return hr;
}

HRESULT DXC::CreateDevice(std::vector<IDXGIAdapter*> AdapterList, int Monitor_index) {

	HRESULT hr = E_FAIL;
	D3D_FEATURE_LEVEL lFeatureLevel;


	//IDXGIAdapter* tempAdap = AdapterList.at(Monitor_index);

	hr = D3D11CreateDevice(
		NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		0,
		gFeatureLevels,
		gNumFeatureLevels,
		D3D11_SDK_VERSION,
		&lDevice,
		&lFeatureLevel,
		&lImmediateContext
	);

	Check(hr);

	//if(tempAdap)
	//	tempAdap->Release();

	//if (lDevice)
	//	lDevice->Release();
	if (lImmediateContext)
		lImmediateContext->Release();

	return hr;
}

void DXC::Release() {


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

	return;
}


HRESULT DXC::Capture() {

	HRESULT hr = S_OK;

	IDXGIResource* DesktopResource = NULL;
	DXGI_OUTDUPL_FRAME_INFO FrameInfo;

	//Get new frame
	hr = lDeskDupl->AcquireNextFrame(500, &FrameInfo, &DesktopResource);
	if (FAILED(hr))
	{
		if ((hr != DXGI_ERROR_ACCESS_LOST) && (hr != DXGI_ERROR_WAIT_TIMEOUT))
		{
			std::cerr << "Failed to acquire next frame in DUPLICATIONMANAGER" << std::endl;
		}
		return hr;
	}

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
		std::cerr << "Failed to QI for ID3D11Texture2D from acquired IDXGIResource in DUPLICATIONMANAGER" << std::endl;
		return hr;
	}
	
	return hr;
}