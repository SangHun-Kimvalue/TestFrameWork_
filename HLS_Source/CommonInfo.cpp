#include <windows.h>
#include <stdio.h>
#include <shellapi.h>
#include <atlstr.h>
#include <process.h>
#include "CommonInfo.h"
#include <atltime.h> 

extern "C"
{
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/opt.h>
}

//#include "../../HLS_Source/Utility.hpp"

CCommonInfo::CCommonInfo()
{
	::GetModuleFileNameW(::GetModuleHandle(nullptr), m_pFullPath, _countof(m_pFullPath));
	wchar_t* pnt = wcsrchr(m_pFullPath, L'\\');

	if (pnt)
	{
		*pnt = L'\0';
		swprintf_s(m_pIniPath, L"%ls\\config.ini", m_pFullPath);
		swprintf_s(m_pLogPath, L"%ls\\logs", m_pFullPath);
		swprintf_s(m_pConverter, L"%ls\\Converter", m_pLogPath);

		_wmkdir(m_pLogPath);
		_wmkdir(m_pConverter);

		pnt = wcsrchr(m_pFullPath, L'\\');
		*pnt = L'\0';
		swprintf_s(m_pConverterData, L"%ls\\m_pConverterData", m_pFullPath);
		//swprintf_s(m_pAlertModuleScreenSavePath, L"%ls\\AlertModuleScreenData", m_pFullPath);

		_wmkdir(m_pConverterData);
		//_wmkdir(m_pAlertModuleScreenSavePath);
	}
}

CCommonInfo * volatile CCommonInfo::m_inst;

static struct CommonInfoInitializer
{
	CRITICAL_SECTION cs;
	CommonInfoInitializer() { InitializeCriticalSection(&cs); }
	~CommonInfoInitializer() { DeleteCriticalSection(&cs); }
} _CommonInfoInitializer;

CCommonInfo* CCommonInfo::GetInstance()
{
	if (m_inst == nullptr)
	{
		ScopeCS cs(_CommonInfoInitializer.cs);
		if (m_inst == nullptr)
		{
			static CCommonInfo info;
			m_inst = &info;
		}
	}

	return m_inst;
}

CCommonInfo::~CCommonInfo()
{
}

void CCommonInfo::commonLock()
{
	m_mutexGlobal.lock();
}
void CCommonInfo::commonunLock()
{
	m_mutexGlobal.unlock();
}

std::wstring CCommonInfo::ReadIniFile(const wchar_t* lpszAppName, const wchar_t* lpszKeyName, const wchar_t* lpszDefaultValue)
{
	wchar_t temp[4096];
	if (GetPrivateProfileStringW(lpszAppName, lpszKeyName, nullptr, temp, 4096, m_pIniPath) == 0)
	{
		WritePrivateProfileStringW(lpszAppName, lpszKeyName, lpszDefaultValue, m_pIniPath);

		return lpszDefaultValue;
	}

	return temp;
}

int CCommonInfo::ReadIniFile(const wchar_t* lpszAppName, const wchar_t* lpszKeyName, int iDefaultValue)
{
	wchar_t temp[16];

	int ret = GetPrivateProfileIntW(lpszAppName, lpszKeyName, iDefaultValue, m_pIniPath);
	if (ret != iDefaultValue)
	{
		_itow_s(ret, temp, 10);
		WritePrivateProfileStringW(lpszAppName, lpszKeyName, temp, m_pIniPath);
		//return -1;
	}

	return ret;
}

void CCommonInfo::DeleteIniFile(const wchar_t* lpszAppName)
{
	WritePrivateProfileStringW(lpszAppName, NULL, NULL, m_pIniPath);
}

void CCommonInfo::WriteIniFile(const wchar_t* lpszAppName, const wchar_t* lpszKeyName, const wchar_t* lpszValue)
{
	WritePrivateProfileStringW(lpszAppName, lpszKeyName, lpszValue, m_pIniPath);
}

void CCommonInfo::WriteIniFile(const wchar_t* lpszAppName, const wchar_t* lpszKeyName, int iValue)
{
	wchar_t temp[16];
	_itow_s(iValue, temp, 10);

	WritePrivateProfileStringW(lpszAppName, lpszKeyName, temp, m_pIniPath);
}

std::wstring CCommonInfo::GetLogDirectory()
{
	return m_pLogPath;
}

void CCommonInfo::KSNCOutputDebugString(LPCTSTR pszStr, ...)
{

#ifdef _DEBUG
	TCHAR szMsg[4096];
	va_list args;
	va_start(args, pszStr);
	_vstprintf_s(szMsg, 4096, pszStr, args);
	OutputDebugString(szMsg);
#endif

}

void CCommonInfo::WriteLog(const wchar_t * title, const wchar_t * format, ...)
{
	m_write_lock.lock();

	CTime time;
	time = CTime::GetCurrentTime();
	wchar_t strlog[2048];
	wchar_t snFileName[256];

	va_list argptr;

	va_start(argptr, format);
	vswprintf_s(strlog, format, argptr);
	va_end(argptr);

	strlog[1024] = L'\0';

	::OutputDebugStringW(strlog);

	//swprintf_s(snFileName, L"%ls\\Converter %d.log", Converter, time.GetDayOfWeek());
	swprintf_s(snFileName, L"%ls\\Converter-%s.log", m_pConverter, (const WCHAR *)time.Format(L"%Y-%m-%d"));// time.GetDayOfWeek());

	FILE *fp;
	fp = _wfsopen(snFileName, L"at", _SH_DENYNO);
	if (fp) {
		fwprintf(fp, L"[%ls] %ls %ls\n", title, (const WCHAR *)time.Format(L"%Y-%m-%d %H:%M:%S"), strlog);
		fclose(fp);
	}

	m_write_lock.unlock();
}

void CCommonInfo::WriteLog(const char* title, const char* format, const char* str, int integer) {

	char buffer[256] = {};
	sprintf_s(buffer, sizeof(buffer), format, str, integer);

	CCommonInfo::GetInstance()->WriteLog(title, buffer);

}

void CCommonInfo::WriteLog(const char * Title, const char * Format, const char * str) {

	char buffer[256] = {};
	sprintf_s(buffer, sizeof(buffer), Format, str);

	CCommonInfo::GetInstance()->WriteLog(Title, buffer);
}


void CCommonInfo::WriteLog(const char * Title, const char * Format, ...)
{
	m_write_lock.lock();

	CTime time;
	time = CTime::GetCurrentTime();
	wchar_t strlog[2048];
	wchar_t snFileName[256];

	std::string title = Title;
	std::string format = Format;

	std::wstring wtitle = L"";
	std::wstring wformat = L"";

	wtitle.assign(title.begin(), title.end());
	wformat.assign(format.begin(), format.end());

	const wchar_t* conwformat = wformat.c_str();
	const wchar_t* conwwtitle = wtitle.c_str();

	va_list argptr;

	va_start(argptr, conwformat);
	vswprintf_s(strlog, conwformat, argptr);
	va_end(argptr);

	strlog[1024] = L'\0';

	::OutputDebugStringW(strlog);

	//swprintf_s(snFileName, L"%ls\\Converter %d.log", Converter, time.GetDayOfWeek());
	swprintf_s(snFileName, L"%ls\\Converter-%s.log", m_pConverter, (const WCHAR *)time.Format(L"%Y-%m-%d"));// time.GetDayOfWeek());

	FILE *fp;
	fp = _wfsopen(snFileName, L"at", _SH_DENYNO);
	if (fp) {
		fwprintf(fp, L"[%ls] %ls %ls\n", conwwtitle, (const WCHAR *)time.Format(L"%Y-%m-%d %H:%M:%S"), strlog);
		fclose(fp);
	}

	m_write_lock.unlock();
}

bool CCommonInfo::GetProtocol(const char* url)
{
	if (_strnicmp(url, "rtsp", 4) == 0)
		return true;
	else
		return false;
}

std::string* CCommonInfo::StringSplit(std::string strTarget, std::string strTok)
{
	size_t     nCutPos;
	int     nIndex = 0;
	std::string* strResult = new std::string[1000];

	while ((nCutPos = strTarget.find_first_of(strTok)) != strTarget.npos)
	{
		if (nCutPos > 0)
		{
			strResult[nIndex++] = strTarget.substr(0, nCutPos);
		}
		strTarget = strTarget.substr(nCutPos + 1);
	}

	if (strTarget.length() > 0)
	{
		strResult[nIndex++] = strTarget.substr(0, nCutPos);
	}

	return strResult;
}

std::vector<std::string> CCommonInfo::split(const std::string &s, char delim) {
	std::stringstream ss(s);
	std::string item;
	std::vector<std::string> tokens;
	while (getline(ss, item, delim)) {
		tokens.push_back(item);
	}
	return tokens;
}

void CCommonInfo::GetIniSectionList(std::vector<std::wstring>* sectionlist)
{
	wchar_t SectionBuffer[256];
	int l = 0;

	GetPrivateProfileSectionNamesW(SectionBuffer, 256, m_pIniPath);

	for (int i = 0; i < 256; ++i)
	{
		if (SectionBuffer[i] == '\0')
		{
			if (i > l)
			{
				std::wstring s = &SectionBuffer[l];
				sectionlist->push_back(s);
			}
			else if (i == l)  // 2 zeros detected
				break;
			l = i + 1;
		}
	}

	return;
}

unsigned char CCommonInfo::FirstHexToBytes(const std::string& hex)
{
	return (unsigned char)strtol(hex.substr(0, 2).c_str(), NULL, 16);
}

std::vector<unsigned char> CCommonInfo::HexToBytes(const std::string& hex)
{
	std::vector<unsigned char> bytes;

	for (unsigned int i = 0; i < hex.length(); i += 2)
	{
		std::string byteString = hex.substr(i, 2);
		unsigned char byte = (unsigned char)strtol(byteString.c_str(), NULL, 16);
		bytes.push_back(byte);
	}

	return bytes;
}
