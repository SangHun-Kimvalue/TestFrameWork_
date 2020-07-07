#pragma once
#include <iostream>


class AgentFlow {
public:
	AgentFlow() {};
	virtual ~AgentFlow() {};

	virtual void Start() = 0;

};