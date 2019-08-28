#pragma once

#include "RTSP_Client.h"
#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"
#include  "RTSPClient.hh"
#pragma comment(lib, "winmm.lib")
#include <Windows.h>
#include <thread>
#include <iostream>

#define REQUEST_STREAMING_OVER_TCP False
#define RTSP_CLIENT_VERBOSITY_LEVEL 1 // by default, print verbose output from each "RTSPClient"

struct StreamClientState {
	MediaSubsessionIterator* iter;
	MediaSession* session;
	MediaSubsession* subsession;
	TaskToken streamTimerTask;
	double duration;
};

class LiveRTSPClient : public RTSP_Client, public RTSPClient
{
public:
	
	//LiveRTSPClient(UsageEnvironment & env, char const * rtspURL, int verbosityLevel, char const * applicationName, portNumBits tunnelOverHTTPPortNum);

	~LiveRTSPClient();

	static LiveRTSPClient* createNew(UsageEnvironment& env, char const* rtspURL,
		int verbosityLevel, char const* applicationName, portNumBits tunnelOverHTTPPortNum);

	virtual void Release();
	virtual void Run();
	virtual const char* Get_URL();
	virtual const char* Get_Name();
	virtual bool Initialize(const char* URI, const char* ProgName);
	virtual void Restart();
	virtual const char* Get_Status();
	virtual const char* Get_SDP();

	bool SetLoopSatus(bool Status);
	
	void Play();
	void Option();
	void Description();
	void Setup();
	void TearDown();
	void GetParameter();

	void startAlive();
	bool m_SAlive;

private:

	const unsigned Get_Timeout();
	bool KeepAlive();

	UsageEnvironment *env;
	//RTSPClient	 *m_Client;

	std::thread AliveThread;

	unsigned timeout;
	char eventLoopWatchVariable;
	char* m_URL;
	char* m_ProgName;


protected:
	LiveRTSPClient(UsageEnvironment& env, char const* rtspURL,
		int verbosityLevel, char const* applicationName, portNumBits tunnelOverHTTPPortNum);

};


