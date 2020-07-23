#pragma once

#ifdef SOURCEMANAGER_EXPORTS
#define SOURCEMANAGER_API __declspec(dllexport)
#else
#define SOURCEMANAGER_API __declspec(dllimport)
#endif

#include "ClientManager.h"

class ClientManager;

extern "C"
{
	SOURCEMANAGER_API ClientManager * __stdcall CM_CreateCM();
	SOURCEMANAGER_API int __stdcall CM_ReleaseCM(ClientManager * pManager);

	SOURCEMANAGER_API int __stdcall CM_CreateClient(ClientManager * pManager, CLI info);
	SOURCEMANAGER_API int __stdcall CM_DeleteClient(ClientManager * pManager, GetClientValue GC);
	//SOURCEMANAGER_API int __stdcall UpdateClient(ClientManager * pManager, CLI info, GetClientValue);
	//SOURCEMANAGER_API int __stdcall CM_StopClient(ClientManager * pManager, GetClientValue GC);
	SOURCEMANAGER_API int __stdcall CM_DoWork(ClientManager * pManager, GetClientValue GC);
	
	SOURCEMANAGER_API int __stdcall CM_Connect(ClientManager * pManager, GetClientValue GC);
	
	//SOURCEMANAGER_API int __stdcall CM_IncreaseRef(ClientManager * pManager, GetClientValue GC);
	//SOURCEMANAGER_API int __stdcall CM_DecreaseRef(ClientManager * pManager, GetClientValue GC);

	//SOURCEMANAGER_API int __stdcall GetClient_Info(ClientManager * pManager, GetClientValue GC);
	//SOURCEMANAGER_API const LinkedList<IClient*>* __stdcall GetClientList(ClientManager * pManager, CT type);
	
	SOURCEMANAGER_API bool __stdcall CM_CheckClient(ClientManager * pManager, GetClientValue GC);
	SOURCEMANAGER_API bool __stdcall CM_GetClient_Status(ClientManager * pManager, GetClientValue GC);
	SOURCEMANAGER_API int  __stdcall CM_GetClient_RefCount(ClientManager * pManager, GetClientValue GC);
	SOURCEMANAGER_API void __stdcall CM_Print_Info(ClientManager * pManager, GetClientValue GC);
	SOURCEMANAGER_API const CLI __stdcall CM_GetClient_Info(ClientManager * pManager, GetClientValue GC);

	//수정 필요
	//SOURCEMANAGER_API int __stdcall CM_SetParam(ClientManager * pManager, GetClientValue GC);

	SOURCEMANAGER_API QQ __stdcall CM_GetFrameQ(ClientManager * pManager, GetClientValue GC);
}
