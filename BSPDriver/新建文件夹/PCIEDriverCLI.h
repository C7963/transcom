#pragma once
#include <cstdint>
#include "Device_Address.h"
#include "Device_MEM32.h"


using namespace System;

namespace PCIEDriverCLI
{
	public ref class PCIEDriver
	{
	public:
		PCIEDriver();
		bool SendData(uint32_t address, uint32_t cmd);
		bool SendData(uint32_t address, cli::array<float>^ cmd, uint32_t len);
		bool SendData(uint32_t address, float cmd);
		bool SendData(uint32_t address, cli::array<uint32_t>^ cmd, uint32_t len);
		bool SendData(uint32_t address, int cmd);
		void ReadDataHDD(uint32_t len, cli::array<uint8_t>^ dataBufOut);
		void ReadBackData(uint32_t address, uint32_t size, cli::array<uint8_t>^ dataBufOut);
	private:
		Device::Device_MEM32* device;
	};
}

