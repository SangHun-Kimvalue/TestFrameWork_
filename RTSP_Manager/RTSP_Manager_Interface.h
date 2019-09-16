#pragma once
#include "RTSP_Manager.h"

#ifdef RTSPMANAGER_EXPORTS
#define RTSPMANAGER_API __declspec(dllexport)
#else
#define RTSPMANAGER_API __declspec(dllimport)
#endif

class RTSP_Manager;

extern "C"
{
	RTSPMANAGER_API RTSP_Manager * __stdcall Create_Manager();
	RTSPMANAGER_API bool __stdcall Initialize(RTSP_Manager* manager);
	RTSPMANAGER_API void __stdcall Release(RTSP_Manager* manager);
	RTSPMANAGER_API void __stdcall Run(RTSP_Manager* manager);
	RTSPMANAGER_API std::string __stdcall Get_URL(RTSP_Manager* manager);
	RTSPMANAGER_API std::string __stdcall Get_Stream_Name(RTSP_Manager* manager);
	RTSPMANAGER_API void __stdcall Get_Status(RTSP_Manager* manager);
}