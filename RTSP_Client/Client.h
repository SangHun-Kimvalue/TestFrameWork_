#pragma once
#include <iostream>

class Client
{
public:
	Client(): m_URL(0),m_ProgName(0){};
	virtual ~Client() {};

	virtual bool Initialize(const char* URI, const char* ProgName) = 0;
	virtual void Release() = 0;
	virtual void Run() = 0;
	virtual const char* Get_URL() = 0;
	virtual const char* Get_Name() = 0;
	virtual bool Get_Status() = 0;


	char* m_URL;
	char* m_ProgName;

};

