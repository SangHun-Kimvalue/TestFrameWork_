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
	//	std::cout << "Text_Match_Class : �˼����� ���°� ����" << std::endl;
	//	break;
	//}

}

TextMatchClass::~TextMatchClass()
{
}

//�ٹٲ� ���� �˰���
std::string TextMatchClass::Delete_Enter(std::string input_string) {

	//std::cout << input_string.c_str() << std::endl;

	while (1) {						//�ٹٲ� ���� �˰���

		size_t pos = input_string.find('\n', 0);
		if (pos == std::string::npos)
			break;

		input_string.erase(pos, 1);
	}

	//std::cout << input_string.c_str() << std::endl;

	return input_string;
}

//���� ���� �˰���
bool TextMatchClass::Han_Delete(std::string input_string) {

	while (1) {						//���� ���� �˰���

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

//�� �� �޼��忡�� ó���� ���ڿ��� ������ ���̽� ���ڿ� ã��
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

int TextMatchClass::Remain_Num(std::string input_string) {

	std::string temp;

	for (int i = 0; i < input_string.length(); i++) {		//Ÿ�� ����


		//else if (48 <= Base_String.at(i) || 57 >= Base_String.at(i)) {		//����			//���� ���� x 
		if (isdigit(input_string.at(i)) != 0) {		//����
			temp = temp + input_string.at(i);
		}
	}

	return atoi(temp.c_str());
}

//Formula�� ���� ���� ã��
bool TextMatchClass::Find_Scope(std::string Input_Num) {

	//Base_int = atoi(input_string.c_str());
	int Only_Num = Remain_Num(Input_Num);

	if (strstr(Fomula.c_str(), "EQUAL") != NULL) {

		if (Only_Num == Base_Num) {
			Detect = true;
			return Detect;
		}
		Detect = false;
		return Detect;
	}
	else if (strstr(Fomula.c_str(), "LESSTHAN") != NULL) {

		if (Only_Num < Base_Num) {
			Detect = true;
			return Detect;
		}
		Detect = false;
		return Detect;
	}
	else if (strstr(Fomula.c_str(), "MORETHAN") != NULL) {

		if (Only_Num > Base_Num) {
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
