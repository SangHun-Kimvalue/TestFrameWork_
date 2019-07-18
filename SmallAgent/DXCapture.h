#pragma once

#include "pch.h"
#include <iostream>
#include <vector>
#include <shlobj.h>
#include <algorithm>

#include <d3d11.h>
#include <dxgi1_2.h>
#include <d3dcommon.h>

#pragma comment(lib, "dxguid.lib" )
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")

class DXC {

public:

	DXC();
	~DXC();

private:

	void InitCap();
	void Release();
	HRESULT Capture();
	std::vector<IDXGIAdapter*> GetAdapter();
	HRESULT CreateDevice();
	HRESULT InitResource();

	// Driver types supported
	ID3D11Device* lDevice;
	ID3D11DeviceContext* lImmediateContext;
	IDXGIOutputDuplication* lDeskDupl;
	ID3D11Texture2D* lAcquiredDesktopImage;
	ID3D11Texture2D* lGDIImage;
	ID3D11Texture2D* lDestImage;

	DXGI_OUTPUT_DESC lOutputDesc;
	DXGI_OUTDUPL_DESC lOutputDuplDesc;

	std::vector<IDXGIAdapter*> AdapterList;
	IDXGIAdapter* lDxgiAdapter;

};