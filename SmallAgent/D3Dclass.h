#pragma once;
////////////////////////////////////////////////////////////////////////////////
// Filename: d3dclass.h
////////////////////////////////////////////////////////////////////////////////
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dx11.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dxguid.lib")

//////////////
// INCLUDES //
//////////////
#include <dxgi.h>
#include <d3dcommon.h>
#include <d3d11.h>
#include <d3dx10math.h>
#include <d3dx11async.h>
#include <d3dx11tex.h>
//#include "Debug_logClass.h"
#include <d2d1.h>
#include <d2d1_3.h>
#include <dwrite_3.h>
#include <D2d1_1helper.h>
#include <Pdh.h>

////////////////////////////////////////////////////////////////////////////////
// Class name: D3DClass
////////////////////////////////////////////////////////////////////////////////
enum Texture_Format {YUV420P, NV12};

class D3DClass
{
public:
	D3DClass();
	~D3DClass();

	struct VertexType
	{
		D3DXVECTOR3 position;
		D3DXVECTOR4 color;
		D3DXVECTOR2 texture;
	};

	bool InitializeBuffers3D(HWND, const int selectdecode);
	bool Update_Tex_RGB(AVFrame* VFrame);
	bool Update_Tex_YUV420P(AVFrame* VFrame);
	bool Update_Tex_NV12(AVFrame* VFrame);
	void Shutdown();
	int Render(bool Show_Debug);
	bool Debug_position(bool Show_Debug);
	AVFrame* pFrame;
	int Megabitrate;

	int k_i = VK_SPACE;
	//D3DXMATRIX m_orthoMatrix;					//직교 투영

	ID3D11ShaderResourceView* m_ShaderResourceView;
	ID3D11ShaderResourceView* YUV_SRView[3];
	ID3D11Device* m_device;
	ID3D11DeviceContext* m_deviceContext;
	float VideoFPS;
	float RenderCycle;
	float sleeptime;
	int m_imageWidth, m_imageHeight;
	int queuesize;
	//Debug_logClass *log;
	HWND d3d_hwnd;
	

private:
	int b_k_i = 0;
	bool m_vsync_enabled;

	ID3D11Buffer *m_vertexBuffer, *m_indexBuffer;
	IDXGISwapChain* m_swapChain;
	ID3D11RenderTargetView* m_renderTargetView;
	ID3D11InputLayout* m_layout;
	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;

	ID3D11Texture2D* m_2DTex;
	ID3D11Texture2D* m_Ybuffer;
	ID3D11Texture2D* m_Ubuffer;
	ID3D11Texture2D* m_Vbuffer;
	D3D11_MAPPED_SUBRESOURCE S_mappedResource_tt;
	DWORD	Stride;

	ID2D1Device*	m_D2DDevice;
	ID2D1Factory1* pD2DFactory_;
	IDXGIDevice*	pdxgiDevice;
	IDXGISurface*	BackBuffer;
	ID2D1Bitmap1* BitmapTarget;
	IDWriteFactory* pDWriteFactory_;
	IDWriteTextFormat* pTextFormat_;
	ID2D1Bitmap1* pD2DTargetBitmap;
	ID2D1SolidColorBrush* pBlackBrush_;
	ID2D1RenderTarget *D2_RenderTarget;
	ID2D1DCRenderTarget *D2DC_RenderTarget;
	const wchar_t* wszText_;
	UINT32 cTextLength_;
	D2D1_RECT_F layoutRect;
	IDWriteTextLayout* pTextLayout_;
	D2D1_POINT_2F origin;
	ID2D1DeviceContext* m_D2DDevicctx;

	int m_vertexCount, m_indexCount;
	int RenderFPS;
	unsigned long m_startTime;
	int cpuusage;

	void Get_Render_FPS();
	void Draw_Text();
	bool Text_Set();
	bool CreateResource_RGB();
	bool CreateResource_YUV420P(enum Texture_Format);
	void OutputShaderErrorMessage(ID3D10Blob*, HWND, WCHAR*);

};