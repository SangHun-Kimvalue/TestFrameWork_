#pragma once
class Server
{
public:
	Server();
	virtual ~Server();
	
	virtual void Release() = 0;
	virtual bool Initialize();
	virtual bool Init_Net() = 0;
	virtual bool Init_Media() = 0;
	virtual void Set_Format() = 0;
	virtual void Set_Port() = 0;
	virtual void Run() = 0;

	virtual void Restart();
private:

};

