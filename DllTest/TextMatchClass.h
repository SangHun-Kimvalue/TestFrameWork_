#pragma once
#include <iostream>

typedef enum {EQUAL, NEQUAL, MORET, LESST} FORMU;

class TextMatchClass
{
public:

	TextMatchClass(std::string find_string, int Base_Num, std::string fomula);
	~TextMatchClass();

	bool Han_Delete(std::string input_string);
	bool Find_Base_String(std::string input_string);
	bool Find_Scope(std::string Input_Num);
	
	const int Base_Num;
	std::string StringFomula;
	bool Detect;

private:

	void ConvFomula();
	std::string Delete_Enter(std::string input_string);
	int Remain_Num(std::string input_string);

	const std::string Base_String;
	bool Consistent;		//true 일치 false 불일치
	FORMU ENumFomula;

};

