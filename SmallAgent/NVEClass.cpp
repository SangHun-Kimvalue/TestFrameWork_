#include "pch.h"
#include "NVEClass.h"

NVEClass::NVEClass(int wid, int hei)
	: nwid(wid), nhei(hei), enc(Init(wid, hei)) {

}


NVEClass::NVEClass(int wid, int hei,ComPtr<ID3D11Texture2D>Tex, ID3D11Device* eDevice, ID3D11DeviceContext* mDeviceContext)
	: nwid(wid), nhei(hei), eDevice(eDevice), eImmediateContext(mDeviceContext), enc(Create_Enc(wid, hei)), Temp(Tex.Get()){

}


NVEClass::~NVEClass()
{
	Release();
}

NvEncoderD3D11 NVEClass::Init(int wid, int hei) {

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

	//eDevice->AddRef();
	//eImmediateContext->AddRef();

	//CreateDXGIFactory1(__uuidof(IDXGIFactory1);
	//if (FAILED(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&iFactory))) {
	//	return E_FAIL;
	//}
	//hr = iFactory->EnumAdapters(0, &iAdapter);
	//if (FAILED(hr)) {
	//	return E_FAIL;
	//}

	//디바이스를 새로 안만들고 하나가지고 돌려 쓰는 것도 구현 해
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

	//IDXGIDevice* lDxgiDevice;
	//hr = eDevice->QueryInterface(IID_PPV_ARGS(&lDxgiDevice));
	//Check(hr, "eDevice  QueryInterface");

	//// Get DXGI adapter
	//hr = lDxgiDevice->GetParent(
	//	__uuidof(IDXGIAdapter),
	//	reinterpret_cast<void**>(&eAdapter));
	//
	//lDxgiDevice->Release();
	//
	//DXGI_ADAPTER_DESC adapterDesc;
	//eAdapter->GetDesc(&adapterDesc);
	//char szDesc[80];
	//wcstombs(szDesc, adapterDesc.Description, sizeof(szDesc));
	//std::cout << "GPU in use: " << szDesc << std::endl;

	//멀티 GPU 사용시 선택해 어댑터를 생성함.
	//ck(iFactory->EnumAdapters(iGpu, iAdapter->GetAddressOf()));
	//ck(D3D11CreateDevice(iAdapter.Get(), D3D_DRIVER_TYPE_UNKNOWN, NULL, 0,
	//	NULL, 0, D3D11_SDK_VERSION, pDevice.GetAddressOf(), NULL, pContext.GetAddressOf()));

	//D3D11_SUBRESOURCE_DATA subdesc;
	//subdesc.pSysMem;
	//
	//D3D11_TEXTURE2D_DESC desc;
	//ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
	//desc.Width = wid;
	//desc.Height = hei;
	//desc.MipLevels = 1;
	//desc.ArraySize = 1;
	//desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	//desc.SampleDesc.Count = 1;
	//desc.Usage = D3D11_USAGE_STAGING;
	//desc.BindFlags = 0;
	//desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	//hr = eDevice->CreateTexture2D(&desc, NULL, &eTexSysMem);
	//Check(hr, "eDevice->CreateTexture2D(&desc, NULL, &eTexSysMem);");

	return Create_Enc(wid, hei);
}

NvEncoderD3D11 NVEClass::Create_Enc(int nwid, int nhei) {

	HRESULT hr = E_FAIL;
	NvEncoderD3D11 enc (eDevice, nwid, nhei, true ? NV_ENC_BUFFER_FORMAT_NV12 : NV_ENC_BUFFER_FORMAT_ARGB);
	
	/// NVENCODEAPI session intialization parameters
	NV_ENC_INITIALIZE_PARAMS encInitParams = { NV_ENC_INITIALIZE_PARAMS_VER };
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

	//NV_ENC_PRESET_LOSSLESS_HP_GUID 이랑 얼마나 china?
	enc.CreateDefaultEncoderParams(&encInitParams, NV_ENC_CODEC_H264_GUID, NV_ENC_PRESET_LOW_LATENCY_HP_GUID);
	
	encInitParams.frameRateNum = 60;// einfo.fps;
	encInitParams.frameRateDen = 1;
	encInitParams.encodeConfig->rcParams.averageBitRate = 4096000;
	encInitParams.encodeConfig->gopLength = 30;
	encInitParams.encodeConfig->rcParams.rateControlMode = NV_ENC_PARAMS_RC_CBR_LOWDELAY_HQ;

	enc.CreateEncoder(&encInitParams);

	//NV12 변환용		//나중에 다시 결정
	hr = eDevice->QueryInterface(__uuidof(ID3D11VideoDevice), (void **)&eVideoDevice);
	if (FAILED(hr))
	{
		Check(hr, "eDevice->QueryInterface(__uuidof(ID3D11VideoDevice), (void **)&eVideoDevice)");
	}

	hr = eImmediateContext->QueryInterface(__uuidof(ID3D11VideoContext), (void **)&eVideoContext);
	if (FAILED(hr))
	{
		Check(hr, "eImmediateContext->QueryInterface(__uuidof(ID3D11VideoContext), (void **)&eVideoContext)");
	}

	eSize = nwid * nhei * 4;

	D3D11_VIDEO_PROCESSOR_CONTENT_DESC contentDesc =
	{
		D3D11_VIDEO_FRAME_FORMAT_PROGRESSIVE,
		{ 1, 1 }, nwid, nhei,
		{ 1, 1 }, nwid, nhei,
		D3D11_VIDEO_USAGE_PLAYBACK_NORMAL
	};
	hr = eVideoDevice->CreateVideoProcessorEnumerator(&contentDesc, &eVideoProcessorEnumerator);
	if (FAILED(hr))
	{
		Check(hr, "CreateVideoProcessorEnumerator");
	}

	hr = eVideoDevice->CreateVideoProcessor(eVideoProcessorEnumerator, 0, &eVideoProcessor);
	if (FAILED(hr))
	{
		Check(hr, "CreateVideoProcessor");
	}

	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
	desc.Width = nwid;
	desc.Height = nhei;
	desc.MipLevels = 1;
	desc.MiscFlags = D3D11_RESOURCE_MISC_GDI_COMPATIBLE;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_RENDER_TARGET;
	desc.CPUAccessFlags = 0;

	hr = eDevice->CreateTexture2D(&desc, NULL, &eTexSysMem);
	if (FAILED(hr))
	{
		Check(hr, " eDevice->CreateTexture2D(&desc, NULL, &eTexSysMem);");
	}
	//desc.BindFlags =0;
	//desc.MiscFlags = 0;
	//desc.SampleDesc.Count = 1;
	//desc.MipLevels = 1;
	//desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
	//desc.Usage = D3D11_USAGE_DEFAULT;
	//hr = eDevice->CreateTexture2D(&desc, NULL, &m_pEncBuf);
	//if (FAILED(hr))
	//{
	//	Check(hr, " eDevice->CreateTexture2D(&desc, NULL, &m_pEncBuf);");
	//}

	//D3D11_VIDEO_PROCESSOR_INPUT_VIEW_DESC inputViewDesc = { 0, D3D11_VPIV_DIMENSION_TEXTURE2D, { 0, 0 } };
	//hr = eVideoDevice->CreateVideoProcessorInputView(eTexSysMem, eVideoProcessorEnumerator, &inputViewDesc, &eInputView);
	//if (FAILED(hr))
	//{
	//	Check(hr, "CreateVideoProcessorInputView");
	//}

	if (!pColorConv)
	{
		pColorConv = new RGBToNV12(eDevice, eImmediateContext);
		HRESULT hr = pColorConv->Init();
		Check(hr, "pColorConv->Init()");
	}



	return enc;
}

HRESULT NVEClass::Convert(ID3D11Texture2D* pRGB, ID3D11Texture2D* pYUV) {

	HRESULT hr = S_OK;
	ID3D11VideoProcessorInputView* pVPIn = nullptr;

	D3D11_TEXTURE2D_DESC inDesc = { 0 };
	D3D11_TEXTURE2D_DESC outDesc = { 0 };
	//pRGB->GetDesc(&inDesc);
	//pYUV->GetDesc(&outDesc);
	outDesc = inDesc;
	/// Check if VideoProcessor needs to be reconfigured
	/// Reconfiguration is required if input/output dimensions have changed
	
	if (!eVideoProcessor)
	{
		/// Initialize Video Processor
		//m_inDesc = inDesc;
		//m_outDesc = outDesc;
		D3D11_VIDEO_PROCESSOR_CONTENT_DESC contentDesc =
		{
			D3D11_VIDEO_FRAME_FORMAT_PROGRESSIVE,
			{ 1, 1 }, inDesc.Width, inDesc.Height,
			{ 1, 1 }, outDesc.Width, outDesc.Height,
			D3D11_VIDEO_USAGE_PLAYBACK_NORMAL
		};
		hr = eVideoDevice->CreateVideoProcessorEnumerator(&contentDesc, &eVideoProcessorEnumerator);
		if (FAILED(hr))
		{
			PRINTERR(hr, "CreateVideoProcessorEnumerator");
		}
		hr = eVideoDevice->CreateVideoProcessor(eVideoProcessorEnumerator, 0, &eVideoProcessor);
		if (FAILED(hr))
		{
			PRINTERR(hr, "CreateVideoProcessor");
		}
	}

	/// Obtain Video Processor Input view from input texture
	D3D11_VIDEO_PROCESSOR_INPUT_VIEW_DESC inputVD = { 0, D3D11_VPIV_DIMENSION_TEXTURE2D,{ 0,0 } };
	hr = eVideoDevice->CreateVideoProcessorInputView(pRGB, eVideoProcessorEnumerator, &inputVD, &pVPIn);
	if (FAILED(hr))
	{
		PRINTERR(hr, "CreateVideoProcessInputView");
		return hr;
	}

	/// Obtain Video Processor Output view from output texture
	ID3D11VideoProcessorOutputView* pVPOV = nullptr;
	auto it = viewMap.find(pYUV);
	/// Optimization: Check if we already created a video processor output view for this texture
	if (it == viewMap.end())
	{
		/// We don't have a video processor output view for this texture, create one now.
		D3D11_VIDEO_PROCESSOR_OUTPUT_VIEW_DESC ovD = { D3D11_VPOV_DIMENSION_TEXTURE2D };
		hr = eVideoDevice->CreateVideoProcessorOutputView(pYUV, eVideoProcessorEnumerator, &ovD, &pVPOV);
		if (FAILED(hr))
		{
			SAFE_RELEASE(pVPIn);
			PRINTERR(hr, "CreateVideoProcessorOutputView");
			return hr;
		}
		viewMap.insert({ pYUV, pVPOV });
	}
	else
	{
		pVPOV = it->second;
	}

	/// Create a Video Processor Stream to run the operation
	D3D11_VIDEO_PROCESSOR_STREAM stream = { TRUE, 0, 0, 0, 0, nullptr, pVPIn, nullptr };

	/// Perform the Colorspace conversion
	hr = eVideoContext->VideoProcessorBlt(eVideoProcessor, pVPOV, 0, 1, &stream);
	if (FAILED(hr))
	{
		SAFE_RELEASE(pVPIn);
		PRINTERR(hr, "VideoProcessorBlt");
		return hr;
	}
	SAFE_RELEASE(pVPIn);
	return hr;


	//HRESULT hr = E_FAIL;
	//
	//std::vector<std::vector<uint8_t>> vPacket;
	//eImmediateContext->CopyResource(eTexSysMem, Temp);
	//
	//
	//const NvEncInputFrame *pEncInput = enc.GetNextInputFrame();
	//m_pEncBuf = (ID3D11Texture2D *)pEncInput->inputPtr;
	//
	///// Obtain Video Processor Output view from output texture
	//ID3D11VideoProcessorOutputView* pVPOV = nullptr;
	//auto it = viewMap.find(m_pEncBuf);
	///// Optimization: Check if we already created a video processor output view for this texture
	//if (it == viewMap.end())
	//{
	//	/// We don't have a video processor output view for this texture, create one now.
	//	D3D11_VIDEO_PROCESSOR_OUTPUT_VIEW_DESC ovD = { D3D11_VPOV_DIMENSION_TEXTURE2D };
	//	hr = eVideoDevice->CreateVideoProcessorOutputView(m_pEncBuf, eVideoProcessorEnumerator, &ovD, &pVPOV);
	//	if (FAILED(hr))
	//	{
	//		Check(hr, "CreateVideoProcessorOutputView");
	//	}
	//	viewMap.insert({ m_pEncBuf, pVPOV });
	//}
	//else
	//{
	//	pVPOV = it->second;
	//}
	//
	///// Create a Video Processor Stream to run the operation
	//D3D11_VIDEO_PROCESSOR_STREAM stream = { TRUE, 0, 0, 0, 0, nullptr, eInputView, nullptr };
	//
	///// Perform the Colorspace conversion
	//hr = eVideoContext->VideoProcessorBlt(eVideoProcessor, pVPOV, 0, 1, &stream);
	//if (FAILED(hr))
	//{
	//	Check(hr, " m_pVidCtx->VideoProcessorBlt(eVideoProcessor, pVPOV, 0, 1, &stream);");
	//
	//}
	////SAFE_RELEASE(pVPIn);
	////return hr;
	//enc.EncodeFrame(vPacket);
	//return hr;
}

bool NVEClass::Encode() {

	int nFrame = 0;
	eImmediateContext->CopyResource(eTexSysMem, Temp);

	const NvEncInputFrame *pEncInput = enc.GetNextInputFrame();
	m_pEncBuf = (ID3D11Texture2D *)pEncInput->inputPtr;
	
	HRESULT hr = pColorConv->Convert(eTexSysMem, m_pEncBuf);
    //hr = Convert(eTexSysMem, m_pEncBuf);
   Check(hr, "Convert");
   //std::vector<std::vector<uint8_t>> vPacket;
   
  // enc.EncodeFrame(vPacket);

	return hr;
}


void NVEClass::Release() {


	//if (eInputView)
	//	eInputView->Release();

	if (eDevice) {
		eDevice->Release();
		eDevice = NULL;
	}

	if (eImmediateContext) {
		eImmediateContext->Release();
		eImmediateContext = NULL;
	}

	if (eTexSysMem) {
		eTexSysMem->Release();
		eTexSysMem = NULL;
	}

	enc.DestroyEncoder();

	return;
}