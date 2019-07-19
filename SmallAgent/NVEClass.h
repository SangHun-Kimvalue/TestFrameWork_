#pragma once
#include "pch.h"

#include <d3d11.h>
#include <dxgi1_2.h>
#include <d3dcommon.h>

#include "NvEncoderD3D11.h"
//#include "NvCodecUtils.h"
//#include "AppEncUtils.h"
//#include "AppEncUtilsD3D11.h"

#pragma comment(lib, "dxguid.lib" )
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")

class NVEClass
{
public:
	NVEClass(int wid, int hei);
	~NVEClass();

	bool Encode();


private:

	HRESULT Init(int wid, int hei);
	void Release();
	

	HRESULT Create_Enc();
	void RGBToNV12();

	ID3D11Device* eDevice;
	ID3D11DeviceContext* eImmediateContext;
	ID3D11VideoDevice *eVideoDevice = NULL;
	ID3D11VideoContext *eVideoContext = NULL;

	IDXGIFactory1* eFactory;
	IDXGIAdapter* eAdapter;
	ID3D11Texture2D* eTexSysMem;
	ID3D11VideoProcessor *eVideoProcessor = NULL;
	ID3D11VideoProcessorEnumerator *eVideoProcessorEnumerator = nullptr;
	ID3D11VideoProcessorInputView *eInputView = NULL;
	ID3D11VideoProcessorOutputView *eOutputView = NULL;


	const int nwid;
	const int nhei;

};

