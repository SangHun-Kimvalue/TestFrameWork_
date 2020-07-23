#include "ClientManager_API.h"

extern "C"
{
	SOURCEMANAGER_API ClientManager * __stdcall CM_CreateCM()
	{
		return new ClientManager();
	}
	SOURCEMANAGER_API int __stdcall CM_ReleaseCM(ClientManager * pManager) {
		delete pManager;
		return 0;
	}
	SOURCEMANAGER_API int __stdcall CM_CreateClient(ClientManager * pManager, CLI info) {
		return pManager->CreateClient(info);
	}
	SOURCEMANAGER_API int __stdcall CM_DeleteClient(ClientManager * pManager, GetClientValue GC)
	{
		return pManager->DeleteClient(GC);
	}
	//SOURCEMANAGER_API int __stdcall UpdateClient(ClientManager * pManager, CLI info, GetClientValue GC) {
	//	return pManager->UpdateClient(info ,GC);
	//}
	SOURCEMANAGER_API int __stdcall CM_StopClient(ClientManager * pManager, GetClientValue GC) {
		return pManager->StopClient(GC);
	}
	SOURCEMANAGER_API int __stdcall CM_DoWork(ClientManager * pManager, GetClientValue GC) {
		return pManager->DoWorkClient(GC);
	}

	//SOURCEMANAGER_API int __stdcall CM_IncreaseRef(ClientManager * pManager, GetClientValue GC) {
	//	return pManager->IncreaseRefClient(GC);
	//}
	//SOURCEMANAGER_API int __stdcall CM_DecreaseRef(ClientManager * pManager, GetClientValue GC) {
	//	return pManager->DecreaseRefClient(GC);
	//}

	SOURCEMANAGER_API int __stdcall CM_Connect(ClientManager * pManager, GetClientValue GC) {
		return pManager->Connect(GC);
	}
	SOURCEMANAGER_API bool __stdcall CM_CheckClient(ClientManager * pManager, GetClientValue GC) {
		return pManager->CheckClient(GC);
	}
	SOURCEMANAGER_API bool __stdcall CM_GetClient_Status(ClientManager * pManager, GetClientValue GC) {
		return pManager->GetClient_Status(GC);
	}
	SOURCEMANAGER_API int __stdcall CM_GetClient_RefCount(ClientManager * pManager, GetClientValue GC) {
		return pManager->GetClient_RefCount(GC);
	}

	//SOURCEMANAGER_API int __stdcall GetClient_Info(ClientManager * pManager, GetClientValue GC) {
//	//return pManager->GetClient_Info(GC, info);
//	return 0;
//}
	//SOURCEMANAGER_API const LinkedList<IClient*>* __stdcall GetClientList(ClientManager * pManager, CT type) {
	//	return pManager->GetClientList(type);
	//}

	SOURCEMANAGER_API void __stdcall CM_Print_Info(ClientManager * pManager, GetClientValue GC) {
		pManager->PrintInfo(GC);
		return;
	}

	SOURCEMANAGER_API const CLI __stdcall CM_GetClient_Info(ClientManager * pManager, GetClientValue GC) {
		return pManager->GetClient_Info(GC);
	}

	//SOURCEMANAGER_API int __stdcall CM_SetParam(ClientManager * pManager, GetClientValue GC) {
	//	return false;
	//}

	SOURCEMANAGER_API QQ __stdcall CM_GetFrameQ(ClientManager * pManager, GetClientValue GC)
	{
		return pManager->GetFrameQ(GC);
	}

}