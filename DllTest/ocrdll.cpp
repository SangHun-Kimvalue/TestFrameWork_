#pragma once

#include "stdafx.h"
#include "DllClass.h"

#define EXPORT extern "C" __declspec(dllexport)

EXPORT BaseAlertModule* GetInstance()
{
	return new DllClass();
}


