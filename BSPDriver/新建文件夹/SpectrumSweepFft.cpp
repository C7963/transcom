#include "SpectrumSweepFft.h"

#include "Device_MEM32.h"
#include "Global.h"

#include <cmath>

namespace SWEEPCONFIG
{
    namespace
    {
        constexpr double kPi = 3.14159265358979323846;

        uint64_t FrequencyToFtw(double frequency, double sampleRate)
        {
            return static_cast<uint64_t>(frequency / sampleRate * std::pow(2.0, 48));
        }

        double CSharpRound(double value)
        {
            return std::nearbyint(value);
        }
    }

    uint32_t SpectrumSweepFft::Low32(uint64_t value)
    {
        return static_cast<uint32_t>(value & 0xffffffffULL);
    }

    uint32_t SpectrumSweepFft::High16(uint64_t value)
    {
        return static_cast<uint32_t>((value >> 32) & 0xffffffffULL);
    }

    void SpectrumSweepFft::Config()
    {
        pcieMem_ = Device::Device_MEM32::getInstance();
        CalculateParameters();
        ConfigureSweepParameter();

        if (Mode == 2)
            UpdateMscanSweepTable(MscanFreqs);
        else
            UpdateAutoSweepTable(CenterFrequency, Span, BW);
    }

    void SpectrumSweepFft::CalculateDetectFft()
    {
        Det_Nums_ = static_cast<uint32_t>(std::floor(Span / FFT_Bin_) + 1);

        if (Det_Nums_ > Det_Len)
        {
            Det_Num_ = static_cast<uint32_t>(std::floor(static_cast<double>(Det_Nums_) / Det_Len));
            Det_Len = static_cast<uint32_t>(std::floor(static_cast<double>(Det_Nums_) / Det_Num_));
            F_Step_Num_ = static_cast<uint16_t>(std::ceil(BW / FFT_BW_));
            FFT_Trun_Len_ = static_cast<uint16_t>(std::ceil(BW / FFT_Bin_ / F_Step_Num_));
            BW = static_cast<double>(F_Step_Num_) * FFT_Trun_Len_ * FFT_Bin_;
            DataNum = static_cast<int>(F_Step_Num_ * FFT_Trun_Len_ * BW_Num_);
        }
        else
        {
            Det_Num_ = 1;
            F_Step_Num_ = 1;
            Det_Rem_ = 0;
            FFT_Trun_Len_ = static_cast<uint16_t>(std::ceil(BW / FFT_Bin_));
            BW = static_cast<double>(FFT_Trun_Len_) * FFT_Bin_;
            Det_Len = Det_Nums_;
            DataNum = static_cast<int>(FFT_Trun_Len_ * BW_Num_);
        }
        F_Step_ = BW / F_Step_Num_;
    }

    void SpectrumSweepFft::CalculateParameters()
    {
        if (Mode == 0)
        {
            FC = 0;
            FS = 409.6e6;
            FFT_Enable = 1;
            Sort_ = 1;
        }
        else
        {
            FC = FS_ADC / 4.0;
            FS = FS_ADC;
            FFT_Enable = 0;
            Sort_ = 0;
        }
        if (Mode == 2)
            Span = 0;

        if (BW >= Span)
        {
            BW_Num_ = 1;
            BW = Span;
        }
        else
        {
            BW_Num_ = static_cast<uint32_t>(std::ceil(Span / BW));
            BW = Span / BW_Num_;
        }

        if (FFT_Enable == 1)
        {
            if (Step == 200e3)
            {
                FFT_HB_Sel_ = 0;
                FFT_HB_Decim_ = 1;
                FFT_HB_Shift_ = 1;
                FFT_BW_ = 320e6;
                FFT_Sel_ = 3;
                FFT_Len_ = 2048;
            }
            else if (Step == 100e3)
            {
                FFT_HB_Sel_ = 0;
                FFT_HB_Decim_ = 1;
                FFT_HB_Shift_ = 1;
                FFT_BW_ = 320e6;
                FFT_Sel_ = 2;
                FFT_Len_ = 4096;
            }
            else if (Step == 50e3)
            {
                FFT_HB_Sel_ = 0;
                FFT_HB_Decim_ = 1;
                FFT_HB_Shift_ = 1;
                FFT_BW_ = 320e6;
                FFT_Sel_ = 1;
                FFT_Len_ = 8192;
            }
            else if (Step == 25e3)
            {
                FFT_HB_Sel_ = 0;
                FFT_HB_Decim_ = 1;
                FFT_HB_Shift_ = 1;
                FFT_BW_ = 320e6;
                FFT_Sel_ = 0;
                FFT_Len_ = 16384;
            }
            else if (Step == 12.5e3)
            {
                FFT_HB_Sel_ = 1;
                FFT_HB_Decim_ = 2;
                FFT_HB_Shift_ = 0;
                FFT_BW_ = 160e6;
                FFT_Sel_ = 0;
                FFT_Len_ = 16384;
            }
            else if (Step == 6.25e3)
            {
                FFT_HB_Sel_ = 2;
                FFT_HB_Decim_ = 4;
                FFT_HB_Shift_ = 0;
                FFT_BW_ = 80e6;
                FFT_Sel_ = 0;
                FFT_Len_ = 16384;
            }
            else if (Step == 3.125e3)
            {
                FFT_HB_Sel_ = 3;
                FFT_HB_Decim_ = 8;
                FFT_HB_Shift_ = 0;
                FFT_BW_ = 40e6;
                FFT_Sel_ = 0;
                FFT_Len_ = 16384;
            }
            FFT_Bin_ = FS / FFT_HB_Decim_ / FFT_Len_;
        }
        if (FFT_Enable == 0)
        {
            Decim = FpgaIqRate / IF_BW;
            if (Decim == 2) { Filt_Order_ = 97; Filt_Sel = 0; }
            else if (Decim == 4) { Filt_Order_ = 97 * 2; Filt_Sel = 1; }
            else if (Decim == 8) { Filt_Order_ = 97 * 4; Filt_Sel = 2; }
            else if (Decim == 16) { Filt_Order_ = 97 * 8; Filt_Sel = 3; }
            else if (Decim == 32) { Filt_Order_ = 97 * 16; Filt_Sel = 4; }
            else if (Decim == 80) { Filt_Order_ = 67 * 8; Filt_Sel = 5; }
            else if (Decim == 160) { Filt_Order_ = 97 * 80; Filt_Sel = 6; }
            else if (Decim == 320) { Filt_Order_ = 97 * 160; Filt_Sel = 7; }
            else if (Decim == 800) { Filt_Order_ = 67 * 80; Filt_Sel = 8; }
            else if (Decim == 1600) { Filt_Order_ = 97 * 800; Filt_Sel = 9; }
            else if (Decim == 3200) { Filt_Order_ = 97 * 1600; Filt_Sel = 10; }
            else if (Decim == 8000) { Filt_Order_ = 67 * 800; Filt_Sel = 11; }
            else if (Decim == 16000) { Filt_Order_ = 97 * 8000; Filt_Sel = 12; }
            else if (Decim == 32000) { Filt_Order_ = 97 * 16000; Filt_Sel = 13; }
            else if (Decim == 80000) { Filt_Order_ = 67 * 8000; Filt_Sel = 14; }
            else if (Decim == 160000) { Filt_Order_ = 97 * 80000; Filt_Sel = 15; }
            else { Filt_Order_ = 97; Filt_Sel = 0; }

            FTW_Hold_Num = static_cast<uint32_t>(CSharpRound(Hold_time / kClk));
            FTW_Hold_ = FTW_Hold_Num + static_cast<uint32_t>(Decim + 10);
            Dwell_hold_ = static_cast<uint64_t>(CSharpRound(Dwell_hold_time / kClk));
            Det_Num_ = static_cast<uint32_t>(std::floor(FTW_Hold_Num / Decim));
            Fir_Config_ = static_cast<uint32_t>(Filt_Sel * 65536.0 + std::ceil(static_cast<double>(Filt_Order_) / FTW_Hold_));
            if (Span != 0)
            {
                F_Step_ = Step;
                if (BW >= Span)
                {
                    F_Start_ = FC - BW / 2;
                    F_Stop_ = FC + BW / 2;
                }
                else if (Sort_ == 0)
                {
                    F_Start_ = FC - BW / 2 + F_Step_;
                    F_Stop_ = FC + BW / 2;
                }
                else
                {
                    F_Start_ = FC - BW / 2;
                    F_Stop_ = FC + BW / 2 - F_Step_;
                }
                if (Rf_Stop <= Rf_Sub)
                {
                    BW_Sub_Num_ = BW_Num_;
                    F_Start_Sub_ = Rf_Start;
                    F_Stop_Sub_ = Rf_Stop - F_Step_;
                }
                else if (Rf_Start < Rf_Sub)
                {
                    BW_Sub_Num_ = static_cast<uint32_t>(std::floor((Rf_Sub - Rf_Start + BW / 2) / BW));
                    if (BW_Sub_Num_ == 0)
                    {
                        F_Start_Sub_ = 0;
                        F_Stop_Sub_ = 0;
                    }
                    else
                    {
                        F_Start_Sub_ = Rf_Start;
                        F_Stop_Sub_ = Rf_Start + BW * BW_Sub_Num_ - F_Step_;
                    }
                }
                else
                {
                    BW_Sub_Num_ = 0;
                    F_Start_Sub_ = 0;
                    F_Stop_Sub_ = 0;
                }
                Det_Nums_ = static_cast<uint32_t>(std::floor(Span / F_Step_) + 1);
                F_Step_Num_ = static_cast<uint16_t>(std::floor((F_Stop_ - F_Start_) / F_Step_) + 1);
                F_Rem_ = Det_Nums_ - F_Step_Num_ * BW_Num_;
                if (BW >= Span)
                    BW = (F_Step_Num_ - 1) * F_Step_;
                else
                    BW = F_Step_Num_ * F_Step_;
            }
            else
            {
                F_Step_ = 200e3;
                F_Start_ = FC;
                F_Stop_ = FC;
                BW_Num_ = F_Num;
                BW_Sub_Num_ = F_Sub_Num;
                if (BW_Sub_Num_ != 0)
                {
                    F_Start_Sub_ = 20e6;
                    F_Stop_Sub_ = 20e6 + F_Step_ * (BW_Sub_Num_ - 1);
                }
                else
                {
                    F_Start_Sub_ = 0;
                    F_Stop_Sub_ = 0;
                }
                Det_Nums_ = F_Num;
                F_Step_Num_ = 1;
                F_Rem_ = 0;
            }
        }

        if (FFT_Enable == 1)
        {
            Fir_Config_ = 0;
            FFT_HB_Config_ = static_cast<uint16_t>(FFT_HB_Sel_ * 16 + FFT_HB_Shift_);
            FTW_Hold_Num = FFT_Len_; FTW_Hold_ = FTW_Hold_Num * FFT_HB_Decim_;
            CalculateDetectFft();
            F_Start_ = FC - BW / 2 * (1 - 1.0 / F_Step_Num_);
            F_Stop_ = FC + BW / 2 * (1 - 1.0 / F_Step_Num_);
            if (Rf_Stop <= Rf_Sub)
            {
                BW_Sub_Num_ = BW_Num_;
                F_Start_Sub_ = Rf_Start + BW / (2 * F_Step_Num_);
                F_Stop_Sub_ = Rf_Stop - BW / (2 * F_Step_Num_);
            }
            else if (Rf_Start < Rf_Sub)
            {
                BW_Sub_Num_ = static_cast<uint32_t>(std::floor((Rf_Sub - Rf_Start + BW / 2) / BW));
                if (BW_Sub_Num_ == 0)
                {
                    F_Start_Sub_ = 0;
                    F_Stop_Sub_ = 0;
                }
                else
                {
                    F_Start_Sub_ = Rf_Start + BW / (2 * F_Step_Num_);
                    F_Stop_Sub_ = Rf_Start + BW * BW_Sub_Num_ - BW / (2 * F_Step_Num_);
                }
            }
            else
            {
                BW_Sub_Num_ = 0;
                F_Start_Sub_ = 0;
                F_Stop_Sub_ = 0;
            }
            Blackman_Win_.assign(FFT_Len_, 0);
            for (uint32_t i = 0; i < FFT_Len_ / 2; ++i)
            {
                const double x = 0.35875 - 0.48829 * std::cos(2 * kPi * i / (FFT_Len_ - 1)) + 0.14128 * std::cos(4 * kPi * i / (FFT_Len_ - 1)) - 0.01168 * std::cos(6 * kPi * i / (FFT_Len_ - 1));
                Blackman_Win_[i] = Blackman_Win_[FFT_Len_ - 1 - i] = static_cast<uint16_t>(CSharpRound(x * 2047));
            }
        }

        Det_Cal_Config_ = 10;
        if (Mode == 0)
        {
            if (F_Start_ < 0) F_Start_ += FS;
            if (F_Stop_ < 0) F_Stop_ += FS;
            // The corresponding low-frequency conversion block is commented
            // out in BSPDriver/SweepLogic_fft.cs.
        }
        FTW_Start_ = FrequencyToFtw(F_Start_, FS); FTW_Stop_ = FrequencyToFtw(F_Stop_, FS); FTW_Step_ = FrequencyToFtw(F_Step_, FS);
        FTW_Start_Sub_ = FrequencyToFtw(F_Start_Sub_, FS); FTW_Stop_Sub_ = FrequencyToFtw(F_Stop_Sub_, FS);
        Dwell_hold_L32_ = Low32(Dwell_hold_); Dwell_hold_H32_ = High16(Dwell_hold_);
    }

    void SpectrumSweepFft::ConfigureSweepParameter()
    {
        uint32_t data[27] = { 0, ADC_Hold_, Low32(FTW_Start_), High16(FTW_Start_), Low32(FTW_Stop_), High16(FTW_Stop_), Low32(FTW_Step_), High16(FTW_Step_), FTW_Hold_, (BW_Sub_Num_ << 16) + BW_Num_, Det_Num_, Det_Len, Det_Rem_, Det_Type_, Fir_Config_, VCO_Lock, YTF_Lock, Det_Cal_Config_, FFT_HB_Config_, (static_cast<uint32_t>(FFT_Trun_Len_) << 16) + F_Step_Num_, (static_cast<uint32_t>(FFT_Enable) << 31) + (static_cast<uint32_t>(FFT_Sel_) << 17) + FFT_Len_, 0, 0, 0, DMA_Hold_, F_Step_Num_, F_Rem_ };
        pcieMem_->SendData(0x10010000, data, 27);
        pcieMem_->SendData(0x10010000 + 27, Pow_thr);
        pcieMem_->SendData(0x10010000 + 28, 0u);
        pcieMem_->SendData(0x10010000 + 29, Det_Nums_);
        pcieMem_->SendData(0x10010000 + 30, static_cast<uint32_t>(kInterHoldTime / kClk));
        pcieMem_->SendData(0x10010000 + 31, Low32(FTW_Start_Sub_));
        pcieMem_->SendData(0x10010000 + 32, High16(FTW_Start_Sub_));
        pcieMem_->SendData(0x10010000 + 33, Low32(FTW_Stop_Sub_));
        pcieMem_->SendData(0x10010000 + 34, High16(FTW_Stop_Sub_));
        pcieMem_->SendData(0x10010000 + 35, Dwell_hold_L32_);
        pcieMem_->SendData(0x10010000 + 36, Dwell_hold_H32_);
        if (FFT_Enable == 1)
        {
            std::vector<uint32_t> window(FFT_Len_ / 2);
            for (uint32_t i = 0; i < FFT_Len_ / 2; ++i)
                window[i] = (static_cast<uint32_t>(Blackman_Win_[i * 2 + 1]) << 16) + Blackman_Win_[i * 2];
            pcieMem_->SendData(0x10018000, window.data(), static_cast<int>(window.size()));
        }
        pcieMem_->SendData(0x10010000, static_cast<uint32_t>(Sort_ * 2 + 1));
    }

    void SpectrumSweepFft::UpdateAutoSweepTable(double center, double span, double adcBw)
    {
        double count;
        if (span == 0)
            count = 1;
        else
            count = span / adcBw;
        for (uint32_t i = 0; i < static_cast<uint32_t>(std::ceil(count)); ++i)
        {
            const double frequency = center - span / 2 + adcBw / 2 + i * adcBw;
            FrequencyDataList(frequency, i, GetFreqErrorData(frequency, i + 1));
        }
    }

    void SpectrumSweepFft::UpdateMscanSweepTable(const std::vector<double>& frequencies)
    {
        for (uint32_t i = 0; i < frequencies.size(); ++i)
            FrequencyDataList(frequencies[i], i, GetFreqErrorData(frequencies[i], i + 1));
    }

    double SpectrumSweepFft::GetFreqErrorData(double frequency, uint32_t) const
    {
        const auto& errors = Global::FreqErrorValue;
        if (errors.empty())
            return 0.0;
        auto upper = errors.lower_bound(frequency);
        if (upper != errors.end() && upper->first == frequency)
            return upper->second;
        if (upper == errors.begin() || upper == errors.end())
            return 0.0;
        auto lower = std::prev(upper);
        return lower->second + (upper->second - lower->second) *
            (frequency - lower->first) / (upper->first - lower->first);
    }

    void SpectrumSweepFft::FrequencyDataList(double frequency, uint32_t number, double errorValue)
    {
        try
        {
            const uint64_t quantizedFrequency = static_cast<uint64_t>(CSharpRound(frequency / RFRBW)) * static_cast<uint64_t>(RFRBW);
            const uint32_t frequencyWordLow = static_cast<uint32_t>(quantizedFrequency & 0xffffffffULL);
            const uint32_t frequencyWordHigh = static_cast<uint32_t>(quantizedFrequency >> 32);
            const uint32_t errorWord = static_cast<uint16_t>(std::pow(10.0, errorValue / 20.0 + 3.01));
            pcieMem_->SendData(0x00006002, frequencyWordLow);
            pcieMem_->SendData(0x00006003, frequencyWordHigh);
            pcieMem_->SendData(0x00006004, errorWord);
            uint32_t selectWord;
            if (Mode == 0)
                selectWord = 0x80000000u + number;
            else
                selectWord = 0x40000000u + number;
            pcieMem_->SendData(0x00006001, selectWord);
            pcieMem_->SendData(0x00006001, number);
        }
        catch (...)
        {
        }
    }
}
