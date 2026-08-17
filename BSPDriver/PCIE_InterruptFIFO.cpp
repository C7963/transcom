#include "PCIE_InterruptFIFO.h"
#include <exception>
using namespace CommBus;

namespace Device {
	// 单例实现 
	PCIE_InterruptFIFO& PCIE_InterruptFIFO::Instance()
	{
		static PCIE_InterruptFIFO instance;
		return instance;
	}

	// 构造函数
	PCIE_InterruptFIFO::PCIE_InterruptFIFO()
	{ 
		// 初始化设备
		if (!device) { 
			device.reset(new CommBus::XillyFile(filePath));
		}
	}

	bool PCIE_InterruptFIFO::DeviceFileStatus()
	{
		return false;
	}

	bool PCIE_InterruptFIFO::SendData(uint32_t address, uint32_t data, bool needKeepFileOpen)
	{
		return false;
	}

	bool PCIE_InterruptFIFO::ReadData(uint32_t address, uint32_t len)
	{
		try
		{
			// 1. 清空现有数据
			ListInterrupt.clear();

			// 2. 检查设备状态
			if (!device->is_opened())
			{
				device->open_file(XillyFile::DeviceFileOpenType::e_ReadOnly);
			}

			if (len == 0) return true;

			// 3. 性能优化写法：
			// C# 原版：申请 byte[] -> read -> 循环 add 到 List (两次内存分配，一次循环拷贝)
			// C++ 优化：直接 resize vector，将数据直接读入 vector 的内存 (一次分配，零拷贝)
			ListInterrupt.resize(len);

			// 调用 read_data，直接传入 vector 内部数据的指针
			// vector.data() 返回指向内部数组的指针 (C++11)
			auto ret = device->read_data(ListInterrupt.data(), len);
			if (ret > 0)
				return true;
			else
				return false;
		}
		catch (const std::exception& ex)
		{
			Log::Error("中断异常", ex.what());
			return false;
		}
		catch (...)
		{
			Log::Error("中断异常", "Unknown error");
			return false;
		}
	}

	void PCIE_InterruptFIFO::Close()
	{
		if (device) {
			device->close_file();
		}
	}

	bool PCIE_InterruptFIFO::CanOpenFile()
	{
		if (device) {
			return device->open_file(XillyFile::DeviceFileOpenType::e_ReadOnly);
		}
		return false;
	}
}