#pragma once

#include <string>
#include <map>
//#include "httplib.h"

class HttpServer;

class Config
{
public:
	Config();
	~Config();

	bool LoadConfig(const std::wstring& cfgPath);
private:
	std::wstring _ip;
	unsigned short _port;
	std::map<std::string, std::string> streamMap;

	friend HttpServer;
};

