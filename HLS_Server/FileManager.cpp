#include "FileManager.h"

FileManager::FileManager() : SegCount(DEFAULTSEGCOUNT), arr(nullptr), m_UUID({}) {
	InitBitrate(SegCount);
}

FileManager::FileManager(int segCount, UUID uuid) : SegCount(segCount), arr(nullptr), m_UUID(uuid) {
	InitBitrate(SegCount);
}

FileManager::~FileManager() {
	DeleteFolder();
	
	free(arr);
}

bool FileManager::InitBitrate(int SegCount) {

	int preset[5] = { 1080, 720, 480, 360 };
	arr = (int *)malloc(sizeof(int) * SegCount);


	for (int i = 0; i < SegCount; i++) {
		*(arr + i) = preset[i];
	}
	
	return true;
}

bool FileManager::MakeFolder() {

	unsigned short* str = nullptr;

	UuidToStringW(&m_UUID, &str);

	std::wstring Path;

	Path = (wchar_t*)str;
	
	CreateDirectory((L".\\LOG"), NULL);
	
	return true;
}

bool FileManager::DeleteFolder() {
	return true;
}