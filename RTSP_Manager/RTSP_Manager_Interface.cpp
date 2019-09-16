// SmartAlertManager.cpp: DLL 응용 프로그램을 위해 내보낸 함수를 정의합니다.
//

#include "stdafx.h"
#include "RTSP_Manager_Interface.h"

extern "C"
{
	RTSPMANAGER_API RTSP_Manager * __stdcall Create_Manager()
	{
		return new RTSP_Manager();
	}

	RTSPMANAGER_API bool __stdcall Initialize(RTSP_Manager* manager)
	{
		return manager->Initialize();
	}

	RTSPMANAGER_API void __stdcall DeleteManager(RTSP_Manager* manager)
	{
		manager->Release();
		return;
	}

	RTSPMANAGER_API void __stdcall Run(RTSP_Manager* manager)
	{
		manager->Run();
		return;
	}

	RTSPMANAGER_API std::string __stdcall Get_URL(RTSP_Manager* manager)
	{
		return manager->Get_URL();
	}

	RTSPMANAGER_API std::string __stdcall Get_Stream_Name(RTSP_Manager* manager)
	{
		return manager->Get_Stream_Name();
	}

	RTSPMANAGER_API void __stdcall Get_Status(RTSP_Manager* manager)
	{
		manager->Get_Status();
		return;
	}
}