#include "pch.h"
#include "NVEClass.h"


NVEClass::NVEClass(int wid, int hei) : nwid(wid), nhei(hei){
	Init(nwid, nhei);
}


NVEClass::~NVEClass()
{
	Release();
}

HRESULT NVEClass::Init(int wid, int hei) {

	int iGpu;
	HRESULT hr = E_FAIL;

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
	D3D_FEATURE_LEVEL eFeatureLevel;

	//CreateDXGIFactory1(__uuidof(IDXGIFactory1);
	//if (FAILED(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&iFactory))) {
	//	return E_FAIL;
	//}
	//hr = iFactory->EnumAdapters(0, &iAdapter);
	//if (FAILED(hr)) {
	//	return E_FAIL;
	//}

	hr = D3D11CreateDevice(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		0,
		gFeatureLevels,
		gNumFeatureLevels,
		D3D11_SDK_VERSION,
		&eDevice,
		&eFeatureLevel,
		&eImmediateContext);
	Check(hr, "eDevice  D3D11CreateDevice");

	IDXGIDevice* lDxgiDevice;
	hr = eDevice->QueryInterface(IID_PPV_ARGS(&lDxgiDevice));
	if (FAILED(hr))
		return hr;

	// Get DXGI adapter
	hr = lDxgiDevice->GetParent(
		__uuidof(IDXGIAdapter),
		reinterpret_cast<void**>(&eAdapter));

	lDxgiDevice->Release();

	DXGI_ADAPTER_DESC adapterDesc;
	eAdapter->GetDesc(&adapterDesc);
	char szDesc[80];
	wcstombs(szDesc, adapterDesc.Description, sizeof(szDesc));
	std::cout << "GPU in use: " << szDesc << std::endl;
	//ck(iFactory->EnumAdapters(iGpu, iAdapter->GetAddressOf()));
	//ck(D3D11CreateDevice(iAdapter.Get(), D3D_DRIVER_TYPE_UNKNOWN, NULL, 0,
	//	NULL, 0, D3D11_SDK_VERSION, pDevice.GetAddressOf(), NULL, pContext.GetAddressOf()));

	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
	desc.Width = wid;
	desc.Height = hei;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_STAGING;
	desc.BindFlags = 0;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	hr = eDevice->CreateTexture2D(&desc, NULL, &eTexSysMem);
	Check(hr, "eDevice->CreateTexture2D(&desc, NULL, &eTexSysMem);");

	hr = Create_Enc();
	Check(hr, "Create_Enc Error");

	return hr;
}

HRESULT NVEClass::Create_Enc() {

	HRESULT hr = E_FAIL;
	NvEncoderD3D11 enc(eDevice, nwid, nhei, true ? NV_ENC_BUFFER_FORMAT_NV12 : NV_ENC_BUFFER_FORMAT_ARGB);
	
	/// NVENCODEAPI session intialization parameters
	NV_ENC_INITIALIZE_PARAMS encInitParams = { 0 };
	/// NVENCODEAPI video encoding configuration parameters

	NV_ENC_CONFIG encodeConfig = { NV_ENC_CONFIG_VER };
	
	//NvEncoderInitParam* pEncodeCLIOptions = {0};
	//pEncodeCLIOptions->SetInitParams();
	ZeroMemory(&encInitParams, sizeof(encInitParams));
	ZeroMemory(&encodeConfig, sizeof(encodeConfig));
	encInitParams.encodeConfig = &encodeConfig;
	encInitParams.encodeWidth = nwid;
	encInitParams.encodeHeight = nhei;
	encInitParams.maxEncodeWidth = nwid;
	encInitParams.maxEncodeHeight = nhei;

	enc.CreateDefaultEncoderParams(&encInitParams, NV_ENC_CODEC_H264_GUID, NV_ENC_PRESET_LOW_LATENCY_HP_GUID);

	enc.CreateEncoder(&encInitParams);

	return hr;
}

void NVEClass::RGBToNV12() {

	ID3D11Texture2D* pTexBgra = NULL;
	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
	desc.Width = nwid;
	desc.Height = nhei;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_RENDER_TARGET;
	desc.CPUAccessFlags = 0;
	eDevice->CreateTexture2D(&desc, NULL, &pTexBgra);



	//NV12 변환용		//나중에 다시 결정
	eDevice->QueryInterface(__uuidof(ID3D11VideoDevice), (void **)&eVideoDevice);
	eImmediateContext->QueryInterface(__uuidof(ID3D11VideoContext), (void **)&eVideoContext);
	
	D3D11_VIDEO_PROCESSOR_CONTENT_DESC contentDesc =
	{
		D3D11_VIDEO_FRAME_FORMAT_PROGRESSIVE,
		{ 1, 1 }, desc.Width, desc.Height,
		{ 1, 1 }, desc.Width, desc.Height,
		D3D11_VIDEO_USAGE_PLAYBACK_NORMAL
	};
	eVideoDevice->CreateVideoProcessorEnumerator(&contentDesc, &eVideoProcessorEnumerator);
	
	eVideoDevice->CreateVideoProcessor(eVideoProcessorEnumerator, 0, &eVideoProcessor);
	D3D11_VIDEO_PROCESSOR_INPUT_VIEW_DESC inputViewDesc = { 0, D3D11_VPIV_DIMENSION_TEXTURE2D, { 0, 0 } };
	eVideoDevice->CreateVideoProcessorInputView(pTexBgra, eVideoProcessorEnumerator, &inputViewDesc, &eInputView);

	return;
}

bool NVEClass::Encode() {

	return true;
}


void NVEClass::Release() {

	if (eDevice) {
		eDevice->Release();
		eDevice = NULL;
	}

	if (eImmediateContext) {
		eImmediateContext->Release();
		eImmediateContext = NULL;
	}

	return;
}