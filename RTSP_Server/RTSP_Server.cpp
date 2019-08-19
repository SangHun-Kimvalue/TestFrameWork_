#include "RTSP_Server.h"

RTSP_Server::RTSP_Server(){
}

RTSP_Server::~RTSP_Server() 
{
	Release();
}

void RTSP_Server::Release() {

	m_RTSP_Server->Release();
}

const char* RTSP_Server::GetURL() {

	URL = m_RTSP_Server->URL;
	if (URL[0] == ' ') {
		return "\nNot valid URL\n";
	}

	const char* temp = URL;

	return temp;
}

void RTSP_Server::Restart() {

	Release();
	Initialize(m_port);
	Run();
}

bool RTSP_Server::Initialize(int port) {

	bool res = false;
	m_port = port;

	res = Init_Net();
	if (res) {
		//std::cout << "\nInit_Net Error" << std::endl;
		return res;
	};

	//Set_Format();
	//res = Init_Stream();
	//if (res) {};

	return res;
}

bool RTSP_Server::Init_Net() {

	bool res = false;
	m_RTSP_Server = new LiveRTSPServer(m_port);
	res = m_RTSP_Server->Initialize();
	//URL = m_RTSP_Server->URL;

	return res = true;
}

//bool RTSP_Server::Init_Stream() {
//	bool res = false;
//	return res;
//}
//
//void RTSP_Server::Set_Format() {
//
//	return;
//}

void RTSP_Server::Run() {

	m_RTSP_Server->run();

	return;
}
