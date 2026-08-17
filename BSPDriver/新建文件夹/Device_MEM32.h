#pragma once
#include <string>
#include <vector>
#include "XillyFile.h"
#include <iostream>  
#include <thread>  
#include <mutex> 



namespace Device
{

#define APIOUT __declspec (dllexport)

using namespace std;

/// <summary>
/// Memory32 Device操作类
/// </summary>
class APIOUT Device_MEM32
{
public:
	~Device_MEM32();
	Device_MEM32();
	static Device_MEM32* instance;
	bool privateEOF = false;
	std::mutex mtx;
	string filePath;
	//string filePathHDD;
	//CommBus::XillyFile* deviceHDD;
	CommBus::XillyFile* device;
public:
	static Device_MEM32* getInstance() {
		if (instance == nullptr)
		{
			instance = new Device_MEM32();
			
			//cout << "init mem32"<<endl;
		}
		//cout << "get mem32" <<instance<< endl;
		int t = (int)instance;
		return instance;
	};
	vector<unsigned char> data; //读取的数据缓存
	vector<unsigned char> dataHDD; //读取的数据缓存

	bool getEOF() const;

	void setEOF(const bool& value);

	/// <summary>
	/// 最后一次使用记得关闭文件
	/// </summary>
	/// <param name="address"></param>
	/// <param name="cmd"></param>
	/// <param name="needKeepFileOpen"></param>
	/// <returns></returns>
	bool SendData(unsigned int address, unsigned int cmd);

	bool SendData(unsigned int address, unsigned int cmd,bool needKeepFileOpen);

	/// <summary>
/// 最后一次使用记得关闭文件
/// </summary>
/// <param name="address"></param>
/// <param name="cmd"></param>
/// <param name="needKeepFileOpen"></param>
/// <returns></returns>
	bool SendData(unsigned int address, string cmd);

	/// <summary>
	/// 设置float类型命令参数
	/// </summary>
	/// <param name="address"></param>
	/// <param name="cmd"></param>
	/// <returns></returns>
	bool SendData(unsigned int address, float cmd);

	/// <summary>
	/// 设置int类型命令参数
	/// </summary>
	/// <param name="address"></param>
	/// <param name="cmd"></param>
	/// <param name="needKeepFileOpen"></param>
	/// <returns></returns>
	bool SendData(unsigned int address, unsigned int* cmd, int length);

	bool SendData(unsigned int address, float* cmd, int length);

	bool SendData(unsigned int address, int cmd);

	//bool SendData(unsigned int address, const std::vector<uint32_t>& cmd, int length);
	
	/// <summary>
	/// 读取IQ数据
	/// </summary>
	/// <param name="address"></param>
	/// <param name="len"></param>
	/// <returns></returns>
	bool ReadDataHDD(unsigned char* data, unsigned int size);

	bool ReadBackData(unsigned int addr, unsigned int size, uint8_t* readdata);

	bool DeviceFileStatus();
};
}

