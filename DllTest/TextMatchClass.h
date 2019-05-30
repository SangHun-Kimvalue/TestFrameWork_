#pragma once
#include <iostream>

typedef enum { TENG = 1, TKOR = 2, TNUM = 3}INTYPE;

class TextMatchClass
{
public:
	TextMatchClass();
	TextMatchClass(std::string input_string, std::string find_string, int type, int threshold, std::string fomula);
	~TextMatchClass();

	bool Han_Delete(std::string input_string);
	bool Find_Base_String(std::string input_string);
	bool Find_Scope(std::string input_string);


private:

	const int threshold;
	std::string Base_String;
	const std::string Find_String;
	const std::string fomula;

	INTYPE Type;

	bool Detect;
};

