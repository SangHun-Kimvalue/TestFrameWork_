#pragma once
#include <iostream>

//typedef enum { TENG = 1, TKOR = 2, TNUM = 3}INTYPE;

class TextMatchClass
{
public:

	TextMatchClass(std::string find_string, int type, int Base_Num, std::string fomula, bool Consistent);
	TextMatchClass(std::string input_string, std::string find_string, int type, int Base_Num, std::string fomula, bool Consistent);
	~TextMatchClass();

	bool Han_Delete(std::string input_string);
	bool Find_Base_String(std::string input_string);
	bool Find_Scope(std::string Input_Num);
	
	bool Detect;

private:

	int Remain_Num(std::string input_string);
	std::string Delete_Enter(std::string input_string);

	const int Base_Num;
	std::string Input_String;
	const std::string Base_String;
	const std::string Fomula;
	const bool Consistent;

};

