#include "TextMatchClass.h"

TextMatchClass::TextMatchClass(std::string find_string, int type, int Base_Num, std::string fomula, bool Consistent)
	: Base_String(find_string), Base_Num(Base_Num), Fomula("EQUAL"), Consistent(Consistent), Detect(false) {
	
	Detect = false;

}

//std::string input_string, std::string find_string, int type, int threshold, std::string fomula
TextMatchClass::TextMatchClass(std::string input_string, std::string find_string, int type, int Base_Num, std::string fomula, bool Consistent)
	: Input_String(input_string), Base_Num(Base_Num), Base_String(find_string), Fomula(fomula), Consistent(false)
{
	Detect = false;

	//switch (Type) {
	//
	//case 1 :
	//	Detect = Find_Base_String(input_string);
	//	break;
	//case 2 :
	//	Detect = Han_Delete(input_string);
	//	break;
	//case 3 :
	//	Detect = Find_Scope(input_string);
	//	break;
	//default:
	//	Detect = false;
	//	std::cout << "Text_Match_Class : 알수없는 형태가 들어옴" << std::endl;
	//	break;
	//}

}

TextMatchClass::~TextMatchClass()
{
}

//줄바꿈 제거 알고리즘
std::string TextMatchClass::Delete_Enter(std::string input_string) {

	std::cout << input_string.c_str() << std::endl;

	while (1) {						//줄바꿈 제거 알고리즘

		size_t pos = input_string.find('\n', 0);
		if (pos == std::string::npos)
			break;

		input_string.erase(pos, 1);
	}

	std::cout << input_string.c_str() << std::endl;

	return input_string;
}

//공백 제거 알고리즘
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

//위 두 메서드에서 처리한 문자열을 가지고 베이스 문자열 찾기
bool TextMatchClass::Find_Base_String(std::string input_string) {

	std::string Deleted_String = Delete_Enter(input_string);

	if (strstr(Deleted_String.c_str(), Base_String.c_str()) != NULL) {
		if (Consistent == false) {
			Detect = false;
			return Detect;
		}
		Detect = true;
		return Detect;
	}

	return false;
}

//Formula에 따라 숫자 찾기
bool TextMatchClass::Find_Scope(int Input_Num) {

	//Base_int = atoi(input_string.c_str());

	if (strstr(Fomula.c_str(), "EQUAL") != NULL ) {

		if (Input_Num == Base_Num) {
			Detect = true;
			return Detect;
		}
		Detect = false;
		return Detect;
	}
	else if (strstr(Fomula.c_str(), "LESSTHAN") != NULL ) {
		
		if (Input_Num < Base_Num) {
			Detect = true;
			return Detect;
		}
		Detect = false;
		return Detect;
	}
	else if (strstr(Fomula.c_str(), "MORETHAN") != NULL ) {
		
		if (Input_Num > Base_Num) {
			Detect = true;
			return Detect;
		}
		Detect = false;
		return Detect;
	}
	else {
		std::cout << "Formual Error" << std::endl;
		Detect = false;
		return Detect;
	}

	return false;
}
