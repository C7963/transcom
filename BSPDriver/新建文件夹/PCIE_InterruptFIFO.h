#pragma once

#include <vector>
#include <string>
#include <memory>    
#include <cstdint>   
#include <iostream>
#include "Device_Address.h"    
#include "XillyFile.h"

namespace Device {  
	class Log {
	public:
		static void Error(const std::string& title, const std::string& msg) {
			std::cerr << "[Error] " << title << ": " << msg << std::endl;
		}
	};
	// =========================================================

	class PCIE_InterruptFIFO
	{
	public:
		// 获取单例实例 (线程安全)
		static PCIE_InterruptFIFO& Instance();

		// 禁止拷贝和赋值
		PCIE_InterruptFIFO(const PCIE_InterruptFIFO&) = delete;
		PCIE_InterruptFIFO& operator=(const PCIE_InterruptFIFO&) = delete;

		// 公共属性
		bool EOF_Flag = false; // C++一般不使用属性语法，直接用成员变量或Get/Set

		// 数据缓冲区
		std::vector<uint8_t> ListInterrupt;

		// 业务方法
		bool DeviceFileStatus();
		bool SendData(uint32_t address, uint32_t data, bool needKeepFileOpen);
		bool ReadData(uint32_t address = 0, uint32_t len = 0);
		void Close();
		bool CanOpenFile();

	private:
		// 私有构造函数
		PCIE_InterruptFIFO(); 
		std::string filePath = Device::Device_Address::InterruptPath;
		std::unique_ptr<CommBus::XillyFile> device; // 使用智能指针管理设备生命周期
	};
}