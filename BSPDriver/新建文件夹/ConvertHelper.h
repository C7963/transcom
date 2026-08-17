#pragma once
#include <vector>
using namespace std;

class ConvertHelper
{
public:
	static vector<unsigned char> ToUnsignedChar(vector<unsigned int> int_value);
	static vector<unsigned char> ToUnsignedChar(vector<float> float_value);

	static uint8_t* ToUnsignedCharArray(vector<unsigned int> int_value);
	//static uint8_t* ToListArray(list<BYTE> int_value);
	static uint8_t* ToUnsignedCharArray(vector<float> float_value);
	static unsigned int Convert4CharToInt(const unsigned char* Chars, int start);
	static vector<unsigned int> ConvertCharArrayToIntArray(const unsigned char* Chars, int size);
	//static string WString2String(const wstring& ws);

	//static wstring String2WString(const string& s);
};
