#pragma once
#include <iostream>

class RTSP_Client
{
public:

	RTSP_Client(): m_URL(0),m_ProgName(0), m_SAlive(false), timeout(0){};
	virtual ~RTSP_Client() {};

	virtual void Release() = 0;
	virtual void Run() = 0;
	virtual const char* Get_URL() = 0;
	virtual const char* Get_Name() = 0;
	virtual bool Initialize(const char* URI, const char* ProgName) = 0;
	virtual void Restart() = 0;
	virtual const char* Get_Status() = 0;
	virtual const char* Get_SDP() = 0;

	//virtual bool SetLoopSatus(bool Status);
	virtual void Play() = 0;
	virtual void Option() = 0;
	virtual void Description() = 0;
	virtual void Setup() = 0;
	virtual void TearDown() = 0;
	virtual void GetParameter() = 0;

	//void startAlive();
	bool m_SAlive;
	char* m_URL;
	char* m_ProgName;

	//bool KeepAlive();

	unsigned timeout;
	
private:

};


