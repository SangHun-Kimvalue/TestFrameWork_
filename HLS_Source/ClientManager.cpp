#include "ClientManager.h"
#include "Client_Loader.hpp"
//#include "ClientManager_API.hpp"

ClientManager::ClientManager() {

	TypeList = new std::vector<LLIST*> ();
	bool Set_List = SetList();
	
}

ClientManager::~ClientManager() {

	TypeList = new std::vector<LLIST*>();
}

void ClientManager::DeleteList(){

	if (TypeList->size() != 0) {

		for (int i = 0; i < CT_TYPE_NUMBER; i++) {		//클라이언트 타입만큼

			int temp_size = TypeList->at(i)->List->size();
			if (temp_size < 1)
				continue;
			else {
				for (int k = 0; k < temp_size; k++) {	//타입리스트 내부 실제 리스트 숫자만큼
					auto client = TypeList->at(i)->List->get(0);
					client->StopClient();				//remove 구현해야 할듯
					TypeList->at(i)->List->remove((size_t)0);
				}
				TypeList->at(i)->List->clear();
				//TypeList->at(i)->List.removeAll();
			}
		}
	}

	delete TypeList;

	return;
}

int ClientManager::ReleaseCM() {
	DeleteList();
	return 0;
}

int ClientManager::CreateClient(CLI info) {
	
	auto temp = GetClientList(info.Type);
	if (info.uuid.Data1 == 0) {
		info.uuid = Create_UUID(info.Type);
	}

	bool Check = CheckClient({ info.Type, info.uuid, info.URL });
	if (Check) {
		return 1;
	}
	else {

		IClient* Client = Client_Loader::GetInstance()->CreateClient(info);
		if (Client == nullptr) {
			return -1;
		}
		else {
			
			if (Client->InitClient(info) >= 0) {
				temp->add(Client);
				std::cout << "insert to List - Type : " << ParseCT(info.Type) << " / URL : " << info.URL << std::endl;
				Check = true;
			}
		}
		//bool check = temp->isEmpty();
	}
	return 0;
}

UUID ClientManager::Create_UUID(CT Type) {

	UUID ReturnUUID; //= new UUID();
	
	//try {
		for (int i = 0; i < 3; i++) {
			if (UuidCreate(&ReturnUUID) == RPC_S_OK) {

				auto ClientforCheck = GetClient({ Type, ReturnUUID, "" });
				if (ClientforCheck == nullptr)
					return ReturnUUID;

				unsigned char * str = nullptr;
				UuidToStringA(&ReturnUUID, &str);

				std::cout << " Alreay existed before same uuid : " << str << " - retrying ..." << std::endl;
			}
		}
		//throw;
	//}
	//catch(...) {
	//	std::cout << " Error - Failed to Create uuid 3 times." << std::endl;
	//	return { 0, };
	//}

	return { 0, };
}

//int ClientManager::UpdateClient(CLI info, GetClientValue GC, char* dataType, ...) {
//
//	IClient* des = GetClient(GC);
//	if (des != nullptr) {
//
//
//
//
//	}
//
//
//	return 0;
//}

int ClientManager::DeleteClient(GetClientValue GC) {

	auto DeleteClient = GetClient(GC);
	std::string URL = DeleteClient->GetCLI().URL;
	UUID uuid = DeleteClient->GetCLI().uuid;
	unsigned char* struuid = nullptr;


	int RefCount = DeleteClient->DecreaseRef();
	if (RefCount <= 0) {

		DeleteClient->StopClient();
		bool Check = GetClientList(DeleteClient->GetCLI().Type)->remove(DeleteClient);
		if (Check) {
			if ((int)DeleteClient != 0xcd)
				delete DeleteClient;
			std::cout << " Successed Delete Client URL : " << URL.c_str() << ",  UUID : " << UuidToStringA(&uuid, &struuid) << std::endl;
		}
		else {
			std::cout << " Failed Delete Client URL : " << URL.c_str() << ",  UUID : " << UuidToStringA(&uuid, &struuid) << std::endl;
		}
		return RefCount;
	}

	else {
		std::cout << " Decrease Ref Count Client URL : " << URL.c_str() << " Ref : " << RefCount << std::endl;
		return RefCount;
	}
}

//int ClientManager::StopClient(GetClientValue GC) {
//
//	GetClient(GC)->StopClient();
//	return 0;
//}

int ClientManager::DoWorkClient(GetClientValue GC) {
	return GetClient(GC)->Play();
}

bool ClientManager::Connect(GetClientValue GC) {

	IClient* Client = GetClient(GC);

	int Check = Client->Connect();
	if (Check == 0) {
		if (Client->GetSDP() != "") {
			//CCommonInfo::GetInstance()->WriteLog(L"INFO", L"New Client Inqueue UUID : %s", Client->GetCLI().URL);
			return true;
		}
		else {
			//CCommonInfo::GetInstance()->WriteLog(L"ERROR", L"New Client Inqueue UUID : %s %s", L"Can not connect to server", Client->GetCLI().URL);
			return false;
		}
	}
	else if (Check == 1) {

	}
	else {
		std::cout << "Error when Connecting to server URL - " << GC.URL.c_str() << std::endl;
		return false;
	}

	return false;
}

//int ClientManager::IncreaseRefClient(GetClientValue GC) {
//
//	IClient* Client = GetClient(GC);
//	int ref = Client->IncreaseRef();
//
//	if (ref > 0) {
//		std::cout << "URL - " << Client->GetCLI().URL << " Increase Ref : " << Client->GetCLI().Ref << std::endl;
//		return Client->GetCLI().Ref;
//	}
//	else {
//		std::cout << "URL - " << Client->GetCLI().URL << " Cannot increase Ref more "  << "" << std::endl;
//		return Client->GetCLI().Ref;
//	}
//
//	return -1;
//}
//
//int ClientManager::DecreaseRefClient(GetClientValue GC) {
//
//	IClient* Client = GetClient(GC);
//	int ref = Client->DecreaseRef();
//
//	if (ref >= 0) {
//		std::cout << "URL - " << Client->GetCLI().URL << "Decrease Ref : " << Client->GetCLI().Ref << std::endl;
//		return Client->GetCLI().Ref;
//	}
//	else {
//		std::cout << "URL - " << Client->GetCLI().URL << " Cannot decrease Ref anymore " << std::endl;
//		return Client->GetCLI().Ref;
//	}
//
//	return -1;
//}

bool ClientManager::CheckClient(GetClientValue GC) {

	bool Find = false;

	if (GC.Type == CT_NOT_DEFINED) {
		for (int i = 0; i < CT_TYPE_NUMBER; i++) {
			if (GC.URL == "" && GC.uuid.Data1 != 0) {
				Find = (GetClient({ (CT)i, GC.uuid, "" }) != nullptr) ? true : false;
			}
			else if (GC.URL != "" && GC.uuid.Data1 == 0) {
				Find = (GetClient({ (CT)i, {0,}, GC.URL }) != nullptr) ? true : false;
			}
			return Find ? true : false;
		}
	}
	else {
		if (GC.URL == "" && GC.uuid.Data1 != 0) {
			Find = (GetClient({ GC.Type, GC.uuid, "" }) != nullptr) ? true : false;
		}
		else if (GC.URL != "" && GC.uuid.Data1 == 0) {
			Find = (GetClient({ GC.Type, {0,}, GC.URL }) != nullptr) ? true : false;
		}
		else if (GC.URL != "" && GC.uuid.Data1 != 0) {
			Find = (GetClient({ GC.Type, GC.uuid, GC.URL }) != nullptr) ? true : false;
		}
		if (Find)
		{
			return Find;
		}
	}

	return false;
}

bool ClientManager::CheckClient(GetClientValue GC, CLI &info) {

	bool Find = false;

	if (GC.Type == CT_NOT_DEFINED) {
		for (int i = 0; i < CT_TYPE_NUMBER; i++) {
			if (GC.URL == "" && GC.uuid.Data1 != 0) {
				Find = (GetClient({ (CT)i, GC.uuid, "" }) != nullptr) ? true : false;
				info = GetClient({ (CT)i, GC.uuid, "" })->GetCLI();
			}
			else if (GC.URL != "" && GC.uuid.Data1 == 0) {
				Find = (GetClient({ (CT)i, {0,}, GC.URL }) != nullptr) ? true : false;
				info = GetClient({ (CT)i, {0,}, GC.URL })->GetCLI();
			}
			return Find ? true : false;
		}
	}
	else {
		if (GC.URL == "" && GC.uuid.Data1 != 0) {
			Find = (GetClient({ GC.Type, GC.uuid, "" }) != nullptr) ? true : false;
			info = GetClient({ GC.Type, GC.uuid, "" })->GetCLI();
		}
		else if (GC.URL != "" && GC.uuid.Data1 == 0) {
			Find = (GetClient({ GC.Type, {0,}, GC.URL }) != nullptr) ? true : false;
			info = GetClient({ GC.Type, {0,}, GC.URL })->GetCLI();
		}
		else if (GC.URL != "" && GC.uuid.Data1 != 0) {
			Find = (GetClient({ GC.Type, GC.uuid, GC.URL }) != nullptr) ? true : false;
			info = GetClient({ GC.Type, GC.uuid, GC.URL })->GetCLI();
		}
		if (Find)
		{
			return Find;
		}
	}

	return false;
}


bool ClientManager::GetClient_Status(GetClientValue GC) {
	if (GetClient(GC)->GetCLI().Connected) {
		return true;
	}
	else
		return false;
}

int ClientManager::GetClient_RefCount(GetClientValue GC) {
	return GetClient(GC)->GetCLI().Ref;
}

void ClientManager::PrintInfo(GetClientValue GC) {
	GetClient(GC)->PrintClientInfo();
	return;
}

const CLI ClientManager::GetClient_Info(GetClientValue GC)
{
	if (GC.Type == CT_NOT_DEFINED) {
		CLI info = {};
		if (CheckClient(GC, info)) {
			GC.Type = info.Type;
		}
		else
			return info;
	}

	return GetClient(GC)->GetCLI();
}

std::shared_ptr<QQ> ClientManager::GetFrameQ(GetClientValue GC) {

	//IClient* temp = GetClientList((CT)Type)->get(0);
	//return /*const_cast<IClient*>*/(GetClient({ GC.Type, GC.uuid, GC.URL }))->GetQptr();
	
	try {
		auto Client = GetClient(GC);
		if (Client == nullptr)
			throw;
	}
	catch (...) {
		return nullptr;
	}

	return GetClient(GC)->GetQptr();
}

IClient* ClientManager::GetClient(GetClientValue GC) {

	bool TypeNot = false;

	if (GC.URL == "" && GC.uuid.Data1 == 0) {
		return nullptr;
	}

	if (GC.Type == CT_NOT_DEFINED) {

		for (int i = 1; i <= CT_TYPE_NUMBER; i++) {

			LinkedList<IClient*>* tempsearchlist = GetClientList((CT)i);

			bool Check = tempsearchlist->isEmpty();
			if (Check) {
				continue;
			}
			else {
				IClient* Client = nullptr;
				for (int i = 0; i < tempsearchlist->size(); i++) {
					CLI compare = tempsearchlist->get(i)->GetCLI();
					if (GC.URL == "" && GC.uuid.Data1 != 0)
						return Client = (compare.uuid == GC.uuid) ? tempsearchlist->get(i) : nullptr;
					else if (GC.URL != "" && GC.uuid.Data1 == 0)
						return Client = (compare.URL == GC.URL) ? tempsearchlist->get(i) : nullptr;
					else if ((GC.URL != "" && GC.uuid.Data1 != 0)) {
						return Client = (compare.URL == GC.URL) ? tempsearchlist->get(i) : nullptr;
					}
				}
			}
		}
	}
	else {
		LinkedList<IClient*>* templist = GetClientList(GC.Type);

		if (templist->isEmpty()) {
			std::cout << "List is Empty / type : " << ParseCT(GC.Type) << std::endl;
			return nullptr;
		}
		else {
			IClient* Client = nullptr;
			for (int i = 0; i < templist->size(); i++) {
				CLI compare = templist->get(i)->GetCLI();
				if (GC.URL == "" && GC.uuid.Data1 != 0)
					return Client = (compare.uuid == GC.uuid) ? templist->get(i) : nullptr;
				else if (GC.URL != "" && GC.uuid.Data1 == 0)
					return Client = (compare.URL == GC.URL) ? templist->get(i) : nullptr;
				else if ((GC.URL != "" && GC.uuid.Data1 != 0)) {
					return Client = (compare.URL == GC.URL) ? templist->get(i) : nullptr;
				}
			}
		}
	}

	unsigned char* str = nullptr;
	std::cout << "Could not find Client in list - " << ParseCT(GC.Type)
		<< " / URL - " << GC.URL << " / uuid = " << UuidToStringA(&GC.uuid, &str) << std::endl;
	return nullptr;
}

LinkedList<IClient*>* ClientManager::GetClientList(CT type) {
	if (type != CT_NOT_DEFINED)
		return GetTypeList(type)->List;
	else
		return nullptr;
}

bool Setparam() {
	return true;
}

template < typename Ty, typename ... Args >
bool SetParams(Ty ty, Args... args) {
	return true;
}

bool ClientManager::SetList() {

	int size_temp = TypeList->size();

	if (TypeList->size() == 0) {
		for (int i = 0; i < CT_TYPE_NUMBER; i++) {
			CT tmp_enum = (CT)(i + 1);
			LLIST* L_Type = new LLIST(tmp_enum, new LinkedList<IClient*>());
			//LinkedList<IClient*>* Cli;
			//LLIST* temp = new LLIST(tmp_enum, Cli);
	
			TypeList->push_back(L_Type);
		}
		return true;
	}
	else 
		return false;
}
