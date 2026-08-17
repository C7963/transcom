//register definition
//descript function register region 

//function definition
//how to operate device
#include "Device_MEM32.h"
#include <stdexcept>
#pragma once

#include <cstdint>
#include "Device_Address.h"

namespace HLJS386
{
	class HLJS386_LL
	{
	public:
		//user public variable
		uint64_t center_freq;
		uint32_t rf_att;
		uint32_t if_att;
		uint32_t rf_mode;

		unsigned char element_Type;
		unsigned char slot_Address;
		unsigned char channel_Id;
		unsigned char instruction_Code;
		unsigned char Length;
		unsigned char Checkcode;
		HLJS386_LL();
		bool SetCenterFreq();
		void SetRFATT();
		void SetIFATT();
		void SetRFMode();
	
		void Set_Switch_On();
		void Set_Switch_Off();
		uint32_t Get_Temperature();
		uint32_t Get_Status();

	private:
		Device::Device_MEM32* pcie_mem;
		//Ö¡Í·Ö¡Î²		
		const unsigned char frame_Header = 0x55;
		const unsigned char frame_Tail = 0xee;


		//»ùµØÖ·
		const uint32_t Base_Address = 0x00010000;

		//¼Ä´æÆ÷µØÖ·
		const uint32_t REG_SPI_CTRL1 = 0;
		const uint32_t REG_SPI_CTRL2 = 1;
		const uint32_t REG_SPI_CTRL3 = 2;
		const uint32_t REG_SPI_PAYLOAD1 = 3;
		const uint32_t REG_SPI_PAYLOAD2 = 4;
		const uint32_t REG_SPI_FORMAT1 = 5;
		const uint32_t REG_SPI_FORMAT2 = 6;
		const uint32_t REG_SPI_READBACK1 = 7;
		const uint32_t REG_SPI_READBACK2 = 8;
		const uint32_t REG_SPI_READBACK3 = 9;
		const uint32_t REG_SPI_READBACK4 = 10;


		uint32_t spi_enable = 1;//1:·¢ÆðSPIÊ±Ðò 0£ºÎÞ¶¯×÷

		//ÆµÂÊÖ¸Áî
		const unsigned char frame_header = 0;//offset
		const unsigned char frame_header_len = 1;//length

		const unsigned char element_type = 1;//offset
		const unsigned char element_type_len = 1;//length

		const unsigned char slot_address = 2;//offset
		const unsigned char slot_address_len = 1;//length

		const unsigned char channel_id = 3;//offset
		const unsigned char channel_id_len = 1;//length

		const unsigned char instruction_code = 4;//offset
		const unsigned char instruction_code_len = 1;//length

		const unsigned char length = 5;//offset
		const unsigned char length_len = 1;//length

		const unsigned char freq_instruction_data = 6;//offset
		const unsigned char freq_instruction_data_len = 8;//length

		const unsigned char freq_check = 14;//offset
		const unsigned char freq_check_len = 1;//length

		const unsigned char freq_frame_tail = 15;//offset
		const unsigned char freq_frame_tail_len = 1;//length


		// ÉäÆµË¥¼õÖ¸Áî
		const unsigned char rfatt_instruction_data = 6;//offset
		const unsigned char rfatt_instruction_data_len = 1;//length

		const unsigned char rfatt_check = 7;//offset
		const unsigned char rfatt_check_len = 1;//length

		const unsigned char rfatt_frame_tail = 8;//offset
		const unsigned char rfatt_frame_tail_len = 1;//length


		//ÖÐÆµË¥¼õÖ¸Áî
		const unsigned char ifatt_instruction_data = 6;//offset
		const unsigned char ifatt_instruction_data_len = 1;//length

		const unsigned char ifatt_check = 7;//offset
		const unsigned char ifatt_check_len = 1;//length

		const unsigned char ifatt_frame_tail = 8;//offset
		const unsigned char ifatt_frame_tail_len = 1;//length


		//¹¤×÷Ä£Ê½Ö¸Áî
		const unsigned char mode_instruction_data = 6;//offset
		const unsigned char mode_instruction_data_len = 1;//length

		const unsigned char mode_check = 7;//offset
		const unsigned char mode_check_len = 1;//length

		const unsigned char mode_frame_tail = 8;//offset
		const unsigned char mode_frame_tail_len = 1;//length


		//¼Ä´æÆ÷SPI_CTRL1 region definition, offset
		const uint32_t SPI_CTRL_SPI_ENABLE = 31;//offset
		const uint32_t SPI_CTRL_SPI_ENABLE_LEN = 1;//length

		const uint32_t SPI_CTRL_SPI_CMD_ID = 0;//offset
		const uint32_t SPI_CTRL_SPI_CMD_ID_LEN = 8;//length



		//¼Ä´æÆ÷SPI_CTRL2 region definition, offset
		const uint32_t SPI_CTRL_SPI_MODULE_TYPE = 16;//offset
		const uint32_t SPI_CTRL_SPI_MODULE_TYPE_LEN = 8;//length

		const uint32_t SPI_CTRL_SPI_SLOT_ID = 8;//offset
		const uint32_t SPI_CTRL_SPI_SLOT_ID_LEN = 8;//length

		const uint32_t SPI_CTRL_SPI_CHANNEL_ID = 0;//offset
		const uint32_t SPI_CTRL_SPI_CHANNEL_ID_LEN = 8;//length


		//¼Ä´æÆ÷SPI_PAYLOAD1 region definition, offset
		const uint32_t SPI_LOW_DATA = 0;
		const uint32_t SPI_LOW_DATA_LEN = 32;

		//¼Ä´æÆ÷SPI_PAYLOAD2 region definition, offset
		const uint32_t SPI_HIGH_DATA = 0;
		const uint32_t SPI_HIGH_DATA_LEN = 32;

		//¼Ä´æÆ÷SPI_READBACK1 region definition, offset
		const uint32_t SPI_BACKDATA1 = 0;
		const uint32_t SPI_BACKDATA1_LEN = 32;

		//¼Ä´æÆ÷SPI_READBACK2 region definition, offset
		const uint32_t SPI_BACKDATA2 = 0;
		const uint32_t SPI_BACKDATA2_LEN = 32;

		//¼Ä´æÆ÷SPI_READBACK3 region definition, offset
		const uint32_t SPI_BACKDATA3 = 0;
		const uint32_t SPI_BACKDATA3_LEN = 32;

		//¼Ä´æÆ÷SPI_READBACK4 region definition, offset
		const uint32_t SPI_BACKDATA4 = 0;
		const uint32_t SPI_BACKDATA4_LEN = 32;


		uint64_t set_reg_region(uint64_t spi_rw, unsigned char offset, unsigned char length);
		//¼ÆËãÐ£ÑéÖµ
		uint8_t calculateXOR(uint8_t type, uint8_t address, uint8_t channel, uint8_t code, uint8_t len, uint64_t data);
		//ÖÐÐÄÆµÂÊ
		uint64_t format_reg_set_center_freq_data();
		//ÉäÆµË¥¼õ
		uint32_t format_reg_set_rf_att_data();
		//ÖÐÆµË¥¼õ
		uint32_t format_reg_set_if_att_data();
		//¹¤×÷Ä£Ê½
		uint32_t format_reg_set_rf_mode_data();
		//¿ØÖÆ¼Ä´æÆ÷0 SPI_CTRL1
		uint32_t format_reg_spi_ctrl1(uint32_t cmd_id, uint32_t enable);
	};

}