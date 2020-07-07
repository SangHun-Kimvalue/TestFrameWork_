// rtsp_ff_client.cpp : Defines the exported functions for the DLL application.
//
#include "ClientManager.h"

#define EXPORT extern "C" __declspec(dllexport)

EXPORT ClientManager* GetCM()
{
	return CreateCM();
}

