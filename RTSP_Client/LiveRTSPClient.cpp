#include "LiveRTSPClient.h"

void shutdownStream(RTSPClient* rtspClient, LiveRTSPClient* Client);
void continueAfterDESCRIBE(RTSPClient* rtspClient, int resultCode, char* resultString);
void continueAfterSETUP(RTSPClient* rtspClient, int resultCode, char* resultString);
void continueAfterPLAY(RTSPClient* rtspClient, int resultCode, char* resultString);

StreamClientState scs; // alias
LiveRTSPClient* m_Live;
unsigned long m_startTime ;

void continueAfterPLAY(RTSPClient* rtspClient, int resultCode, char* resultString) {
	Boolean success = False;

	std::cout <<  "Started playing session" << std::endl;
	m_startTime = timeGetTime();
	m_Live->m_SAlive = true;
	m_Live->GetParameter();

	std::cout << "\nProgram Name : " <<m_Live->Get_Name() << std::endl;
	std::cout << "\nURL : " <<m_Live->Get_URL() << std::endl;
	std::cout << "\nStatus : " <<m_Live->Get_Status() << std::endl;

	//m_Live->startAlive();
	success = True;
	//shutdownStream((RTSPClient*)rtspClient, m_Live);
	delete[] resultString;

}

void continueAfterSETUP(RTSPClient* rtspClient, int resultCode, char* resultString) {
	
	m_Live->GetParameter();
	rtspClient->sendPlayCommand(continueAfterPLAY);
	
}

void shutdownStream(RTSPClient* rtspClient, LiveRTSPClient* m_Client) {

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

LiveRTSPClient::LiveRTSPClient() :scheduler(nullptr), env(nullptr), m_Client(nullptr), timeout(0), eventLoopWatchVariable(0)
{

}

LiveRTSPClient::~LiveRTSPClient()
{
	Release();
}

void LiveRTSPClient::Restart() {

	const char* URL_temp = Get_URL();
	const char* Name_temp = Get_Name();

	Release();

	bool res = Initialize(URL_temp, Name_temp);
	if (res == true) {
		Run();
	}
	else {
		std::cerr << "\nInitialize Error" << std::endl;
	}
}

void LiveRTSPClient::Release() {

	eventLoopWatchVariable = 1;
	env->reclaim();
	delete scheduler; scheduler = NULL;
}

const char* LiveRTSPClient::Get_URL() {
	
	if (m_URL == m_Client->url())
		return m_Client->url();
	else
		return m_URL;
}

const char* LiveRTSPClient::Get_Name() {
	return m_Client->getprogName();
}

const char* LiveRTSPClient::Get_Status() {

	if (eventLoopWatchVariable == 0)
		return "Alive";
	else
		return "Pause";
}

const char* LiveRTSPClient::Get_SDP() {
	
	const char* temp_sdp = m_Client->SDP;
	if (temp_sdp == "") {
		return "Nothing";
	}

	return temp_sdp;
}

bool LiveRTSPClient::Initialize(const char* URI, const char* ProgName) {
	scheduler = BasicTaskScheduler::createNew();
	env = BasicUsageEnvironment::createNew(*scheduler);

	m_URL = const_cast<char*>(URI);
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

void LiveRTSPClient::Option() {
	m_Client->sendOptionsCommand();
}

void LiveRTSPClient::Description() {
	
	if (eventLoopWatchVariable == 0) {
		m_Client->sendDescribeCommand(continueAfterDESCRIBE);
	}
	else {
		
		std::cerr << "\nLoop Status Paused" << std::endl;
		m_Client->sendDescribeCommand();
	}

}

void LiveRTSPClient::Setup() {
	
	if (eventLoopWatchVariable == 0) {
		m_Client->sendSetupCommand(continueAfterSETUP);
	}
	else {

		std::cerr << "\nLoop Status Paused" << std::endl;
		m_Client->sendSetupCommand();
	}

}

void LiveRTSPClient::Play() {

	if (eventLoopWatchVariable == 0) {
		timeout = m_Client->sessionTimeoutParameter();
		m_Client->sendPlayCommand(continueAfterPLAY);

	}
	else {

		std::cerr << "\nLoop Status Paused" << std::endl;
		m_Client->sendPlayCommand();
	}

	m_SAlive = true;
}

void LiveRTSPClient::TearDown() {

	m_Client->sendTeardownCommand();
	
	m_SAlive = false;
}

void LiveRTSPClient::GetParameter() {
	m_Client->sendGetParameterCommand();
}

//스레드 실행
bool LiveRTSPClient::KeepAlive() {
	
	static int m_count;
	timeout;


	while (eventLoopWatchVariable == 0) {
		if (m_SAlive == true) {

			if (timeGetTime() >= (m_startTime + 5000)) {
				m_startTime = timeGetTime();
				//m_Client->sendGetParameterCommand();
				Option();
			}

		}

		Sleep(10);
	}
	
	if (m_SAlive == true)
		return true;
	else
		return false;
}

void LiveRTSPClient::startAlive() {

	AliveThread = std::thread(&LiveRTSPClient::KeepAlive, this);


}

void LiveRTSPClient::Run() {

	m_Client->sendOptionsCommand();

	m_Client->sendDescribeCommand(continueAfterDESCRIBE);
	
	eventLoopWatchVariable = 0;
	
	env->taskScheduler().doEventLoop(&eventLoopWatchVariable);
}

const unsigned LiveRTSPClient::Get_Timeout() {

	return timeout;
}

bool LiveRTSPClient::SetLoopSatus(bool Status) {
	
	if (Status == true) {
		eventLoopWatchVariable = 0;
	}
	else {
		eventLoopWatchVariable = 1;
		m_SAlive = false;
	}

	return Status;
}