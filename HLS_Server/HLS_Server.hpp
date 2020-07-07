#pragma once
#include "HLS_MediaServer.h"

class HLS_Server {

public:


	HLS_Server() {
		m_MediaServer = new HLS_MediaServer();

	}
	~HLS_Server() {

	}



private:

	HLS_MediaServer* m_MediaServer;

};