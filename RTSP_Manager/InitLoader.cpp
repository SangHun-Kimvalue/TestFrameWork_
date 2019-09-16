#include "stdafx.h"
#include "InitLoader.h"


InitLoader::InitLoader(const wchar_t* iniName)
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

InitLoader::~InitLoader()
{
}

InitLoader* InitLoader::createNew(const wchar_t* ininame) {

	const wchar_t* defaultini = L"config.ini";

	if (ininame == L"") {
		return new InitLoader(defaultini);
	}

	return new InitLoader(ininame);
}

//InitLoader* InitLoader::createNew(const char* ininame) {
//
//	if (ininame == "") {
//		return new InitLoader(L"config.ini");
//	}
//
//	std::string stemp = ininame;
//	std::wstring wtemp;
//	wtemp.assign(stemp.begin(), stemp.end());
//
//	return new InitLoader((wchar_t *)wtemp.c_str());
//}

std::string InitLoader::Load(const wchar_t* Section, const wchar_t* Key) {

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

std::string InitLoader::Load(const char* Section, const char* Key) {

	LPCSTR PathTemp = MFullPath.c_str();
	CHAR  lpReturnedString[100];

	GetPrivateProfileStringA(const_cast<LPCSTR>(Section), const_cast<LPCSTR>(Key), const_cast<LPCSTR>("config.ini")
		, lpReturnedString, 100, PathTemp);

	std::string con = lpReturnedString;
	//con.assign(temp.begin(), temp.end());

	return con;
}