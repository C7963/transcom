#pragma once
#include <cstdint>
#include "Device_Mem32.h"

namespace PLAYBACK
{
	enum class freq_list_1228d8MHz {
		freq307d2MHz = 307200000,
		freq204d8MHz = 204800000,
		freq153d6MHz = 153600000,
		freq122d88MHz = 122880000,
		freq102d4MHz = 102400000,
		freq81d92MHz = 81920000,
		freq76d8MHz = 76800000,
		freq61d44MHz = 61440000,
		freq51d2MHz = 51200000,
		freq40d96MHz = 40960000,
		freq38d4MHz = 38400000,
		freq30d72MHz = 30720000,
		freq25d6MHz = 25600000,
		freq20d48MHz = 20480000,
		freq15d36MHz = 15360000,
		freq12d8MHz = 12800000
	};

	enum class freq_list_819d2MHz {
		freq204d8MHz = 204800000,
		freq102d4MHz = 102400000,
		freq68d265MHz = 68265000,
		freq51d2MHz = 51200000,
		freq40d96MHz = 40960000,
		freq34d13MHz = 34130000,
		freq27d305MHz = 27305000,
		freq25d6MHz = 25600000,
		freq20d48MHz = 20480000,
		freq17d065MHz = 17065000,
		freq13d65MHz = 13650000,
		freq10d24MHz = 10240000,
		freq8d53MHz = 8530000,
	};

	union uint32_chararray
	{
		unsigned char c[4];
		int i;
	};

	class AUX_CTRL
	{
	private:
		Device::Device_MEM32* pcie_mem = Device::Device_MEM32::getInstance();
		uint32_t CLK_DIV_CDDCREQ = 0x001D1000;
		uint32_t DATA_SHIFT = 0x001D1001;
		uint32_t DATA_BYPASS = 0x001D1002;

	public:
		void set_clk_request(bool flag);

		void interpolation_fir_dout_shift(uint32_t value);

		void interpolation_data_bypass(bool bypass);
	};

	class CLK_DIV
	{
	private:
		Device::Device_MEM32* pcie_mem = Device::Device_MEM32::getInstance();
		uint32_t CLKOUT0_REG1 = 0x001D0304;
		uint32_t CLKOUT0_REG2 = 0x001D0308;
		uint32_t CLKOUT5_REG2 = 0x001D0330;
		uint32_t CLKOUT6_REG2 = 0x001D0338;
		uint32_t DIV_CLK_REG = 0x001D033C;
		uint32_t CLKFBOUT_REG1 = 0x001D0340;
		uint32_t CLKFBOUT_REG2 = 0x001D0344;
		uint32_t LOCK_REG1 = 0x001D0348;
		uint32_t LOCK_REG2 = 0x001D034C;
		uint32_t LOCK_REG3 = 0x001D0350;
		uint32_t FILTER_REG1 = 0x001D0354;
		uint32_t FILTER_REG2 = 0x001D0358;

		uint32_t SRR = 0x001D0000;    //Software Reset Register
		uint32_t SR = 0x001D0004;     //Status Register
		uint32_t CCR = 0x001D035C;    //Clock Configuration Register(CCR)

	public:

		void set_clkout0(freq_list_1228d8MHz freq);
		void set_clkout1(freq_list_819d2MHz freq);
		void reset_clk();
		int get_clk_status();
	};

	class FIR_CONFIG
	{
	private:
		Device::Device_MEM32* pcie_mem = Device::Device_MEM32::getInstance();
		uint32_t TDFR = 0x001D2008;
		uint32_t TDFD = 0x001D2010;
		uint32_t TLR = 0x001D2014;
		uint32_t SRR = 0x001D2028;

	public:
		void fir_config_txdata_fifo_reset();
		void config_din(uint32_t* value, uint32_t length);
		void fir_config_axis_reset();

	};

}