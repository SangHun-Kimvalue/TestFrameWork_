#pragma once

#include "Client.h"
#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"

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
	virtual bool Get_Status();
	virtual bool KeepAlive();
	
	bool SetLoopSatus(bool Status);
	const char* Get_SDP();
	void Play();
	void Option();
	void Description();
	void Setup();
	void TearDown();

private:

	const unsigned Get_Timeout();

	TaskScheduler    *scheduler;
	UsageEnvironment *env;
	RTSPClient	 *m_Client;

	unsigned timeout;
	char eventLoopWatchVariable;

};


