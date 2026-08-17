#include "pch.h"
#include "NVMe_Write.h"
#include <clocale>
#include "ConvertHelper.h"
#include <iostream>
#include "Device_Address.h"
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

using namespace std;

Device::NVMe_Write::~NVMe_Write()
{
	delete device;
	if (instance != nullptr)
		delete instance;
}

Device::NVMe_Write::NVMe_Write()
{
	filePath = Device_Address::Xillybus_NvmeWrite_32;
	device = new CommBus::XillyFile(filePath);
};

bool Device::NVMe_Write::getEOF() const
{
	return privateEOF;
}

void Device::NVMe_Write::setEOF(const bool& value)
{
	privateEOF = value;
}

bool Device::NVMe_Write::SendData(unsigned char* cmd, bool needKeepFileOpen)
{
	try
	{
		auto ret = device->open_file(device->e_WriteOnly); //以只写的方式(WriteOnly)打开D.F.
		if (!ret)
		{
			return ret;
		}
		//ret = device->set_offset(address * 4); //设置即将写入的地址在D.F.中的位置（address/offset）
		if (!ret)
		{
			return ret;
		}
		//std::list<BYTE> byteList{4096};
		/*uint8_t* byteList = new uint8_t[4096];
		memset(byteList, 0, 4096);*/
	/*	for (int i = 0; i < 6; i++)
		{
			byteList[i] = cmd[i];
		}
		*/
		ret = device->write_data(cmd, 4096); //传入驱动方法
		if (!needKeepFileOpen)
		{
			device->close_file();
		}
		//delete[] byteList;
		return ret;
	}
	catch (exception& ex)
	{
		//Log->Error(std::wstring::Format(L"SendData To  Address:{0}  Error {1}", address.ToString(L"X2"), ex.what()));
		return false;
	}
	//finally
	//{
	//	_lock->ExitWriteLock();
	//}
}

Device::NVMe_Write* Device::NVMe_Write::instance = NULL;