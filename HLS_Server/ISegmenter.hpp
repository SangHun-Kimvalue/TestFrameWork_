#pragma once
#include <vector>
#include <shared_mutex>

#define DEFAULTFRAMERATE 30

typedef enum SegmenterType {
	ST_NOT_DEFINE, ST_FFSW, ST_FFHW, ST_NV, ST_CV
}ST;

class ISegmenter
{
public:

	ISegmenter(int index) : Index(index){}

	virtual ~ISegmenter() {}

	virtual int Init() = 0;
	virtual int Run(std::shared_ptr<MediaFrame>) = 0;
	virtual int Close() = 0;
	virtual int Stop() = 0;
	
public:

	int Ref;
	bool Running;
	const int Index;
	std::string Filename;
};

