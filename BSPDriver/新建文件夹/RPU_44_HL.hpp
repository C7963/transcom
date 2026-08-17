#include "RPU_44_LL.hpp"
#include <map>
#pragma once

namespace RPU_44 
{
	class  RPU_44_HL
	{
	public:
		
		void SetCenterFreq(uint64_t centerfreq)
		{
			RPU44.center_freq = centerfreq;
			if (centerfreq >= 5E6 && centerfreq < 50E6)
			{
				RPU44.rf_channel = 0;
			}
			else if(centerfreq>=50E6 && centerfreq <3.2E9)
			{
				RPU44.rf_channel = 1;
			}
			else if (centerfreq >= 3.2E9 && centerfreq < 4.96E9)
			{
				RPU44.rf_channel = 2;
			}
			else if (centerfreq >= 4.96E9 && centerfreq < 6.1E9)
			{
				RPU44.rf_channel = 3;
			}
			else if (centerfreq >= 6.1E9 && centerfreq < 7.38E9)
			{
				RPU44.rf_channel = 4;
			}
			else if (centerfreq >= 7.38E9 && centerfreq < 11E9)
			{
				RPU44.rf_channel = 5;
			}
			else if (centerfreq >= 11E9 && centerfreq < 14.5E9)
			{
				RPU44.rf_channel = 6;
			}
			else if (centerfreq >= 14.5E9 && centerfreq < 18.4E9)
			{
				RPU44.rf_channel = 7;
			}
			else if (centerfreq >= 18.4E9 && centerfreq < 22E9)
			{
				RPU44.rf_channel = 8;
			}
			else if (centerfreq >= 22E9 && centerfreq < 26.2E9)
			{
				RPU44.rf_channel = 9;
			}
			else if (centerfreq >= 26.2E9 && centerfreq < 35E9)
			{
				RPU44.rf_channel = 10;
			}
			else if (centerfreq >= 35E9 && centerfreq <= 44E9)
			{
				RPU44.rf_channel = 11;
			}
			RPU44.RPU44_Config();
			RPU44.RPU44_Sweep();
		}

		uint32_t SetRefLevel(int reflevel)
		{
			if (reflevel >= -10 && reflevel <= 20)
			{
				RPU44.rf_ATT = scores[reflevel]/2;
				RPU44.rf_LNA = 0;
			}
			else if (reflevel < -10 && reflevel >= -30)
			{
				RPU44.rf_ATT = 0;
				RPU44.rf_LNA = 0;
			}
			else if (reflevel < -30)
			{
				RPU44.rf_ATT = 0;
				RPU44.rf_LNA = 1;
			}
			RPU44.RPU44_Config();
			return RPU44.rf_ATT*2;
		}

		void PowerOnOff(uint32_t flag)
		{
			RPU44.power_state = flag;
			RPU44.RPU44_PowerOnOff();
		}
	private:
		RPU_44::RPU_44_LL RPU44;
		std::map<uint32_t, uint32_t> scores = 
		{
			{ 20,30 },
			{ 19,30 },
			{ 18,28 },
			{ 17,28 },
			{ 16,26 },
			{ 15,26 },
			{ 14,24 },
			{ 13,24 },
			{ 12,22 },
			{ 11,22 },
			{ 10,20 },
			{ 9,20 },
			{ 8,18 },
			{ 7,18 },
			{ 6,16 },
			{ 5,16 },
			{ 4,14 },
			{ 3,14 },
			{ 2,12 },
			{ 1,12 },
			{ 0,10 },
			{ -1,10 },
			{ -2,8 },
			{ -3,8 },
			{ -4,6 },
			{ -5,6 },
			{ -6,4 },
			{ -7,4 },
			{ -8,2 },
			{ -9,2 },
			{ -10,0 } };
	};

	
}