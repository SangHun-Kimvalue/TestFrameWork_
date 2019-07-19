#pragma once
#include <iostream>
#include <dxgiformat.h>
#include <winerror.h>
#include <windows.h>

typedef struct Frame_Info {

	int Width;
	int Height;
	DXGI_FORMAT Format;

}Frame_info;

static void Check(HRESULT hr, const char* string) {

	if (FAILED(hr))
	{
		std::cerr << string << std::endl;
		Sleep(1000);
		std::cin; std::cin; std::cin;
		return;
	}
}