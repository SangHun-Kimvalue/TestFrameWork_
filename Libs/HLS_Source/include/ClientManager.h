#pragma once

#include <iostream>
#include <string>
#include <vector>
#include "IClient.h"
#include "ClientFormat.h"

//Control
class ClientManager {	

public:

	ClientManager();
	~ClientManager();
	int ReleaseCM();

	int CreateClient(CLI info);
	//int UpdateClient(CLI info, GetClientValue, char* dataType, ...);
	int DeleteClient(GetClientValue GC);
	int StopClient(GetClientValue GC) { return 0; }
	int DoWorkClient(GetClientValue GC);

	bool Connect(GetClientValue GC);

	//int IncreaseRefClient(GetClientValue GC);
	//int DecreaseRefClient(GetClientValue GC);

	bool CheckClient(GetClientValue GC);
	bool CheckClient(GetClientValue GC, CLI &info);
	bool GetClient_Status(GetClientValue GC);
	int GetClient_RefCount(GetClientValue GC);
	void PrintInfo(GetClientValue GC);
	const CLI GetClient_Info(GetClientValue GC);

	QQ GetFrameQ(GetClientValue GC);
	//const IClient* GetClient(CT ClientType, UUID uuid);


private:

	//UUID Create_UUID(CT Type);

	bool SetList();
	void DeleteList();
	//std::vector<LLIST*>* GetList(CT type);

	IClient* GetClient(GetClientValue GC);
	TYPELIST* GetTypeList(CT type) { return TypeList->at((int)type);	 }
	LinkedList<IClient*>* GetClientList(CT type);

	std::vector<TYPELIST*>* TypeList;
};
