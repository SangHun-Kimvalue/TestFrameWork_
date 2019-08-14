#include "Server.h"



Server::Server()
{
}


Server::~Server()
{
}

void Server::Restart() {

	Release();
	Initialize();
	Run();
}

bool Server::Initialize() {

	bool res = false;

	Set_Port();
	
	res = Init_Net();
	if (res) {};

	Set_Format();
	res = Init_Media();
	if (res) {};

	return res;
}