#pragma once
#include "Server.h"
#include "LiveRTSPServer.h"

class RTSP_Server : public Server
{
public:
	RTSP_Server();
	~RTSP_Server();

	virtual void Release();
	virtual void Run();
	const char* GetURL();
	virtual bool Initialize(int port);
	virtual void Restart();

private:

	LiveRTSPServer* m_RTSP_Server;

	virtual bool Init_Net();

};

