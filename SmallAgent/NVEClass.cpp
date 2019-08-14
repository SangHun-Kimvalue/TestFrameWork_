#include "pch.h"
#include "NVEClass.h"

NVEClass::NVEClass(int wid, int hei)
	: nwid(wid), nhei(hei) {
	//Init(wid, hei);
	//Create_Enc(nwid, nhei);
	//Capture();
}


NVEClass::NVEClass(int wid, int hei, ID3D11Device* eDevice, ID3D11DeviceContext* mDeviceContext)
	: nwid(wid), nhei(hei), eDevice(eDevice), eImmediateContext(mDeviceContext)/*, enc(Create_Enc(wid, hei)), Temp(Tex.Get())*/ {
	//Init(wid, hei);
	Create_Enc(nwid, nhei);
}

NVEClass::~NVEClass()
{
	Release();
}

HRESULT NVEClass::Init(int wid, int hei) {


	HRESULT hr = S_OK;

	return hr;
}

HRESULT NVEClass::Create_Enc(int nwid, int nhei) {

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

		encInitParams.frameRateNum = 60;
		encInitParams.frameRateDen = 1;
		encInitParams.encodeConfig->rcParams.averageBitRate = 4096000;
		encInitParams.encodeConfig->gopLength = 30;
		encInitParams.encodeConfig->rcParams.rateControlMode = NV_ENC_PARAMS_RC_CBR_LOWDELAY_HQ;

		if (FAILED(hr))
		{
			Check(hr, "CreateDefaultEncoderParams");
		}

		enc->CreateEncoder(&encInitParams);
	}
	catch (...)
	{
		if (FAILED(hr))
		{
			Check(hr, "Create_Enc");
		}
	}

	return hr;
}

bool NVEClass::Encode(ID3D11Texture2D* Tex) {

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

	hr = pColorConv->Convert(Tex, m_pEncBuf);

	enc->EncodeFrame(vPacket);
	SAFE_RELEASE(m_pEncBuf);
	Tex->Release();
	
	return hr;
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

	enc->EndEncode(vPacket);
	enc->DestroyEncoder();
	
	//enc.DestroyEncoder();

	return;
}