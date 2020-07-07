#pragma once
#include <vector>

class ISegmenter
{
public:

	ISegmenter(int index) : Index(index){}

	virtual ~ISegmenter() {}

	virtual int Init() = 0;
	virtual int Run() = 0;
	virtual int Close() = 0;
	virtual int Standby() = 0;
	virtual int Stop() = 0;

public:

	const int Index;
};

