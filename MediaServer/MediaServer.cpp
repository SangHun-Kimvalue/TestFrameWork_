#include "MediaServer.h"

void MediaServer::InitLoad() {

	Loader = InitLoader::createNew();

	std::string Server_type = Loader->Load("SET", "SERVERTYPE");

	if (Server_type == "live") {
		Server_Type = LIVE555;
		std::cout << "Live555 Server Type" << std::endl;
	}
	else {
		Server_Type = INVALID;
		std::cerr << "Invalid Server Type" << std::endl;
		delete Loader;
		return;
	}

	std::string t_Source_type = Loader->Load("SET", "SOURCETYPE");
	
	if (t_Source_type == "local") {
		Source_Type = LOCALFILE;
		std::cout << "Use localfile" << std::endl;

		FileName = Loader->Load("SET", "FILENAME");
		if (FileName == "") {
			std::cerr << "File Name is empty" << std::endl;
			delete Loader;
			return;
		}

		std::cout << "Find file : " << FileName << std::endl;
	}
	else {
		Source_Type = NOTSUPPORT;
		delete Loader;
		return;
	}

	delete Loader;
	return;
}

bool MediaServer::Initialize() {

	InitLoad();

	wchar_t* dllname = nullptr;

	if (Server_Type == INVALID) {

		std::cerr << "Invalid Server Type" << std::endl;
		return false;
	}
	else if (Server_Type == LIVE555) {

		//wsprintf(dllname , L"m_%sRTSP_Server.dll", L"Live");
		
		RTSP_Server = DllLoad("LiveRTSP_Server.dll");
		if (RTSP_Server == nullptr) {
			std::cerr << "Initailize Failed" << std::endl;
			return false;
		}
	}

	if (Source_Type == NOTSUPPORT || RTSP_Server == NULL) {
		std::cerr << "Source Not Support Type" << std::endl;
		FreeLibrary(hDLL);
		return false;
	}
	else if (Source_Type == LOCALFILE) {

		RTSP_Server->Initialize(554, FileName);
	}

	return true;
}

m_RTSPServer* MediaServer::DllLoad(std::string sdllname) {


	if (sdllname == "LiveRTSP_Server.dll") {
		//wsprintf(dllname, L"%s.dll", );
		hDLL = LoadLibrary(L"LiveRTSP_Server.dll");
	}
	else {

		std::cerr << "Dll file not found" << std::endl;
		return nullptr;
	}

	if (hDLL == NULL) {
		std::cerr << "FIle Not Found" << std::endl;
		return NULL;
	}

	fptr = (RTSP_Server_fptr)::GetProcAddress(hDLL, "GetInstance");
	//DllClass* GetInstance = (DllClass*)GetProcAddress(hdll, "_GetInstance");
	if (fptr == NULL) {
		std::cerr << "File Not Found" << std::endl;
		FreeLibrary(hDLL);
		return NULL;
	}

	return fptr();

}

void MediaServer::Release() {

	RTSP_Server->Release();
	if (RunThread.joinable() == true) {
		RunThread.join();
	}
	delete RTSP_Server;

}

void MediaServer::Run() {

	//Thread_D_P = thread(&Local_fileClass::Thread_Decode, this);
	RunThread = std::thread(&m_RTSPServer::Run, RTSP_Server);

	return;
}

//void RTSP_Manager::Restart() {
//
//	RTSP_Server->Restart();
//
//	return;
//}

char* MediaServer::Get_URL() {


	return RTSP_Server->GetURL();
}

char* MediaServer::Get_Stream_Name() {
	return RTSP_Server->GetStreamName();
}

bool MediaServer::Get_Status() {

	if (RTSP_Server->quit == 0) {
		std::cout << "Server stopped" << std::endl;
		return true;
	}
	else {
		std::cout << "Server is runing" << std::endl;
		return false;
	}
}