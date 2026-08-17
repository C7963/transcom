#pragma once
#include "SweepLogic.h" 
#include "CommonManager.h" 

namespace SWEEPCONFIG {
    enum class DetectorType
    {
        AutoPeak,      // ˫ͨ�� High + Low
        PositivePeak,
        NegativePeak,
        RMS,
        Average,
        Sample
    };

    struct RefLevelResults
    {
        int Att;
        int FFTGainOffset;
    };

    class Sweep
    {
    public:
        Sweep();
        ~Sweep();
        static Sweep* getInstance() {
            if (instance == nullptr)
            {
                instance = new Sweep();
            }
            return instance;
        };
        static Sweep* instance;
        void InitPara(double CF, double Span, double RBW);
        void Config();
        void SetCFSpan(double CF, double Span);
        void SetCF(double CF);
        void SetSpan(double Span);
        void SetDetLen(uint32_t len);
        void SetRBW(uint32_t Rbw);
        void SetStep(double step);
        void SetSweepTime(double sweeptime);
        void TriggerSweep();
        RefLevelResults SetRefLevel(int reflevel);
        int GetAutoPeakData(std::vector<double>& HighData, std::vector<double>& LowData);
        bool GetAutoPeakData(std::vector<double>& outputBuffer);
        bool GetPositivePeakData(std::vector<double>& outputBuffer);
        bool GetNegativeData(std::vector<double>& outputBuffer);
        bool GetSampleData(std::vector<double>& outputBuffer);
        bool GetRMSData(std::vector<double>& outputBuffer);
        bool GetAverageData(std::vector<double>& outputBuffer);
        bool GetTraceData(DetectorType type, std::vector<double>& buf1, std::vector<double>& buf2);
        double GetSweepTimeBack();
        double GetIFOffset();
        uint32_t GetDataLen();
        void SetCalibrationData(std::map<double, double> FreqErrorValue);
        void UpdateCorrectValue(int RFATT);
        double GetCF_Offset() const { return sweep ? sweep->GetCF_Offset() : 0.0; }
        void SetTriggerOnNextRead(bool trigger) { triggerOnNextRead_ = trigger; }
        void SetReverseSpectrumData(bool reverse) { reverseSpectrumData_ = reverse; }
        bool GetReverseSpectrumData() const { return reverseSpectrumData_; }
        typedef int(*HardwareReadFunc)(unsigned char*, int);
        bool GetSingleChannelData(std::vector<double>& outputBuffer, HardwareReadFunc readFunc);

    private:
        void RefreshOutputPointCount();
        void ResetSpectrumFifos();
        void InvalidateTrigger() {}

        SWEEPLOGIC::SweepLogic* sweep;
        Common::CommonManager* common;
        std::mutex _bufferLock;
        std::vector<uint8_t> _sharedRawBuffer;
        int SetPoints = Global::SweepSpectrumPointCount;
        double CurrentSpan;
        double RBW = 200000;
        // 问题1修复：连续模式标志，避免每帧都触发TriggerSweep
        // Trigger the first read after configuration; continuous PSCAN reads the running FIFO afterwards.
        bool triggerOnNextRead_ = true;
        bool reverseSpectrumData_ = false; // C# PscanDataProvider maps FIFO data in raw order.
    };

}
