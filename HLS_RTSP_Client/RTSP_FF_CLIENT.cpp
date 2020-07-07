// rtsp_ff_client.cpp : Defines the exported functions for the DLL application.
//
#include "FFMPEG_Client.h"

#define EXPORT extern "C" __declspec(dllexport)

EXPORT IClient* GetInstance(CLI info, int Qsize)
{
	return new FFMPEG_Client(info, Qsize);
}