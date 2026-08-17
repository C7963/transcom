#pragma once
#include "RPU44.h"
#include "CM18_500M.h"
#include "RF12.h"
#include "HLJS386_HL.h"  


namespace RFCONTROL {
	enum class RFType
	{
		RF12,
		CM18,
		RPU44,
		MZ116,
		MZ121,
		MZ121B
	};

	struct RefLevelResult {
		int Att = 0;
		int FFTGainOffset = 0; 
	};

	class  RFControl
	{
	public:
		static RFType RF_SELECT;
		RFControl();
		~RFControl();
		void SetRFCard(RFType rf_select);
		bool SetCenterFreq(uint64_t centerfreq);
		RefLevelResult SetRefLevel(int reflevel); 
		void SetPowerOnOff(uint32_t flag);
		void SetOutBW(int ifbw);
		static bool isInvertedFreq(double freq);
		void SwapIQByFreq(double freq);
		uint32_t GetIFATT();
		uint32_t GetRFATT();
	 private:
		 Device::Device_MEM32* pcie_mem;
	
	};
}
