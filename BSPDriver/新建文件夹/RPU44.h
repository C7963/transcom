#pragma once
#include "Device_MEM32.h"
#include <stdexcept>
#include <cstdint>
#include <map>
#include "Device_Address.h"
enum ENUM_CMD_ID
{
	CONFIG = 0x00, //length = 24
	SWEEP = 0x01, //length = 40
	TUNING = 0x02,
	DIRECT = 0x03,
	LOCKDET = 0x04
};

namespace RPU_44
{
	class RPU_44_LL
	{
	public:
		//user public variable
		uint32_t rf_ATT;
		uint32_t if_AMP1;
		uint32_t if_AMP2;
		uint32_t if_FILT;
		uint32_t rf_channel;
		uint32_t rf_LNA;
		uint64_t center_freq;
		uint32_t three_length = 0;
		uint32_t power_state;
		//hardware access variable
	private:
		Device::Device_MEM32* pcie_mem = Device::Device_MEM32::getInstance();
		uint32_t spi_rw = 0; //0:Write 1:Read(暂不支持)
		ENUM_CMD_ID spi_cmd_id; //command id
		const uint32_t spi_tx_length_config = 24;
		const uint32_t spi_tx_length_sweep = 48;
		uint32_t spi_clk_div = 0;
		uint32_t spi_enable = 1;//1:发起SPI时序 0：无动作
		//bus address access
		const uint32_t Base_Address = 0x00010000;
		const uint32_t DOMAIN_BASE = 0x0000;
		const uint32_t DEV_BASE = 0x0;

		//fpga register definition
		const uint32_t REG_SPI_CTRL = 0; //32bit register
		const uint32_t REG_SPI_DATA1 = 1;
		const uint32_t REG_SPI_DATA2 = 2;
		const uint32_t REG_SPI_TX_LEN = 3;
		const uint32_t REG_SPI_CS_ADJ = 4;
		const uint32_t REG_SPI_DEV_CONFIG = 5;
		const uint32_t REG_SPI_READBACK1 = 6;
		const uint32_t REG_SPI_READBACK2 = 7;

		//fpga register SPI_CTRL region definition, offset
		const uint32_t SPI_CTRL_SPI_RW = 3; //offset
		const uint32_t SPI_CTRL_SPI_RW_LEN = 1; //length

		const uint32_t SPI_CTRL_SPI_CMD_ID = 0;//offset
		const uint32_t SPI_CTRL_SPI_CMD_ID_LEN = 3;//3bit

		const uint32_t SPI_CTRL_SPI_CLK_DIV = 8;//offset
		const uint32_t SPI_CTRL_SPI_CLK_DIV_LEN = 8;//8bit

		const uint32_t SPI_CTRL_SPI_ENABLE = 31;//offset
		const uint32_t SPI_CTRL_SPI_ENABLE_LEN = 1;//8bit

		//fpga register SPI_DATA0 region definition, offset
		const uint32_t SPI_DATA0 = 0;
		const uint32_t SPI_DATA0_LEN = 32;

		//fpga register SPI_DATA1 region definition, offset
		const uint32_t SPI_DATA1 = 0;
		const uint32_t SPI_DATA1_LEN = 32;

		//fpga register SPI_TX_LEN region definition, offset
		const uint32_t SPI_LENGTH = 0;
		const uint32_t SPI_LENGTH_LEN = 8;
		const uint32_t SPI_LENGTH_cs_deassert_cpol_cpha = 8;
		const uint32_t SPI_LENGTH_cs_deassert_cpol_cpha_LEN = 3;

		//fpga register DEV_CONFIG region definition, offset
		const uint32_t SPI_DEV_CONFIG = 0;
		const uint32_t SPI_DEV_CONFIG_LEN = 2;

		//CONFIG data payload region

		const uint32_t congif_att = 4;//offset
		const uint32_t congif_att_len = 4;//length

		const uint32_t congif_channel = 8;//offset
		const uint32_t congif_channel_len = 4;//length

		const uint32_t congif_LNA = 3;//offset
		const uint32_t congif_LNA_len = 1;//length

		const uint32_t congif_IF_AMP1 = 2;//offset
		const uint32_t congif_IF_AMP1_len = 1;//length

		const uint32_t congif_IF_AMP2 = 1;//offset
		const uint32_t congif_IF_AMP2_len = 1;//length


		const uint32_t congif_IF_FLT = 0;//offset
		const uint32_t congif_IF_FLT_len = 1;//length


		//SWEEP data payload region
		const uint32_t sweep_centerfreq_low = 0; //offset
		const uint32_t sweep_centerfreq_low_LEN = 32; //length

		const uint32_t sweep_centerfreq_high = 32; //offset
		const uint32_t sweep_centerfreq_high_LEN = 8; //length


		uint32_t set_reg_region(uint32_t spi_rw, uint32_t offset, uint32_t length);
		//register 0, ctrl, data format function
		uint32_t format_reg_spi_ctrl(uint32_t rw, uint32_t cmd_id, uint32_t spi_clk_div, uint32_t enable);
		uint32_t format_reg_spi_config_data();
		uint32_t format_reg_spi_sweep_data();
		uint32_t format_reg_spi_sweep_data1();
		//register 3, spi length, data format function
		uint32_t format_reg_spi_length(uint32_t length, uint32_t three_len);

	public:
		void RPU44_Config();
		void RPU44_Sweep();
		void RPU44_PowerOnOff();
	}
	;

	class RPU_44_HL
	{
	public:

		void SetCenterFreq(uint64_t centerfreq);
		uint32_t SetRefLevel(int reflevel);
		void PowerOnOff(uint32_t flag);
	private:
		RPU_44::RPU_44_LL RPU44;
		std::map<uint32_t, uint32_t> scores =
		{
			{ 20,30 },
			{ 19,30 },
			{ 18,28 },
			{ 17,28 },
			{ 16,26 },
			{ 15,26 },
			{ 14,24 },
			{ 13,24 },
			{ 12,22 },
			{ 11,22 },
			{ 10,20 },
			{ 9,20 },
			{ 8,18 },
			{ 7,18 },
			{ 6,16 },
			{ 5,16 },
			{ 4,14 },
			{ 3,14 },
			{ 2,12 },
			{ 1,12 },
			{ 0,10 },
			{ -1,10 },
			{ -2,8 },
			{ -3,8 },
			{ -4,6 },
			{ -5,6 },
			{ -6,4 },
			{ -7,4 },
			{ -8,2 },
			{ -9,2 },
			{ -10,0 } };
	};
}