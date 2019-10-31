#pragma once
#include <iostream>
#include <string>

typedef enum RTSP_Server_Type { LIVE555, INVALID } RST;
typedef enum Source_Type { LOCALFILE, RTSP_CLIENT, NOTSUPPORT } ST;
typedef enum RTSP_Status {DEFAULT, READY, RUNNING, STOPPED, D_EOF} RS;

class __declspec(dllexport) m_RTSPServer
{
public:

	//virtual void createNew() = 0;
	m_RTSPServer() : m_port(0), quit(-1) {}
	virtual ~m_RTSPServer() {}
	
	virtual void Release() = 0;
	virtual bool Initialize(int port, std::string Filename) = 0;
	virtual void Start() = 0;
	virtual void Stop() = 0;
	virtual char*  GetStatus() = 0;
	virtual char*  PrintSessionList() = 0;
	virtual int SessionCount() = 0;

	char* URL;
	int m_port;
	char quit;

protected:

};
