#pragma once

#include <cstdint>
#include <vector>

namespace Device
{
    class Device_MEM32;
}

namespace SWEEPCONFIG
{
    // //  TransComReceiver.DataService/Utilities/ 中 sweepLogic_fft.cs 的 C++ 移植版本。

    class SpectrumSweepFft
    {
    public:
        int Mode = 0; // 0=PSCAN, 1=FSCAN, 2=MSCAN
        double FC = 0.0;
        double FS = 409.6e6;  //ADC采样率
        double CenterFrequency = 2.4e9;
        double Step = 200e3;
        double CF = 0.06e9;
        double Span = 614.4e6;;///FFM 模式
        double IF_BW = 800e3;///代表FPGA一次性能实时“抓”进来进行 FFT 分析的有效频谱宽度，设定值为 80 MHz。
        double Decim = 0.0;
        double FS_ADC = 0.0;
        double FpgaIqRate = 204.8e6;
        double BW = 80e6;
        double RFRBW = 1000.0; // C# Global.RFRBW, Hz quantization step
        double Rf_Sub = 180e6;
        double Hold_time = 0.003;
        double Dwell_hold_time = 0.0;
        uint32_t Det_Len = 1001;
        uint32_t FTW_Hold_Num = 1;
        uint32_t VCO_Lock = 56250 * 2;
        uint32_t YTF_Lock = 1000;
        uint32_t F_Num = 3;
        uint32_t F_Sub_Num = 3;
        int32_t DataNum = 0;
        float Pow_thr = 20.72f;
        uint16_t Filt_Sel = 0;
        uint16_t FFT_Enable = 1;
        double Rf_Start = 0.0;
        double Rf_Stop = 0.0;
        std::vector<double> MscanFreqs;

        void Config();
        void UpdateAutoSweepTable(double uiFreqCenter, double uiSpan, double adcBw);
        void UpdateMscanSweepTable(const std::vector<double>& frequencies);
        double GetFreqErrorData(double frequency, uint32_t index) const;

    private:
        static constexpr double kClk = 4.8828125e-9;
        static constexpr double kInterHoldTime = 0.1;

        uint32_t FFT_Len_ = 2048;
        double FFT_BW_ = 320e6;
        double FFT_Bin_ = 0.0;
        uint16_t FFT_Sel_ = 0;
        uint16_t FFT_HB_Sel_ = 0;
        uint16_t FFT_HB_Shift_ = 0;
        uint16_t FFT_HB_Config_ = 0;
        uint16_t FFT_Trun_Len_ = 0;
        uint32_t Fir_Config_ = 0;
        uint32_t Det_Num_ = 1;
        uint32_t Det_Nums_ = 1;
        uint32_t Det_Rem_ = 0;
        uint32_t Det_Type_ = 3;
        uint32_t DMA_Hold_ = 1000;
        uint32_t ADC_Hold_ = 204800 * 2;
        uint16_t F_Step_Num_ = 0;
        uint32_t F_Rem_ = 0;
        uint64_t Dwell_hold_ = 0;
        uint32_t Dwell_hold_L32_ = 0;
        uint32_t Dwell_hold_H32_ = 0;
        uint32_t Det_Cal_Config_ = 0;
        uint32_t BW_Num_ = 0;
        uint32_t BW_Sub_Num_ = 0;
        uint32_t Filt_Order_ = 533;
        int Sort_ = 0;
        uint32_t FFT_HB_Decim_ = 0;
        uint32_t FTW_Hold_ = 0;
        double F_Step_ = 0.0;
        double F_Start_ = 0.0;
        double F_Stop_ = 0.0;
        double F_Start_Sub_ = 0.0;
        double F_Stop_Sub_ = 0.0;
        uint64_t FTW_Start_ = 0, FTW_Stop_ = 0, FTW_Step_ = 0;
        uint64_t FTW_Start_Sub_ = 0, FTW_Stop_Sub_ = 0;
        std::vector<uint16_t> Blackman_Win_;
        Device::Device_MEM32* pcieMem_ = nullptr;

        void CalculateParameters();
        void CalculateDetectFft();
        void ConfigureSweepParameter();
        void FrequencyDataList(double frequency, uint32_t number, double errorValue);
        void WriteFtwParts();
        static uint32_t Low32(uint64_t value);
        static uint32_t High16(uint64_t value);
    };
}
