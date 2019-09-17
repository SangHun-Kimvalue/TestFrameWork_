#pragma once
#include <iostream>
#include <string>

typedef enum RTSP_Server_Type { LIVE555, NOTVALID } RST;
typedef enum Source_Type { LOCALFILE, NOTSUPPORT } ST;

class __declspec(dllexport) m_RTSPServer
{
public:

	//virtual void createNew() = 0;
	m_RTSPServer() : m_port(0), quit(-1) {}
	virtual ~m_RTSPServer() {}
	
	virtual void Release() = 0;
	virtual bool Initialize(int port, std::string Filename) = 0;
	virtual void Run() = 0;
	virtual char*  GetURL() = 0;
	virtual char*   GetStreamName() = 0;

	char* URL;
	int m_port;
	char quit;

	//InitLoader* IniLoader;
	//
	//class InitLoader
	//{
	//public:
	//
	//	static InitLoader* createNew(const wchar_t* FullPath = L""){
	//
	//		const wchar_t* defaultini = L"config.ini";
	//
	//		if (FullPath == L"") {
	//			return new InitLoader(defaultini);
	//		}
	//
	//		return new InitLoader(FullPath);
	//	}
	//	//static InitLoader* createNew(const char* FullPath = "");
	//
	//	std::string Load(const wchar_t* Section, const wchar_t* Key) {
	//
	//		//LPWSTR lpAppName = const_cast<LPWSTR>(Section);
	//		//LPWSTR lpKeyName = const_cast<LPWSTR>(Key);
	//		//LPWSTR lpDefault = const_cast<LPWSTR>(L"config.ini");
	//		WCHAR  lpReturnedString[100];
	//
	//		GetPrivateProfileStringW(const_cast<LPWSTR>(Section), const_cast<LPWSTR>(Key), const_cast<LPWSTR>(L"config.ini")
	//			, lpReturnedString, 100, IniPath);
	//
	//		std::wstring temp = lpReturnedString;
	//		std::string con;
	//		con.assign(temp.begin(), temp.end());
	//
	//		return con;
	//	}
	//
	//	std::string Load(const char* Section, const char* Key) {
	//
	//		LPCSTR PathTemp = MFullPath.c_str();
	//		CHAR  lpReturnedString[100];
	//
	//		GetPrivateProfileStringA(const_cast<LPCSTR>(Section), const_cast<LPCSTR>(Key), const_cast<LPCSTR>("config.ini")
	//			, lpReturnedString, 100, PathTemp);
	//
	//		std::string con = lpReturnedString;
	//		//con.assign(temp.begin(), temp.end());
	//
	//		return con;
	//	}
	//
	//	~InitLoader() {};
	//
	//private:
	//
	//	InitLoader(const wchar_t* iniName)
	//	{
	//		std::wstring wtemp = iniName;
	//		TCHAR* temp = (wchar_t *)wtemp.c_str();
	//
	//		TCHAR FullPath[1024];
	//
	//		::GetModuleFileNameW(::GetModuleHandle(nullptr), FullPath, _countof(FullPath));
	//		wchar_t* pnt = nullptr;
	//		pnt = wcsrchr(FullPath, L'\\');
	//
	//		if (pnt != nullptr) {
	//			*pnt = L'\0';
	//			swprintf_s(IniPath, L"%ls\\%s", FullPath, temp);
	//		}
	//
	//		wtemp = (wchar_t*)IniPath;
	//		MFullPath.assign(wtemp.begin(), wtemp.end());
	//
	//		//std::string temp = Load_Profile(L"SET", L"FILENAME");
	//		//std::cout << Load_Profile(L"SET", L"FILENAME") << std::endl;
	//	}
	//
	//	TCHAR IniPath[2048];
	//	std::string MFullPath;
	//};

protected:

	
	//int portNumber;
	//int httpTunnelingPort;

};
