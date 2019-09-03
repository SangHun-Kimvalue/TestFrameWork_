//
//  main.cpp
//  FFmpegRTSPServer
//
//  Created by Mina Saad on 9/22/15.
//  Copyright (c) 2015 Mina Saad. All rights reserved.
//

//#define _ITERATOR_DEBUG_LEVEL 2
//#define MT_StaticRelease

#include <iostream>
#include "m_LiveRTSPServer.h"


int main(int argc, const char * argv[])
{
	int Port = 554;
	TaskScheduler    *scheduler;
	UsageEnvironment *env;

	scheduler = BasicTaskScheduler::createNew();
	if (scheduler == nullptr) {
		return false;
	}
	env = BasicUsageEnvironment::createNew(*scheduler);
	if (env == nullptr) {
		return false;
	}

	m_RTSPServer *server = LiveRTSPServer::createNew(*env, Port);
	
	server->Initialize(Port);

	server->Run();

	delete server;
}
