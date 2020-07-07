#pragma once
#include <iostream>
#include <string>
#include <Windows.h>
//#include "../HLS_Source/ClientManager.h"
#include "../Libs/HLS_Source/include/ClientManager_API.h"

class SourceManager
{

public:
	SourceManager();
	~SourceManager();

	//0이면 생성 1이면 이미 존재 -1 에러
	int Create_Client(TT TransportType, GetClientValue GC);
	int Create_Client(CLI);
	int Create_Client(GetClientValue);
	bool Check_Client(GetClientValue GC);
	int DeleteClient(GetClientValue GC);
	int DoWorkClient(GetClientValue GC);

	const CLI GetClientInfo(GetClientValue GC) const;
	
	void PrintInfo(GetClientValue GC);

	UUID GetUUID(std::string URL);

	std::shared_ptr<QQ> GetFrameQ(GetClientValue GC);

private:
	bool Connect(GetClientValue GC);
	bool GetClient_Status(GetClientValue GC);	//Return Connected

	int GetClient_RefCount(GetClientValue GC);
	int IncreaseRefClient(GetClientValue GC);
	int DecreaseRefClient(GetClientValue GC);
	int StopClient(GetClientValue GC);

	//UUID Create_UUID();
	ClientManager* ClientM;

};

