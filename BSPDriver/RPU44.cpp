#include "RPU44.h"

using namespace RPU_44;


uint32_t RPU_44_LL::set_reg_region(uint32_t spi_rw, uint32_t offset, uint32_t length)
{
	uint32_t temp;
	uint32_t all_ones = 0xffffffff;
	all_ones >>= 32 - length;//?为何需要右移
	temp = spi_rw & all_ones;

	temp <<= offset;
	return temp;
}

//register 0, ctrl, data format function
uint32_t RPU_44_LL::format_reg_spi_ctrl(uint32_t rw, uint32_t cmd_id, uint32_t spi_clk_div, uint32_t enable)
{
	uint32_t spi_rw = set_reg_region(rw, SPI_CTRL_SPI_RW, SPI_CTRL_SPI_RW_LEN);
	uint32_t spi_cmd_id = set_reg_region(cmd_id, SPI_CTRL_SPI_CMD_ID, SPI_CTRL_SPI_CMD_ID_LEN);
	uint32_t spi_clk_div1 = set_reg_region(spi_clk_div, SPI_CTRL_SPI_CLK_DIV, SPI_CTRL_SPI_CLK_DIV_LEN);
	uint32_t spi_en = set_reg_region(enable, SPI_CTRL_SPI_ENABLE, SPI_CTRL_SPI_ENABLE_LEN);
	return spi_rw | spi_cmd_id | spi_clk_div1 | spi_en;

}

uint32_t RPU_44_LL::format_reg_spi_config_data()
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

uint32_t RPU_44_LL::format_reg_spi_sweep_data()
{
	uint32_t low32 = center_freq;
	uint32_t center_freq = set_reg_region(low32, sweep_centerfreq_low, sweep_centerfreq_low_LEN);
	return center_freq;
}

uint32_t RPU_44_LL::format_reg_spi_sweep_data1()
{
	return center_freq >> 32;
	/*uint32_t center_freq = set_reg_region(high8, sweep_centerfreq_high, sweep_centerfreq_high_LEN);
	return center_freq;*/
}

//register 3, spi length, data format function
uint32_t RPU_44_LL::format_reg_spi_length(uint32_t length, uint32_t three_len)
{
	uint32_t spi_length = set_reg_region(length, SPI_LENGTH, SPI_LENGTH_LEN);
	uint32_t spi_length_Three = set_reg_region(three_len, SPI_LENGTH_cs_deassert_cpol_cpha, SPI_LENGTH_cs_deassert_cpol_cpha_LEN);
	return spi_length | spi_length_Three;
}


void RPU_44_LL::RPU44_Config()
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

void RPU_44_LL::RPU44_Sweep()
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

void RPU_44_LL::RPU44_PowerOnOff()
{
	pcie_mem->SendData(Base_Address + REG_SPI_DEV_CONFIG, (unsigned int)power_state);
}


void RPU_44_HL::SetCenterFreq(uint64_t centerfreq)
{
	RPU44.center_freq = centerfreq;
	if (centerfreq >= 5E6 && centerfreq < 50E6)
	{
		RPU44.rf_channel = 0;
	}
	else if (centerfreq >= 50E6 && centerfreq < 3.2E9)
	{
		RPU44.rf_channel = 1;
	}
	else if (centerfreq >= 3.2E9 && centerfreq < 4.96E9)
	{
		RPU44.rf_channel = 2;
	}
	else if (centerfreq >= 4.96E9 && centerfreq < 6.1E9)
	{
		RPU44.rf_channel = 3;
	}
	else if (centerfreq >= 6.1E9 && centerfreq < 7.38E9)
	{
		RPU44.rf_channel = 4;
	}
	else if (centerfreq >= 7.38E9 && centerfreq < 11E9)
	{
		RPU44.rf_channel = 5;
	}
	else if (centerfreq >= 11E9 && centerfreq < 14.5E9)
	{
		RPU44.rf_channel = 6;
	}
	else if (centerfreq >= 14.5E9 && centerfreq < 18.4E9)
	{
		RPU44.rf_channel = 7;
	}
	else if (centerfreq >= 18.4E9 && centerfreq < 22E9)
	{
		RPU44.rf_channel = 8;
	}
	else if (centerfreq >= 22E9 && centerfreq < 26.2E9)
	{
		RPU44.rf_channel = 9;
	}
	else if (centerfreq >= 26.2E9 && centerfreq < 35E9)
	{
		RPU44.rf_channel = 10;
	}
	else if (centerfreq >= 35E9 && centerfreq <= 44E9)
	{
		RPU44.rf_channel = 11;
	}
	RPU44.RPU44_Config();
	RPU44.RPU44_Sweep();
}

uint32_t RPU_44_HL::SetRefLevel(int reflevel)
{
	if (reflevel >= -10 && reflevel <= 20)
	{
		RPU44.rf_ATT = scores[reflevel] / 2;
		RPU44.rf_LNA = 0;
	}
	else if (reflevel < -10 && reflevel >= -30)
	{
		RPU44.rf_ATT = 0;
		RPU44.rf_LNA = 0;
	}
	else if (reflevel < -30)
	{
		RPU44.rf_ATT = 0;
		RPU44.rf_LNA = 1;
	}
	RPU44.RPU44_Config();
	return RPU44.rf_ATT * 2;
}

void RPU_44_HL::PowerOnOff(uint32_t flag)
{
	RPU44.power_state = flag;
	RPU44.RPU44_PowerOnOff();
}
