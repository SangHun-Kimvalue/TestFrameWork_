#include "pch.h"
#include "DXCaptureClass.h"
#include "NVEClass.h"
#include <thread>

int main()
{
	HRESULT hr;
	DXCapClass* Capture = new DXCapClass();
	NVEClass* Encode = new NVEClass(1920, 1080);			//���� ���� �޾ƿ;���.


	//std::shared_ptr<BYTE> I_data(new BYTE[m_CaptureWidth * m_CaptureHeight * 4], std::default_delete<BYTE[]>());
	std::shared_ptr<BYTE> I_data(new BYTE[1920 * 1080 * 4], std::default_delete<BYTE[]>());
	static int Count;

	while (1) {
		
		hr = Capture->Capture(I_data);
		if (SUCCEEDED(hr)) {
			std::cout << "SUCCEEDED!!" << std::endl;
			//ť�� ���̺�
			Sleep(1000);
			Count = 0;
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

	delete Capture;
	delete Encode;

	return 0;
}