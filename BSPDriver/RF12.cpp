#include "RF12.h"
#include "ADC_Api.h"

using namespace RF12;
using namespace ADC;

void RF_12::rf_12_init()
{
	pcie_mem->SendData(Channel1BaseAddress, 0x1);		//射频0电源开
	pcie_mem->SendData(Channel2BaseAddress, 0x1);		//射频1电源开
	pcie_mem->SendData(Channel3BaseAddress, 0x1);		//射频2电源开
	pcie_mem->SendData(Channel4BaseAddress, 0x1);		//射频3电源开
	Sleep(1);
	SetChannelSa();
	SetRFATT(0, 0);    //43610初始化设置衰减为30dB
	pcie_mem->SendData(Channel1BaseAddress + 0x7, 0x3F);  //新衰减为0dB HMC472
	pcie_mem->SendData(Channel2BaseAddress + 0x7, 0x3F);  //新衰减为0dB HMC472
	pcie_mem->SendData(Channel3BaseAddress + 0x7, 0x3F);  //新衰减为0dB HMC472
	pcie_mem->SendData(Channel4BaseAddress + 0x7, 0x3F);  //新衰减为0dB HMC472
}


bool RF_12::SetRFATT(double att, int ch)
{
	uint32_t cmd = uint32_t(att * 2);
	if (ch == 0)
	{
		pcie_mem->SendData(0x000A0004, cmd);
		Sleep(1);
		pcie_mem->SendData(0x000A1004, cmd);
		Sleep(1);
		pcie_mem->SendData(0x000A2004, cmd);
		Sleep(1);
		pcie_mem->SendData(0x000A3004, cmd);
		Sleep(1);
		return true;
	}
	else
	{
		return false;
	}
}

bool RF_12::SetCenterFreq(double FrequencyValue)
{
	try
	{
		double Lofreq;
		if (FrequencyValue <= 1.5E9)
		{
			Lofreq = FrequencyValue + 307.2E6;
			ADCApi::adc_adc_set_ddc_nco(0, ADCApi::h->adc_clk_freq_hz * 0.75);
			ADCApi::adc_adc_set_ddc_nco(1, ADCApi::h->adc_clk_freq_hz * 0.75);
		}
		else
		{
			Lofreq = FrequencyValue - 307.2E6;
			ADCApi::adc_adc_set_ddc_nco(0, ADCApi::h->adc_clk_freq_hz / 4);
			ADCApi::adc_adc_set_ddc_nco(1, ADCApi::h->adc_clk_freq_hz / 4);
		}
		if (FrequencyValue > 500E6)
		{
			ADCApi::adc_filter_init(FilterCoe, 0x2);
			uint64_t freqvalue = (uint64_t)(Lofreq * 10);
			freqvalue <<= 16;  // 左移16位
			uint32_t low32 = (uint32_t)(freqvalue & 0xFFFFFFFF);  // 低32位
			uint32_t high32 = (uint32_t)((freqvalue >> 32) & 0xFFFFFFFF);  // 高32位

			pcie_mem->SendData(0x000A0201, low32);
			pcie_mem->SendData(0x000A0202, high32);
			pcie_mem->SendData(0x000A0200, 0xa0000002);
			pcie_mem->SendData(0x000A0200, 0xe0000002); Sleep(10);
			pcie_mem->SendData(0x000A0200, 0x0);
		
		}
		else
		{
			SetChannelDirect();  //切换到直通通道
			ADCApi::adc_filter_init(FilterLowCoe, 0x0);
			ADCApi::adc_adc_set_ddc_nco(0, FrequencyValue);
			ADCApi::adc_adc_set_ddc_nco(1, FrequencyValue);
			
			//ADCApi::filter_ctrl(0);
			//ADCApi::adc_register_chip_transfer();
			ChannelAttValue = 0;
		}

		return true;
	}
	catch (const std::exception& ex) {
		std::cerr << "频率设置错误: " << ex.what() << std::endl;
		return false;
	}
}


void RF_12::SetRefLevel(int reflevel)
{
	reflevelvalue = reflevel;
	if (reflevel >= 0 && reflevel <= 30)
	{
		//切衰减通道
		SetChannelSa();
		SetRFATT(reflevel, 0);
		ChannelAttValue = reflevel;
	}

	if (reflevel < 0 && reflevel >= -20)
	{
		//切衰减通道
		SetChannelSa();
		SetRFATT(0, 0);
		ChannelAttValue = 0;
	}

	if (reflevel < -20 && reflevel >= -170)
	{
		//切放大通道
		SetChannelAmp();
		SetRFATT(0, 0);
		ChannelAttValue = -24;
	}
}


void RF_12::SetChannelSa()
{
	pcie_mem->SendData(Channel1BaseAddress + 1, 0x1);		//rf v3.0
	pcie_mem->SendData(Channel1BaseAddress + 2, 0x1);
	pcie_mem->SendData(Channel1BaseAddress + 3, 0x3);

	pcie_mem->SendData(Channel2BaseAddress + 1, 0x1);		//rf v3.0
	pcie_mem->SendData(Channel2BaseAddress + 2, 0x1);
	pcie_mem->SendData(Channel2BaseAddress + 3, 0x3);

	pcie_mem->SendData(Channel3BaseAddress + 1, 0x1);		//rf v3.0
	pcie_mem->SendData(Channel3BaseAddress + 2, 0x1);
	pcie_mem->SendData(Channel3BaseAddress + 3, 0x3);

	pcie_mem->SendData(Channel4BaseAddress + 1, 0x1);		//rf v3.0
	pcie_mem->SendData(Channel4BaseAddress + 2, 0x1);
	pcie_mem->SendData(Channel4BaseAddress + 3, 0x3);
	Sleep(1);
}

void RF_12::SetChannelAmp()
{
	pcie_mem->SendData(Channel1BaseAddress + 1, 0x3);//rf v3.0
	pcie_mem->SendData(Channel1BaseAddress + 2, 0x0);
	pcie_mem->SendData(Channel1BaseAddress + 3, 0x3);

	pcie_mem->SendData(Channel2BaseAddress + 1, 0x3);//rf v3.0
	pcie_mem->SendData(Channel2BaseAddress + 2, 0x0);
	pcie_mem->SendData(Channel2BaseAddress + 3, 0x3);

	pcie_mem->SendData(Channel3BaseAddress + 1, 0x3);//rf v3.0
	pcie_mem->SendData(Channel3BaseAddress + 2, 0x0);
	pcie_mem->SendData(Channel3BaseAddress + 3, 0x3);

	pcie_mem->SendData(Channel4BaseAddress + 1, 0x3);//rf v3.0
	pcie_mem->SendData(Channel4BaseAddress + 2, 0x0);
	pcie_mem->SendData(Channel4BaseAddress + 3, 0x3);
	Sleep(1);
}

void RF_12::SetChannelDirect()
{
	pcie_mem->SendData(Channel1BaseAddress + 1, 0x2);//rf v3.0
	pcie_mem->SendData(Channel1BaseAddress + 2, 0x1);
	pcie_mem->SendData(Channel1BaseAddress + 3, 0x0);

	pcie_mem->SendData(Channel2BaseAddress + 1, 0x2);//rf v3.0
	pcie_mem->SendData(Channel2BaseAddress + 2, 0x1);
	pcie_mem->SendData(Channel2BaseAddress + 3, 0x0);

	pcie_mem->SendData(Channel3BaseAddress + 1, 0x2);//rf v3.0
	pcie_mem->SendData(Channel3BaseAddress + 2, 0x1);
	pcie_mem->SendData(Channel3BaseAddress + 3, 0x0);

	pcie_mem->SendData(Channel4BaseAddress + 1, 0x2);//rf v3.0
	pcie_mem->SendData(Channel4BaseAddress + 2, 0x1);
	pcie_mem->SendData(Channel4BaseAddress + 3, 0x0);
	Sleep(1);
}