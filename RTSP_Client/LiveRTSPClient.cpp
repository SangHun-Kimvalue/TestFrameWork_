#include "LiveRTSPClient.h"

void shutdownStream(RTSPClient* rtspClient, LiveRTSPClient* Client);
void continueAfterDESCRIBE(RTSPClient* rtspClient, int resultCode, char* resultString);
void continueAfterSETUP(RTSPClient* rtspClient, int resultCode, char* resultString);
void continueAfterPLAY(RTSPClient* rtspClient, int resultCode, char* resultString);

StreamClientState scs; // alias
LiveRTSPClient* m_Live;


void continueAfterPLAY(RTSPClient* rtspClient, int resultCode, char* resultString) {
	Boolean success = False;

	UsageEnvironment& env = rtspClient->envir(); // alias

	std::cout <<  "Started playing session" << std::endl;

	success = True;
	shutdownStream((RTSPClient*)rtspClient, m_Live);
	delete[] resultString;

}

void continueAfterSETUP(RTSPClient* rtspClient, int resultCode, char* resultString) {

	rtspClient->sendPlayCommand(continueAfterPLAY);
}

void shutdownStream(RTSPClient* rtspClient, LiveRTSPClient* m_Client) {
	UsageEnvironment& env = rtspClient->envir(); // alias

	rtspClient->sendTeardownCommand();
	std::cout << "Closing the stream.\n" << std::endl;
	Medium::close(rtspClient);
	// Note that this will also cause this stream's "StreamClientState" structure to get reclaimed.
	m_Client->SetLoopSatus(false);
	
}

void continueAfterDESCRIBE(RTSPClient* rtspClient, int resultCode, char* resultString) {

	UsageEnvironment& env = rtspClient->envir(); // alias
		

	if (resultCode != 0) {
		std::cerr<< "Failed to get a SDP description: " << resultString << std::endl;
		delete[] resultString;
		return;
	}

	char* const sdpDescription = resultString;
	rtspClient->SDP = resultString;
	//env << *rtspClient << "Got a SDP description:\n" << sdpDescription << "\n";

	scs.session = MediaSession::createNew(env, sdpDescription);
	if (scs.session == NULL) {
		std::cerr << "Failed to create a MediaSession object from the SDP description: " << env.getResultMsg() << "\n";
		return ;
	}

	rtspClient->sendSetupCommand(continueAfterSETUP);

	return;

}

LiveRTSPClient::LiveRTSPClient()
{
}

LiveRTSPClient::~LiveRTSPClient()
{
	Release();
}

void LiveRTSPClient::Restart() {

	const char* URL_temp = Get_URL();
	const char* Name_temp = Get_Name();

	eventLoopWatchVariable = 1;
	env->reclaim();
	delete scheduler; scheduler = NULL;

	bool res = Initialize(URL_temp, Name_temp);
	Run();
}

void LiveRTSPClient::Release() {

	eventLoopWatchVariable = 1;
	env->reclaim();
	delete scheduler; scheduler = NULL;
}

const char* LiveRTSPClient::Get_URL() {
	
	if (URL == m_Client->url())
		return m_Client->url();
	else
		return URL;
}

const char* LiveRTSPClient::Get_Name() {
	return m_Client->getprogName();
}

bool LiveRTSPClient::Initialize(const char* URI, const char* ProgName) {
	scheduler = BasicTaskScheduler::createNew();
	env = BasicUsageEnvironment::createNew(*scheduler);
	URL = const_cast<char*>(URI);
	m_ProgName = const_cast<char*>(ProgName);
	m_Client = RTSPClient::createNew(*env, URI, RTSP_CLIENT_VERBOSITY_LEVEL, ProgName);
	if (m_Client == NULL) {
		std::cerr << "\nRTSP_URL Error" << std::endl;
		return false;
	}
	m_Live = this;
	eventLoopWatchVariable = 0;

	return true;
}

void LiveRTSPClient::Run() {

	m_Client->sendOptionsCommand();
	m_Client->sendDescribeCommand(continueAfterDESCRIBE);
	
	eventLoopWatchVariable = 0;
	
	env->taskScheduler().doEventLoop(&eventLoopWatchVariable);
}

bool LiveRTSPClient::SetLoopSatus(bool Status) {
	
	if (Status == true) {
		eventLoopWatchVariable = 0;
	}
	else {
		eventLoopWatchVariable = 1;
	}

	return Status;
}