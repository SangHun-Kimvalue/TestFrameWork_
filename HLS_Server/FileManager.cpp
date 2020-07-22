#include "FileManager.h"

//FileManager::FileManager() : SegCount(DEFAULTSEGCOUNT), arr(nullptr), m_UUID({}) {
//	InitBitrate(SegCount);
//}

FileManager::FileManager(std::string URL) : SegCount(1), ConnectURL(URL) ,arr(nullptr), m_Dir(""){
	
	ParseURL();
	MakeFolder();

	//unsigned char* astr = nullptr;
	//UuidToStringA(&m_UUID, &astr);
	//string str_array(reinterpret_cast<char const *>(astr));
	//unsigned short* wstr = nullptr;
	//UuidToStringW(&m_UUID, &wstr);
	//std::wstring Path;
	//Path = (wchar_t*)wstr;
	//CreateDirectory((Path.c_str()), NULL);
	//InitBitrate(SegCount);
}

FileManager::~FileManager() {
	
	DeleteFolder();
	
	free(arr);
}

void FileManager::ParseURL(){

	size_t fpos = ConnectURL.find("[");
	size_t epos = ConnectURL.find("]");

	m_Dir = ConnectURL.substr(fpos + 1, (fpos - epos) - 1);
	size_t syntax = m_Dir.find("://");
	m_Dir.replace(syntax, 3, "_");
	syntax = m_Dir.find("/");
	m_Dir.replace(syntax, 1, "_");

	return ;
}

bool FileManager::CopytoDummy() {


	//fs::copy(originalTextPath, m_Dir);
	//fs::copy(originalDirPath, copiedDirPath1);
	//fs::copy(originalDirPath, copiedDirPath2, fs::copy_options::skip_existing);

	return false;
}

bool FileManager::InitBitrate() {

	int preset[5] = { 1080, 720, 480, 360 };
	arr = (int *)malloc(sizeof(int) * SegCount);


	for (int i = 0; i < SegCount; i++) {
		*(arr + i) = preset[i];
	}
	
	return true;
}

int FileManager::MakeFolder() {
	
	if (m_Dir == "") {
		return -1;
	}

	cout << m_Dir.c_str() << endl;
	_mkdir(m_Dir.c_str());

	cout << " Create Dir count : " << MakeFolder() << endl;

	int count = 0;
	
	//For ABS
	//for(int i = 0 ; i < SegCount ; i++) {
	//
	//	string* makedir = new string("");
	//
	//	*makedir = m_UUIDDir + "\\" + BitratePreset[i]+ "\\";
	//	_mkdir(makedir->c_str());
	//
	//	CreatedDir[i] = const_cast<char*>(makedir->c_str());
	//	count++;
	//}

	return count;
}

char** FileManager::GetDirPath() {
	return CreatedDir;
}

int isFileOrDir(_finddata_t fd)
//�������� ���丮���� �Ǻ�
{
	if (fd.attrib & _A_SUBDIR)
		return 0; // ���丮�� 0 ��ȯ
	else
		return 1; // �׹��� ���� "�����ϴ� ����"�̱⿡ 1 ��ȯ
}

void remove_dir(const wchar_t* folder)
{
	std::wstring search_path = std::wstring(folder) + (L"/*.*");
	std::wstring s_p = std::wstring(folder) + (L"/");
	WIN32_FIND_DATA fd;
	HANDLE hFind = ::FindFirstFile(search_path.c_str(), &fd);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				if (wcscmp(fd.cFileName, (L".")) != 0 && wcscmp(fd.cFileName, (L"..")) != 0)
				{
					remove_dir((wchar_t*)(s_p + fd.cFileName).c_str());
				}
			}
			else {
				DeleteFile((s_p + fd.cFileName).c_str());
			}
		} while (::FindNextFile(hFind, &fd));
		::FindClose(hFind);
		_wrmdir(folder);
	}
}

int DeleteDirectory(const std::string &refcstrRootDirectory,
	bool              bDeleteSubdirectories = true)
{
	bool            bSubdirectory = false;       // Flag, indicating whether
												 // subdirectories have been found
	HANDLE          hFile;                       // Handle to directory
	std::string     strFilePath;                 // Filepath
	std::string     strPattern;                  // Pattern
	WIN32_FIND_DATAA FileInformation;             // File information


	strPattern = refcstrRootDirectory + "\\*.*";
	hFile = ::FindFirstFileA(strPattern.c_str(), &FileInformation);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (FileInformation.cFileName[0] != '.')
			{
				strFilePath.erase();
				strFilePath = refcstrRootDirectory + "\\" + FileInformation.cFileName;

				if (FileInformation.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					if (bDeleteSubdirectories)
					{
						// Delete subdirectory
						int iRC = DeleteDirectory(strFilePath, bDeleteSubdirectories);
						if (iRC)
							return iRC;
					}
					else
						bSubdirectory = true;
				}
				else
				{
					// Set file attributes
					if (::SetFileAttributesA(strFilePath.c_str(),
						FILE_ATTRIBUTE_NORMAL) == FALSE)
						return ::GetLastError();

					// Delete file
					if (::DeleteFileA(strFilePath.c_str()) == FALSE)
						return ::GetLastError();
				}
			}
		} while (::FindNextFileA(hFile, &FileInformation) == TRUE);

		// Close handle
		::FindClose(hFile);

		DWORD dwError = ::GetLastError();
		if (dwError != ERROR_NO_MORE_FILES)
			return dwError;
		else
		{
			if (!bSubdirectory)
			{
				// Set directory attributes
				if (::SetFileAttributesA(refcstrRootDirectory.c_str(),
					FILE_ATTRIBUTE_NORMAL) == FALSE)
					return ::GetLastError();

				// Delete directory
				if (::RemoveDirectoryA(refcstrRootDirectory.c_str()) == FALSE)
					return ::GetLastError();
			}
		}
	}

	return 0;
}

bool FileManager::DeleteFolder() {

	int Delete = 0;
	for (int i = SegCount-1; i >= 0; i--) {

		string temp = CreatedDir[i];
		wstring wwtemp;
		wwtemp.assign(temp.begin(), temp.end());
		//const wchar_t* wtemp = wwtemp.c_str();
		remove_dir(const_cast<wchar_t*>(wwtemp.c_str()));
		Delete++;
	}

	if (Delete == SegCount) {
		if (_rmdir(m_Dir.c_str()) == 0) {
			CCommonInfo::GetInstance()->WriteLog("INFO", "Success Delete Folder %s", CreatedDir[0]);
			return true;
		}
		else {
			CCommonInfo::GetInstance()->WriteLog("ERROR", "Error Delete Folder %s", CreatedDir[0]);
			return false;
		}
	}
	else {
		CCommonInfo::GetInstance()->WriteLog("ERROR", "Error Delete Folder %s", CreatedDir[0]);
		return false;
	}
}

int searchingDir(string path)
{
	int checkDirFile = 0;
	bool Find = false;
	string dirPath = path + "\\*.*";
	struct _finddata_t fd;//���丮 �� ���� �� ���� ���� ���� ��ü
	intptr_t handle;
	list<_finddata_t> fdlist;//���丮, ���� ���� ��ü ����Ʈ

	if ((handle = _findfirst(dirPath.c_str(), &fd)) == -1L) //fd ����ü �ʱ�ȭ.
	{
		//�����̳� ���丮�� ���� ���.
		cout << "No file in directory!" << endl;
		return -1;
	}

	do //���� Ž�� �ݺ� ����
	{
		checkDirFile = isFileOrDir(fd);//���� ��ü ���� Ȯ��(���� or ���丮)
		if (checkDirFile == 0 && fd.name[0] != '.') {
			//���丮�� ���� ����
			cout << "Dir  : " << path << "\\" << fd.name << endl;
			int check = searchingDir(path + "\\" + fd.name);//����� ȣ��
			if (check = 1)
				Find = true;
		}
		else if (checkDirFile == 1 && fd.name[0] != '.') {
			//������ ���� ����
			cout << "File : " << path << "\\" << fd.name << endl;
			fdlist.push_back(fd);
			Find = true;
		}

	} while (_findnext(handle, &fd) == 0);
	_findclose(handle);

	if (Find)
		return 1;
	else
		return -1;

}