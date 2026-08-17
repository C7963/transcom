#pragma once
#include "SweepLogic.h" 
#include "CommonManager.h" 
#include "CommonCLI.h"
#include "Sweep.h"

using namespace System;
using namespace System::Collections::Generic;
using namespace CommonControlCLI;
using namespace SWEEPCONFIG;

enum class DetectorType
{
    AutoPeak,      // 双通道 High + Low
    PositivePeak,
    NegativePeak,
    RMS,
    Average,
    Sample
};
public ref class SweepCli
{
public:
    SweepCli();
    ~SweepCli();

    void InitPara(double CF, double Span, double Rbw);
    void Config();
    void SetCFSpan(double CF, double Span);
    void SetCF(double CF);
    void SetSpan(double Span);
    void SetDetLen(uint32_t len);
    void SetRBW(uint32_t Rbw);
    void SetSweepTime(double sweeptime); 
    CommonCli::RefLevelResultsCli SetRefLevel(int reflevel);
    double GetSweepTimeBack();
    double GetIFOffset();
    uint32_t GetDataLen();
    void SetCalibrationData(std::map<double, double> FreqErrorValue);
    void UpdateCorrectValue(int RFATT);

    bool GetAutoPeakData(cli::array<double>^% HighData, cli::array<double>^% LowData); 
    bool GetAutoPeakData(cli::array<double>^% outputBuffer);
    bool GetPositivePeakData(cli::array<double>^% outputBuffer); 
    bool GetNegativeData(cli::array<double>^% outputBuffer); 
    bool GetSampleData(cli::array<double>^% outputBuffer); 
    bool GetRMSData(cli::array<double>^% outputBuffer); 
    bool GetAverageData(cli::array<double>^% outputBuffer); 
    bool GetTraceData(SWEEPCONFIG::DetectorType type, cli::array<double>^% buf1, cli::array<double>^% buf2);

   

private: 

    // 共享的原始字节缓冲区，避免为每个检波器分配单独内存
     // 只要加了锁，复用一个 buffer 既省内存又对 CPU 缓存友好
    cli::array<System::Byte>^ _sharedRawBuffer;
     
    // 定义硬件读取函数的函数指针类型 (Native C++ function signature)
    // 假设 ReadSpectrumData 签名是 int(unsigned char*, int)
    typedef int(*HardwareReadFunc)(unsigned char*, int);
     
    bool GetSingleChannelData(cli::array<double>^% outputBuffer, HardwareReadFunc readFunc);
    System::Object^ _bufferLock;  
    static const int SetPoints = Global::SweepSpectrumPointCount;

    //SWEEPLOGIC::SweepLogic* sweepLogic;
    SWEEPCONFIG::Sweep* sweep;
    //Common::CommonManager* common_;
};