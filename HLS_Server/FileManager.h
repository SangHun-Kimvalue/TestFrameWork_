#pragma once
#include <Windows.h>
#include <iostream>
#include <string>
#include <direct.h>
#include <rpc.h>
#include <io.h>
#include <list>
#include <conio.h>
#include "CommonInfo.h"

using namespace std;

#define DEFAULTSEGCOUNT 2

typedef enum MediaFileType {
	MFT_NOT_DEFINED, MFT_M3U8, MFT_TS, MFT_MP4, MFT_AVI, MFT_MKV
}MFT;

//Client IP 가져와야함
class FileManager
{
public:
	
	//FileManager();
	FileManager(int SegCount, UUID uuid);
	~FileManager();

	char** GetDirPath();

private:

	bool InitBitrate(int SegCount);

	int MakeFolder();
	bool DeleteFolder();

private:

	const UUID m_UUID;
	const int SegCount;
	int* arr;

	string m_UUIDDir = "";

	const char* BitratePreset[4] = {"1080_", "720_", "480_", "360_"};
	char* CreatedDir[4] = {};

};

