#pragma once
//register definition
//descript function register region 

//function definition
//how to operate device
//#include "Global.h"
#pragma once
#include <stdexcept>
#include <cstdint>
#include <iostream>
#include <Windows.h>
#include "Device_Address.h"
#include "Device_MEM32.h"

namespace CM18_500M
{
	class CM18_500M_LL
	{
	public:
		//user public variable
		uint32_t center_freq;
		uint32_t rf_att;
		uint32_t if_att;
		uint32_t rf_mode;
		uint32_t center_freq_code = 00000;
		uint32_t rf_att_code = 00001;
		uint32_t if_att_code = 00010;
		uint32_t rf_mode_code = 00011;
		uint32_t if_bw_code = 00100;
		uint32_t out_bw;

		//hardware access variable

	private:
		//基地址
		const uint32_t Base_Address = 0x00010000;

		//寄存器地址
		const uint32_t REG_SPI_CTRL = 0; //32bit register
		const uint32_t REG_SPI_PAYLOAD = 1;
		const uint32_t REG_SPI_FORMAT1 = 2;
		const uint32_t REG_SPI_FORMAT2 = 3;
		const uint32_t REG_SPI_READBACK = 4;


		uint32_t spi_enable = 1;//1:发起SPI时序 0：无动作

		//tuning frequency control 调谐频率控制

		const uint32_t center_freq_data = 5;//offset
		const uint32_t center_freq_data_len = 25;//length

		const uint32_t center_freq_code1 = 0;//offset
		const uint32_t center_freq_code_len = 5;//length

		//rf_att frequency control 射频衰减频率控制

		const uint32_t rf_att_data = 5;//offset
		const uint32_t rf_att_data_len = 6;//length

		const uint32_t rf_att_code1 = 0;//offset
		const uint32_t rf_att_code_len = 5;//length

		//if_att frequency control 中频衰减频率控制

		const uint32_t if_att_data = 5;//offset
		const uint32_t if_att_data_len = 6;//length

		const uint32_t if_att_code1 = 0;//offset
		const uint32_t if_att_code_len = 5;//length

		//work mode control 工作模式控制

		const uint32_t rf_mode_data = 5;//offset
		const uint32_t rf_mode_data_len = 3;//length

		const uint32_t rf_mode_code1 = 0;//offset
		const uint32_t rf_mode_code_len = 5;//length

		//work if_bw 控制
		const uint32_t if_bw_data = 5;
		const uint32_t if_bw_data_len = 3;//length

		//寄存器SPI_CTRL region definition, offset
		const uint32_t SPI_CTRL_SPI_ENABLE = 31;//offset
		const uint32_t SPI_CTRL_SPI_ENABLE_LEN = 1;//length


		const uint32_t SPI_CTRL_SPI_CMD_ID = 0;//offset
		const uint32_t SPI_CTRL_SPI_CMD_ID_LEN = 4;//length


		//
		uint32_t set_reg_region(uint32_t spi_rw, uint32_t offset, uint32_t length);
		//中心频率
		uint32_t format_reg_set_center_freq_data();
		//射频衰减
		uint32_t format_reg_set_rf_att_data();
		//中频衰减
		uint32_t format_reg_set_if_att_data();
		//工作模式
		uint32_t format_reg_set_rf_mode_data();
		//控制寄存器0 SPI_CTRL
		uint32_t format_reg_spi_ctrl(uint32_t cmd_id, uint32_t enable);

	public:

		void CM18_SetCenterFreq();
		void CM18_SetRFATT();
		void CM18_SetIFATT();
		void CM18_SetRFMode();
		void CM18_SetOutBW();
		void CM18_ReadBack();
	}
	;

	class  CM18_500M_HL
	{
	public:

		void SetCenterFreq(uint64_t centerfreq);
		uint32_t SetRefLevel(int reflevel);
		void SetRFATT(uint32_t rfatt);
		void SetIFATT(uint32_t ifatt);
		void SetRFMode(uint32_t rfmode);
		void SetOutBW(uint32_t outbw);

	private:
		CM18_500M::CM18_500M_LL CM18;
	};

}
