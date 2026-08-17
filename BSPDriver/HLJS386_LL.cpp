#include "HLJS386_LL.h"

using namespace HLJS386;

HLJS386_LL::HLJS386_LL()
{
	pcie_mem = Device::Device_MEM32::getInstance();
}

uint64_t HLJS386_LL::set_reg_region(uint64_t spi_rw, unsigned char offset, unsigned char length)
{
	uint64_t temp;
	uint64_t all_ones = 0xffffffffffffffff;
	all_ones >>= 64 - length;
	temp = spi_rw & all_ones;
	temp <<= offset;
	return temp;
}


//计算校验值
uint8_t HLJS386_LL::calculateXOR(uint8_t type, uint8_t address, uint8_t channel, uint8_t code, uint8_t len, uint64_t data) {
	uint8_t xorValue = 0;
	unsigned char* p = (unsigned char*)&data;
	unsigned char t1 = p[0];
	unsigned char t2 = p[1];
	unsigned char t3 = p[2];
	unsigned char t4 = p[3];
	unsigned char t5 = p[4];
	unsigned char t6 = p[5];
	unsigned char t7 = p[6];
	unsigned char t8 = p[7];
	xorValue = type ^ address ^ channel ^ code ^ len ^ t1 ^ t2 ^ t3 ^ t4 ^ t5 ^ t6 ^ t7 ^ t8;
	return xorValue;
}


//中心频率
uint64_t HLJS386_LL::format_reg_set_center_freq_data()
{
	unsigned char Head = set_reg_region(frame_Header, frame_header, frame_header_len);
	unsigned char Type = set_reg_region(0x03, element_type, element_type_len);
	unsigned char Address = set_reg_region(0x00, slot_address, slot_address_len);
	unsigned char Channel = set_reg_region(0x01, channel_id, channel_id_len);
	unsigned char Instruction = set_reg_region(0x00, instruction_code, instruction_code_len);
	unsigned char Len = set_reg_region(8, length, length_len);
	unsigned char check = calculateXOR(Type, Address, Channel, Instruction, Len, center_freq);
	unsigned char Check = set_reg_region(check, freq_check, freq_check_len);
	unsigned char Tail = set_reg_region(frame_Tail, freq_frame_tail, freq_frame_tail_len);
	return Head | Type | Address | Channel | Instruction | Len | Check | Tail;
}


//射频衰减
uint32_t HLJS386_LL::format_reg_set_rf_att_data()
{
	unsigned char Head = set_reg_region(frame_Header, frame_header, frame_header_len);
	unsigned char Type = set_reg_region(0x03, element_type, element_type_len);
	unsigned char Address = set_reg_region(0x00, slot_address, slot_address_len);
	unsigned char Channel = set_reg_region(0x01, channel_id, channel_id_len);
	unsigned char Instruction = set_reg_region(0x01, instruction_code, instruction_code_len);
	unsigned char Len = set_reg_region(1, length, length_len);
	unsigned char check = calculateXOR(Type, Address, Channel, Instruction, Len, rf_att * 4);
	unsigned char Check = set_reg_region(check, rfatt_check, rfatt_check_len);
	unsigned char Tail = set_reg_region(frame_Tail, rfatt_frame_tail, rfatt_frame_tail_len);
	return Head | Type | Address | Channel | Instruction | Len | Check | Tail;
}


//中频衰减
uint32_t HLJS386_LL::format_reg_set_if_att_data()
{
	unsigned char Head = set_reg_region(frame_Header, frame_header, frame_header_len);
	unsigned char Type = set_reg_region(0x03, element_type, element_type_len);
	unsigned char Address = set_reg_region(0x00, slot_address, slot_address_len);
	unsigned char Channel = set_reg_region(0x01, channel_id, channel_id_len);
	unsigned char Instruction = set_reg_region(0x02, instruction_code, instruction_code_len);
	unsigned char Len = set_reg_region(1, length, length_len);
	unsigned char check = calculateXOR(Type, Address, Channel, Instruction, Len, if_att * 4);
	unsigned char Check = set_reg_region(check, ifatt_check, ifatt_check_len);
	unsigned char Tail = set_reg_region(frame_Tail, ifatt_frame_tail, ifatt_frame_tail_len);
	return Head | Type | Address | Channel | Instruction | Len | Check | Tail;
}


//工作模式
uint32_t HLJS386_LL::format_reg_set_rf_mode_data()
{
	unsigned char Head = set_reg_region(frame_Header, frame_header, frame_header_len);
	unsigned char Type = set_reg_region(0x03, element_type, element_type_len);
	unsigned char Address = set_reg_region(0x00, slot_address, slot_address_len);
	unsigned char Channel = set_reg_region(0x01, channel_id, channel_id_len);
	unsigned char Instruction = set_reg_region(0x03, instruction_code, instruction_code_len);
	unsigned char Len = set_reg_region(1, length, length_len);
	unsigned char check = calculateXOR(Type, Address, Channel, Instruction, Len, rf_mode);
	unsigned char Check = set_reg_region(check, mode_check, mode_check_len);
	unsigned char Tail = set_reg_region(frame_Tail, mode_frame_tail, mode_frame_tail_len);
	return Head | Type | Address | Channel | Instruction | Len | Check | Tail;
}


//控制寄存器0 SPI_CTRL1
uint32_t HLJS386_LL::format_reg_spi_ctrl1(uint32_t cmd_id, uint32_t enable)
{
	uint32_t spi_cmd_id = set_reg_region(cmd_id, SPI_CTRL_SPI_CMD_ID, SPI_CTRL_SPI_CMD_ID_LEN);
	uint32_t spi_en = set_reg_region(enable, SPI_CTRL_SPI_ENABLE, SPI_CTRL_SPI_ENABLE_LEN);
	return spi_cmd_id | spi_en;
}


bool HLJS386_LL::SetCenterFreq()
{
	uint32_t Ctrlvalue = format_reg_spi_ctrl1(0, spi_enable);
	uint32_t* p1 = (uint32_t*)&center_freq;
	uint32_t low = p1[0];
	uint32_t high = p1[1];
	const bool payloadLowOk = pcie_mem->SendData(Base_Address + REG_SPI_PAYLOAD1, (unsigned int)low);
	const bool payloadHighOk = pcie_mem->SendData(Base_Address + REG_SPI_PAYLOAD2, (unsigned int)high);
	const bool ctrl2Ok = pcie_mem->SendData(Base_Address + REG_SPI_CTRL2, (unsigned int)0x00030001);
	const bool commandOk = pcie_mem->SendData(Base_Address, (unsigned int)Ctrlvalue);
	return payloadLowOk && payloadHighOk && ctrl2Ok && commandOk;
}
void HLJS386_LL::SetRFATT()
{
	uint32_t Ctrlvalue = format_reg_spi_ctrl1(1, spi_enable);
	uint32_t RFATT = rf_att * 4;
	pcie_mem->SendData(Base_Address + REG_SPI_CTRL2, (unsigned int)0x00030001);
	pcie_mem->SendData(Base_Address + REG_SPI_PAYLOAD1, (unsigned int)RFATT);
	pcie_mem->SendData(Base_Address + REG_SPI_PAYLOAD2, (unsigned int)0);
	pcie_mem->SendData(Base_Address + REG_SPI_CTRL1, (unsigned int)Ctrlvalue);
}


void HLJS386_LL::SetIFATT()
{
	uint32_t Ctrlvalue = format_reg_spi_ctrl1(2, spi_enable);
	uint32_t IFATT = if_att * 4;
	pcie_mem->SendData(Base_Address + REG_SPI_CTRL2, (unsigned int)0x00030001);
	pcie_mem->SendData(Base_Address + REG_SPI_PAYLOAD1, (unsigned int)IFATT);
	pcie_mem->SendData(Base_Address + REG_SPI_PAYLOAD2, (unsigned int)0);
	pcie_mem->SendData(Base_Address + REG_SPI_CTRL1, (unsigned int)Ctrlvalue);
}

void HLJS386_LL::SetRFMode()
{
	uint32_t Ctrlvalue = format_reg_spi_ctrl1(3, spi_enable);
	uint32_t RFMode = rf_mode;
	pcie_mem->SendData(Base_Address + REG_SPI_PAYLOAD1, (unsigned int)RFMode);
	pcie_mem->SendData(Base_Address + REG_SPI_PAYLOAD2, (unsigned int)0);
	pcie_mem->SendData(Base_Address + REG_SPI_CTRL2, (unsigned int)0x00030001);
	pcie_mem->SendData(Base_Address + REG_SPI_CTRL1, (unsigned int)Ctrlvalue);
}

void HLJS386_LL::Set_Switch_On()
{
	pcie_mem->SendData(0x00010003, (unsigned int)0xFF);
	pcie_mem->SendData(0x00010004, (unsigned int)0);
	pcie_mem->SendData(0x00010001, (unsigned int)0x00030001);
	pcie_mem->SendData(0x00010000, (unsigned int)0x80000010);
	pcie_mem->SendData(0x00006000, (unsigned int)0); //
	pcie_mem->SendData(0x00006008, (unsigned int)1); //切换直通通道
}

void HLJS386_LL::Set_Switch_Off()
{
	pcie_mem->SendData(0x00010003, (unsigned int)0xFF);
	pcie_mem->SendData(0x00010004, (unsigned int)0);
	pcie_mem->SendData(0x00010001, (unsigned int)0x00030001);
	pcie_mem->SendData(0x00010000, (unsigned int)0x80000010);
	pcie_mem->SendData(0x00006000, (unsigned int)0); //
	pcie_mem->SendData(0x00006008, (unsigned int)0); //切换变频通道
}

uint32_t HLJS386_LL::Get_Temperature()
{
	pcie_mem->SendData(0x00010001, 0x00030001);
	pcie_mem->SendData(0x00010000, 0x800000E2);
	unsigned char res[4];
	pcie_mem->ReadBackData(0x00010007, 4, res);
	int temp = res && 0x1FFF;
	int temp_res = temp / 32;
	return temp_res;
}

uint32_t HLJS386_LL::Get_Status()
{
	pcie_mem->SendData(0x00010001, 0x00030001);
	pcie_mem->SendData(0x00010000, 0x800000E5);
	unsigned char res[4];
	pcie_mem->ReadBackData(0x00010007, 4, res);
	int temp = res && 0x0F;
	return temp;
}