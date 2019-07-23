#pragma once

#include "pch.h"
#include <vector>
#include <shlobj.h>
#include <algorithm>
#include <thread>
#include <wrl.h>

#include <d3d11.h>
#include <dxgi1_2.h>
#include <d3dcommon.h>

#pragma comment(lib, "dxguid.lib" )
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")

using Microsoft::WRL::ComPtr;

class DXCapClass {

public:

	DXCapClass();
	~DXCapClass();
	HRESULT Capture(std::shared_ptr<BYTE> I_data);
	ComPtr<ID3D11Texture2D> GetTex();
	ID3D11Device* GetDevice();
	ID3D11DeviceContext* GetDC();

private:

	void InitCap();
	void Release();
	
	HRESULT GetAdapter();
	HRESULT CreateDevice();
	HRESULT InitResource(int index);
	//HRESULT Capture(std::shared_ptr<BYTE> I_data);

	// Driver types supported
	ID3D11Device* lDevice;
	ID3D11DeviceContext* lImmediateContext;
	ComPtr<IDXGIOutputDuplication> lDeskDupl;
	ID3D11Texture2D* lAcquiredDesktopImage;
	ComPtr<ID3D11Texture2D> lGDIImage;
	ComPtr<ID3D11Texture2D> lDestImage;
	ComPtr<IDXGIResource> DesktopResource;

	DXGI_OUTPUT_DESC lOutputDesc;
	DXGI_OUTDUPL_DESC lOutputDuplDesc;

	//std::vector<IDXGIAdapter*> AdapterList;
	IDXGIAdapter* lDxgiAdapter;
	bool Savetobmp(int wid, int hei, unsigned char * sptr);


	std::thread t_capture;

};