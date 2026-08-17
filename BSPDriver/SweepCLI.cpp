#include "SweepClI.h" 
#include "CommonCLI.h" 
#include "PCIE_SweepData.h"


using namespace SWEEPLOGIC; 
using namespace SWEEPCONFIG;
 
SweepCli::SweepCli()
{
    //sweepLogic = new SweepLogic();
    sweep = Sweep::getInstance();
    //sweep = new Sweep();
    _sharedRawBuffer = gcnew cli::array<System::Byte>(1001 * 4);
    _bufferLock = gcnew Object();
    //common_= Common::CommonManager::getInstance(); 
    SetCalibrationData(Global::FreqErrorValue); 
}
//double RBW;
//double CurrentSpan;
SweepCli::~SweepCli()
{
    delete sweep;
    //delete sweepLogic;
    //delete common_;
}

void SweepCli::InitPara(double CF, double Span, double Rbw)
{
    /*sweepLogic->SetCFSpan(CF, Span);
    sweepLogic->SetRBW(RBW);
    sweepLogic->Config();*/
    sweep->InitPara(CF, Span, Rbw);
    //CurrentSpan = Span;
    //RBW = Rbw;
}

// Sweeplogic 方法实现
void SweepCli::Config() {
    //sweepLogic->Config(); 
    sweep->Config();
}

void SweepCli::SetCFSpan(double CF, double Span) {
   /* sweepLogic->SetCFSpan(CF, Span);
    sweepLogic->Config();*/
    sweep->SetCFSpan(CF, Span);
    sweep->Config();
    //CurrentSpan = Span;
}

void SweepCli::SetCF(double CF) {
   /* sweepLogic->SetCF(CF);
    sweepLogic->Config();*/
    sweep->SetCF(CF);
    sweep->Config();
}


void SweepCli::SetSpan(double Span) {
   /* sweepLogic->SetSpan(Span);
    sweepLogic->Config(); */
    sweep->SetSpan(Span);
    sweep->Config();
    //CurrentSpan = Span;
}
void SweepCli::SetDetLen(uint32_t len) {
 /*   sweepLogic->SetDetLen(len);
    sweepLogic->Config();*/
    sweep->SetDetLen(len);
    sweep->Config();
}
void SweepCli::SetRBW(uint32_t Rbw) {
    //RBW = Rbw;
   /* sweepLogic->SetRBW(Rbw);
    sweepLogic->Config(); */
    sweep->SetRBW(Rbw);
    sweep->Config();
}

void SweepCli::SetSweepTime(double sweeptime) {
    /*sweepLogic->SetSweepTime(sweeptime);
    sweepLogic->Config();*/
    sweep->SetSweepTime(sweeptime);
    sweep->Config();
}

 

CommonCli::RefLevelResultsCli SweepCli::SetRefLevel(int reflevel)
{
    //auto nativeResult = common_->rfControl_->SetRefLevel(reflevel);
    auto nativeResult = sweep->SetRefLevel(reflevel);
    CommonCli::RefLevelResultsCli result{ 0,0 };
    result.Att = nativeResult.Att;
    result.FFTGainOffset = nativeResult.FFTGainOffset; 
    Global::RefLevel = reflevel;  
    UpdateCorrectValue(nativeResult.Att);
    return result;
}
double SweepCli::GetSweepTimeBack() {
    //return sweepLogic->GetSweepTime();
    return sweep->GetSweepTimeBack();
}

double SweepCli::GetIFOffset() {
    //return sweepLogic->GetIFOffset();
    return sweep->GetIFOffset();
}

uint32_t SweepCli::GetDataLen() {
    //return sweepLogic->GetDataLen();
    return sweep->GetDataLen();
}

void SweepCli::SetCalibrationData(std::map<double, double> FreqErrorValue)
{ 
    //sweepLogic->SetCalibrationData(FreqErrorValue);
    sweep->SetCalibrationData(FreqErrorValue);
}

void SweepCli::UpdateCorrectValue(int RFATT) {
    //if (Global::RefLevel >= -50)
    //{
    //    auto IFATT = common_->rfControl_->GetIFATT();
    //    int delta = (int)IFATT - 10; 
    //    Global::CorrectValue = RFATT + delta;

    //}
    //else
    //{ 
    //     Global::CorrectValue = -25; //去除低噪放增益以及DDC增益 
    //}
    sweep->UpdateCorrectValue(RFATT);
}

// 核心私有辅助函数：单路检波通用实现
bool SweepCli::GetSingleChannelData(cli::array<double>^% outputBuffer, HardwareReadFunc readFunc)
{
    // 1. 确保托管数组已初始化（模拟原有的分配逻辑）
    int setPoints = sweep->GetDataLen(); // 假设底层 Sweep 有这个方法获取点数
    if (outputBuffer == nullptr || outputBuffer->Length < setPoints)
    {
        outputBuffer = gcnew cli::array<double>(setPoints);
    }

    // 2. 准备一个临时的原生 std::vector
    // 虽然可以直接用托管指针，但为了保证 C++ 底层的线程安全和 std::vector 接口兼容，
    // 建议准备一个临时 vector
    std::vector<double> nativeBuffer(outputBuffer->Length);

    // 3. 调用纯 C++ 层的逻辑
    bool result = sweep->GetSingleChannelData(nativeBuffer, readFunc);

    // 4. 如果执行成功，将结果从 std::vector 拷贝回 cli::array
    if (result)
    {
        // 使用 Runtime 提供的高效拷贝方法
        Runtime::InteropServices::Marshal::Copy(
            IntPtr(nativeBuffer.data()), // 源：原生指针
            outputBuffer,                // 目的：托管数组
            0,                           // 起始索引
            nativeBuffer.size()          // 长度
        );
    }

    return result;
}
 

bool SweepCli::GetPositivePeakData(cli::array<double>^% outputBuffer)
{
    // 传递 Wrapper 函数或静态成员函数指针
    return GetSingleChannelData(outputBuffer, &HardwareWrappers::ReadPos);
}

bool SweepCli::GetNegativeData(cli::array<double>^% outputBuffer)
{
    return GetSingleChannelData(outputBuffer, &HardwareWrappers::ReadNeg);
}

bool SweepCli::GetSampleData(cli::array<double>^% outputBuffer)
{
    return GetSingleChannelData(outputBuffer, &HardwareWrappers::ReadSmp);
}

bool SweepCli::GetRMSData(cli::array<double>^% outputBuffer)
{
    return GetSingleChannelData(outputBuffer, &HardwareWrappers::ReadRMS);
}

bool SweepCli::GetAverageData(cli::array<double>^% outputBuffer)
{
    return GetSingleChannelData(outputBuffer, &HardwareWrappers::ReadAvg);
}

bool SweepCli::GetAutoPeakData(cli::array<double>^% HighData, cli::array<double>^% LowData)
{
    // 1. 参数检查
    int setPoints = sweep->GetDataLen(); // 获取 C++ 层定义的点数
    if (HighData == nullptr || HighData->Length < setPoints ||
        LowData == nullptr || LowData->Length < setPoints)
    {
        throw gcnew System::ArgumentException("HighData/LowData buffer size is insufficient.");
    }

    // 2. 准备纯 C++ 层的临时容器
    std::vector<double> nativeHigh(setPoints);
    std::vector<double> nativeLow(setPoints);

    // 3. 调用纯 C++ 底层逻辑
    // 所有的锁、硬件读取、Log计算、插值算法都在 sweep->GetAutoPeakData 内部完成
    int readpoints = sweep->GetAutoPeakData(nativeHigh, nativeLow);

    // 4. 数据封送：从 std::vector 拷贝回托管数组
    if (readpoints>0)
    {
        // 拷贝 High 数据
        System::Runtime::InteropServices::Marshal::Copy(
            System::IntPtr(nativeHigh.data()),
            HighData,
            0,
            readpoints
        );

        // 拷贝 Low 数据
        System::Runtime::InteropServices::Marshal::Copy(
            System::IntPtr(nativeLow.data()),
            LowData,
            0,
            readpoints
        );
        return false; 
    }

    return false;
}

bool SweepCli::GetAutoPeakData(cli::array<double>^% outputBuffer)
{
    int setPoints = sweep->GetDataLen();
    int requiredSize = setPoints * 2;

    if (outputBuffer == nullptr || outputBuffer->Length < requiredSize)
    {
        outputBuffer = gcnew cli::array<double>(requiredSize);
    }

    std::vector<double> nativeBuffer(requiredSize);

    // 调用纯 C++ 底层
    bool result = sweep->GetAutoPeakData(nativeBuffer);

    if (result)
    {
        Runtime::InteropServices::Marshal::Copy(
            IntPtr(nativeBuffer.data()),
            outputBuffer,
            0,
            nativeBuffer.size()
        );
    }
    return result;
}

bool SweepCli::GetTraceData(SWEEPCONFIG::DetectorType type, cli::array<double>^% buf1, cli::array<double>^% buf2)
{
    switch (type)
    {
    case SWEEPCONFIG::DetectorType::AutoPeak:
        // AutoPeak 需要两个缓冲区
        return GetAutoPeakData(buf1, buf2);
    case SWEEPCONFIG::DetectorType::PositivePeak:
        return GetPositivePeakData(buf1);
    case SWEEPCONFIG::DetectorType::NegativePeak:
        return GetNegativeData(buf1);
    case SWEEPCONFIG::DetectorType::Sample:
        return GetSampleData(buf1);
    case SWEEPCONFIG::DetectorType::RMS:
        return GetRMSData(buf1);
    case SWEEPCONFIG::DetectorType::Average:
        return GetAverageData(buf1);
    default:
        return false;
    }
}
