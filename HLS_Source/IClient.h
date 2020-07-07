#pragma once
#include <iostream>
#include "ClientFormat.h"

class IClient {

public:

	std::shared_ptr<QQ> m_pRecFrameQ = nullptr;
	//std::shared_ptr<QQ> m_pRecAFrameQ;

	virtual ~IClient() { m_pRecFrameQ->clear();/* m_pRecAFrameQ->clear(); */ };

	virtual int InitClient(CLI info) = 0;
	virtual int Connect() = 0;
	virtual int Play() = 0;
	virtual void PrintClientInfo() = 0;
	virtual void StopClient() = 0;
	virtual const CLI GetCLI()const = 0;
	virtual const std::string GetSDP()const = 0;
	virtual int IncreaseRef() = 0;
	virtual int DecreaseRef() = 0;


	virtual std::shared_ptr<QQ> CreateRecievedFrameQueue(int max_size) { return std::make_shared<QQ>(max_size); }
	virtual std::shared_ptr<QQ> GetQptr() { return m_pRecFrameQ; }
	//virtual std::shared_ptr<QQ> GetAQptr() { return m_pRecAFrameQ; }

};