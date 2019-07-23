#pragma once
#include "pch.h"

#include <d3d11.h>
#include <dxgi1_2.h>
#include <d3dcommon.h>
#include <wrl.h>
#include "Preproc.h"

#include "NvEncoderD3D11.h"

#pragma comment(lib, "dxguid.lib" )
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")

using Microsoft::WRL::ComPtr;



class NVEClass
{
public:
	NVEClass(int wid, int hei);
	NVEClass(int wid, int hei, ComPtr<ID3D11Texture2D>, ID3D11Device* eDevice, ID3D11DeviceContext* mDeviceContext);
	~NVEClass();

	bool Encode();


private:

	RGBToNV12 *pColorConv = nullptr;
	NvEncoderD3D11 Init(int wid, int hei);
	void Release();
	
	NvEncoderD3D11 Create_Enc(int nwid, int nhei);
	HRESULT Convert(ID3D11Texture2D* pRGB, ID3D11Texture2D*pYUV);

	ID3D11Device* eDevice;
	ID3D11DeviceContext* eImmediateContext;
	ID3D11VideoDevice *eVideoDevice = NULL;
	ID3D11VideoContext *eVideoContext = NULL;

	IDXGIFactory1* eFactory;
	IDXGIAdapter* eAdapter;
	ID3D11Texture2D* eTexSysMem;
	ID3D11Texture2D* Temp;
	ID3D11VideoProcessor *eVideoProcessor = NULL;
	ID3D11VideoProcessorEnumerator *eVideoProcessorEnumerator = nullptr;
	//ID3D11VideoProcessorInputView *eInputView = NULL;
	//ID3D11VideoProcessorOutputView *eOutputView = NULL;
	ID3D11Texture2D *m_pEncBuf = nullptr;
	std::unordered_map<ID3D11Texture2D*, ID3D11VideoProcessorOutputView*> viewMap;

	NvEncoderD3D11 enc;
	std::unique_ptr<uint8_t[]> pHostFrame;

	const int nwid;
	const int nhei;
	int eSize;

};

