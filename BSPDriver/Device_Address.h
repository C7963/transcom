#pragma once
#include <string>
using namespace std;
namespace Device
{
	/// <summary>
	/// PCIE设备地址汇总类
	/// 即设备文件路径
	/// </summary>
	class Device_Address
	{
	public:
		static const std::string InterruptPath;
		static const std::string Xillybus_Mem32;
		//static const std::string Xillybus_Mem32_1;
		static const std::string Xillybus_Read0_32;
		static const std::string Xillybus_Read1_32;
		static const std::string Xillybus_Read2_32;
		static const std::string Xillybus_Dma32;
		static const std::string Xillybus_Read4_32;
		static const std::string Xillybus_NvmeWrite_32;
		static const std::string Xillybus_Read3_32;
		static const std::string Xillybus_Read5_32;
		static const std::string Xillybus_Read6_32;
		static const std::string Xillybus_spectrum_Positive;
		static const std::string Xillybus_spectrum_Negative;
		static const std::string Xillybus_spectrum_RMS	   ;
		static const std::string Xillybus_spectrum_Average ;
		static const std::string Xillybus_spectrum_Sample  ;
		static const std::string Xillybus_spectrum_Auto	   ;
	};
}
