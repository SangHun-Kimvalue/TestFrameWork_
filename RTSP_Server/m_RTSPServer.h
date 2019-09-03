#pragma once
#include <iostream>

class m_RTSPServer
{
public:

	m_RTSPServer(): m_port(0), quit(-1) {}
	
	virtual ~m_RTSPServer() {}
	
	virtual void Release() = 0;
	virtual bool Initialize(int port) = 0;
	virtual void Run() = 0;
	virtual std::string GetURL() = 0;
	virtual void Restart() = 0;
	virtual std::string  GetStreamName() = 0;
	

	std::string URL;
	int m_port;
	char quit;

private:
	//int portNumber;
	//int httpTunnelingPort;

};
