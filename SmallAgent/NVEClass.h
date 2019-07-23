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
	NVEClass(int wid, int hei, ID3D11Device* eDevice, ID3D11DeviceContext* mDeviceContext);
	~NVEClass();

	bool Encode(ID3D11Texture2D* Tex);


private:

	RGBToNV12 *pColorConv = nullptr;
	HRESULT Init(int wid, int hei);
	void Release();

	HRESULT Create_Enc(int nwid, int nhei);

	ID3D11Device* eDevice;
	ID3D11DeviceContext* eImmediateContext;

	NvEncoderD3D11* enc;

	std::vector<std::vector<uint8_t>> vPacket;

	const int nwid;
	const int nhei;

};

