#include "SourceManager.h"

SourceManager::SourceManager() {

	if(ClientM == nullptr)
		ClientM = CM_CreateCM();

}

SourceManager::~SourceManager() {

	int check = CM_ReleaseCM(ClientM);

}

//Value로 생성
int SourceManager::Create_Client(GetClientValue GC) {
	return  Create_Client(TT_TCP, GC);
}

//구조체로 생성
int SourceManager::Create_Client(CLI CLI_Input) {
	return  Create_Client(CLI_Input.TransportType, { CLI_Input.Type, CLI_Input.uuid, CLI_Input.URL});
}

//디폴트로 생성
int SourceManager::Create_Client(TT TransportType, GetClientValue GC) {

	auto CC = Check_Client(GC);
	if (!CC) {
		CLI CLI_temp = { GC.Type , TransportType, GC.uuid, GC.URL };
		//CT ClientType, TT TransportType, VCodec,   ACodec,   URL,  Reference, interval, uuid, int index, Connect, UseAudio, Transcoding
		
		int res = CM_CreateClient(ClientM, CLI_temp);
		if (res > -1) {
			Connect(GC);
		}

		return 0;
	}
	else if (CC) {
		Connect(GC);
		return 1;
	}

	return 0;
}

bool SourceManager::Check_Client(GetClientValue GC){
	
	return CM_CheckClient(ClientM, GC);
}

int SourceManager::DeleteClient(GetClientValue GC) {

	return CM_DeleteClient(ClientM, GC);
}

int SourceManager::StopClient(GetClientValue GC) {

	return CM_DeleteClient(ClientM, GC);
}

int SourceManager::DoWorkClient(GetClientValue GC) {

	int Check = 0;
	bool Connected = GetClient_Status(GC);
	if (!Connected) {
		Check = CM_Connect(ClientM,GC);
	}

	Check = CM_DoWork(ClientM, GC);

	if(!Check) {
		return -1;
	}

	return Check;
}

bool SourceManager::Connect(GetClientValue GC) {
	
	bool Check = CM_GetClient_Status(ClientM, GC);
	if (Check) {
		return Check;
	}
	else 
		return CM_Connect(ClientM, GC);
}

//int SourceManager::IncreaseRefClient(GetClientValue GC) {
//	return CM_IncreaseRef(ClientM, GC);
//}
//
//int SourceManager::DecreaseRefClient(GetClientValue GC) {
//	return CM_DecreaseRef(ClientM, GC);
//}

//std::shared_ptr<QQ> SourceManager::GetFrameQ(CT ClientType, UUID uuid) {
//	GetFrameQ(ClientM, )
//}
//

bool SourceManager::GetClient_Status(GetClientValue GC) {
	return CM_GetClient_Status(ClientM, GC);
}

int SourceManager::GetClient_RefCount(GetClientValue GC) {
	return CM_GetClient_RefCount(ClientM, GC);
}

void SourceManager::PrintInfo(GetClientValue GC) {
	return CM_Print_Info(ClientM, GC);
}

const CLI SourceManager::GetClientInfo(GetClientValue GC) const {
	return CM_GetClient_Info(ClientM, GC);
}

std::shared_ptr<QQ> SourceManager::GetFrameQ(GetClientValue GC) {
	return CM_GetFrameQ(ClientM, GC);
}

UUID SourceManager::GetUUID(std::string URL) {

	auto info = CM_GetClient_Info(ClientM, { (CT)0, {0,}, URL });
	const char* m_url = info.URL.c_str();
	if (strcmp(m_url, URL.c_str()) == 0) {
		return info.uuid;
	}
	else {
		return {};
	}
		
	return {};
}



