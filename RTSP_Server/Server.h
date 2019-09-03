#pragma once

#include <string>

class Server
{
public:
	Server() {};
	virtual ~Server() {};
	
	virtual void Release() = 0;
	virtual bool Initialize(int port) = 0;
	virtual void Run() = 0;
	virtual const char* GetURL() = 0;
	virtual void Restart() = 0;





private:

	//virtual bool Init_Net() = 0;
	//virtual bool Init_Stream() = 0;
	//virtual void Set_Format() = 0;

};

