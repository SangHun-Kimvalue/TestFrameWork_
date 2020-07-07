#pragma once
#include <windows.h>
#include <list>
#include <string>
#include <mutex>

#include <iostream>
#include <vector>
#include <sstream>

struct ScopeCS
{
	CRITICAL_SECTION *cs;
	explicit ScopeCS(CRITICAL_SECTION &cs) :cs(&cs) { EnterCriticalSection(&cs); }
	~ScopeCS() { LeaveCriticalSection(cs); }
};

class CCommonInfo
{
public:
	~CCommonInfo();
	static CCommonInfo* GetInstance();

	std::mutex m_mutexGlobal;

private:
	CCommonInfo();
	static CCommonInfo* volatile m_inst;

	wchar_t m_pIniPath[2048];
	wchar_t m_pFullPath[2048];
	wchar_t m_pLogPath[2048];
	wchar_t m_pConverter[2048];
	wchar_t m_pConverterData[2048];

	std::mutex m_write_lock;
	std::string m_serverip;

public:
	std::wstring ReadIniFile(const wchar_t* lpszAppName, const wchar_t* lpszKeyName, const wchar_t* lpszDefaultValue);
	int ReadIniFile(const wchar_t* lpszAppName, const wchar_t* lpszKeyName, int iDefaultValue);
	void WriteIniFile(const wchar_t* lpszAppName, const wchar_t* lpszKeyName, const wchar_t* lpszValue);
	void WriteIniFile(const wchar_t* lpszAppName, const wchar_t* lpszKeyName, int iValue);
	void DeleteIniFile(const wchar_t* lpszAppName);

	void KSNCOutputDebugString(LPCTSTR pszStr, ...);
	void WriteLog(const wchar_t* title, const wchar_t* format, ...);
	void WriteLog(const char* title, const char* format, ...);
	void WriteLog(const char* title, const char* format, const char* str);
	void WriteLog(const char* title, const char* format, const char* str, int integer);
	bool GetProtocol(const char* url);
	std::string* StringSplit(std::string strTarget, std::string strTok);
	std::vector<std::string> split(const std::string &s, char delim);

	void commonLock();// { m_mutexGlobal.lock(); }
	void commonunLock();// { m_mutexGlobal.unlock(); }

	void GetIniSectionList(std::vector<std::wstring>* m_pConverterip);

	std::wstring string2wstring(const std::string& str);

	void SetServerIP(std::string ip) { m_serverip = ip; }
	std::string GetServerIP() { return m_serverip; }
	std::wstring GetLogDirectory();

public:

private:
	unsigned char FirstHexToBytes(const std::string& hex);
	std::vector<unsigned char> HexToBytes(const std::string& hex);

};