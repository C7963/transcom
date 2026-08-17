#pragma once
#include <iostream>
#include "HLJS386_LL.h"
#include <cmath>
#include "ADC_Api.h"


extern int32_t global_rfatt;
extern int32_t global_ifatt;

namespace HLJS386
{

	class  HLJS386_MZ116_HL
	{
	public:

		bool SetCenterFreq(uint64_t centerfreq);
		uint32_t SetRefLevel(int reflevel);
		void SetRFATT(uint32_t rfatt);
		void SetIFATT(uint32_t ifatt);
		void SetRFMode(uint32_t rfmode);

	private:
		HLJS386::HLJS386_LL MZ116;
	};


	class  HLJS386_MZ121A_HL
	{
	public:
		uint64_t adcsamplerate;

		void SetDirectFreq(uint64_t centerfreq);
		bool SetCenterFreq(uint64_t centerfreq);
		void SetRFATT(uint32_t rfatt);
		void SetIFATT(uint32_t ifatt);
		uint32_t GetIFATT();
		uint32_t GetRFATT();
		void SetRFMode(uint32_t rfmode);
		uint32_t SetRefLevel(int reflevel);
		uint32_t SetDirectRefLevel(int reflevel);
		uint32_t Get_Temperature();
		uint32_t Get_Status();

	private:
		HLJS386::HLJS386_LL MZ121;
	};

}
