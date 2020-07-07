#pragma once
#include "AgentFlow.hpp"

class VideoFlow : public AgentFlow {
	
public:

	VideoFlow() {};
	VideoFlow(int a) {};
	virtual ~VideoFlow() {};

	virtual void Start() {

		std::cout << "Call Video" << std::endl;

		return;
	}


};

class Flow : public VideoFlow {

	Flow() {};
	~Flow() {};

	void Start() {

		std::cout << "Call Flow" << std::endl;

		return;
	}

};

