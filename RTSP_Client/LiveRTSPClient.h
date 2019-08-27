#pragma once

#include "Client.h"
#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"
#pragma comment(lib, "winmm.lib")
#include <Windows.h>
#include <thread>

#define REQUEST_STREAMING_OVER_TCP False
#define RTSP_CLIENT_VERBOSITY_LEVEL 1 // by default, print verbose output from each "RTSPClient"

struct StreamClientState {
	MediaSubsessionIterator* iter;
	MediaSession* session;
	MediaSubsession* subsession;
	TaskToken streamTimerTask;
	double duration;
};

class LiveRTSPClient : public Client
{
public:
	LiveRTSPClient();
	~LiveRTSPClient();

	virtual void Release();
	virtual void Run();
	virtual const char* Get_URL();
	virtual const char* Get_Name();
	virtual bool Initialize(const char* URI, const char* ProgName);
	virtual void Restart();
	virtual const char* Get_Status();
	
	

	bool SetLoopSatus(bool Status);
	const char* Get_SDP();
	void Play();
	void Option();
	void Description();
	void Setup();
	void TearDown();
	void GetParameter();

	void startAlive();


private:

	const unsigned Get_Timeout();
	bool KeepAlive();

	TaskScheduler    *scheduler;
	UsageEnvironment *env;
	RTSPClient	 *m_Client;

	std::thread AliveThread;

	unsigned timeout;
	char eventLoopWatchVariable;

};


