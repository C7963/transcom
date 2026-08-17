#include "pch.h"
#include "Device_MEM32.h"
#include <clocale>
#include "ConvertHelper.h"
#include <iostream>
#include "Device_Address.h"
#include <atomic>
#include <fstream>
#include <sstream>
#include <windows.h>

using namespace std;

namespace {
std::atomic<bool> g_firstMem32WriteStarted{ false };

std::string GetDeviceMem32DllDir()
{
	char path[MAX_PATH] = { 0 };
	HMODULE module = nullptr;
	if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
		GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
		reinterpret_cast<LPCSTR>(&GetDeviceMem32DllDir), &module)) {
		GetModuleFileNameA(module, path, MAX_PATH);
	}
	std::string fullPath(path);
	const auto pos = fullPath.find_last_of("\\/");
	return pos == std::string::npos ? "." : fullPath.substr(0, pos);
}

void LogFirstMem32Write(const std::string& message) noexcept
{
	try {
		std::ofstream log(GetDeviceMem32DllDir() + "\\bsdriver_diag.log", std::ios::app);
		if (log.is_open()) {
			log << "[MEM32_FIRST_WRITE] " << message << std::endl;
		}
	}
	catch (...) {
	}
}
}

Device::Device_MEM32::~Device_MEM32()
{
	delete device;
}

Device::Device_MEM32::Device_MEM32()
{
	filePath = Device_Address::Xillybus_Mem32;
	device = new CommBus::XillyFile(filePath);

};

bool Device::Device_MEM32::getEOF() const
{
	return privateEOF;
}

void Device::Device_MEM32::setEOF(const bool& value)
{
	privateEOF = value;
}

bool Device::Device_MEM32::SendData(unsigned int address, unsigned int cmd)
{
	try
	{
		const bool traceFirstWrite = !g_firstMem32WriteStarted.exchange(true);
		if (traceFirstWrite) {
			std::ostringstream oss;
			oss << "stage=enter path=" << filePath
				<< " address=0x" << std::hex << address
				<< " value=0x" << cmd;
			LogFirstMem32Write(oss.str());
		}
		unique_lock<mutex> lck(mtx);
		if (traceFirstWrite) {
			LogFirstMem32Write("stage=mutex_acquired");
		}
		bool ret;
		if (!device->is_opened())
		{
			if (traceFirstWrite) {
				LogFirstMem32Write("stage=open_file begin mode=WriteOnly");
			}
			ret = device->open_file(device->e_WriteOnly); //以只写的方式(WriteOnly)
			if (traceFirstWrite) {
				std::ostringstream oss;
				oss << "stage=open_file done result=" << (ret ? "OK" : "FAILED")
					<< " errno=" << device->get_errno();
				LogFirstMem32Write(oss.str());
			}
		}
		else
		{
			ret = true;
		}
		if (!ret) {
			return false;
		}
		if (traceFirstWrite) {
			LogFirstMem32Write("stage=set_offset begin");
		}
		ret = device->set_offset(address * 4); //设置即将写入的地址在D.F.中的位置（address/offset）
		if (traceFirstWrite) {
			std::ostringstream oss;
			oss << "stage=set_offset done result=" << (ret ? "OK" : "FAILED")
				<< " errno=" << device->get_errno();
			LogFirstMem32Write(oss.str());
		}
		if (!ret) {
			return false;
		}
		if (traceFirstWrite) {
			LogFirstMem32Write("stage=write_data begin bytes=4");
		}
		ret = device->write_data((uint8_t*)&cmd, 4); //传入驱动方法
		if (traceFirstWrite) {
			std::ostringstream oss;
			oss << "stage=write_data done result=" << (ret ? "OK" : "FAILED")
				<< " errno=" << device->get_errno();
			LogFirstMem32Write(oss.str());
		}
		return ret;

	}
	catch (const std::exception& ex)
	{
		//Log->Error(std::wstring::Format(L"SendData To  Address:{0}  Cmd:{1}Error {2}", address.ToString(L"X2"), cmd.ToString(L"X2"), ex.what()));
		return false;
	}
}

bool Device::Device_MEM32::SendData(unsigned int address, float* cmd, int length)
{
	try
	{
		unique_lock<mutex> lck(mtx);
		bool ret;
		if (!device->is_opened())
		{
			ret = device->open_file(device->e_WriteOnly); //以只写的方式(WriteOnly)
		}
		else
		{
			ret = true;
		}
		if (!ret) return false;
		ret = device->set_offset(address * 4); //设置即将写入的地址在D.F.中的位置（address/offset）
		if (!ret) return false;
		ret = device->write_data((uint8_t*)cmd, length * 4); //传入驱动方法
		return ret;

	}
	catch (exception& ex)
	{
		//Log->Error(std::wstring::Format(L"SendData To  Address:{0}  Error {1}", address.ToString(L"X2"), ex.what()));
		return false;
	}
}

bool Device::Device_MEM32::SendData(unsigned int address, float cmd)
{
	try
	{
		unique_lock<mutex> lck(mtx);
		bool ret;
		if (!device->is_opened())
		{
			ret = device->open_file(device->e_WriteOnly); //以只写的方式(WriteOnly)
		}
		else
		{
			ret = true;
		}
		if (!ret) return false;
		ret = device->set_offset(address * 4); //设置即将写入的地址在D.F.中的位置（address/offset）
		if (!ret) return false;
		ret = device->write_data((uint8_t*)&cmd, 4); //传入驱动方法
		return ret;

	}
	catch (const std::exception& ex)
	{
		//Log->Error(std::wstring::Format(L"SendData To  Address:{0}  Cmd:{1}Error {2}", address.ToString(L"X2"), cmd.ToString(L"X2"), ex.what()));
		return false;
	}
}

bool Device::Device_MEM32::SendData(unsigned int address,  int cmd)
{
	try
	{
		const bool traceFirstWrite = !g_firstMem32WriteStarted.exchange(true);
		if (traceFirstWrite) {
			std::ostringstream oss;
			oss << "stage=enter path=" << filePath
				<< " address=0x" << std::hex << address
				<< " value=0x" << static_cast<unsigned int>(cmd)
				<< " overload=int";
			LogFirstMem32Write(oss.str());
		}
		unique_lock<mutex> lck(mtx);
		if (traceFirstWrite) {
			LogFirstMem32Write("stage=mutex_acquired");
		}
		bool ret;
		if (!device->is_opened())
		{
			if (traceFirstWrite) {
				LogFirstMem32Write("stage=open_file begin mode=WriteOnly");
			}
			ret = device->open_file(device->e_WriteOnly); //以只写的方式(WriteOnly)
			if (traceFirstWrite) {
				std::ostringstream oss;
				oss << "stage=open_file done result=" << (ret ? "OK" : "FAILED")
					<< " errno=" << device->get_errno();
				LogFirstMem32Write(oss.str());
			}
		}
		else
		{
			ret = true;
		}
		if (!ret) {
			return false;
		}
		if (traceFirstWrite) {
			LogFirstMem32Write("stage=set_offset begin");
		}
		ret = device->set_offset(address * 4); //设置即将写入的地址在D.F.中的位置（address/offset）
		if (traceFirstWrite) {
			std::ostringstream oss;
			oss << "stage=set_offset done result=" << (ret ? "OK" : "FAILED")
				<< " errno=" << device->get_errno();
			LogFirstMem32Write(oss.str());
		}
		if (!ret) {
			return false;
		}
		if (traceFirstWrite) {
			LogFirstMem32Write("stage=write_data begin bytes=4");
		}
		ret = device->write_data((uint8_t*)&cmd,  4); //传入驱动方法
		if (traceFirstWrite) {
			std::ostringstream oss;
			oss << "stage=write_data done result=" << (ret ? "OK" : "FAILED")
				<< " errno=" << device->get_errno();
			LogFirstMem32Write(oss.str());
		}
	    return ret;
	}
	catch (exception& ex)
	{
		//Log->Error(std::wstring::Format(L"SendData To  Address:{0}  Error {1}", address.ToString(L"X2"), ex.what()));
		return false;
	}
}

bool Device::Device_MEM32::SendData(unsigned int address, unsigned int* cmd, int length)
{
	try
	{
		unique_lock<mutex> lck(mtx);
		bool ret;
		if (!device->is_opened())
		{
			ret = device->open_file(device->e_WriteOnly); //以只写的方式(WriteOnly)
		}
		else
		{
			ret = true;
		}
		if (!ret) return false;
		ret = device->set_offset(address * 4); //设置即将写入的地址在D.F.中的位置（address/offset）
		if (!ret) return false;
		ret = device->write_data((uint8_t*)cmd, length * 4); //传入驱动方法
		return ret;

	}
	catch (exception& ex)
	{
		//Log->Error(std::wstring::Format(L"SendData To  Address:{0}  Error {1}", address.ToString(L"X2"), ex.what()));
		return false;
	}
}

bool Device::Device_MEM32::SendData(unsigned int address, string cmd)
{
	try
	{
		unique_lock<mutex> lck(mtx);
		bool ret;
		if (!device->is_opened())
		{
			ret = device->open_file(device->e_WriteOnly); //以只写的方式(WriteOnly)
		}
		else
		{
			ret = true;
		}
		if (!ret) return false;
		ret = device->set_offset(address * 4); //设置即将写入的地址在D.F.中的位置（address/offset）
		if (!ret) return false;
		string temp;
		temp.append(cmd.substr(24, 8));
		temp.append(cmd.substr(16, 8));
		temp.append(cmd.substr(8, 8));
		temp.append(cmd.substr(0, 8));

		uint8_t* t = (uint8_t*)temp.data();
		ret = device->write_data(t, 4); //传入驱动方法
		//mtx.unlock();
		return ret;

	}
	catch (const std::exception& ex)
	{
		//Log->Error(std::wstring::Format(L"SendData To  Address:{0}  Cmd:{1}Error {2}", address.ToString(L"X2"), cmd.ToString(L"X2"), ex.what()));
		return false;
	}
}

bool Device::Device_MEM32::ReadDataHDD(unsigned char* data, unsigned int dataLen)
{
	auto ret = device->open_file(device->e_ReadOnly); //以只读的方式(ReadOnly)打开D.F.
	if (ret)
	{
		ret = device->read_data(data, dataLen);
		return data;
	}
	return data;
}

bool Device::Device_MEM32::ReadBackData(unsigned int addr, unsigned int size, uint8_t* readdata)
{
	try
	{
		unique_lock<mutex> lck(mtx);
		bool ret = device->open_file(device->e_WriteOnly);
		if (!ret) return false;
		ret = device->set_offset(0);
		if (!ret) return false;
		unsigned char result[8];
		result[0] = (addr) & 0xFF;
		result[1] = (addr >> 8) & 0xFF;
		result[2] = (addr >> 16) & 0xFF;
		result[3] = (addr >> 24) & 0xFF;
		result[4] = (size) & 0xFF;
		result[5] = (size >> 8) & 0xFF;
		result[6] = (size >> 16) & 0xFF;
		result[7] = (size >> 24) & 0xFF;
		//unsigned int result = (addr << (sizeof(unsigned int) * 4)) | size;
		//std::vector<unsigned int> cmd = { addr, size };
		//auto temp = ConvertHelper::ToUnsignedCharArray(cmd);
		ret = device->write_data((uint8_t*)&result, 8); //传入驱动方法
		device->close_file();
		if (!ret) return false;

		ret = device->open_file(device->e_ReadOnly);
		if (!ret) return false;
		unsigned int byteLen = size * 4;
		ret = device->set_offset(32 * 4);
		if (!ret) {
			device->close_file();
			return false;
		}
		ret = device->read_data(readdata, byteLen);
		device->close_file();
		return ret;
	}
	catch (const std::exception& ex)
	{
		//Log->Error(L"获取设备寄存器数据失败", ex.what());
		return false;
	}
}

bool Device::Device_MEM32::DeviceFileStatus()
{
	if (device == nullptr)
	{
		return false;
	}
	return device->is_opened();
}

Device::Device_MEM32* Device::Device_MEM32::instance = NULL;

//
//bool Device::Device_MEM32::SendData(unsigned int address, const std::vector<uint32_t>& cmd, int length)
//{
//	try
//	{
//		unique_lock<mutex> lck(mtx);
//		bool ret;
//		if (!device->is_opened())
//		{
//			ret = device->open_file(device->e_WriteOnly); //以只写的方式(WriteOnly)
//		}
//		else
//		{
//			ret = true;
//		}
//		ret = device->set_offset(address * 4); //设置即将写入的地址在D.F.中的位置（address/offset）
//		ret = device->write_data((uint8_t*)(cmd.data()), length * 4); //传入驱动方法
//		return ret;
//
//	}
//	catch (exception& ex)
//	{
//		//Log->Error(std::wstring::Format(L"SendData To  Address:{0}  Error {1}", address.ToString(L"X2"), ex.what()));
//		return false;
//	}
//}
