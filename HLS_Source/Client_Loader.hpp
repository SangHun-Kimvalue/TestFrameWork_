#pragma once
#include "ClientFormat.h"
#include "IClient.h"
//#include "../HLS_RTSP_Client/FFMPEG_Client.h"

typedef IClient* (*GetClientInstance)(CLI info, int Qsize);
static bool Loader_Instanced;

class Client_Loader
{
public:
	static Client_Loader* GetInstance(){
		
		static Client_Loader* Instance;

		if (Instance != nullptr)	
			return Instance;
		else						
			return Instance = new Client_Loader();
	}

	IClient* CreateClient(CLI info) {

		std::string ClientType = "";

		switch (info.Type) {
		case CT_RTSP_FF_CLIENT: {
			ClientType = "FFMPEG_Client";
			break;
		}
		case CT_RTSP_LIVE_CLIENT:
			ClientType = "LIVE_Client";
			break;
		case CT_LOCAL_FILE_CLIENT:
			ClientType = "FILE_Client";
			break;
		case CT_HLS_CLIENT:
			ClientType = "HLS_Client";
			break;
		case CT_RTMP_CLIENT:
			ClientType = "RTMP_Client";
			break;
		case CT_NOT_DEFINED:
		default:
			std::cout << " Client Type was not Defined - Create Default " << std::endl;
			return nullptr;
			break;
		}
		return CallInstance(info, ClientType);
	}

private:

	IClient* CallInstance(CLI info, std::string ClientType) {

		std::wstring wClientType = L"";
		wchar_t clientdll[128] = {};
		memset(clientdll, 0, 128);
		wClientType = std::wstring(ClientType.begin(), ClientType.end());
#ifdef _DEBUG
		swprintf_s(clientdll, L"%sD.dll", wClientType.c_str());
#else
		swprintf_s(clientdll, L"%s.dll", wClientType.c_str());
#endif

		//HINSTANCE hdll = LoadLibrary(L"DllTest.dll");
		//std::wstring wtemp = clientdll;			ClientType.assign(wtemp.begin(), wtemp.end());
		hdll = LoadLibrary(clientdll);
		if (hdll == NULL) {
			std::cerr << "File Not Found : " << ClientType << std::endl;
			//break;
		}

		auto Instance = (GetClientInstance)::GetProcAddress(hdll, "GetInstance");
		if (Instance == NULL) {
			std::cerr << "Not Found" << std::endl;
			FreeLibrary(hdll);
			//break;
		}
		IClient* New_Client = nullptr;
		New_Client = Instance(info, 50);
		//break;

		return New_Client;
	}


private:

	Client_Loader(){
		Loader_Instanced = true;
	}
	~Client_Loader() { Loader_Instanced = false; FreeLibrary(hdll);	}

	HMODULE hdll;

	Client_Loader* m_CLoader;

};

