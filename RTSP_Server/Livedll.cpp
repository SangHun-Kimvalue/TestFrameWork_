#include "stdafx.h"
#include "m_LiveRTSPServer.h"


#define EXPORT extern "C" __declspec(dllexport)

EXPORT m_RTSPServer* GetInstance()
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

	return LiveRTSPServer::createNew(*env, Port);
}
