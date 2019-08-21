//
//  LiveRTSPServer.h
//  FFmpegRTSPServer
//
//  Created by Mina Saad on 9/22/15.
//  Copyright (c) 2015 Mina Saad. All rights reserved.
//

#include <UsageEnvironment.hh>
#include <BasicUsageEnvironment.hh>
#include <GroupsockHelper.hh>
#include <liveMedia.hh>
#include "LiveServerMediaSubsession.h"
#include "Server.h"
#include "Connect_Handler.h"

//#include "FFmpegH264Source.h"
//#include "FFmpegH264Encoder.h"


	class LiveRTSPServer : public Server
	{
	public:

		LiveRTSPServer(/*FFmpegH264Encoder  * a_Encoder, int port,*/ int httpPort = 0);
		//LiveRTSPServer();
		~LiveRTSPServer();
		
		virtual void Release();
		virtual void Run();
		const char* GetURL();
		virtual bool Initialize(int port);
		virtual void Restart();

		char* URL;

	private:
		int portNumber;
		int httpTunnelingPort;
		//FFmpegH264Encoder * m_Encoder;
		char quit;

		TaskScheduler    *scheduler;
		UsageEnvironment *env;
		UserAuthenticationDatabase* authDB;
		RTSPServer* rtspServer;
		Connect_Handler* Commander;

	};
