#include "TextMatchClass.h"

TextMatchClass::TextMatchClass() : Base_Num(0) {
	ConvFomula();
}

TextMatchClass::TextMatchClass(std::string Base_String, int Base_Num, std::string fomula)
	: Base_String(Base_String), Base_Num(Base_Num), Detect(false), Consistent(true), ENumFomula(EQUAL),
	StringFomula(fomula), minor(false){
	
	ConvFomula();
}

TextMatchClass::~TextMatchClass()
{
}

//줄바꿈 제거 알고리즘
std::string TextMatchClass::Delete_Enter(std::string input_string) {

	//std::cout << input_string.c_str() << std::endl;

	while (1) {						//줄바꿈 제거 알고리즘

		size_t pos = input_string.find('\n', 0);
		if (pos == std::string::npos)
			break;

		input_string.erase(pos, 1);
	}

	//std::cout << input_string.c_str() << std::endl;

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
	
	if (Find_Base_String(input_string) == true) {
		return true;
	}
	else
		return false;
}

//위 두 메서드에서 처리한 문자열을 가지고 베이스 문자열 찾기
bool TextMatchClass::Find_Base_String(std::string input_string) {

	//std::string Deleted_String = Delete_Enter(input_string);

	if (strstr(input_string.c_str(), Base_String.c_str()) != NULL) {
		if (Consistent == true) {
			Detect = true;
			return Detect;
		}
		Detect = false;
	}
	else {
		if (Consistent == false) {
			Detect = true;
			return Detect;
		}
		Detect = false;
	}

	return false;
}

int TextMatchClass::Remain_Num(std::string input_string) {

	std::string temp;
	int leng = input_string.length();

	for (int i = 0; i < leng; i++) {				//타입 검출
		if (input_string.at(i) < 0)
			continue;
		if (isdigit(input_string.at(i)) != 0) {		//(48 <= input_string.at(i) || 57 >= input_string.at(i)){//		//숫자
			temp = temp + input_string.at(i);
		}
		//if (input_string.at(i) == '-') {
		//	minor = true;
		//}
	}
	if (temp == "" && minor == false) {
		return -1;
	}

	return atoi(temp.c_str());
}

void TextMatchClass::ConvFomula() {

	if (Base_Num < 0) {
		minor = true;
	}

	if (strstr(StringFomula.c_str(), "NOTEQUAL") != NULL) {
		ENumFomula = NEQUAL;
		Consistent = false;
	}
	else if (strstr(StringFomula.c_str(), "EQUAL") != NULL) {
		ENumFomula = EQUAL;
		Consistent = true;
	}
	else if (strstr(StringFomula.c_str(), "LESSTHAN") != NULL) {
		ENumFomula = LESST;
	}
	else if (strstr(StringFomula.c_str(), "MORETHAN") != NULL) {
		ENumFomula = MORET;
	}
	else {
		std::cerr << "Formual Error" << std::endl;
	}
	return ;
}


//Formula에 따라 숫자 찾기
bool TextMatchClass::Find_Scope(std::string Input_Num) {

	if (minor == true) {
		Base_String = std::to_string(Base_Num);

		return Find_Base_String(Input_Num);
	}

	int Only_Num = Remain_Num(Input_Num);

	if (ENumFomula == EQUAL || ENumFomula == NEQUAL) {

		if (Only_Num == Base_Num) {
			if (Consistent == true) {
				Detect = true;
				return Detect;
			}
			Detect = false;
			return Detect;
		}
		if (Consistent == false) {
			Detect = true;
			return Detect;
		}
		Detect = false;
		return Detect;
	}
	else if (ENumFomula == LESST) {
		
		if (Only_Num < Base_Num) {
			Detect = true;
			return Detect;
		}
		Detect = false;
		return Detect;
	}
	else if (ENumFomula == MORET) {
		
		if (Only_Num > Base_Num) {
			Detect = true;
			return Detect;
		}
		Detect = false;
		return Detect;
	}
	else {
		std::cerr << "Formual Error" << std::endl;
		Detect = false;
		return Detect;
	}

	return false;
}
