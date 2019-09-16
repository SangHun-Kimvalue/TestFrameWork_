
#pragma once
#include "stdafx.h"
#include "RTSP_Manager.h"

#define EXPORT extern "C" __declspec(dllexport)

EXPORT RTSP_Manager* GetInstance()
{
	return new RTSP_Manager();
}


