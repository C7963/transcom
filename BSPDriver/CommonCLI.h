#pragma once  
#include "Device_Data.h"
#include "CommonManager.h" 

using namespace System;

namespace CommonControlCLI {

	public ref class CommonCli {

	public:

		void SetADCChannel(uint32_t channel);
		void update_si5386_firmware(uint64_t adc_clk, uint32_t ref_clk);
		float Get_fpga_volt();
		int Get_system_status();
		bool Get_ddr_status(uint8_t ddrNo); 
		void CloseDevice(); 
		void InitDevice();
		void SetRFType(RFCONTROL::RFType rf_select);

		void SetWorkMode(Global::WorkMode workmode);
		void set_reference_mode(uint8_t mode);
		static CommonCli^ Instance() {
			return GetInstance();
		}
		void SetFilterCoe(cli::array<unsigned int>^ FilterLowCoe, cli::array<unsigned int>^ FilterCoe);

		uint32_t GetIFATT();

		float Get_fpga_temp();

		value struct RefLevelResultsCli {
			int Att;
			int FFTGainOffset;
		};
	private:
		CommonCli();
		~CommonCli();
		static CommonCli^ instance;
		// 获取实例的私有方法
		static CommonCli^ GetInstance() {
			if (instance == nullptr) {
				instance = gcnew CommonCli();
			}
			return instance;
		}
		Common::CommonManager* common;
	};
}