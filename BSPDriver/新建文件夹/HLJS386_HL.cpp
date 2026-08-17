#pragma once
#include "HLJS386_HL.h"

using namespace HLJS386;

int32_t global_rfatt = 0;
int32_t global_ifatt = 0;

bool HLJS386_MZ116_HL::SetCenterFreq(uint64_t centerfreq)
{
	if (centerfreq >= 20E6 && centerfreq <= 26500E6)
	{
		MZ116.center_freq = centerfreq;
	}
	else
	{
		cout << "数值有误" << endl;
	}
	return MZ116.SetCenterFreq();
}

uint32_t  HLJS386_MZ116_HL::SetRefLevel(int reflevel)
{
	if (reflevel >= -10 && reflevel <= 20)
	{
		MZ116.if_att = 10;
		MZ116.rf_att = abs(reflevel + 10);
		MZ116.rf_mode = 0x00; //常规模式
		global_rfatt = MZ116.rf_att;
		global_ifatt = MZ116.if_att;
	}
	else if (reflevel < -10 && reflevel >= -50)
	{
		MZ116.if_att = 10;
		MZ116.rf_att = 0;
		MZ116.rf_mode = 0x00; //常规模式
		global_rfatt = MZ116.rf_att;
		global_ifatt = MZ116.if_att;
	}
	else if (reflevel < -50)
	{
		MZ116.if_att = 0;
		MZ116.rf_att = 0;
		MZ116.rf_mode = 0x01; //低失真模式
		global_rfatt = MZ116.rf_att;
		global_ifatt = MZ116.if_att;
	}

	MZ116.SetRFATT();
	MZ116.SetIFATT();
	MZ116.SetRFMode();
	return MZ116.rf_att;
}

void  HLJS386_MZ116_HL::SetRFATT(uint32_t rfatt)
{
	if (rfatt >= 0 && rfatt <= 30)
	{
		MZ116.rf_att = rfatt;
		global_rfatt = MZ116.rf_att;
	}
	else
	{
		cout << "数值有误" << endl;
	}
	MZ116.SetRFATT();
}

void  HLJS386_MZ116_HL::SetIFATT(uint32_t ifatt)
{
	if (ifatt >= 0 && ifatt <= 30)
	{
		MZ116.if_att = ifatt;
		global_ifatt = MZ116.if_att;
	}
	else
	{
		cout << "数值有误" << endl;
	}
	MZ116.SetIFATT();
}

void HLJS386_MZ116_HL::SetRFMode(uint32_t rfmode)
{
	if (rfmode == 0x00)
	{
		MZ116.rf_mode = 0x00;
	}
	else if (rfmode == 0x01)
	{
		MZ116.rf_mode = 0x01;
	}
	else if (rfmode == 0x02)
	{
		MZ116.rf_mode = 0x02;
	}
	else
	{

	}
	MZ116.SetRFMode();
}


void HLJS386_MZ121A_HL::SetDirectFreq(uint64_t centerfreq)
{

	MZ121.Set_Switch_On();
	MZ121.center_freq = centerfreq;
	//ADC::ADCApi::adc_adc_set_ddc_nco(0, centerfreq);

}

bool HLJS386_MZ121A_HL::SetCenterFreq(uint64_t centerfreq)
{
	MZ121.Set_Switch_Off();
	MZ121.center_freq = centerfreq;
	return MZ121.SetCenterFreq();
	//MZ121本振精度只有10kHz,通过ADC的DDC环节进行中心频点补偿 
	//uint64_t frequency2 = std::floor(centerfreq / 10000); //10KHz
	//uint64_t Lo_compensation2 = (int)(centerfreq - frequency2 * 10000);
	//ADC::ADCApi::adc_adc_set_ddc_nco(0, adcsamplerate / 4 + Lo_compensation2);

}

uint32_t  HLJS386_MZ121A_HL::SetRefLevel(int reflevel)
{
	if (reflevel >= -10 && reflevel <= 20)
	{
		MZ121.if_att = 15;
		MZ121.rf_att = abs(reflevel + 10);
		MZ121.rf_mode = 0x00; //常规模式
		global_rfatt = MZ121.rf_att;
		global_ifatt = MZ121.if_att;
	}
	else if (reflevel < -10 && reflevel >= -50)  //refLevel=-11,ifatt=14 ...以此类推  
	{
		int delta = 15 + (reflevel + 10);
		if (delta < 0) delta = 0;
		MZ121.if_att = delta;
		MZ121.rf_att = 0;
		MZ121.rf_mode = 0x00; //常规模式
		global_rfatt = MZ121.rf_att;
		global_ifatt = MZ121.if_att;
	}
	else if (reflevel < -50)
	{
		MZ121.if_att = 0;
		MZ121.rf_att = 0;
		MZ121.rf_mode = 0x01; //低失真模式
		global_rfatt =-15;     //这个模式下射频通道增益为15
		global_ifatt = 0;
	}

	MZ121.SetRFATT();
	MZ121.SetIFATT();
	MZ121.SetRFMode();
	return MZ121.rf_att;
}

void  HLJS386_MZ121A_HL::SetRFATT(uint32_t rfatt)
{
	if (rfatt >= 0 && rfatt <= 30)
	{
		MZ121.rf_att = rfatt;
		global_rfatt = MZ121.rf_att;
	}
	else
	{
		cout << "数值有误" << endl;
	}
	MZ121.SetRFATT();
}

void  HLJS386_MZ121A_HL::SetIFATT(uint32_t ifatt)
{
	if (ifatt >= 0 && ifatt <= 30)
	{
		MZ121.if_att = ifatt;
		global_ifatt = MZ121.if_att;
	}
	else
	{
		cout << "数值有误" << endl;
	}
	MZ121.SetIFATT();
}

uint32_t HLJS386::HLJS386_MZ121A_HL::GetIFATT()
{
	return MZ121.if_att;
}
uint32_t HLJS386::HLJS386_MZ121A_HL::GetRFATT()
{
	return MZ121.rf_att;
}
void HLJS386_MZ121A_HL::SetRFMode(uint32_t rfmode)
{
	if (rfmode == 0x00)
	{
		MZ121.rf_mode = 0x00;
	}
	else if (rfmode == 0x01)
	{
		MZ121.rf_mode = 0x01;
	}
	else if (rfmode == 0x02)
	{
		MZ121.rf_mode = 0x02;
	}
	else
	{

	}
	MZ121.SetRFMode();
}

uint32_t HLJS386_MZ121A_HL::SetDirectRefLevel(int reflevel)
{
	if (reflevel >= -10 && reflevel <= 20)
	{
		MZ121.rf_att = abs(reflevel + 20);
		global_rfatt = MZ121.rf_att;

	}
	else if (reflevel < -10 && reflevel >= -50)
	{
		MZ121.rf_att = 10;
		global_rfatt = MZ121.rf_att;

	}
	else if (reflevel < -50)
	{
		MZ121.rf_att = 0;
		global_rfatt = MZ121.rf_att;
	}
	MZ121.if_att = 0;
	MZ121.SetRFATT();
	/*MZ121.SetIFATT();
	MZ121.SetRFMode();*/
	return MZ121.rf_att;
}

uint32_t  HLJS386_MZ121A_HL::Get_Temperature()
{
	return MZ121.Get_Temperature();
}

uint32_t  HLJS386_MZ121A_HL::Get_Status()
{
	return MZ121.Get_Status();
}

