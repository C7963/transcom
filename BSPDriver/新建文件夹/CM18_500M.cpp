#include "CM18_500M.h"

using namespace CM18_500M;

uint32_t CM18_500M_LL::set_reg_region(uint32_t spi_rw, uint32_t offset, uint32_t length)
{
	uint32_t temp;
	uint32_t all_ones = 0xffffffff;
	all_ones >>= 32 - length;
	temp = spi_rw & all_ones;
	temp <<= offset;
	return temp;
}

//中心频率
uint32_t CM18_500M_LL::format_reg_set_center_freq_data()
{
	uint32_t low = center_freq_code;
	uint32_t Center_Freq_Code = set_reg_region(low, center_freq_code, center_freq_code_len);
	uint32_t Center_Freq_Data = set_reg_region(center_freq, center_freq_data, center_freq_data_len);
	return Center_Freq_Code | Center_Freq_Data;
}

//射频衰减
uint32_t CM18_500M_LL::format_reg_set_rf_att_data()
{
	uint32_t low = rf_att_code;
	uint32_t RF_ATT_Code = set_reg_region(low, rf_att_code, rf_att_code_len);
	uint32_t RF_ATT_Data = set_reg_region(rf_att, rf_att_data, rf_att_data_len);
	return RF_ATT_Code | RF_ATT_Data;
}

//中频衰减
uint32_t CM18_500M_LL::format_reg_set_if_att_data()
{
	uint32_t low = if_att_code;
	uint32_t IF_ATT_Code = set_reg_region(low, if_att_code, if_att_code_len);
	uint32_t IF_ATT_Data = set_reg_region(if_att, if_att_data, if_att_data_len);
	return IF_ATT_Code | IF_ATT_Data;
}

//工作模式
uint32_t CM18_500M_LL::format_reg_set_rf_mode_data()
{
	uint32_t low = rf_mode_code;
	uint32_t RF_Mode_Code = set_reg_region(low, rf_mode_code, rf_mode_code_len);
	uint32_t RF_Mode_Data = set_reg_region(rf_mode, rf_mode_data, rf_mode_data_len);
	return RF_Mode_Code | RF_Mode_Data;
}

//控制寄存器0 SPI_CTRL
uint32_t CM18_500M_LL::format_reg_spi_ctrl(uint32_t cmd_id, uint32_t enable)
{
	uint32_t spi_cmd_id = set_reg_region(cmd_id, SPI_CTRL_SPI_CMD_ID, SPI_CTRL_SPI_CMD_ID_LEN);
	uint32_t spi_en = set_reg_region(enable, SPI_CTRL_SPI_ENABLE, SPI_CTRL_SPI_ENABLE_LEN);
	return spi_cmd_id | spi_en;
}

void CM18_500M_LL::CM18_SetCenterFreq()
{
	uint32_t Ctrlvalue = format_reg_spi_ctrl(0, spi_enable);
	uint32_t CenterFreq = center_freq;

	auto pcie_mem = Device::Device_MEM32::getInstance();
	pcie_mem->SendData(Base_Address + REG_SPI_PAYLOAD, (unsigned int)CenterFreq);
	pcie_mem->SendData(Base_Address + REG_SPI_CTRL, (unsigned int)Ctrlvalue);
}

void CM18_500M_LL::CM18_SetRFATT()
{
	uint32_t Ctrlvalue = format_reg_spi_ctrl(1, spi_enable);
	uint32_t RFATT = rf_att * 2;
	auto pcie_mem = Device::Device_MEM32::getInstance();
	pcie_mem->SendData(Base_Address + REG_SPI_PAYLOAD, (unsigned int)RFATT);
	pcie_mem->SendData(Base_Address + REG_SPI_CTRL, (unsigned int)Ctrlvalue);
}
void CM18_500M_LL::CM18_SetIFATT()
{
	uint32_t Ctrlvalue = format_reg_spi_ctrl(2, spi_enable);
	uint32_t IFATT = if_att * 2;
	auto pcie_mem = Device::Device_MEM32::getInstance();
	pcie_mem->SendData(Base_Address + REG_SPI_PAYLOAD, (unsigned int)IFATT);
	pcie_mem->SendData(Base_Address + REG_SPI_CTRL, (unsigned int)Ctrlvalue);
}
void CM18_500M_LL::CM18_SetRFMode()
{
	uint32_t Ctrlvalue = format_reg_spi_ctrl(3, spi_enable);
	uint32_t RFMode = rf_mode;
	auto pcie_mem = Device::Device_MEM32::getInstance();
	pcie_mem->SendData(Base_Address + REG_SPI_PAYLOAD, (unsigned int)RFMode);
	pcie_mem->SendData(Base_Address + REG_SPI_CTRL, (unsigned int)Ctrlvalue);
}

void CM18_500M_LL::CM18_SetOutBW()
{
	uint32_t Ctrlvalue = format_reg_spi_ctrl(5, spi_enable);
	uint32_t OutBW = out_bw;
	auto pcie_mem = Device::Device_MEM32::getInstance();
	pcie_mem->SendData(Base_Address + REG_SPI_PAYLOAD, (unsigned int)OutBW);
	pcie_mem->SendData(Base_Address + REG_SPI_CTRL, (unsigned int)Ctrlvalue);
}

void CM18_500M_LL::CM18_ReadBack()
{
	//
}

void CM18_500M_HL::SetCenterFreq(uint64_t centerfreq)
{
	if (centerfreq >= 20E6 && centerfreq <= 18000E6)
	{
		CM18.center_freq = centerfreq / 1000;
	}
	else
	{
		cout << "数值有误" << endl;
	}
	CM18.CM18_SetCenterFreq();
}


uint32_t CM18_500M_HL::SetRefLevel(int reflevel)
{
	if (reflevel >= -10 && reflevel <= 20)
	{
		CM18.if_att = 10;
		CM18.rf_att = abs(reflevel + 10);
		CM18.rf_mode = 0x01; //常规模式
	}
	else if (reflevel < -10 && reflevel >= -50)
	{
		CM18.if_att = 10;
		CM18.rf_att = 0;
		CM18.rf_mode = 0x01; //常规模式
	}
	else if (reflevel < -50)
	{
		CM18.if_att = 0;
		CM18.rf_att = 0;
		CM18.rf_mode = 0x02; //低失真模式
	}
	CM18.CM18_SetRFMode();
	Sleep(10);
	CM18.CM18_SetRFATT();
	Sleep(10);
	CM18.CM18_SetIFATT();
	Sleep(10);

	return CM18.rf_att;
}

void CM18_500M_HL::SetRFATT(uint32_t rfatt)
{
	if (rfatt >= 0 && rfatt <= 30)
	{
		CM18.rf_att = rfatt;
	}
	else
	{
		cout << "数值有误" << endl;
	}
	CM18.CM18_SetRFATT();
}


void CM18_500M_HL::SetIFATT(uint32_t ifatt)
{
	if (ifatt >= 0 && ifatt <= 30)
	{
		CM18.if_att = ifatt;
	}
	else
	{
		cout << "数值有误" << endl;
	}
	CM18.CM18_SetIFATT();
}


void CM18_500M_HL::SetRFMode(uint32_t rfmode)
{
	CM18.rf_mode = rfmode;
	CM18.CM18_SetRFMode();
}

void CM18_500M_HL::SetOutBW(uint32_t outbw)
{
	CM18.out_bw = outbw;
	CM18.CM18_SetOutBW();
}

