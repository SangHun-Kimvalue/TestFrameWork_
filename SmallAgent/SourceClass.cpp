#include "SourceClass.h"

void SourceClass::Thread_Decode_Test() {

	Thread_D_P = thread(&SourceClass::Thread_Decode, this);
	//Thread_Decode();

	return;
}

string SourceClass::open_source_name() {

	FILE* pFile;
	char temp[50] = "";
	string source_name = "";
	string fileloc = "Src_name\\Src_name.txt";
	fopen_s(&pFile, fileloc.c_str(), "r");
	if (!pFile) {
		printf("\nSrc_name not founded\n");
		return false;
	}
	fseek(pFile, 0, SEEK_SET);

	fgets(temp, sizeof(temp), pFile);
	fclose(pFile);

	source_name = temp;
	//string source_extands = temp;
	//int loc = source_extands.find(".");
	//extand = source_extands.erase(0, loc);

	return source_name;
}

string SourceClass::Find_src_file() {

	string src_name = "";
	src_name = open_source_name();
	char str[] = "*.*";

	string temp = "*.*";
	string path = "SRC\\";
	string temp2 = path + temp;
	struct _finddata_t fd;
	intptr_t handle;

	if ((handle = _findfirst(temp2.c_str(), &fd)) == -1L) {
		std::printf("No file in directory!");
		_findclose(handle);
		return false;
	}

	do {
		temp = fd.name;
		if (temp.find(".", 0) == 0)
			continue;
		if (temp.find(src_name + to_string(index_t), 0) == 0)
			break;

		//count++;
	} while (_findnext(handle, &fd) == 0);

	_findclose(handle);
	src_name = path + fd.name;

	return src_name;
}
