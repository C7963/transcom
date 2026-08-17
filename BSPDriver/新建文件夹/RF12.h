#pragma once
#include <Windows.h>
#include <iostream>
#include <bitset>
#include <thread>
#include <stdexcept>
#include "Device_MEM32.h"

namespace RF12
{
	class RF_12
	{
	public:
		int ChannelAttValue = 0;
		void rf_12_init();
		void SetChannelSa();
		bool SetRFATT(double att, int ch);
		bool SetCenterFreq(double FrequencyValue);
		void SetRefLevel(int attvalue);
		void SetChannelAmp();
		void SetChannelDirect();

	private:
		Device::Device_MEM32* pcie_mem = Device::Device_MEM32::getInstance();
		uint32_t reflevelvalue;
		uint32_t Channel1BaseAddress = 0x000A0000;
		uint32_t Channel2BaseAddress = 0x000A1000;
		uint32_t Channel3BaseAddress = 0x000A2000;
		uint32_t Channel4BaseAddress = 0x000A3000;
	};
}