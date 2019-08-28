//
//  LiveRTSPServer.cpp
//  FFmpegRTSPServer
//
//  Created by Mina Saad on 9/22/15.
//  Copyright (c) 2015 Mina Saad. All rights reserved.
//

#include "LiveRTSPServer.h"

//LiveRTSPServer::LiveRTSPServer() :portNumber(0), httpTunnelingPort(0){
//}

LiveRTSPServer::LiveRTSPServer(/*int port,*/ int httpPort)
	:portNumber(0), httpTunnelingPort(httpPort) {
}

bool LiveRTSPServer::Initialize(int port) {

	portNumber = port;
	quit = 0;
	scheduler = BasicTaskScheduler::createNew();
	if (scheduler == nullptr) {
		return false;
	}
	env = BasicUsageEnvironment::createNew(*scheduler);
	if (env == nullptr) {
		return false;
	}

	authDB = NULL;


	char const* SessionName = "Streaming Session";
	char RTSP_Address[1024];	RTSP_Address[0] = 0x00;

	//rtspServer = RTSPServer::createNew(*env, portNumber, authDB);
	//if (rtspServer == NULL)
	//{
	//	*env << "LIVE555: Failed to create RTSP server: %s\n", env->getResultMsg();
	//	return false;
	//}
	//ServerMediaSession* sms = ServerMediaSession::createNew(*env, RTSP_Address, RTSP_Address, SessionName);
	//// sms->addSubsession(MESAI::LiveServerMediaSubsession::createNew(*env, inputDevice));
	//sms->addSubsession(LiveServerMediaSubsession::createNew(*env));
	//rtspServer->addServerMediaSession(sms);
	//URL = rtspServer->rtspURL();

	Commander = Connect_Handler::createNew(*env, portNumber, authDB);
	if (Commander == NULL)
	{
		*env << "LIVE555: Cmmander to create Failed: %s\n", env->getResultMsg();
		return false;
	}
	ServerMediaSession* sms = ServerMediaSession::createNew(*env, RTSP_Address, RTSP_Address, SessionName);
	sms->addSubsession(LiveServerMediaSubsession::createNew(*env));
	Commander->addServerMediaSession(sms);
	URL = Commander->rtspURL();

	*env << "Play this stream using the URL \"" << URL << "\"\n";

	//delete[] url;

	return true;
}

void LiveRTSPServer::Restart() {

	Release();
	Initialize(m_port);
	Run();

	return;
}

const char* LiveRTSPServer::GetURL() {

	if (URL[0] == ' ') {
		return "\nNot valid URL\n";
	}

	const char* temp = URL;

	return temp;
}

LiveRTSPServer::~LiveRTSPServer()
{
	Release();
}

void LiveRTSPServer::Run()
{

	//char RTSP_Address[1024];
	//RTSP_Address[0]=0x00;
	//scheduler = BasicTaskScheduler::createNew();
	//env = BasicUsageEnvironment::createNew(*scheduler);
	//UserAuthenticationDatabase* authDB = NULL;
	// if (m_Enable_Pass){
	// 	authDB = new UserAuthenticationDatabase;
	// 	authDB->addUserRecord(UserN, PassW);
	// }
	//OutPacketBuffer::maxSize = 2000000;
	//RTSPServer* rtspServer = RTSPServer::createNew(*env, portNumber, authDB);

	//else {
	//if(httpTunnelingPort)
	//{
	//    rtspServer->setUpTunnelingOverHTTP(httpTunnelingPort);
	//}
	//char const* descriptionString = "MESAI Streaming Session";     
	//FFmpegH264Source * source = FFmpegH264Source::createNew(*env,m_Encoder);
	//StreamReplicator * inputDevice = StreamReplicator::createNew(*env, source, false);
	//ServerMediaSession* sms = ServerMediaSession::createNew(*env, RTSP_Address, RTSP_Address, descriptionString);
	////sms->addSubsession(MESAI::LiveServerMediaSubsession::createNew(*env, inputDevice));
	//sms->addSubsession(MESAI::LiveServerMediaSubsession::createNew(*env));
	//rtspServer->addServerMediaSession(sms);
	//signal(SIGNIT,sighandler);

	env->taskScheduler().doEventLoop(&quit); // does not return

	//Medium::close(inputDevice);
	//}


}

void LiveRTSPServer::Release() {

	bool res = false;

	Medium::close(Commander);
	env->reclaim();

	delete scheduler;

	return;
}