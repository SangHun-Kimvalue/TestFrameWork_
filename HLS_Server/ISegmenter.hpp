#pragma once
#include <vector>

typedef enum SegmenterType {
	ST_NOT_DEFINE, ST_FFSW, ST_FFHW, ST_NV, ST_CV
}ST;

class ISegmenter
{
public:

	ISegmenter(int index, int Bitrate) : Index(index), Bitrate(Bitrate){}

	virtual ~ISegmenter() {}

	virtual int Init() = 0;
	virtual int Run() = 0;
	virtual int Close() = 0;
	virtual int Stop() = 0;
	//using thread
	virtual bool TimeCheck(ISegmenter* mx) = 0;
	
public:

	int Ref;
	bool Running;
	const int Index;
	const int Bitrate;
	std::string Filename;
};

