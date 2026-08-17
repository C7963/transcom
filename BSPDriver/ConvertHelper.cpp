#include "pch.h"
#include "ConvertHelper.h"
#include <iostream>
#include <vector>
#include <list>
#include <unordered_map>
#include <vector>
#include <cmath>
#include <stdexcept>
#include <thread>
#include <chrono>
#include <iostream>
#include <windows.h>
#include <string>
#include "Device_MEM32.h"
#include "NVMe_Write.h"
#include <bitset>
#include <unordered_map>
#include <vector>
#include <cmath>
#include <stdexcept>
#include <thread>
#include <chrono>
#include <time.h>

using namespace std;
/// <summary>
/// int转字节数组
/// </summary>
/// <param name="data"></param>
/// <returns></returns>
vector<unsigned char> ConvertHelper::ToUnsignedChar(vector<unsigned int> int_value)
{
	vector<unsigned char> arrayOfByte(4 * int_value.size());
	for (int j = 0; j < int_value.size(); j++)
	{
		for (int i = 0; i < 4; i++)
			arrayOfByte[(3 - i) + (j * 4)] = (int_value[j] >> (i * 8));
	}
	return arrayOfByte;
}

/// <summary>
/// float转字节数组
/// </summary>
/// <param name="float_value"></param>
/// <returns></returns>
vector<unsigned char> ConvertHelper::ToUnsignedChar(vector<float> float_value)
{
	vector<unsigned char> bytess(4 * float_value.size());
	for (int j = 0; j < float_value.size(); j++)
	{
		unsigned char* pdata = (unsigned char*)&float_value[j];
		for (int i = 0; i < 4; i++)
		{
			//指针地址索引转换数据
			bytess[i + j * 4] = *(pdata + i);
		}
	}
	return bytess;
}

uint8_t* ConvertHelper::ToUnsignedCharArray(vector<unsigned int> int_value)
{
	vector<unsigned char> input = ToUnsignedChar(int_value);
	unsigned char* charray = new unsigned char[input.size()];
	for (int i = 0; i < input.size(); i++) {
		charray[3-i] = input[i];
	}
	/*for (int i = 0; i < 4; i++) {
		std::cout << (int)charray[i] << std::endl;
	}*/
	return charray;
}

//uint8_t* ConvertHelper::ToListArray(list<BYTE> int_value)
//{
//	vector<unsigned char> input = ToUnsignedChar(int_value);
//	unsigned char* charray = new unsigned char[input.size()];
//	for (int i = 0; i < input.size(); i++) {
//		charray[3 - i] = input[i];
//	}
//	return charray;
//}

uint8_t* ConvertHelper::ToUnsignedCharArray(vector<float> int_value)
{
	vector<unsigned char> input = ToUnsignedChar(int_value);
	unsigned char* charray = new unsigned char[input.size()];
	for (int i = 0; i < input.size(); i++) {
		charray[i] = input[i];
	}
	return charray;
}

//将4个char转换位一个int
unsigned int ConvertHelper::Convert4CharToInt(const unsigned char* Chars, int start)
{
	int n0 = Chars[start + 0];      //0~8位
	int n1 = Chars[start + 1];      //8~16位
	int n2 = Chars[start + 2];      //16~24位
	int n3 = Chars[start + 3];      //24~32位
	return
		(n0 << 0)           //0~8位
		+ (n1 << 8)         //8~16位
		+ (n2 << 16)        //16~24位
		+ (n3 << 24);       //24~32位
}

//多个char 转int Array
vector<unsigned int> ConvertHelper::ConvertCharArrayToIntArray(const unsigned char* Chars, int size)
{
	vector<unsigned int> res(size / 4);
	for (size_t i = 0; i < size / 4; i++)
	{
		res[i] = Convert4CharToInt(Chars, i * 4);
	}
	return res;
}

////wstring=>string
//string ConvertHelper::WString2String(const wstring& ws)
//{
//	std::string strLocale = setlocale(LC_ALL, "");
//	const wchar_t* wchSrc = ws.c_str();
//	size_t nDestSize = wcstombs(NULL, wchSrc, 0) + 1;
//	char* chDest = new char[nDestSize];
//	memset(chDest, 0, nDestSize);
//	wcstombs(chDest, wchSrc, nDestSize);
//	std::string strResult = chDest;
//	delete[]chDest;
//	setlocale(LC_ALL, strLocale.c_str());
//	return strResult;
//}
//
//// string => wstring
//wstring ConvertHelper::String2WString(const string& s)
//{
//	std::string strLocale = setlocale(LC_ALL, "");
//	const char* chSrc = s.c_str();
//	size_t nDestSize = mbstowcs(NULL, chSrc, 0) + 1;
//	wchar_t* wchDest = new wchar_t[nDestSize];
//	wmemset(wchDest, 0, nDestSize);
//	mbstowcs(wchDest, chSrc, nDestSize);
//	std::wstring wstrResult = wchDest;
//	delete[]wchDest;
//	setlocale(LC_ALL, strLocale.c_str());
//	return wstrResult;
//}