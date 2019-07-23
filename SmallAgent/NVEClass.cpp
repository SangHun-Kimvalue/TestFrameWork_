#include "pch.h"
#include "NVEClass.h"

NVEClass::NVEClass(int wid, int hei)
	: nwid(wid), nhei(hei) {
	Init(wid, hei);
	Create_Enc(nwid, nhei);
	Capture();
	Encode();
}


NVEClass::NVEClass(int wid, int hei,ComPtr<ID3D11Texture2D>Tex, ID3D11Device* eDevice, ID3D11DeviceContext* mDeviceContext)
	: nwid(wid), nhei(hei), eDevice(eDevice), eImmediateContext(mDeviceContext)/*, enc(Create_Enc(wid, hei)), Temp(Tex.Get())*/ {

}


NVEClass::~NVEClass()
{
	Release();
}

HRESULT NVEClass::Init(int wid, int hei) {

	DWORD w = nwid;
	DWORD h = nhei;

	HRESULT hr = S_OK;

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

	hr = D3D11CreateDevice(
		nullptr,
		gDriverTypes[0],
		nullptr,
		0,
		gFeatureLevels,
		gNumFeatureLevels,
		D3D11_SDK_VERSION,
		&eDevice,
		&lFeatureLevel,
		&eImmediateContext);

	IDXGIDevice* lDxgiDevice;
	IDXGIAdapter* lDxgiAdapter;

	hr = eDevice->QueryInterface(IID_PPV_ARGS(&lDxgiDevice));
	if (FAILED(hr))
	{
		Check(hr, "D3D11CreateDevice");
	}
	// Get DXGI adapter

	hr = lDxgiDevice->GetParent(
		__uuidof(IDXGIAdapter),
		reinterpret_cast<void**>(&lDxgiAdapter));

	lDxgiDevice->Release();
	if (FAILED(hr))
	{
		Check(hr, "D3D11CreateDevice");
	}

	IDXGIOutput* lDxgiOutput;
	IDXGIAdapter* Adaptertemp = lDxgiAdapter;
	hr = Adaptertemp->EnumOutputs(0, &lDxgiOutput);
	Check(hr, "EnumOutputs");
	if (Adaptertemp)
		Adaptertemp->Release();

	HWND hWnd = GetDesktopWindow();
	HDC hdc = GetWindowDC(hWnd);

	

	hr = lDxgiOutput->GetDesc(&lOutputDesc);
	Check(hr, "lDxgiOutput->GetDesc");

	IDXGIOutput1* Outputtemp;
	hr = lDxgiOutput->QueryInterface(IID_PPV_ARGS(&Outputtemp));
	Check(hr, "lDxgiOutput->QueryInterface");
	if (lDxgiOutput)
		lDxgiOutput->Release();

	hr = Outputtemp->DuplicateOutput(eDevice, &lDeskDupl);
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

	hr = eDevice->CreateTexture2D(&Due_desc, NULL, &lGDIImage);
	Check(hr, "lDevice->CreateTexture2D(&Due_desc, NULL, &lGDIImage);");
	//if (lGDIImage == nullptr)
	//	return E_FAIL;

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

	hr = eDevice->CreateTexture2D(&CPUdesc, NULL, &lDestImage);
	Check(hr, "lDevice->CreateTexture2D(&CPUdesc, NULL, &lDestImage);");
	

	return hr;
}

NvEncoderD3D11* NVEClass::Create_Enc(int nwid, int nhei) {

	HRESULT hr = S_OK;

	const static bool bNoVPBlt = false;
	NV_ENC_BUFFER_FORMAT fmt = bNoVPBlt ? NV_ENC_BUFFER_FORMAT_ARGB : NV_ENC_BUFFER_FORMAT_NV12;
	enc = new NvEncoderD3D11(eDevice, nwid, nhei, fmt);
	if (FAILED(hr))
	{
		Check(hr, "D3D11CreateDevice"); 
	}
	NV_ENC_INITIALIZE_PARAMS encInitParams = { NV_ENC_INITIALIZE_PARAMS_VER };
	/// NVENCODEAPI video encoding configuration parameters
	NV_ENC_CONFIG encConfig = { NV_ENC_CONFIG_VER };
	//NvEncoderInitParam* pEncodeCLIOptions = {0};
	//pEncodeCLIOptions->SetInitParams();
	ZeroMemory(&encInitParams, sizeof(encInitParams));
	ZeroMemory(&encConfig, sizeof(encConfig));
	encInitParams.encodeConfig = &encConfig;
	encInitParams.encodeWidth = nwid;
	encInitParams.encodeHeight = nhei;
	encInitParams.maxEncodeWidth = nwid;
	encInitParams.maxEncodeHeight = nhei;

	try
	{
		enc->CreateDefaultEncoderParams(&encInitParams, NV_ENC_CODEC_H264_GUID, NV_ENC_PRESET_LOW_LATENCY_HP_GUID);

		encInitParams.frameRateNum = 60;// einfo.fps;
		encInitParams.frameRateDen = 1;
		encInitParams.encodeConfig->rcParams.averageBitRate = 4096000;
		encInitParams.encodeConfig->gopLength = 30;
		encInitParams.encodeConfig->rcParams.rateControlMode = NV_ENC_PARAMS_RC_CBR_LOWDELAY_HQ;
	
		if (FAILED(hr))
		{
			Check(hr, "D3D11CreateDevice");		
		}

		enc->CreateEncoder(&encInitParams);
	}
	catch (...)
	{
		if (FAILED(hr))
		{
			Check(hr, "D3D11CreateDevice");
		}
	}

	//HRESULT hr = E_FAIL;
	//NvEncoderD3D11 enc (eDevice, nwid, nhei, true ? NV_ENC_BUFFER_FORMAT_NV12 : NV_ENC_BUFFER_FORMAT_ARGB);
	//
	///// NVENCODEAPI session intialization parameters
	//NV_ENC_INITIALIZE_PARAMS encInitParams = { NV_ENC_INITIALIZE_PARAMS_VER };
	///// NVENCODEAPI video encoding configuration parameters
	//
	//NV_ENC_CONFIG encodeConfig = { NV_ENC_CONFIG_VER };
	//
	////NvEncoderInitParam* pEncodeCLIOptions = {0};
	////pEncodeCLIOptions->SetInitParams();
	//ZeroMemory(&encInitParams, sizeof(encInitParams));
	//ZeroMemory(&encodeConfig, sizeof(encodeConfig));
	//encInitParams.encodeConfig = &encodeConfig;
	//encInitParams.encodeWidth = nwid;
	//encInitParams.encodeHeight = nhei;
	//encInitParams.maxEncodeWidth = nwid;
	//encInitParams.maxEncodeHeight = nhei;
	//
	////NV_ENC_PRESET_LOSSLESS_HP_GUID 이랑 얼마나 china?
	//enc.CreateDefaultEncoderParams(&encInitParams, NV_ENC_CODEC_H264_GUID, NV_ENC_PRESET_LOW_LATENCY_HP_GUID);
	//
	//encInitParams.frameRateNum = 60;// einfo.fps;
	//encInitParams.frameRateDen = 1;
	//encInitParams.encodeConfig->rcParams.averageBitRate = 4096000;
	//encInitParams.encodeConfig->gopLength = 30;
	//encInitParams.encodeConfig->rcParams.rateControlMode = NV_ENC_PARAMS_RC_CBR_LOWDELAY_HQ;
	//
	//enc.CreateEncoder(&encInitParams);
	//
	////NV12 변환용		//나중에 다시 결정
	//hr = eDevice->QueryInterface(__uuidof(ID3D11VideoDevice), (void **)&eVideoDevice);
	//if (FAILED(hr))
	//{
	//	Check(hr, "eDevice->QueryInterface(__uuidof(ID3D11VideoDevice), (void **)&eVideoDevice)");
	//}
	//
	//hr = eImmediateContext->QueryInterface(__uuidof(ID3D11VideoContext), (void **)&eVideoContext);
	//if (FAILED(hr))
	//{
	//	Check(hr, "eImmediateContext->QueryInterface(__uuidof(ID3D11VideoContext), (void **)&eVideoContext)");
	//}
	//
	//eSize = nwid * nhei * 4;
	//
	//D3D11_VIDEO_PROCESSOR_CONTENT_DESC contentDesc =
	//{
	//	D3D11_VIDEO_FRAME_FORMAT_PROGRESSIVE,
	//	{ 1, 1 }, nwid, nhei,
	//	{ 1, 1 }, nwid, nhei,
	//	D3D11_VIDEO_USAGE_PLAYBACK_NORMAL
	//};
	//hr = eVideoDevice->CreateVideoProcessorEnumerator(&contentDesc, &eVideoProcessorEnumerator);
	//if (FAILED(hr))
	//{
	//	Check(hr, "CreateVideoProcessorEnumerator");
	//}
	//
	//hr = eVideoDevice->CreateVideoProcessor(eVideoProcessorEnumerator, 0, &eVideoProcessor);
	//if (FAILED(hr))
	//{
	//	Check(hr, "CreateVideoProcessor");
	//}
	//
	//D3D11_TEXTURE2D_DESC desc;
	//ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
	//desc.Width = nwid;
	//desc.Height = nhei;
	//desc.MipLevels = 1;
	//desc.MiscFlags = D3D11_RESOURCE_MISC_GDI_COMPATIBLE;
	//desc.ArraySize = 1;
	//desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	//desc.SampleDesc.Count = 1;
	//desc.SampleDesc.Quality = 0;
	//desc.Usage = D3D11_USAGE_DEFAULT;
	//desc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_RENDER_TARGET;
	//desc.CPUAccessFlags = 0;
	//
	//hr = eDevice->CreateTexture2D(&desc, NULL, &eTexSysMem);
	//if (FAILED(hr))
	//{
	//	Check(hr, " eDevice->CreateTexture2D(&desc, NULL, &eTexSysMem);");
	//}
	////desc.BindFlags =0;
	////desc.MiscFlags = 0;
	////desc.SampleDesc.Count = 1;
	////desc.MipLevels = 1;
	////desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
	////desc.Usage = D3D11_USAGE_DEFAULT;
	////hr = eDevice->CreateTexture2D(&desc, NULL, &m_pEncBuf);
	////if (FAILED(hr))
	////{
	////	Check(hr, " eDevice->CreateTexture2D(&desc, NULL, &m_pEncBuf);");
	////}
	//
	////D3D11_VIDEO_PROCESSOR_INPUT_VIEW_DESC inputViewDesc = { 0, D3D11_VPIV_DIMENSION_TEXTURE2D, { 0, 0 } };
	////hr = eVideoDevice->CreateVideoProcessorInputView(eTexSysMem, eVideoProcessorEnumerator, &inputViewDesc, &eInputView);
	////if (FAILED(hr))
	////{
	////	Check(hr, "CreateVideoProcessorInputView");
	////}

	return enc;
}

void NVEClass::Capture() {

	HRESULT hr = S_OK;
	DXGI_OUTDUPL_FRAME_INFO FrameInfo;
	//GetClientRect(hWnd, &rt);
					//DrawText(hdc, szHello, wcslen(szHello), &naf->m_ScreenRECT, DT_LEFT);

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
		//lAcquiredDesktopImage = NULL;
	}

	// QI for IDXGIResource
	hr = DesktopResource->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void **>(&lAcquiredDesktopImage));
	if (FAILED(hr))
	{
		std::cerr << "DesktopResource->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void **>(&lAcquiredDesktopImage));" << std::endl;
		//return hr;
	}

	//lDestImage = NULL;

	if (lAcquiredDesktopImage == nullptr) {
		std::cerr << "lAcquiredDesktopImage Capture Error" << std::endl;
		//return E_FAIL;
	}

	eImmediateContext->CopyResource(lGDIImage.Get(), lAcquiredDesktopImage);

	IDXGISurface1* lIDXGISurface1;

	hr = lGDIImage->QueryInterface(IID_PPV_ARGS(&lIDXGISurface1));
	if (FAILED(hr))
	{
		std::cerr << "QueryInterface(IID_PPV_ARGS(&lIDXGISurface1));" << std::endl;
		//return hr;
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
	
	HRESULT hr = S_OK;

	ID3D11Texture2D *m_pEncBuf = nullptr;
	const NvEncInputFrame *pEncInput = enc->GetNextInputFrame();
	m_pEncBuf = (ID3D11Texture2D *)pEncInput->inputPtr;

	if (!pColorConv)
	{
		pColorConv = new RGBToNV12(eDevice, eImmediateContext);
		hr = pColorConv->Init();
		Check(hr, "pColorConv->Init()");
	}

	hr = pColorConv->Convert(lAcquiredDesktopImage, m_pEncBuf);

	//int nFrame = 0;
	//eImmediateContext->CopyResource(eTexSysMem, Temp);
	//
	//const NvEncInputFrame *pEncInput = enc.GetNextInputFrame();
	//m_pEncBuf = (ID3D11Texture2D *)pEncInput->inputPtr;
	//
	//HRESULT hr = pColorConv->Convert(eTexSysMem, m_pEncBuf);
    ////hr = Convert(eTexSysMem, m_pEncBuf);
    //Check(hr, "Convert");
   //std::vector<std::vector<uint8_t>> vPacket;
   
  // enc.EncodeFrame(vPacket);
	
	enc->EncodeFrame(vPacket);
	SAFE_RELEASE(m_pEncBuf);

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

	enc->EndEncode(vPacket);
	enc->DestroyEncoder();

	//if (eTexSysMem) {
	//	eTexSysMem->Release();
	//	eTexSysMem = NULL;
	//}
	//
	//enc.DestroyEncoder();

	return;
}