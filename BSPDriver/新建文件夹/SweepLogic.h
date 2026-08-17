#pragma once
#ifndef __SWEEPLOGIC_H__
#define __SWEEPLOGIC_H__

#include <cstdint> 
#include <cmath>
#include <map>

namespace SWEEPLOGIC
{
	class SweepLogic
	{
	private:
		
		uint32_t Det_Num = 1;
		int Det_Num_Min;
		int Det_Num_Max = (uint32_t)(std::pow(2, 32) - 1);
		uint64_t Det_Nums_Min;
		uint64_t Det_Nums_Max;
		uint32_t Det_Rem = 0;
		uint32_t Det_Type = 3; // RMS detection (aligned with C# production client)
		uint32_t F_Step_Num = 0;
		uint32_t F_Rem = 0;
		uint32_t Det_Cal_EN = 1;
		uint32_t Det_Cal_Shift = 10;
		uint32_t Det_Cal_Config;
		uint32_t BW_Num;
		uint16_t Filt_Taps;
		uint16_t Filt_Latency;
		uint16_t Filt_Delay;
		uint16_t Filt_Sel;
		uint32_t Filt_Config;
		double F_Decim;
		uint16_t CIC_Taps = 3;
		uint32_t CIC_Decim;
		uint32_t CIC_Shift;
		uint32_t CIC_Config;
		uint32_t CIC_Delay;
		uint32_t FTW_Hold;
		double F_Step;
		double F_Step_Max;
		double F_Step_K = 5;
		double F_Step_Min = FS / std::pow(2, 48);
		double F_Start;
		double F_Stop;
		uint64_t FTW_Start;
		uint32_t FTW_Start_L32;
		uint32_t FTW_Start_H16;
		uint64_t FTW_Stop;
		uint32_t FTW_Stop_L32;
		uint32_t FTW_Stop_H16;
		uint64_t FTW_Step;
		uint32_t FTW_Step_L32;
		uint32_t FTW_Step_H16;
		double F_Start_Sub;
		double F_Stop_Sub;
		uint32_t BW_Sub_Num;
		uint64_t FTW_Start_Sub;
		uint32_t FTW_Start_Sub_H16;
		uint32_t FTW_Start_Sub_L32;
		uint64_t FTW_Stop_Sub;
		uint32_t FTW_Stop_Sub_H16;
		uint32_t FTW_Stop_Sub_L32;

		// FFT parameters (aligned with C# PSCAN mode)
		uint16_t FFT_Enable = 1;
		uint32_t FFT_Len = 2048;
		double FFT_BW = 320e6;
		double FFT_Bin = 0;
		uint16_t FFT_Sel = 3;
		uint16_t FFT_HB_Sel = 0;
		uint16_t FFT_HB_Shift = 1;
		uint16_t FFT_HB_Decim = 1;
		uint16_t FFT_HB_Config = 0;
		uint16_t FFT_Trun_Len = 0;
		uint32_t DMA_Hold = 1000;
		uint32_t ADC_Hold = 204800 * 2; // 2ms
		double Hold_time = 0.003;
		double Dwell_hold_time = 0;
		double Inter_hold_time = 0.1;
		const double Clk = 4.8828125e-9;
		uint64_t Dwell_hold = 0;
		uint32_t Dwell_hold_L32 = 0;
		uint32_t Dwell_hold_H32 = 0;
		uint16_t Blackman_Win[16384] = {};
		float Pow_thr = 20.72f;

	public:
		std::map<double, double> FreqErrorValue;
		int Sort = 2;  
		int Mode = 0;
		double FC = 0;
		double FS = 409.6e6;           // Aligned with C# PSCAN mode
		double CenterFrequency = 2E9;
		double Span = 50E6;
		uint32_t RBW = 200000;
		double Decim;
		double bw = 320e6;             // Aligned with C# BW=320e6
		double bw_last = 320e6;
		double IF_Offset = 40E6;
		double CF_Offset = 0;
		double Step = 200e3;           // Default Step 200kHz (FFT_Sel=3, FFT_Len=2048)
		double Sweep_Time_Min = 10e-2;
		double Sweep_Time;
		double Sweep_Time_Config;
		int Sweep_Time_Config_Enable;
		uint64_t Det_Nums = 1;
		uint32_t Det_Len = 1001;
		uint32_t FTW_Hold_Num = 1;
		uint32_t VCO_Lock = 56250 * 2; // 300us (aligned with C#)
		uint32_t YTF_Lock = 1000;      // (aligned with C#)
		uint16_t* BW_Cal_Coef;
		int Detect_Type;
		double Rf_Start;
		double Rf_Stop;
		double Rf_Sub = 180e6;         // Aligned with C# Rf_Sub

		void Config();
		void SetSweepOffset();
		void CaculateDetect();
		void CaculateDetectFFT();
		void CaculateParas();
		void ConfigSweepParameter();
		void TriggerSweep();
		void updateAutosweepTable(double ui_freq_center, double ui_span, double bw, double bw_last, uint32_t bw_num);
		double GetFreqErrorData(double freq);
		void FrequencyDataList(double freq, int num, double errorvalue);
		void SetCFSpan(double CF, double span);
		void SetDetLen(uint32_t len);
		void SetCF(double CF);
		void SetSpan(double span);
		void SetRBW(uint32_t Rbw);
		void SetStep(double step);
		void SetSweepTime(double sweeptime);
		double GetSweepTime(); 
		double GetIFOffset();
		uint32_t GetDataLen();
		double GetFStart() const { return F_Start; }
		double GetFStop() const { return F_Stop; }
		double GetFStep() const { return F_Step; }
		double GetBw() const { return bw; }
		double GetBwLast() const { return bw_last; }
		double GetCF_Offset() const { return CF_Offset; }
		void SetCalibrationData(const std::map<double, double>& calibrationData);

		static SweepLogic* Instance;
		static SweepLogic* GetInstance() {
			if (Instance == nullptr) {
				Instance = new SweepLogic();
			}
			return Instance;
		}
	};
}
#endif
