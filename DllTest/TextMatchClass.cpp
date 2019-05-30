#include "TextMatchClass.h"

TextMatchClass::TextMatchClass() : Find_String(""), threshold(0), fomula("EQUAL") {

	Detect = false;

}

//std::string input_string, std::string find_string, int type, int threshold, std::string fomula
TextMatchClass::TextMatchClass(std::string input_string, std::string find_string, int type, int threshold, std::string fomula)
	: Base_String(input_string), Type((INTYPE)type), Find_String(find_string), threshold(threshold), fomula(fomula)
{
	Detect = false;

	switch (Type) {

	case TENG :
		Detect = Find_Base_String(input_string);
		break;
	case TKOR :
		Detect = Han_Delete(input_string);
		break;
	case TNUM :
		Detect = Find_Scope(input_string);
		break;
	default:
		Detect = false;
		std::cout << "Text_Match_Class : 알수없는 형태가 들어옴" << std::endl;
		break;
	}

	if (Detect == true) {
		std::cout << "Detected! return 1" << std::endl;
	}


}

TextMatchClass::~TextMatchClass()
{
}


bool TextMatchClass::Han_Delete(std::string input_string) {

	while (1) {						//공백 제거 알고리즘

		size_t pos = input_string.find(" ", 0);
		if (pos == std::string::npos)
			break;

		input_string.erase(pos, 1);
	}
	
	if (Find_Base_String(input_string)) {
		return true;
	}
	else
		return false;
	
}


bool TextMatchClass::Find_Base_String(std::string input_string) {

	if (strstr(input_string.c_str(), Find_String.c_str()) != NULL) {
		return true;
	}

	return false;
}


bool TextMatchClass::Find_Scope(std::string input_string) {

	int Base_int = 0;
	std::string temp = "";

	Base_int = atoi(input_string.c_str());

	if (strstr(fomula.c_str(), "EQUAL") != NULL ) {

		if (Base_int == threshold) {
			return true;
		}
		return false;
	}
	else if (strstr(fomula.c_str(), "LESSTHAN") != NULL ) {
		
		if (Base_int < threshold) {
			return true;
		}
		return false;
	}
	else if (strstr(fomula.c_str(), "MORETHAN") != NULL ) {
		
		if (Base_int > threshold) {
			return true;
		}
		return false;
	}

	return false;
}
