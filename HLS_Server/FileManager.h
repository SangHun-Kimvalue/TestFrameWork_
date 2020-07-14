#pragma once
#include <Windows.h>
#include <string>
#include <rpc.h>

#define DEFAULTSEGCOUNT 2

typedef enum MediaFileType {
	MFT_NOT_DEFINED, MFT_M3U8, MFT_TS, MFT_MP4, MFT_AVI, MFT_MKV
}MFT;

//Client IP 가져와야함
class FileManager
{
public:
	
	FileManager();
	FileManager(int SegCount, UUID uuid);
	~FileManager();

private:

	bool InitBitrate(int SegCount);


	bool MakeFolder();
	bool DeleteFolder();

private:

	const UUID m_UUID;
	const int SegCount;
	int* arr;

};

