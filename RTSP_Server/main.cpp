//
//  main.cpp
//  FFmpegRTSPServer
//
//  Created by Mina Saad on 9/22/15.
//  Copyright (c) 2015 Mina Saad. All rights reserved.
//

#include "LiveRTSPServer.h"
MESAI::LiveRTSPServer * server;

int UDPPort;
int HTTPTunnelPort;

int main(int argc, const char * argv[])
{
	UDPPort = 5486;

	server = new MESAI::LiveRTSPServer(/*encoder,*/ UDPPort, HTTPTunnelPort);
	server->run();

}
