//
//  main.cpp
//  FFmpegRTSPServer
//
//  Created by Mina Saad on 9/22/15.
//  Copyright (c) 2015 Mina Saad. All rights reserved.
//

//#define _ITERATOR_DEBUG_LEVEL 2
//#define MT_StaticRelease

#include "pch.h"
#include <iostream>
#include "LiveRTSPServer.h"


int main(int argc, const char * argv[])
{
	int Port = 8554;

	Server * server = new LiveRTSPServer();
	server->Initialize(Port);
	
	const char* URL = server->GetURL();
	//printf("\n%s\n", server->GetURL());

	//std::string stdhello = "HELLO";
	//std::cout << stdhello.c_str() << std::endl;

	server->Run();

	
	//server->Release();
	delete server;
}
