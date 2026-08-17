#pragma once
#include "RFControl.h"

using namespace System;

namespace RFControlCLI {

	public ref class  RFControlCli
	{
	public:
		RFControlCli();
		~RFControlCli();
		void SetRFCard(RFCONTROL::RF_CLASS rf_select);
		void SetCenterFreq(uint64_t centerfreq);
		uint32_t SetRefLevel(int reflevel);
		uint32_t SetRefLevel(int reflevel,double cf);
		void SetIFATT(uint32_t ifatt);
		uint32_t GetIFATT();
		void SetCenterFreq_RPU44(uint64_t centerfreq);
		uint32_t SetRefLevel_RPU44(int reflevel);
		void SetPowerOnOff_RPU44(uint32_t flag);
		void SetCenterFreq_MZ116(uint64_t centerfreq);
		uint32_t SetRefLevel_MZ116(int reflevel);
		void SetCenterFreq_CM18(uint64_t centerfreq);
		uint32_t SetRefLevel_CM18(int reflevel);
		void SetOutBW_CM18(int ifbw);
		void Init_RF12();
		void SetCenterFreq_RF12(uint64_t centerfreq);
		void SetRefLevel_RF12(int reflevel);
		int  GetRF12AttValue();
		uint32_t Get_Temperature();
		uint32_t Get_Status();

	private:
		RFCONTROL::RFControl* rfctrl;
	};
}
