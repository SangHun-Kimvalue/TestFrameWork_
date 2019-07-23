#include "pch.h"
#include "DXCaptureClass.h"
#include "NVEClass.h"
#include <thread>

int main()
{
	HRESULT hr;

	DXCapClass* Capture = new DXCapClass();
	std::shared_ptr<ID3D11Device*> mDevice = std::make_shared<ID3D11Device*>(Capture->GetDevice());
	std::shared_ptr<ID3D11DeviceContext*> mDeviceContext = std::make_shared<ID3D11DeviceContext*>(Capture->GetDC());

	NVEClass* Encode = new NVEClass(Capture->nwid, Capture->nhei, *mDevice, *mDeviceContext);

	std::shared_ptr<BYTE> I_data(new BYTE[Capture->nwid * Capture->nhei * 4], std::default_delete<BYTE[]>());
	
	//NVEClass* Encode = new NVEClass(1920, 1080, *mDevice, *mDeviceContext);			//���� ���� �޾ƿ;���.
	//NVEClass* Encode = new NVEClass(1920, 1080);			//���� ���� �޾ƿ;���.

	static int Count;

	while (1) {
		
		hr = Capture->Capture(I_data);
		if (SUCCEEDED(hr)) {
			std::cout << "Capture SUCCEEDED!!" << std::endl;
			//ť�� ���̺�
			Sleep(1000);
			Count = 0;
			hr = Encode->Encode(Capture->GetTex());
			if (SUCCEEDED(hr))
				std::cout << "Encode SUCCEEDED" << std::endl;

		}
		else {
			Count++;
			I_data.reset();
			I_data = NULL;
			if (Count > 59) {
		
				break;
			}
			continue;
		}

		Sleep(10);
	}

	delete Encode;
	delete Capture;
	

	return 0;
}