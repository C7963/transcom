#include "SweepLogic.h"
#include "Device_MEM32.h"
#include "RFControl.h"
#include "CommonManager.h"
#include <thread>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <windows.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace SWEEPLOGIC;
using namespace RFCONTROL;

SWEEPLOGIC::SweepLogic* Instance = nullptr;
Device::Device_MEM32* pcie_mem = nullptr;

static std::string GetSweepLogicDllDir()
{
	char path[MAX_PATH] = { 0 };
	HMODULE module = nullptr;
	if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
		GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
		(LPCSTR)&GetSweepLogicDllDir, &module))
	{
		GetModuleFileNameA(module, path, MAX_PATH);
		std::string fullPath(path);
		size_t pos = fullPath.find_last_of("\\/");
		if (pos != std::string::npos)
			return fullPath.substr(0, pos + 1);
	}
	return ".\\";
}

static void SweepLogicLog(const char* msg)
{
	try
	{
		std::ofstream log(GetSweepLogicDllDir() + "bsdriver_diag.log", std::ios::app);
		if (log.is_open())
			log << msg << "\n";
	}
	catch (...) {}
}
void SweepLogic::Config()
{
	pcie_mem = Device::Device_MEM32::getInstance();
	bw = 320e6;  // Aligned with C#
	Det_Len = 1001;
	CaculateParas();
	ConfigSweepParameter();
	// Match the C# SpectrumSweep.Config() ordering: write the sweep
	// registers first, then write the LO table as a separate step.
	updateAutosweepTable(CenterFrequency, Span, bw, bw_last, BW_Num);
	// C# does not write IF NCO (0x000C0005/6), so skip SetSweepOffset()
	constexpr uint64_t kMaxPscanOutputPoints = 65536;
	uint64_t outputPoints = Det_Nums == 0 ? 1 : Det_Nums;
	if (outputPoints > kMaxPscanOutputPoints)
	{
		outputPoints = kMaxPscanOutputPoints;
	}
	Global::SweepSpectrumPointCount = static_cast<uint32_t>(outputPoints);
	Global::SweepSpectrumPointCountSet = Det_Len;
}

std::pair<uint32_t, uint32_t> GetHighLowValueFromUlong(uint64_t value)
{
	uint32_t low = static_cast<uint32_t>(value & 0x00000000FFFFFFFF);
	uint32_t high = static_cast<uint32_t>((value & 0xFFFFFFFF00000000) >> 32);
	return std::make_pair(high, low);
}

void SweepLogic::SetSweepOffset()
{
	// C# production client does NOT write IF NCO registers (0x000C0005/6)
	// This is a no-op for alignment with C#
}

void SweepLogic::CaculateDetect()
{
	F_Step = RBW / 3.0;
	Det_Nums = (uint64_t)std::round(Span / F_Step + 1);
	Det_Num = (uint32_t)std::round((double)Det_Nums / (double)Det_Len);
	if (Det_Num < 1)
	{
		Det_Num = 1;
		if (Det_Nums % 2 == 0)
			Det_Nums += 1;
		Det_Len = Det_Nums;
	}
	else
	{
		Det_Nums = Det_Len * Det_Num;
	}
	CF_Offset = std::floor(Det_Nums / 2) * (Span / (double)(Det_Nums - 1)) - Span / 2;
	F_Step = Span / (double)(Det_Nums - 1);
}

void SweepLogic::CaculateDetectFFT()
{
	// Aligned with C# CaculateDetectFFT()
	int data_rem;
	double data_div, data_abs_div;
	Det_Nums = (uint32_t)std::floor(Span / FFT_Bin + 1);
	if (Det_Nums >= Det_Len)
	{
		Det_Num = (uint32_t)std::floor((double)Det_Nums / (double)Det_Len);
		Det_Len = (uint32_t)std::ceil((double)Det_Nums / (double)Det_Num);
		F_Step_Num = (uint16_t)std::ceil(bw / FFT_BW);
		FFT_Trun_Len = (uint16_t)std::ceil(bw / FFT_Bin / (double)F_Step_Num);
		bw = (double)F_Step_Num * (double)FFT_Trun_Len * FFT_Bin;
		data_rem = (int)Det_Nums - (int)((Det_Len - 1) * Det_Num);
		data_div = (double)data_rem / (double)Det_Num;
		data_abs_div = std::ceil(std::abs(data_div));
		if (Det_Num == 1)
		{
			Det_Rem = 0;
		}
		else
		{
			if (data_div < 0)
			{
				Det_Len -= (uint32_t)data_abs_div;
				Det_Rem = (uint32_t)((double)Det_Num * data_abs_div + (double)data_rem);
			}
			else
			{
				Det_Len += (uint32_t)(data_abs_div - 1);
				Det_Rem = (uint32_t)((double)data_rem - (double)Det_Num * (data_abs_div - 1));
			}
			if (Det_Rem <= 1)
			{
				Det_Len -= 1;
				Det_Rem += Det_Num;
			}
		}
	}
	else
	{
		Det_Num = 1;
		F_Step_Num = 1;
		Det_Rem = 0;
		FFT_Trun_Len = (uint16_t)std::ceil(bw / FFT_Bin);
		bw = (double)FFT_Trun_Len * FFT_Bin;
		Det_Len = Det_Nums;
	}
	F_Step = bw / (double)F_Step_Num;
}

void SweepLogic::CaculateParas()
{
	// Aligned with C# CaculateParas() for PSCAN mode (Mode=0)
	if (Mode == 0)
	{
		FC = 0;
		FS = 409.6e6;
		FFT_Enable = 1;
	}
	else
	{
		FC = 307.2e6;
		FS = 1228.8e6;
		FFT_Enable = 0;
	}

	if (bw >= Span)
	{
		BW_Num = 1;
		bw = Span;
	}
	else
	{
		BW_Num = (uint32_t)std::ceil(Span / bw);
		bw = Span / BW_Num;
	}

	// FFT configuration based on Step (aligned with C#)
	if (FFT_Enable == 1)
	{
		if (Step == 200e3)
		{
			FFT_HB_Sel = 0; FFT_HB_Decim = 1; FFT_HB_Shift = 1;
			FFT_BW = 320e6; FFT_Sel = 3; FFT_Len = 2048;
		}
		else if (Step == 100e3)
		{
			FFT_HB_Sel = 0; FFT_HB_Decim = 1; FFT_HB_Shift = 1;
			FFT_BW = 320e6; FFT_Sel = 2; FFT_Len = 4096;
		}
		else if (Step == 50e3)
		{
			FFT_HB_Sel = 0; FFT_HB_Decim = 1; FFT_HB_Shift = 1;
			FFT_BW = 320e6; FFT_Sel = 1; FFT_Len = 8192;
		}
		else if (Step == 25e3)
		{
			FFT_HB_Sel = 0; FFT_HB_Decim = 1; FFT_HB_Shift = 1;
			FFT_BW = 320e6; FFT_Sel = 0; FFT_Len = 16384;
		}
		else if (Step == 12.5e3)
		{
			FFT_HB_Sel = 1; FFT_HB_Decim = 2; FFT_HB_Shift = 0;
			FFT_BW = 160e6; FFT_Sel = 0; FFT_Len = 16384;
		}
		else if (Step == 6.25e3)
		{
			FFT_HB_Sel = 2; FFT_HB_Decim = 4; FFT_HB_Shift = 0;
			FFT_BW = 80e6; FFT_Sel = 0; FFT_Len = 16384;
		}
		else if (Step == 3.125e3)
		{
			FFT_HB_Sel = 3; FFT_HB_Decim = 8; FFT_HB_Shift = 0;
			FFT_BW = 40e6; FFT_Sel = 0; FFT_Len = 16384;
		}
		FFT_Bin = FS / (double)FFT_HB_Decim / (double)FFT_Len;
	}

	FTW_Hold_Num = FFT_Len;
	FTW_Hold = FTW_Hold_Num * FFT_HB_Decim;

	// Generate Blackman window (aligned with C#)
	for (uint32_t i = 0; i < FFT_Len / 2; i++)
	{
		double blackman = 0.35875 - 0.48829 * std::cos(2.0 * M_PI * i / (FFT_Len - 1))
			+ 0.14128 * std::cos(4.0 * M_PI * i / (FFT_Len - 1))
			- 0.01168 * std::cos(6.0 * M_PI * i / (FFT_Len - 1));
		Blackman_Win[i] = (uint16_t)std::round(blackman * (std::pow(2, 11) - 1));
		Blackman_Win[FFT_Len - 1 - i] = Blackman_Win[i];
	}

	CaculateDetectFFT();
	// Match C#: CaculateDetectFFT() owns F_Step_Num and F_Step.
	F_Start = FC - bw / 2.0 * (1.0 - 1.0 / (double)F_Step_Num);
	F_Stop = FC + bw / 2.0 * (1.0 - 1.0 / (double)F_Step_Num);

	// Sub-band calculation (aligned with C#)
	if (Rf_Stop <= Rf_Sub)
	{
		BW_Sub_Num = BW_Num;
		F_Start_Sub = Rf_Start + bw / (2.0 * F_Step_Num);
		F_Stop_Sub = Rf_Stop - bw / (2.0 * F_Step_Num);
	}
	else if (Rf_Start < Rf_Sub)
	{
		BW_Sub_Num = (uint32_t)std::floor((Rf_Sub - Rf_Start + bw / 2.0) / bw);
		if (BW_Sub_Num == 0)
		{
			F_Start_Sub = 0;
			F_Stop_Sub = 0;
		}
		else
		{
			F_Start_Sub = Rf_Start + bw / (2.0 * F_Step_Num);
			F_Stop_Sub = Rf_Start + bw * BW_Sub_Num - bw / (2.0 * F_Step_Num);
		}
	}
	else
	{
		BW_Sub_Num = 0;
		F_Start_Sub = 0;
		F_Stop_Sub = 0;
	}

	Det_Cal_Config = Det_Cal_EN * (uint32_t)std::pow(2, 15) + Det_Cal_Shift;

	// PSCAN mode: wrap negative frequencies
	if (Mode == 0)
	{
		if (F_Start < 0) F_Start = F_Start + FS;
		if (F_Stop < 0) F_Stop = F_Stop + FS;
		if (F_Start_Sub != 0) F_Start_Sub = Rf_Sub - F_Start_Sub;
		if (F_Stop_Sub != 0) F_Stop_Sub = Rf_Sub - F_Stop_Sub;
	}

	// Calculate FTW (Frequency Tuning Words)
	FTW_Start = (uint64_t)(F_Start / FS * std::pow(2, 48));
	FTW_Start_H16 = (uint32_t)(FTW_Start / std::pow(2, 32));
	FTW_Start_L32 = (uint32_t)(FTW_Start - FTW_Start_H16 * std::pow(2, 32));
	FTW_Stop = (uint64_t)(F_Stop / FS * std::pow(2, 48));
	FTW_Stop_H16 = (uint32_t)(FTW_Stop / std::pow(2, 32));
	FTW_Stop_L32 = (uint32_t)(FTW_Stop - FTW_Stop_H16 * std::pow(2, 32));
	FTW_Start_Sub = (uint64_t)(F_Start_Sub / FS * std::pow(2, 48));
	FTW_Start_Sub_H16 = (uint32_t)(FTW_Start_Sub / std::pow(2, 32));
	FTW_Start_Sub_L32 = (uint32_t)(FTW_Start_Sub - FTW_Start_Sub_H16 * std::pow(2, 32));
	FTW_Stop_Sub = (uint64_t)(F_Stop_Sub / FS * std::pow(2, 48));
	FTW_Stop_Sub_H16 = (uint32_t)(FTW_Stop_Sub / std::pow(2, 32));
	FTW_Stop_Sub_L32 = (uint32_t)(FTW_Stop_Sub - FTW_Stop_Sub_H16 * std::pow(2, 32));
	FTW_Step = (uint64_t)(F_Step / FS * std::pow(2, 48));
	FTW_Step_H16 = (uint32_t)(FTW_Step / std::pow(2, 32));
	FTW_Step_L32 = (uint32_t)(FTW_Step - FTW_Step_H16 * std::pow(2, 32));

	// Dwell hold
	Dwell_hold = (uint64_t)std::round(Dwell_hold_time / Clk);
	Dwell_hold_H32 = (uint32_t)(Dwell_hold / std::pow(2, 32));
	Dwell_hold_L32 = (uint32_t)(Dwell_hold - Dwell_hold_H32 * std::pow(2, 32));

	// Inter hold
	double inter_hold_val = Inter_hold_time / Clk;

	// FFT_HB_Config (aligned with C#)
	FFT_HB_Config = (uint16_t)(FFT_HB_Sel * 16 + FFT_HB_Shift);

	// Sweep time calculation
	Sweep_Time = (double)((uint64_t)FTW_Hold * Det_Nums + (uint64_t)(VCO_Lock + 1) * (uint64_t)BW_Num + (uint64_t)YTF_Lock);
	Sweep_Time /= FS;
}

void SweepLogic::TriggerSweep()
{
	pcie_mem = Device::Device_MEM32::getInstance();
	// C# writes 1 to 0x10010000 (not Sort*2+1 to 0x10020000)
	pcie_mem->SendData(0x10010000, (uint32_t)1);
}

void SweepLogic::ConfigSweepParameter()
{
	// Aligned with C# ConfigSweepParameter()
	// C# writes 27 elements first, then individual elements [27]~[36]

	uint32_t SweepData[27];
	SweepData[0] = 0;
	SweepData[1] = ADC_Hold;              // C# [1]=ADC_Hold, not CIC_Config
	SweepData[2] = FTW_Start_L32;
	SweepData[3] = FTW_Start_H16;
	SweepData[4] = FTW_Stop_L32;
	SweepData[5] = FTW_Stop_H16;
	SweepData[6] = FTW_Step_L32;
	SweepData[7] = FTW_Step_H16;
	SweepData[8] = FTW_Hold;
	SweepData[9] = (uint32_t)(BW_Sub_Num * std::pow(2, 16) + BW_Num);
	SweepData[10] = Det_Num;
	SweepData[11] = Det_Len;
	SweepData[12] = Det_Rem;
	SweepData[13] = Det_Type;
	SweepData[14] = Filt_Config;
	SweepData[15] = VCO_Lock;
	SweepData[16] = YTF_Lock;
	SweepData[17] = Det_Cal_Config;
	SweepData[18] = FFT_HB_Config;
	SweepData[19] = (uint32_t)(FFT_Trun_Len * std::pow(2, 16) + F_Step_Num);
	SweepData[20] = (uint32_t)(FFT_Enable * std::pow(2, 31) + FFT_Sel * std::pow(2, 17) + FFT_Len);
	SweepData[21] = 0;
	SweepData[22] = 0;
	SweepData[23] = 0;
	SweepData[24] = DMA_Hold;
	SweepData[25] = F_Step_Num;
	SweepData[26] = F_Rem;

	// Write first 27 elements to 0x10010000
	pcie_mem->SendData(0x10010000, SweepData, 27);
	std::this_thread::sleep_for(std::chrono::milliseconds(1));

	// Write individual elements [27]~[36] (aligned with C#)
	pcie_mem->SendData(0x10010000 + 27, Pow_thr);
	std::this_thread::sleep_for(std::chrono::milliseconds(1));
	pcie_mem->SendData(0x10010000 + 28, (uint32_t)0);
	std::this_thread::sleep_for(std::chrono::milliseconds(1));
	pcie_mem->SendData(0x10010000 + 29, (uint32_t)Det_Nums);
	std::this_thread::sleep_for(std::chrono::milliseconds(1));
	uint32_t inter_hold_val = (uint32_t)(Inter_hold_time / Clk);
	pcie_mem->SendData(0x10010000 + 30, inter_hold_val);
	std::this_thread::sleep_for(std::chrono::milliseconds(1));
	pcie_mem->SendData(0x10010000 + 31, FTW_Start_Sub_L32);
	std::this_thread::sleep_for(std::chrono::milliseconds(1));
	pcie_mem->SendData(0x10010000 + 32, FTW_Start_Sub_H16);
	std::this_thread::sleep_for(std::chrono::milliseconds(1));
	pcie_mem->SendData(0x10010000 + 33, FTW_Stop_Sub_L32);
	std::this_thread::sleep_for(std::chrono::milliseconds(1));
	pcie_mem->SendData(0x10010000 + 34, FTW_Stop_Sub_H16);
	std::this_thread::sleep_for(std::chrono::milliseconds(1));
	pcie_mem->SendData(0x10010000 + 35, Dwell_hold_L32);
	std::this_thread::sleep_for(std::chrono::milliseconds(1));
	pcie_mem->SendData(0x10010000 + 36, Dwell_hold_H32);
	std::this_thread::sleep_for(std::chrono::milliseconds(1));

	// Write Blackman window + trigger (aligned with C#)
	if (FFT_Enable == 1)
	{
		std::vector<uint32_t> blackmanData(FFT_Len / 2);
		for (uint32_t i = 0; i < FFT_Len / 2; i++)
		{
			blackmanData[i] = (uint32_t)(Blackman_Win[i * 2 + 1] * std::pow(2, 16) + Blackman_Win[i * 2]);
		}
		pcie_mem->SendData(0x10018000, blackmanData.data(), FFT_Len / 2);
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		pcie_mem->SendData(0x10010000, (uint32_t)1);  // Trigger
	}
	else
	{
		pcie_mem->SendData(0x10010000, (uint32_t)1);  // Trigger
	}

}

void SweepLogic::updateAutosweepTable(double ui_freq_center, double ui_span, double bw_val, double bw_last_val, uint32_t bw_num)
{
	// Aligned with C# updateAutosweepTable()
	double bw_num_d;
	if (ui_span == 0)
		bw_num_d = 1;
	else
		bw_num_d = ui_span / bw_val;

	{
		std::ostringstream oss;
		oss << "[PSCAN_LO_TABLE] begin"
			<< " CF=" << ui_freq_center
			<< " Span=" << ui_span
			<< " BW=" << bw_val
			<< " BWLast=" << bw_last_val
			<< " entries=" << bw_num
			<< " calculatedEntries=" << bw_num_d;
		SweepLogicLog(oss.str().c_str());
	}

	for (uint32_t i = 0; i < bw_num; i++)
	{
		double freq = (ui_freq_center - (ui_span / 2.0) + (bw_val / 2.0)) + i * bw_val;
		FrequencyDataList(freq, i, GetFreqErrorData(freq));
	}
	SweepLogicLog("[PSCAN_LO_TABLE] end");
}

double SweepLogic::GetFreqErrorData(double freq)
{
	// C# returns 0 (calibration disabled)
	return 0.0;
}

void SweepLogic::FrequencyDataList(double freq, int num, double errorvalue)
{
	// Aligned with C# FrequencyDataList()
	// C# uses 0x000A0005~0x000A0007, freq/1000 encoding
	try
	{
		long frequency = (long)(freq / 1000.0);  // 1KHz quantization (C# uses /1000)

		// Convert to 32-bit binary string, pad to 32 bits
		std::string value;
		uint32_t freqBits = (uint32_t)(frequency & 0xFFFFFFFF);
		for (int i = 31; i >= 0; i--)
		{
			value += ((freqBits >> i) & 1) ? '1' : '0';
		}
		uint32_t freqWord = (uint32_t)std::stoul(value, nullptr, 2);

		uint32_t errorWord = (uint16_t)std::pow(10, ((errorvalue / 20.0) + 3.01));
		bool writeA0006 = pcie_mem->SendData(0x000A0006, freqWord);
		bool writeA0007 = pcie_mem->SendData(0x000A0007, errorWord);

		// PSCAN mode: numvalue = 0x80000000 + num
		uint32_t numvalue = 0x80000000 + (uint32_t)num;
		bool writeSlotSelect = pcie_mem->SendData(0x000A0005, numvalue);
		bool writeSlotCommit = pcie_mem->SendData(0x000A0005, (uint32_t)num);

		std::ostringstream oss;
		oss << "[PSCAN_LO_WRITE]"
			<< " slot=" << num
			<< " freqHz=" << freq
			<< " freqKHz=" << frequency
			<< " freqWord=0x" << std::hex << std::uppercase << std::setw(8) << std::setfill('0') << freqWord
			<< " errorWord=0x" << std::setw(8) << errorWord
			<< std::dec
			<< " writeA0006=" << (writeA0006 ? "OK" : "FAILED")
			<< " writeA0007=" << (writeA0007 ? "OK" : "FAILED")
			<< " writeSlotSelect=" << (writeSlotSelect ? "OK" : "FAILED")
			<< " writeSlotCommit=" << (writeSlotCommit ? "OK" : "FAILED");
		SweepLogicLog(oss.str().c_str());
	}
	catch (...)
	{
		SweepLogicLog("[PSCAN_LO_WRITE] exception");
	}
}

void SweepLogic::SetCFSpan(double CF, double span)
{
	Rf_Start = CF - 0.5 * span;
	Rf_Stop = CF + 0.5 * span;
	Span = span;
	CenterFrequency = CF; 
}

void SweepLogic::SetDetLen(uint32_t len)
{
	Det_Len = len;
}

void SweepLogic::SetCF(double CF)
{
	Rf_Start = CF - 0.5 * Span;
	Rf_Stop = CF + 0.5 * Span;
	CenterFrequency = CF; 
}

void SweepLogic::SetSpan(double span)
{
	Rf_Start = CenterFrequency - 0.5 * span;
	Rf_Stop = CenterFrequency + 0.5 * span;
	Span = span;
}

void SweepLogic::SetRBW(uint32_t Rbw)
{
	RBW = Rbw;
}

void SweepLogic::SetStep(double step)
{
	if (step > 0.0)
	{
		Step = step;
	}
}

double SweepLogic::GetSweepTime()
{
	return Sweep_Time;
}

void SweepLogic::SetSweepTime(double sweeptime)
{
	Sweep_Time_Config_Enable = 1;
	Sweep_Time_Config = sweeptime;
}

double SweepLogic::GetIFOffset()
{
	return SweepLogic::IF_Offset;
}

uint32_t SweepLogic::GetDataLen()
{
	return SweepLogic::Det_Len;
}

void SweepLogic::SetCalibrationData(const std::map<double, double>& calibrationData)
{
	FreqErrorValue = calibrationData;
}
