#pragma once

#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"
#include "RTSPClient.hh"
#include "Base64.hh"

#include "RTSP_Client.h"
#include <thread>
#include <iostream>
#include <string>

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
	virtual bool Initialize();
	virtual void Restart();
	virtual void start();
	virtual void stop();

	virtual const char* Get_Status();
	virtual const char* Get_SDP();
	virtual const char* Get_URL();
	virtual const char* Get_Name();


	bool SetLoopSatus(bool Status);

	void SetTrack(const char* input);
	void SetURL(const char* input);
	bool MakeRequest(const char* Command);

	const unsigned Get_Timeout();
	
	StreamClientState scs; // alias

private:

	virtual void Run();

	virtual unsigned sendDescribeCommand(Authenticator* authenticator = NULL);
	virtual unsigned sendOptionsCommand(Authenticator* authenticator = NULL);
	virtual unsigned sendSetupCommand(Authenticator* authenticator = NULL);
	virtual unsigned sendPlayCommand(responseHandler* responseHandler);
	virtual unsigned sendPlayCommand(Authenticator* authenticator = NULL);
	virtual unsigned sendTeardownCommand(Authenticator* authenticator = NULL);
	void CompareCommand(std::string Command);

	
	virtual Boolean handleDESCRIBEResponse(char const* bodyStart);
	virtual Boolean handleOPTIONSResponse(char const* transportParamsStr);
	virtual Boolean handleSETUPResponse(char const* sessionParamsStr, char const* transportParamsStr,
		Boolean streamUsingTCP);
	virtual Boolean handlePLAYResponse(MediaSession& session, char const* scaleParamsStr, const char* speedParamsStr,
		char const* rangeParamsStr, char const* rtpInfoParamsStr);
	virtual Boolean handlePLAYResponse(MediaSession& session, MediaSubsession& subsession,
		char const* scaleParamsStr, const char* speedParamsStr,
		char const* rangeParamsStr, char const* rtpInfoParamsStr);
	//virtual Boolean thandleTEARDOWNResponse(MediaSession& session, MediaSubsession& subsession);
	//virtual Boolean thandleGET_PARAMETERResponse(char const* parameterName, char*& resultValueString, char* resultValueStringEnd);
	//virtual Boolean thandleAuthenticationFailure(char const* wwwAuthenticateParamsStr);
	//virtual Boolean tresendCommand(RequestRecord* request);

	Boolean setRequestFields(RequestRecord* request,
		char*& cmdURL, Boolean& cmdURLWasAllocated,
		char const*& protocolStr,
		char*& extraHeaders, Boolean& extraHeadersWereAllocated);
	// used to implement "sendRequest()"; subclasses may reimplement this (e.g., when implementing a new command name)
	//char* createAuthenticatorString(char const* cmd, char const* url);

	UsageEnvironment *env;
	//RTSPClient	 *m_Client;
	enum STATES { SOPTION, SDESCRIBE, SSETUP, SPLAY, SEXIT, SERROR };
	STATES state;
	STATES handlestate;

	std::thread AliveThread;

	std::string sessiontrack;
	unsigned short ClientPort;
	portNumBits ServerPort;
	std::string prefix; 
	std::string separator;
	std::string suffix;

	bool havesession;
	bool Pharse_SDP;
	bool Use_Getparam;
	char eventLoopWatchVariable;
	//RTSPClient* parent;

	bool m_SAlive;
	bool Looping;

protected:
	LiveRTSPClient(UsageEnvironment& env, char const* rtspURL,
		int verbosityLevel, char const* applicationName, portNumBits tunnelOverHTTPPortNum);

};


