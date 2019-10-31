// DXGI_Duplication.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "DXGI_Duplication.h"


ScreenCaptureInfo* GetMonitorDeviceInfo(int *MonitorCount)
{
	ScreenCaptureInfo* pCaptureInfo = NULL;
	IDirect3D9* pD3D9 = nullptr;

	pD3D9 = Direct3DCreate9(D3D_SDK_VERSION);

	if (pD3D9)
	{
		UINT dwAdapterCount = pD3D9->GetAdapterCount();
		*MonitorCount = (int)dwAdapterCount;
		pCaptureInfo = new ScreenCaptureInfo[dwAdapterCount];

		for (UINT iAdapter = 0; iAdapter < dwAdapterCount; iAdapter++)
		{
			bool isChange = false;
			D3DCAPS9 caps;
			pD3D9->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &caps);

			D3DADAPTER_IDENTIFIER9 outputDesc;
			ZeroMemory(&outputDesc, sizeof(D3DADAPTER_IDENTIFIER9));
			pD3D9->GetAdapterIdentifier(iAdapter, 0, &outputDesc);

			HMONITOR hMonitor = pD3D9->GetAdapterMonitor(iAdapter);

			MONITORINFOEX mi;
			mi.cbSize = sizeof(MONITORINFOEX);
			GetMonitorInfo(hMonitor, &mi);

			pCaptureInfo[iAdapter].MonitorIndex = iAdapter;
			pCaptureInfo[iAdapter].Region.left = mi.rcMonitor.left;
			pCaptureInfo[iAdapter].Region.top = mi.rcMonitor.top;
			pCaptureInfo[iAdapter].Region.right = mi.rcMonitor.right;
			pCaptureInfo[iAdapter].Region.bottom = mi.rcMonitor.bottom;
		}

		SAFE_RELEASE(pD3D9);
	}
	else
	{
		printf("Direct3DCreate9 Fail !!!\n");
	}

	return pCaptureInfo;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{

	ScreenCaptureInfo* pDeviceInfo = new ScreenCaptureInfo();
	int monitorCnt = 0;
	pDeviceInfo = GetMonitorDeviceInfo(&monitorCnt);

	std::cout << "현재 연결된 모니터 개수는 " << monitorCnt << "개 입니다." << std::endl;
	int width; int height;

	for (int i = 0; i < monitorCnt; i++)
	{
		width = pDeviceInfo[i].Region.right - pDeviceInfo[i].Region.left;
		height = pDeviceInfo[i].Region.bottom - pDeviceInfo[i].Region.top;
		std::cout << "모니터 index " << pDeviceInfo[i].MonitorIndex << " 의 width : " << width << " Height: " << height << std::endl;
	}
	int select = 1;
	
	DXGIScreenCapture* Cap = new DXGIScreenCapture(pDeviceInfo[select]);
	std::shared_ptr<BYTE> output(new BYTE[width * height * 4], std::default_delete<BYTE[]>());
	
	while (1) {
		Cap->Capture(output);
	}

	return 0;
}


