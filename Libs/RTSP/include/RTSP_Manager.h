#pragma once
#include <iostream>
#include <string>
#include <thread>
#include "m_RTSPServer.h"

class RTSP_Manager {

public:

	RTSP_Manager() : Source_Type(NOTSUPPORT), Server_Type(NOTVALID) {
		//Initialize();
	};
	~RTSP_Manager() { FreeLibrary(hDLL); };

	bool Initialize();
	void Release();
	void Run();
	//void Restart();
	std::string Get_URL();
	std::string Get_Stream_Name();
	bool Get_Status();

private:

	void InitLoad();
	m_RTSPServer* DllLoad(std::string);

	m_RTSPServer* RTSP_Server;

	HMODULE hDLL = nullptr;
	ST Source_Type;
	RST Server_Type;

	std::string FileName;
	std::thread RunThread;


	class InitLoader;
	InitLoader* Loader;

	class InitLoader
	{
	public:

		std::string Load(const wchar_t* Section, const wchar_t* Key) {

			//LPWSTR lpAppName = const_cast<LPWSTR>(Section);
			//LPWSTR lpKeyName = const_cast<LPWSTR>(Key);
			//LPWSTR lpDefault = const_cast<LPWSTR>(L"config.ini");
			WCHAR  lpReturnedString[100];

			GetPrivateProfileStringW(const_cast<LPWSTR>(Section), const_cast<LPWSTR>(Key), const_cast<LPWSTR>(L"config.ini")
				, lpReturnedString, 100, IniPath);

			std::wstring temp = lpReturnedString;
			std::string con;
			con.assign(temp.begin(), temp.end());

			return con;
		}

		std::string Load(const char* Section, const char* Key) {

			LPCSTR PathTemp = MFullPath.c_str();
			CHAR  lpReturnedString[100];

			GetPrivateProfileStringA(const_cast<LPCSTR>(Section), const_cast<LPCSTR>(Key), const_cast<LPCSTR>("config.ini")
				, lpReturnedString, 100, PathTemp);

			std::string con = lpReturnedString;
			//con.assign(temp.begin(), temp.end());

			return con;
		}

		~InitLoader() {}

		static InitLoader* createNew(const wchar_t* FullPath = L"") {

			const wchar_t* defaultini = L"config.ini";

			if (FullPath == L"") {
				return new InitLoader(defaultini);
			}

			return new InitLoader(FullPath);
		}

	private:

		InitLoader(const wchar_t* iniName)
		{
			std::wstring wtemp = iniName;
			TCHAR* temp = (wchar_t *)wtemp.c_str();

			TCHAR FullPath[1024];

			::GetModuleFileNameW(::GetModuleHandle(nullptr), FullPath, _countof(FullPath));
			wchar_t* pnt = nullptr;
			pnt = wcsrchr(FullPath, L'\\');

			if (pnt != nullptr) {
				*pnt = L'\0';
				swprintf_s(IniPath, L"%ls\\%s", FullPath, temp);
			}

			wtemp = (wchar_t*)IniPath;
			MFullPath.assign(wtemp.begin(), wtemp.end());

			//std::string temp = Load_Profile(L"SET", L"FILENAME");
			//std::cout << Load_Profile(L"SET", L"FILENAME") << std::endl;
		}

		//TCHAR FullPath[2048];
		TCHAR IniPath[2048];
		std::string MFullPath;
	};

};