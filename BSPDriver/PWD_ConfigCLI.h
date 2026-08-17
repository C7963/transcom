#pragma once
#include "PWD_Config.h"
#include <queue>
#include <mutex>
#include <msclr/marshal_cppstd.h>

using namespace System;
using namespace System::Runtime::InteropServices;

namespace PWDCONFIGCLI 
{
	public ref class PWDConfig
	{
	public:
		PWDConfig();
		~PWDConfig();
		void SetChannelNum(uint32_t num);
		void Caculate_F_IFs(cli::array<int64_t>^ Nfreq);
		void PWD_Enable(bool enable);
		void PWD_Threshold(uint32_t channel_num, cli::array<float>^ threshold);
		void PWD_Threshold(uint32_t channel_num, int32_t threshold);
		void PWD_DDC_Config();
		void PWD_DDC_Decim(uint32_t decim);
		void PWD_DDC_FIR_COEF(cli::array<uint32_t>^ value, uint32_t length);
		void fir_config_axis_reset();
		void fir_config_txdata_fifo_reset();
		void Start();
		void Stop();
		void Pop();
		bool DequeueQueue(cli::array<unsigned char>^% buffer);
		cli::array<cli::array <uint8_t>^>^ arr_cli;

	private:
	
		PWDCONFIG::PWDCtrl* pwdctrl;
		PWDCONFIG::PWDStream* pwdstream;
		cli::array <uint8_t>^ array_cli;
		int arrCliLength;
	};
}


