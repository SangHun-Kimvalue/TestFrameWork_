#pragma once

#include "resource.h"

#include <d3dcommon.h>
#include <d3d9.h>
#include <chrono>
#include <dxgi1_2.h>
#include <d3dcommon.h>
#include <d3d11.h>
#include <d3dx11tex.h>
#include <memory>
#include <iostream>
#include <Windows.h>

#pragma comment(lib, "dxguid.lib" )
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dx11.lib")
#ifdef _DEBUG
#pragma comment(lib, "d3d9.lib")
#else
#pragma comment(lib, "d3d9.lib")
#endif


using namespace std::chrono;
#define SAFE_RELEASE(p)      { if (p) { (p)->Release(); (p)=nullptr; } }

enum ScreenCaptureType
{
	DX, OPENGL, DXGI
};

struct ScreenCaptureInfo
{
	ScreenCaptureType	Type;
	int					MonitorIndex;
	RECT				Region;
	int					DownScaleFlag;
	IDXGIAdapter*		pDxgiAdapter;
	IDXGIOutput*		pDxgiOutput;
};

class IScreenCapture
{
public:
	IScreenCapture(ScreenCaptureInfo info) {}
	virtual ~IScreenCapture() {}
	virtual int Capture(std::shared_ptr<BYTE> output) = 0;

	virtual int GetScreenWidth() = 0;
	virtual int GetScreenHeight() = 0;
};

class DXGIScreenCapture : public IScreenCapture
{
public:

	DXGIScreenCapture(ScreenCaptureInfo info) : IScreenCapture(info)
	{
	
		// agent flow에서 선택된 모니터의 rect를 받아옴
		// dx 초기화시에 monitor rect와 비교해서 surface를 생성한다. 

		int width = info.Region.right - info.Region.left;
		int height = info.Region.bottom - info.Region.top;

		RECT rect = {
			info.Region.left,
			info.Region.top,
			info.Region.right,
			info.Region.bottom };

		m_ScreenRECT = rect;
		m_MonitorInx = info.MonitorIndex;

		pDxgiAdapter = info.pDxgiAdapter;
		pDxgiOutput = info.pDxgiOutput;
		InitD3D(m_ScreenRECT, m_MonitorInx);

		m_CaptureFailCnt = 0;

		//m_swsContext = NULL;
		//InitFFmpegEncoder(m_ScreenWidth, m_ScreenHeight);

	}
	~DXGIScreenCapture() {
		DeinitD3D();
	};


	int Capture(std::shared_ptr<BYTE> output)
	{
		HRESULT hr(E_FAIL);
		/*IDXGIResource* DesktopResource;*/
		DXGI_OUTDUPL_FRAME_INFO FrameInfo;

		if (m_DeskDupl == NULL)
		{
				DeinitD3D();
			InitD3D(m_ScreenRECT, m_MonitorInx);
			return -1;
		}

		HWND hWnd = GetDesktopWindow();
		HDC hdc = GetWindowDC(hWnd);
		SetPixel(hdc, m_ScreenRECT.left, m_ScreenRECT.top, RGB(1, 1, 1));
		SetPixel(hdc, m_ScreenRECT.right, m_ScreenRECT.bottom, RGB(1, 1, 1));
		//SetPixel(hdc, m_ScreenRECT.right/2, m_ScreenRECT.bottom/2, RGB(1, 1, 1));
		ReleaseDC(hWnd, hdc);
		InvalidateRect(hWnd, NULL, FALSE);
		//UpdateWindow(hWnd);

		int lTryCount = 4;

		do
		{
			hr = m_DeskDupl->ReleaseFrame();

			// Get new frame
			hr = m_DeskDupl->AcquireNextFrame(500, &FrameInfo, &DesktopResource);

			if (SUCCEEDED(hr))
			{
				break;
			}

			if (hr == DXGI_ERROR_WAIT_TIMEOUT)
			{
				HWND hWnd = GetDesktopWindow();
				HDC hdc = GetWindowDC(hWnd);
				SetPixel(hdc, m_ScreenRECT.left, m_ScreenRECT.top, RGB(1, 1, 1));
				SetPixel(hdc, m_ScreenRECT.right, m_ScreenRECT.bottom, RGB(1, 1, 1));
				//SetPixel(hdc, m_ScreenRECT.right / 2, m_ScreenRECT.bottom / 2, RGB(0, 0, 0));
				ReleaseDC(hWnd, hdc);
				InvalidateRect(hWnd, NULL, FALSE);
				UpdateWindow(hWnd);
				continue;
			}

			Sleep(1);

		} while (--lTryCount > 0);

		//hr = m_DeskDupl->AcquireNextFrame(500, &FrameInfo, &DesktopResource);
		if (FAILED(hr))
		{
			m_CaptureFailCnt++;

			if (m_CaptureFailCnt > 20)
			{
				DeinitD3D();
				InitD3D(m_ScreenRECT, m_MonitorInx);
				m_CaptureFailCnt = 0;
			}
			return -1;
		}

		// If still holding old frame, destroy it
		if (m_AcquiredDesktopImage)
		{
			m_AcquiredDesktopImage->Release();
			m_AcquiredDesktopImage = nullptr;
		}

		// QI for IDXGIResource
		hr = DesktopResource->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void **>(&m_AcquiredDesktopImage));
		/*DesktopResource->Release();
		DesktopResource = nullptr;*/
		if (FAILED(hr))
		{
			return -1;
		}

		// Copy image into GDI drawing texture
		m_ImmediateContext->CopyResource(m_GDIImage, m_AcquiredDesktopImage);

		// Draw cursor image into GDI drawing texture
		IDXGISurface1* lIDXGISurface1;
		hr = m_GDIImage->QueryInterface(IID_PPV_ARGS(&lIDXGISurface1));
		if (FAILED(hr))
			return -1;

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
				DrawIconEx(lHDC, lCursorPosition.x - m_ScreenRECT.left, lCursorPosition.y, lCursorInfo.hCursor,
					0, 0, 0, 0, DI_NORMAL | DI_DEFAULTSIZE);
				lIDXGISurface1->ReleaseDC(nullptr);
			}
		}

		// Copy image into CPU access texture
		m_ImmediateContext->CopyResource(m_DestImage, m_GDIImage);

		// Copy from CPU access texture to bitmap buffer
		D3D11_MAPPED_SUBRESOURCE resource;
		//UINT subresource = D3D11CalcSubresource(0, 0, 0);
		m_ImmediateContext->Map(m_DestImage, 0, D3D11_MAP_READ, 0, &resource);

		BYTE* sptr = reinterpret_cast<BYTE*>(resource.pData);
		memcpy(output.get(), sptr, resource.RowPitch * m_OutputDuplDesc.ModeDesc.Height);
		//output.reset(sptr);
		m_ImmediateContext->Unmap(m_DestImage, 0);

		//m_DeskDupl->ReleaseFrame();

		return 0;
	}

	HRESULT InitD3D(RECT rect, int outputInx)
	{
		HRESULT hr(E_FAIL);
		// Driver types supported
		D3D_DRIVER_TYPE driverType = D3D_DRIVER_TYPE_HARDWARE;

		// Feature levels supported
		D3D_FEATURE_LEVEL gFeatureLevels[] =
		{
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0,
			D3D_FEATURE_LEVEL_9_1
		};
		UINT gNumFeatureLevels = ARRAYSIZE(gFeatureLevels);

		D3D_FEATURE_LEVEL lFeatureLevel;

		m_ScreenWidth = m_ScreenRECT.right - m_ScreenRECT.left;
		m_ScreenHeight = m_ScreenRECT.bottom - m_ScreenRECT.top;

		if (m_ScreenWidth == 0 || m_ScreenHeight == 0)
			return E_FAIL;

		hr = D3D11CreateDevice(
			nullptr,
			driverType,
			nullptr,
			0,
			gFeatureLevels,
			gNumFeatureLevels,
			D3D11_SDK_VERSION,
			&m_Device,
			&lFeatureLevel,
			&m_ImmediateContext);

		if (FAILED(hr))
		{
			printf("D3D11CreateDevice Fail [File:%s , Line:%d]\n", __FILE__, __LINE__);
			m_Device->Release();
			m_ImmediateContext->Release();

			return hr;
		}

		IDXGIDevice* DxgiDevice = nullptr;
		hr = m_Device->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&DxgiDevice));
		if (FAILED(hr))
		{
			printf("Failed to QI for DXGI Device [File:%s , Line:%d]\n", __FILE__, __LINE__);
			return hr;
		}

		// Get DXGI adapter
		IDXGIAdapter* DxgiAdapter = nullptr;
		hr = DxgiDevice->GetParent(__uuidof(IDXGIAdapter), reinterpret_cast<void**>(&DxgiAdapter));
		DxgiDevice->Release();
		DxgiDevice = nullptr;
		if (FAILED(hr))
		{
			printf("Failed to get parent DXGI Adapter [File:%s , Line:%d]\n", __FILE__, __LINE__);
			return hr;
		}

		// Get output
		IDXGIOutput* DxgiOutput = nullptr;
		hr = DxgiAdapter->EnumOutputs(m_MonitorInx, &DxgiOutput);
		DxgiAdapter->Release();
		DxgiAdapter = nullptr;
		if (FAILED(hr))
		{
			printf("Failed to get specified output [File:%s , Line:%d]\n", __FILE__, __LINE__);
			return hr;
		}

		DxgiOutput->GetDesc(&m_OutputDesc);

		// QI for Output 1
		IDXGIOutput1* DxgiOutput1 = nullptr;
		hr = DxgiOutput->QueryInterface(__uuidof(DxgiOutput1), reinterpret_cast<void**>(&DxgiOutput1));
		DxgiOutput->Release();
		DxgiOutput = nullptr;
		if (FAILED(hr))
		{
			printf("Failed to QI for DxgiOutput1 [File:%s , Line:%d]\n", __FILE__, __LINE__);
			return hr;
		}

		// Create desktop duplication
		hr = DxgiOutput1->DuplicateOutput(m_Device, &m_DeskDupl);
		DxgiOutput1->Release();
		DxgiOutput1 = nullptr;
		if (FAILED(hr))
		{
			if (hr == DXGI_ERROR_NOT_CURRENTLY_AVAILABLE)
			{
				printf("There is already the maximum number of applications using the Desktop Duplication API running, please close one of those applications and then try again. [File:%s , Line:%d]\n", __FILE__, __LINE__);
				return hr;
			}
			printf("Failed to get duplicate output [File:%s , Line:%d]\n", __FILE__, __LINE__);
			return hr;
		}

		m_DeskDupl->GetDesc(&m_OutputDuplDesc);

		// Create GUI drawing texture
		D3D11_TEXTURE2D_DESC desc;
		desc.Width = m_OutputDuplDesc.ModeDesc.Width;
		desc.Height = m_OutputDuplDesc.ModeDesc.Height;
		desc.Format = m_OutputDuplDesc.ModeDesc.Format;
		desc.ArraySize = 1;
		desc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_RENDER_TARGET;
		desc.MiscFlags = D3D11_RESOURCE_MISC_GDI_COMPATIBLE;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.MipLevels = 1;
		desc.CPUAccessFlags = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		hr = m_Device->CreateTexture2D(&desc, NULL, &m_GDIImage);
		if (FAILED(hr))
		{
			printf("Failed Create GUI drawing texture [File:%s , Line:%d]\n", __FILE__, __LINE__);
			return hr;
		}

		if (m_GDIImage == nullptr)
			return 0;

		// Create CPU access texture
		desc.Width = m_OutputDuplDesc.ModeDesc.Width;
		desc.Height = m_OutputDuplDesc.ModeDesc.Height;
		desc.Format = m_OutputDuplDesc.ModeDesc.Format;
		desc.ArraySize = 1;
		desc.BindFlags = 0;
		desc.MiscFlags = 0;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.MipLevels = 1;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
		desc.Usage = D3D11_USAGE_STAGING;
		hr = m_Device->CreateTexture2D(&desc, NULL, &m_DestImage);
		if (FAILED(hr))
		{

			printf("Failed Create CPU access texture [File:%s , Line:%d]\n", __FILE__, __LINE__);
			return hr;
		}

		if (m_DestImage == nullptr)
			return 0;


		//printf("InitCapturer %d is Success [File:%s , Line:%d]\n", m_MonitorInx, __FILE__, __LINE__);
		return S_OK;
	}

	void DeinitD3D()
	{
		DesktopResource->Release();
		DesktopResource = nullptr;

		SAFE_RELEASE(m_Device);
		SAFE_RELEASE(m_ImmediateContext);
		SAFE_RELEASE(m_DeskDupl);
		SAFE_RELEASE(m_AcquiredDesktopImage);
		SAFE_RELEASE(m_GDIImage);
		SAFE_RELEASE(m_DestImage);
	}

	int GetScreenWidth() { return m_ScreenWidth; }
	int GetScreenHeight() { return m_ScreenHeight; }

private:
	ID3D11Device* m_Device = nullptr;
	ID3D11DeviceContext* m_ImmediateContext = nullptr;
	IDXGIOutputDuplication* m_DeskDupl = nullptr;
	ID3D11Texture2D* m_AcquiredDesktopImage = nullptr;
	ID3D11Texture2D* m_GDIImage = nullptr;
	ID3D11Texture2D* m_DestImage = nullptr;

	DXGI_OUTPUT_DESC m_OutputDesc;
	DXGI_OUTDUPL_DESC m_OutputDuplDesc;

	int					m_ScreenWidth;
	int					m_ScreenHeight;

	RECT				m_ScreenRECT;
	int					m_MonitorInx;

	int					m_CaptureFailCnt;

	IDXGIAdapter* pDxgiAdapter;
	IDXGIOutput* pDxgiOutput;
	IDXGIResource* DesktopResource;
};