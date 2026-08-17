#include "pch.h"
#include "Device_Address.h"

#if USB3
const std::string Device::Device_Address::InterruptPath = R"(\\.\xillyusb_00_intr)";
const std::string Device::Device_Address::Xillybus_Mem32 = R"(\\.\xillyusb_00_mem)";
//const std::string Device::Device_Address::Xillybus_Mem32_1 = R"(\\.\xillyusb_00_nvme)";
const std::string Device::Device_Address::Xillybus_Read0_32 = R"(\\.\xillyusb_00_read0)";
const std::string Device::Device_Address::Xillybus_Read1_32 = R"(\\.\xillyusb_00_read1)";
const std::string Device::Device_Address::Xillybus_Read2_32 = R"(\\.\xillyusb_00_read2)";
const std::string Device::Device_Address::Xillybus_Dma32 = R"(\\.\\xillyusb_00_dma)";
const std::string Device::Device_Address::Xillybus_Read4_32 = R"(\\.\xillyusb_00_read4)";
const std::string Device::Device_Address::Xillybus_Read3_32 = R"(\\.\xillyusb_00_read3)";
const std::string Device::Device_Address::Xillybus_NvmeWrite_32 = R"(\\.\\xillyusb_00_write)";
const std::string Device::Device_Address::Xillybus_Read5_32 = R"(\\.\\xillyusb_00_read5)";
const std::string Device::Device_Address::Xillybus_Read6_32 = R"(\\.\\xillyusb_00_read6)";
#else 
const std::string Device::Device_Address::InterruptPath = R"(\\.\xillybus_intr)";
const std::string Device::Device_Address::Xillybus_Mem32 = R"(\\.\xillybus_mem)";
//const std::string Device::Device_Address::Xillybus_Mem32_1 = R"(\\.\xillyusb_00_nvme)";
const std::string Device::Device_Address::Xillybus_Read0_32 = R"(\\.\xillybus_read0)";
const std::string Device::Device_Address::Xillybus_Read1_32 = R"(\\.\xillybus_read1)";
const std::string Device::Device_Address::Xillybus_Read2_32 = R"(\\.\xillybus_read2)";
const std::string Device::Device_Address::Xillybus_Dma32 = R"(\\.\\xillybus_dma)";
const std::string Device::Device_Address::Xillybus_Read4_32 = R"(\\.\xillybus_read4)";
const std::string Device::Device_Address::Xillybus_Read3_32 = R"(\\.\xillybus_read3)";
const std::string Device::Device_Address::Xillybus_NvmeWrite_32 = R"(\\.\\xillybus_write)";
const std::string Device::Device_Address::Xillybus_Read5_32 = R"(\\.\\xillybus_read5)";
const std::string Device::Device_Address::Xillybus_Read6_32 = R"(\\.\\xillybus_read6)";
const std::string Device::Device_Address::Xillybus_spectrum_Positive = R"(\\.\xillybus_mem0)";
const std::string Device::Device_Address::Xillybus_spectrum_Negative = R"(\\.\xillybus_mem1)";
const std::string Device::Device_Address::Xillybus_spectrum_RMS		 = R"(\\.\xillybus_mem2)";
const std::string Device::Device_Address::Xillybus_spectrum_Average  = R"(\\.\xillybus_mem3)";
const std::string Device::Device_Address::Xillybus_spectrum_Sample   = R"(\\.\xillybus_mem4)";
const std::string Device::Device_Address::Xillybus_spectrum_Auto     = R"(\\.\xillybus_mem5)";
#endif
