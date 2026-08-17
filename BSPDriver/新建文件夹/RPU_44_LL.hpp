//register definition
//descript function register region 

//function definition
//how to operate device
//#include "Device_MEM32.h"
#include "Device_MEM32.h"
#include <stdexcept>
#pragma once

#include <cstdint>
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
		//const uint32_t spi_tx_length_max = 40; //rpu-44 max spi length is 40
		//const uint32_t spi_tx_length_default = 16; //rpu-44 config spi length is 40
		//const uint32_t spi_tx_length_min = 8; //rpu-44 max spi length is 8
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


		uint32_t set_reg_region(uint32_t spi_rw, uint32_t offset, uint32_t length)
		{
			uint32_t temp;
			uint32_t all_ones = 0xffffffff;
			all_ones >>= 32 - length;//?为何需要右移
			temp = spi_rw & all_ones;

			temp <<= offset;
			return temp;
		}

		//register 0, ctrl, data format function
		uint32_t format_reg_spi_ctrl(uint32_t rw, uint32_t cmd_id, uint32_t spi_clk_div, uint32_t enable)
		{
			uint32_t spi_rw = set_reg_region(rw, SPI_CTRL_SPI_RW, SPI_CTRL_SPI_RW_LEN);
			uint32_t spi_cmd_id = set_reg_region(cmd_id, SPI_CTRL_SPI_CMD_ID, SPI_CTRL_SPI_CMD_ID_LEN);
			uint32_t spi_clk_div1 = set_reg_region(spi_clk_div, SPI_CTRL_SPI_CLK_DIV, SPI_CTRL_SPI_CLK_DIV_LEN);
			uint32_t spi_en = set_reg_region(enable, SPI_CTRL_SPI_ENABLE, SPI_CTRL_SPI_ENABLE_LEN);
			return spi_rw | spi_cmd_id | spi_clk_div1 | spi_en;

		}

		uint32_t format_reg_spi_config_data()
		{
			uint32_t if_filter = set_reg_region(if_FILT, congif_IF_FLT, congif_IF_FLT_len);
			uint32_t if_amp2 = set_reg_region(if_AMP2, congif_IF_AMP2, congif_IF_AMP2_len);
			uint32_t if_amp1 = set_reg_region(if_AMP1, congif_IF_AMP1, congif_IF_AMP1_len);
			uint32_t if_lna = set_reg_region(rf_LNA, congif_LNA, congif_LNA_len);
			uint32_t rf_att = set_reg_region(rf_ATT, congif_att, congif_att_len);
			uint32_t channel = set_reg_region(rf_channel, congif_channel, congif_channel_len);

			uint32_t payload = if_filter | if_amp2 | if_amp1 | if_lna | rf_att | channel;

			uint32_t spi_data = set_reg_region(payload, SPI_DATA0, SPI_DATA0_LEN);
			return spi_data;
		}

		uint32_t format_reg_spi_sweep_data()
		{
			uint32_t low32 = center_freq;
			uint32_t center_freq = set_reg_region(low32, sweep_centerfreq_low, sweep_centerfreq_low_LEN);
			return center_freq;
		}

		uint32_t format_reg_spi_sweep_data1()
		{
			return center_freq >> 32;
			/*uint32_t center_freq = set_reg_region(high8, sweep_centerfreq_high, sweep_centerfreq_high_LEN);
			return center_freq;*/
		}

		//register 3, spi length, data format function
		uint32_t format_reg_spi_length(uint32_t length, uint32_t three_len)
		{
			uint32_t spi_length = set_reg_region(length, SPI_LENGTH, SPI_LENGTH_LEN);
			uint32_t spi_length_Three = set_reg_region(three_len, SPI_LENGTH_cs_deassert_cpol_cpha, SPI_LENGTH_cs_deassert_cpol_cpha_LEN);
			return spi_length | spi_length_Three;
		}

	public:

		void RPU44_Config() 
		{
			 uint32_t Lenvalue = format_reg_spi_length(spi_tx_length_config, three_length);
			 uint32_t Data1value = format_reg_spi_config_data();
			 uint32_t Data2value = 0;
			 uint32_t Ctrlvalue = format_reg_spi_ctrl(spi_rw, 0, spi_clk_div, spi_enable);
			 pcie_mem->SendData(Base_Address + REG_SPI_TX_LEN, (unsigned int)Lenvalue);
			 pcie_mem->SendData(Base_Address + REG_SPI_DATA1, (unsigned int)Data1value);
			 pcie_mem->SendData(Base_Address + REG_SPI_DATA2, (unsigned int)Data2value);
			 pcie_mem->SendData(Base_Address + REG_SPI_CTRL, (unsigned int)Ctrlvalue);
		}

		void RPU44_Sweep()
		{
			uint32_t Lenvalue = format_reg_spi_length(spi_tx_length_sweep, three_length);
			uint32_t Data1value = format_reg_spi_sweep_data(); //CenterFreq低32位
			uint32_t Data2value = format_reg_spi_sweep_data1();//CenterFreq高32位
			uint32_t Ctrlvalue = format_reg_spi_ctrl(spi_rw, 1, spi_clk_div, spi_enable);
			pcie_mem->SendData(Base_Address + REG_SPI_TX_LEN, (unsigned int)Lenvalue);
			pcie_mem->SendData(Base_Address + REG_SPI_DATA1, (unsigned int)Data1value);
			pcie_mem->SendData(Base_Address + REG_SPI_DATA2, (unsigned int)Data2value);
			pcie_mem->SendData(Base_Address + REG_SPI_CTRL, (unsigned int)Ctrlvalue);
		}

		void RPU44_PowerOnOff()
		{
			pcie_mem->SendData(Base_Address + REG_SPI_DEV_CONFIG, (unsigned int)power_state);
		}
	}
	;
}