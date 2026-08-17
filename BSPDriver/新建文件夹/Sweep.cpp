#include "Sweep.h" 
#include "PCIE_SweepData.h"
#include <fstream>
#include <sstream>
#include <thread>
#include <chrono>
#include <limits>
#include <algorithm>
#include <windows.h>

// Sweep内部诊断日志（写到DLL所在目录）
static std::string GetSweepDllDir()
{
    char path[MAX_PATH] = { 0 };
    HMODULE hMod = nullptr;
    if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        (LPCSTR)&GetSweepDllDir, &hMod))
    {
        GetModuleFileNameA(hMod, path, MAX_PATH);
    }
    std::string s(path);
    auto pos = s.find_last_of("\\/");
    return (pos != std::string::npos) ? s.substr(0, pos) : ".";
}

static void SWLog(const char* msg)
{
    std::string logPath = GetSweepDllDir() + "\\bsdriver_diag.log";
    std::ofstream log(logPath, std::ios::app);
    if (log.is_open())
    {
        log << msg << std::endl;
    }
}

using namespace SWEEPLOGIC;
using namespace SWEEPCONFIG;

Sweep* Sweep::instance = nullptr;
SweepLogic* SweepLogic::Instance = nullptr;

Sweep::Sweep()
{
    SWLog("[SWEEP_CTOR] begin");
    SWLog("[SWEEP_CTOR] stage=SweepLogic::GetInstance begin");
    sweep = SweepLogic::GetInstance();
    SWLog("[SWEEP_CTOR] stage=SweepLogic::GetInstance done");

    SWLog("[SWEEP_CTOR] stage=CommonManager::getInstance begin");
    common = Common::CommonManager::getInstance();
    SWLog("[SWEEP_CTOR] stage=CommonManager::getInstance done");

    SWLog("[SWEEP_CTOR] stage=SetCalibrationData begin");
    SetCalibrationData(Global::FreqErrorValue);
    SWLog("[SWEEP_CTOR] stage=SetCalibrationData done");

    // Win_API initializes SWEEP mode here after CommonManager::InitDevice().
    // This configures the ADC NCO, DCM and SWEEP filter coefficients.
    common->SetWorkMode(Global::WorkMode::SWEEP);
    SWLog("[SWEEP_CTOR] stage=SetWorkMode(SWEEP) done (Win_API-compatible)");
    SWLog("[SWEEP_CTOR] success");
}
Sweep::~Sweep()
{

}

void Sweep::RefreshOutputPointCount()
{
    SetPoints = static_cast<int>(Global::SweepSpectrumPointCount);
    if (SetPoints < 2)
    {
        SetPoints = 2;
    }
}

void Sweep::ResetSpectrumFifos()
{
    HardwareWrappers::ResetPos();
    HardwareWrappers::ResetNeg();
    HardwareWrappers::ResetSmp();
    HardwareWrappers::ResetRMS();
    HardwareWrappers::ResetAvg();
}

void Sweep::InitPara(double CF, double Span, double Rbw)
{
    sweep->SetCFSpan(CF, Span);
    sweep->SetRBW(Rbw);
    sweep->SetStep(Rbw);
    Config();
    CurrentSpan = Span;
    RBW = Rbw;
    InvalidateTrigger();

    std::ostringstream oss;
    oss << "[InitPara] CF=" << CF << " Span=" << Span << " RBW=" << Rbw
        << " SetPoints=" << SetPoints << " FIFO reset done"
        << " F_Start=" << sweep->GetFStart()
        << " F_Stop=" << sweep->GetFStop()
        << " F_Step=" << sweep->GetFStep()
        << " Det_Nums=" << sweep->Det_Nums
        << " Det_Len=" << sweep->Det_Len
        << " Rf_Start=" << sweep->Rf_Start
        << " Rf_Stop=" << sweep->Rf_Stop
        << " bw=" << sweep->GetBw();
    SWLog(oss.str().c_str());
}

void Sweep::Config() {
    InvalidateTrigger();
    ResetSpectrumFifos();
    sweep->Config();
    // SweepLogic::Config() performs the single C#-compatible trigger after
    // all sweep registers and the FFT window have been written.
    triggerOnNextRead_ = false;
    RefreshOutputPointCount();
}

void Sweep::SetCFSpan(double CF, double Span) {
    sweep->SetCFSpan(CF, Span);
    CurrentSpan = Span;
}

void Sweep::SetCF(double CF) {
    sweep->SetCF(CF);
}

void Sweep::SetSpan(double Span) {
    sweep->SetSpan(Span);
    CurrentSpan = Span;
}

void Sweep::SetDetLen(uint32_t len) {
    sweep->SetDetLen(len);
}

void Sweep::SetRBW(uint32_t Rbw) {
    RBW = Rbw;
    sweep->SetRBW(Rbw);
}

void Sweep::SetStep(double step) {
    sweep->SetStep(step);
}

void Sweep::SetSweepTime(double sweeptime) {
    sweep->SetSweepTime(sweeptime);
}

void Sweep::TriggerSweep() {
    sweep->TriggerSweep();
}

RefLevelResults Sweep::SetRefLevel(int reflevel)
{
    auto nativeResult = common->rfControl_->SetRefLevel(reflevel);
    RefLevelResults result{ 0,0 };
    result.Att = nativeResult.Att;
    result.FFTGainOffset = nativeResult.FFTGainOffset;
    Global::RefLevel = reflevel;
    UpdateCorrectValue(nativeResult.Att);
    return result;
}

double Sweep::GetSweepTimeBack() {
    return sweep->GetSweepTime();
}

double Sweep::GetIFOffset() {
    return sweep->GetIFOffset();
}

uint32_t Sweep::GetDataLen() {
    return sweep->GetDataLen();
}

void Sweep::SetCalibrationData(std::map<double, double> FreqErrorValue)
{
    sweep->SetCalibrationData(FreqErrorValue);
}

void Sweep::UpdateCorrectValue(int RFATT) {
    if (Global::RefLevel >= -50)
    {
        auto IFATT = common->rfControl_->GetIFATT();
        int delta = (int)IFATT - 10;
        Global::CorrectValue = RFATT + delta;

    }
    else
    {
        Global::CorrectValue = -25;
    }
}

bool Sweep::GetSingleChannelData(std::vector<double>& outputBuffer, HardwareReadFunc readFunc)
{
    // 1. Parameter Check & Resizing
    if (outputBuffer.size() < (size_t)SetPoints)
    {
        outputBuffer.resize(SetPoints);
    }

    // 2. Thread Safety
    std::lock_guard<std::mutex> lock(_bufferLock);

    // 3. 计算期望的硬件点�?
    uint64_t hwPoints = sweep->Det_Nums;
    if (hwPoints < 1) hwPoints = 1;
    if (hwPoints > 100000) hwPoints = 100000;

    const int expectedPoints = static_cast<int>(hwPoints);
    const int expectedBytes = expectedPoints * 4;

    if (_sharedRawBuffer.size() < (size_t)expectedBytes)
    {
        _sharedRawBuffer.resize(expectedBytes);
    }

    // ========== 时序追踪日志 ==========
    static int g_traceCount = 0;
    auto t_start = std::chrono::high_resolution_clock::now();

    if (g_traceCount < 10 || g_traceCount % 20 == 0) {
        std::ostringstream oss;
        oss << "[TRACE][" << g_traceCount << "] GetSingleChannelData ENTER"
            << " hwPoints=" << hwPoints << " expectedPoints=" << expectedPoints
            << " SetPoints=" << SetPoints << " RBW=" << RBW
            << " CurrentSpan=" << CurrentSpan
            << " triggerOnNextRead=" << (triggerOnNextRead_ ? "YES" : "NO");
        SWLog(oss.str().c_str());
    }
    // C# triggers once at the end of ConfigSweepParameter(); continuous Pscan
    // frames only consume the already-running sweep FIFO. Single mode can set
    // triggerOnNextRead_ to request one explicit trigger.
    bool triggeredThisFrame = false;
    long long trigger_us = 0;
    if (triggerOnNextRead_)
    {
        auto t_trigger = std::chrono::high_resolution_clock::now();
        sweep->TriggerSweep();
        auto t_after_trigger = std::chrono::high_resolution_clock::now();
        trigger_us = std::chrono::duration_cast<std::chrono::microseconds>(
            t_after_trigger - t_trigger).count();
        triggerOnNextRead_ = false;
        triggeredThisFrame = true;
    }
    if (g_traceCount < 10 || g_traceCount % 20 == 0) {
        std::ostringstream oss;
        oss << "[PSCAN_FRAME_TRIGGER] frame=" << g_traceCount
            << " triggered=" << (triggeredThisFrame ? "YES" : "NO")
            << " trigger_us=" << trigger_us
            << " config_triggered_once=YES";
        SWLog(oss.str().c_str());
    }

    // Match the C# acquisition path: consume the continuous FIFO immediately
    // and let read_data() block until one complete frame is available.
    if (g_traceCount < 10 || g_traceCount % 20 == 0) {
        std::ostringstream oss;
        oss << "[TRACE][" << g_traceCount << "] FIFO read"
            << " wait_mode=blocking_read wait_ms=0";
        SWLog(oss.str().c_str());
    }

    const auto t_read_start = std::chrono::high_resolution_clock::now();
    const int readBytes = readFunc(_sharedRawBuffer.data(), expectedPoints);
    const auto t_read_end = std::chrono::high_resolution_clock::now();
    const auto read_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t_read_end - t_read_start).count();

    if (readBytes != expectedBytes || (readBytes % 4) != 0)
    {
        auto total_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t_read_end - t_start).count();
        std::ostringstream oss;
        oss << "[TRACE][" << g_traceCount << "] FAIL: readBytes=" << readBytes
            << " expectedBytes=" << expectedBytes << " read_ms=" << read_ms
            << " total_ms=" << total_ms;
        SWLog(oss.str().c_str());
        g_traceCount++;
        return false;
    }

    int readPoints = readBytes / 4;
    uint32_t* pData = reinterpret_cast<uint32_t*>(_sharedRawBuffer.data());

    // Detect stale/repeated FIFO frames without changing the acquired data.
    static std::vector<uint32_t> previousRawFrame;
    static uint64_t previousRawHash = 0;
    static uint64_t consecutiveRepeatFrames = 0;
    uint64_t rawHash = 1469598103934665603ULL;
    int identicalPoints = 0;
    for (int i = 0; i < readPoints; ++i)
    {
        rawHash ^= static_cast<uint64_t>(pData[i]);
        rawHash *= 1099511628211ULL;
        if (previousRawFrame.size() == static_cast<size_t>(readPoints) &&
            previousRawFrame[static_cast<size_t>(i)] == pData[i])
        {
            ++identicalPoints;
        }
    }

    const bool sameAsPrevious =
        previousRawFrame.size() == static_cast<size_t>(readPoints) &&
        identicalPoints == readPoints && rawHash == previousRawHash;
    consecutiveRepeatFrames = sameAsPrevious ? consecutiveRepeatFrames + 1 : 0;

    if (g_traceCount < 10 || g_traceCount % 20 == 0 ||
        (sameAsPrevious && (consecutiveRepeatFrames <= 3 || consecutiveRepeatFrames % 20 == 0)))
    {
        std::ostringstream oss;
        oss << "[PSCAN_FIFO_FRAME] frame=" << g_traceCount
            << " raw_hash=0x" << std::hex << rawHash << std::dec
            << " identical_points=" << identicalPoints << "/" << readPoints
            << " same_as_previous=" << (sameAsPrevious ? "YES" : "NO")
            << " consecutive_repeats=" << consecutiveRepeatFrames
            << " read_ms=" << read_ms;
        SWLog(oss.str().c_str());
    }

    previousRawFrame.assign(pData, pData + readPoints);
    previousRawHash = rawHash;

    // 7. Calculation Parameters
    double correction = -Global::SweepBaseErrorValue + Global::CorrectValue + Global::RbwErrDIC[(uint32_t)RBW];

    // 8. 数据转换：原始ADC�?-> dBm
    int zeroCount = 0;
    std::vector<double> tempDb(readPoints);
    
    // 查找原始ADC数据中的峰值位�?
    uint32_t maxAdcVal = 0;
    int maxAdcIdx = 0;
    for (int i = 0; i < readPoints; ++i)
    {
        uint32_t v = pData[i];
        if (v > maxAdcVal)
        {
            maxAdcVal = v;
            maxAdcIdx = i;
        }
    }
    
    for (int i = 0; i < readPoints; ++i)
    {
        uint32_t v = pData[i];
        if (v == 0)
        {
            tempDb[i] = -300.0;
            zeroCount++;
        }
        else
        {
            tempDb[i] = 20.0 * std::log10((double)v) + correction;
        }
    }

    // 问题2修复：反转数据数�?
    // 硬件从高中频向低中频降频扫描（F_Start > F_Stop�?
    // 数据索引0=最高中�?最低射频，索引末尾=最低中�?最高射�?
    // 显示要求：左=低频，右=高频（索�?=低频，末�?高频�?
    // 反转后索�?=最低射频（左），索引末�?最高射频（右）
    if (reverseSpectrumData_)
    {
        std::reverse(tempDb.begin(), tempDb.begin() + readPoints);
    }
    
    // 查找dB数据中的峰值位置（反转后）
    double maxDbVal = -999.0;
    int maxDbIdx = 0;
    double minDbVal = 999.0;
    int minDbIdx = 0;
    for (int i = 0; i < readPoints; ++i)
    {
        if (tempDb[i] > maxDbVal) { maxDbVal = tempDb[i]; maxDbIdx = i; }
        if (tempDb[i] < minDbVal) { minDbVal = tempDb[i]; minDbIdx = i; }
    }

    // 9. Item 4 fix: 线性插值替代三次样条（对齐C#不重采样的理念，避免峰值失真）
    // C# 直接�?Det_Nums �?:1映射，不做重采样
    // C++ 仍需映射�?SetPoints(1001)，但用线性插值替代三次样条以保持峰值保真度
    if (readPoints >= SetPoints)
    {
        // 降采样：线性插�?
        for (int i = 0; i < SetPoints; ++i)
        {
            double srcIdx = (double)i * (double)(readPoints - 1) / (double)(SetPoints - 1);
            int idx0 = (int)srcIdx;
            int idx1 = (idx0 + 1 < readPoints) ? idx0 + 1 : idx0;
            double frac = srcIdx - (double)idx0;
            outputBuffer[i] = tempDb[idx0] * (1.0 - frac) + tempDb[idx1] * frac;
        }
    }
    else if (readPoints > 1)
    {
        // 升采样：线性插�?
        for (int i = 0; i < SetPoints; ++i)
        {
            double srcIdx = (double)i * (double)(readPoints - 1) / (double)(SetPoints - 1);
            int idx0 = (int)srcIdx;
            int idx1 = (idx0 + 1 < readPoints) ? idx0 + 1 : idx0;
            double frac = srcIdx - (double)idx0;
            outputBuffer[i] = tempDb[idx0] * (1.0 - frac) + tempDb[idx1] * frac;
        }
    }
    else if (readPoints == 1)
    {
        for (int i = 0; i < SetPoints; ++i)
            outputBuffer[i] = tempDb[0];
    }
    else
    {
        g_traceCount++;
        return false;
    }

    // 10. 查找插值后输出缓冲的峰值位�?
    double outMaxDb = -999.0;
    int outMaxIdx = 0;
    double outMinDb = 999.0;
    int outMinIdx = 0;
    for (int i = 0; i < SetPoints; ++i)
    {
        if (outputBuffer[i] > outMaxDb) { outMaxDb = outputBuffer[i]; outMaxIdx = i; }
        if (outputBuffer[i] < outMinDb) { outMinDb = outputBuffer[i]; outMinIdx = i; }
    }
    
    // 计算峰值在Span中的位置比例 (0.0=起始, 1.0=结束)
    double peakRatio = (SetPoints > 1) ? (double)outMaxIdx / (double)(SetPoints - 1) : 0.0;
    double peakFractionalSpan = peakRatio * CurrentSpan;
    
    // 获取SweepLogic的公开参数用于日志追踪
    auto* logic = sweep;
    double fStart = logic ? logic->GetFStart() : 0;
    double fStop = logic ? logic->GetFStop() : 0;
    double fStep = logic ? logic->GetFStep() : 0;
    uint64_t detNums = logic ? logic->Det_Nums : 0;
    uint32_t detLen = logic ? logic->Det_Len : 0;
    double cfOffset = logic ? logic->CF_Offset : 0;
    double spanVal = logic ? logic->Span : 0;
    double centerFreq = logic ? logic->CenterFrequency : 0;
    double rfStart = logic ? logic->Rf_Start : 0;
    double rfStop = logic ? logic->Rf_Stop : 0;
    double bwVal = logic ? logic->GetBw() : 0;
    double bwLastVal = logic ? logic->GetBwLast() : 0;
    const double csharpRawStep = (readPoints > 1) ? spanVal / static_cast<double>(readPoints - 1) : 0.0;
    const double csharpRawPeakFreq = rfStart + static_cast<double>(maxAdcIdx) * csharpRawStep;
    const double rawPoint600Db = (readPoints > 600) ? 20.0 * std::log10(static_cast<double>(pData[600] ? pData[600] : 1)) + correction : -300.0;
    
    // 计算峰值对应的近似硬件频率
    double approxPeakFreq_HW = 0;
    if (logic && detNums > 1 && fStep > 0)
    {
        double hwPeakRatio = (readPoints > 1) ? (double)maxDbIdx / (double)(readPoints - 1) : 0.0;
        approxPeakFreq_HW = fStart + hwPeakRatio * (detNums - 1) * fStep;
    }
    
    // 日志: 完整的峰值追踪链�?
    if (g_traceCount < 10 || g_traceCount % 20 == 0) {
        std::ostringstream oss;
        oss << "[TRACE][" << g_traceCount << "] PEAK_TRACE"
            << " ADC_max_idx=" << maxAdcIdx << "/" << readPoints
            << " ADC_max_val=" << maxAdcVal
            << " -> dB_max_idx=" << maxDbIdx << "/" << readPoints << " reverse=" << (reverseSpectrumData_ ? "1" : "0")
            << " dB_max_val=" << maxDbVal << " dBm"
            << " -> OUT_max_idx=" << outMaxIdx << "/" << SetPoints
            << " OUT_max_val=" << outMaxDb << " dBm"
            << " peakRatio=" << peakRatio
            << " approxHWPeakFreq=" << approxPeakFreq_HW << " Hz"
            << " F_Start=" << fStart
            << " F_Stop=" << fStop
            << " F_Step=" << fStep
            << " Det_Nums=" << detNums
            << " Det_Len=" << detLen
            << " CF_Offset=" << cfOffset
            << " CenterFreq=" << centerFreq
            << " Span_cfg=" << spanVal
            << " Rf_Start=" << rfStart
            << " Rf_Stop=" << rfStop
            << " bw=" << bwVal
            << " bw_last=" << bwLastVal
            << " correction=" << correction;
        SWLog(oss.str().c_str());
        
        // 日志: 完整的成功信�?
        auto t_end = std::chrono::high_resolution_clock::now();
        auto total_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t_end - t_start).count();
        oss.str("");
        oss << "[TRACE][" << g_traceCount << "] SUCCESS"
            << " readPoints=" << readPoints << " readBytes=" << readBytes
            << " zeroPoints=" << zeroCount << "/" << readPoints
            << " outRange=[" << outMinDb << "(" << outMinIdx << ")" << ", " 
            << outMaxDb << "(" << outMaxIdx << ")" << "] dBm"
            << " read_ms=" << read_ms << " total_ms=" << total_ms;
        SWLog(oss.str().c_str());

        // 输出首尾�?个点用于检查稳定�?
        if (SetPoints >= 5) {
            oss.str("");
            oss << "[TRACE][" << g_traceCount << "] First5: ";
            for (int i = 0; i < 5; ++i) oss << outputBuffer[i] << " ";
            oss << "| Last5: ";
            for (int i = SetPoints - 5; i < SetPoints; ++i) oss << outputBuffer[i] << " ";
            SWLog(oss.str().c_str());
        }
        
        // 输出峰值附近各10个点，查看峰�?
        int peakStart = (std::max)(0, outMaxIdx - 10);
        int peakEnd = (std::min)(SetPoints - 1, outMaxIdx + 10);
        oss.str("");
        oss << "[TRACE][" << g_traceCount << "] PeakRegion[" << peakStart << "-" << peakEnd << "]: ";
        for (int i = peakStart; i <= peakEnd; ++i) {
            oss << "[" << i << "]=" << outputBuffer[i] << " ";
        }
        SWLog(oss.str().c_str());
    }

    {
        std::ostringstream oss;
        oss << "[PSCAN_CSHARP_MAP]"
            << " frame=" << g_traceCount
            << " reverse=" << (reverseSpectrumData_ ? "1" : "0")
            << " rawPeakIdx=" << maxAdcIdx
            << " rawPeakFreq=" << csharpRawPeakFreq
            << " outputPeakIdx=" << outMaxIdx
            << " outputPeakFreq=" << (rfStart + peakRatio * spanVal)
            << " rawPoint600Db=" << rawPoint600Db
            << " outputPoint600Db=" << ((SetPoints > 600) ? outputBuffer[600] : -300.0);
        SWLog(oss.str().c_str());
    }

    {
        std::ostringstream oss;
        oss << "[SWEEP_FRAME_DIAG]"
            << " frame=" << g_traceCount
            << " CF=" << centerFreq
            << " Span=" << spanVal
            << " SetPoints=" << SetPoints
            << " readPoints=" << readPoints
            << " readBytes=" << readBytes
            << " reverse=" << (reverseSpectrumData_ ? "1" : "0")
            << " rawPeakIdx=" << maxAdcIdx
            << " csharpRawPeakFreq=" << csharpRawPeakFreq
            << " rawPoint600Db=" << rawPoint600Db
            << " outputPoint600Db=" << ((SetPoints > 600) ? outputBuffer[600] : -300.0)
            << " mappedPeakIdx=" << maxDbIdx
            << " outputPeakIdx=" << outMaxIdx
            << " approxHWPeakFreq=" << approxPeakFreq_HW
            << " outPeakRatio=" << peakRatio
            << " F_Start=" << fStart
            << " F_Stop=" << fStop
            << " F_Step=" << fStep
            << " Det_Nums=" << detNums
            << " Det_Len=" << detLen
            << " Rf_Start=" << rfStart
            << " Rf_Stop=" << rfStop
            << " zeroPoints=" << zeroCount
            << " correction=" << correction
            << " read_ms=" << read_ms;
        SWLog(oss.str().c_str());
    }

    g_traceCount++;
    return true;
}

bool Sweep::GetPositivePeakData(std::vector<double>& outputBuffer) 
{
    return GetSingleChannelData(outputBuffer, &HardwareWrappers::ReadPos);
}

bool Sweep::GetNegativeData(std::vector<double>& outputBuffer) 
{
    return GetSingleChannelData(outputBuffer, &HardwareWrappers::ReadNeg);
}

bool Sweep::GetSampleData(std::vector<double>& outputBuffer)
{
    return GetSingleChannelData(outputBuffer, &HardwareWrappers::ReadSmp);
}

bool Sweep::GetRMSData(std::vector<double>& outputBuffer)
{
    return GetSingleChannelData(outputBuffer, &HardwareWrappers::ReadRMS);
}

bool Sweep::GetAverageData(std::vector<double>& outputBuffer)
{
    return GetSingleChannelData(outputBuffer, &HardwareWrappers::ReadAvg);
}

int Sweep::GetAutoPeakData(std::vector<double>& HighData, std::vector<double>& LowData)
{
    if (HighData.size() < (size_t)SetPoints) HighData.resize(SetPoints);
    if (LowData.size() < (size_t)SetPoints) LowData.resize(SetPoints);

    std::lock_guard<std::mutex> lock(_bufferLock);

    const int expectedBytes = Global::SweepSpectrumPointCountSet * 2 * 4;
    if (_sharedRawBuffer.size() < (size_t)expectedBytes)
    {
        _sharedRawBuffer.resize(expectedBytes);
    }

    // 问题1修复：条件触�?
    if (triggerOnNextRead_)
    {
        sweep->TriggerSweep();
        triggerOnNextRead_ = false;
    }

    // 轮询等待数据就绪�?00ms超时�?
    auto t_wait_start = std::chrono::high_resolution_clock::now();
    const int timeoutMs = 300;
    int readBytes = 0;
    while (true)
    {
        auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::high_resolution_clock::now() - t_wait_start).count();
        if (elapsed_ms >= timeoutMs) break;

        readBytes = HardwareWrappers::ReadAuto(_sharedRawBuffer.data(), expectedBytes);
        if (readBytes > 0 && (readBytes % 8) == 0)
        {
            int readPts = readBytes / 8;
            uint32_t* pChk = reinterpret_cast<uint32_t*>(_sharedRawBuffer.data());
            int validCount = 0;
            for (int k = 0; k < readPts; ++k) {
                if (pChk[k * 2] != 0 || pChk[k * 2 + 1] != 0) validCount++;
            }
            if (validCount > readPts / 2) break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    if (readBytes <= 0 || (readBytes % 8) != 0) return false;

    int readPoints = readBytes / 8;
    uint32_t* pUint = reinterpret_cast<uint32_t*>(_sharedRawBuffer.data());
    double correction = -Global::SweepBaseErrorValue + Global::CorrectValue + Global::RbwErrDIC[(uint32_t)RBW];

    std::vector<double> tempHigh(readPoints), tempLow(readPoints);
    int zeroCount = 0;
    for (int i = 0; i < readPoints; ++i)
    {
        uint32_t H = pUint[i * 2] ? pUint[i * 2] : 1;
        uint32_t L = pUint[i * 2 + 1] ? pUint[i * 2 + 1] : 1;
        if (pUint[i * 2] == 0) { tempHigh[i] = -300.0; zeroCount++; }
        else tempHigh[i] = 20.0 * std::log10((double)H) + correction;
        if (pUint[i * 2 + 1] == 0) { tempLow[i] = -300.0; zeroCount++; }
        else tempLow[i] = 20.0 * std::log10((double)L) + correction;
    }

    // 问题2修复：反转数据（硬件降频扫描 �?正序显示�?
    std::reverse(tempHigh.begin(), tempHigh.begin() + readPoints);
    std::reverse(tempLow.begin(), tempLow.begin() + readPoints);

    if (readPoints >= SetPoints || readPoints > 1)
    {
        common->Interp(tempHigh.data(), readPoints, SetPoints, CurrentSpan, HighData.data());
        common->Interp(tempLow.data(), readPoints, SetPoints, CurrentSpan, LowData.data());
        return SetPoints;
    }
    else if (readPoints == 1)
    {
        for (int i = 0; i < SetPoints; ++i) { HighData[i] = tempHigh[0]; LowData[i] = tempLow[0]; }
        return SetPoints;
    }
    return 0;  
}

bool Sweep::GetAutoPeakData(std::vector<double>& outputBuffer)
{
    if (outputBuffer.size() < static_cast<size_t>(SetPoints * 2))
    {
        outputBuffer.resize(SetPoints * 2);
    }

    std::lock_guard<std::mutex> lock(_bufferLock);

    const int expectedBytes = Global::SweepSpectrumPointCountSet * 8;

    if (_sharedRawBuffer.size() < static_cast<size_t>(expectedBytes))
    {
        _sharedRawBuffer.resize(expectedBytes);
    }

    uint8_t* pRaw = _sharedRawBuffer.data();

    // 问题1修复：条件触�?
    if (triggerOnNextRead_)
    {
        sweep->TriggerSweep();
        triggerOnNextRead_ = false;
    }

    // 轮询等待数据就绪�?00ms超时�?
    auto t_wait_start = std::chrono::high_resolution_clock::now();
    const int timeoutMs = 300;
    int readBytes = 0;
    while (true)
    {
        auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::high_resolution_clock::now() - t_wait_start).count();
        if (elapsed_ms >= timeoutMs) break;

        readBytes = HardwareWrappers::ReadAuto(pRaw, expectedBytes);
        if (readBytes > 0 && (readBytes % 8) == 0)
        {
            int readPts = readBytes / 8;
            uint32_t* pChk = reinterpret_cast<uint32_t*>(pRaw);
            int validCount = 0;
            for (int k = 0; k < readPts; ++k) {
                if (pChk[k * 2] != 0 || pChk[k * 2 + 1] != 0) validCount++;
            }
            if (validCount > readPts / 2) break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    if (readBytes <= 0 || (readBytes % 8) != 0)
        return false;

    int readPoints = readBytes / 8;
    if (readPoints == 0)
        return false;

    double correction = -Global::SweepBaseErrorValue +
        Global::CorrectValue +
        Global::RbwErrDIC[(uint32_t)RBW];

    double* pOut = outputBuffer.data();
    uint32_t* pUint = reinterpret_cast<uint32_t*>(pRaw);

    if (readPoints >= SetPoints)
    {
        for (int i = 0; i < SetPoints; ++i)
        {
            uint32_t H = pUint[i * 2] ? pUint[i * 2] : 1;
            uint32_t L = pUint[i * 2 + 1] ? pUint[i * 2 + 1] : 1;

            pOut[i * 2] = 20.0 * std::log10(static_cast<double>(H)) + correction;
            pOut[i * 2 + 1] = 20.0 * std::log10(static_cast<double>(L)) + correction;
        }
        // 问题2修复：反转数据（硬件降频扫描 �?正序显示�?
        // readPoints >= SetPoints 时直接从pUint取SetPoints个点，需要反�?
        for (int i = 0; i < SetPoints / 2; ++i)
        {
            int j = SetPoints - 1 - i;
            std::swap(pOut[i * 2], pOut[j * 2]);
            std::swap(pOut[i * 2 + 1], pOut[j * 2 + 1]);
        }
    }
    else if (readPoints > 1)
    {
        std::vector<double> tempHigh(readPoints), tempLow(readPoints);
        for (int i = 0; i < readPoints; ++i)
        {
            uint32_t H = pUint[i * 2] ? pUint[i * 2] : 1;
            uint32_t L = pUint[i * 2 + 1] ? pUint[i * 2 + 1] : 1;
            tempHigh[i] = 20.0 * std::log10(static_cast<double>(H)) + correction;
            tempLow[i] = 20.0 * std::log10(static_cast<double>(L)) + correction;
        }
        // 问题2修复：反转数据（硬件降频扫描 �?正序显示�?
        std::reverse(tempHigh.begin(), tempHigh.begin() + readPoints);
        std::reverse(tempLow.begin(), tempLow.begin() + readPoints);
        common->Interp(tempHigh.data(), readPoints, SetPoints, CurrentSpan, pOut);
        common->Interp(tempLow.data(), readPoints, SetPoints, CurrentSpan, pOut + SetPoints);
    }
    else if (readPoints == 1)
    {
        uint32_t H = pUint[0] ? pUint[0] : 1;
        uint32_t L = pUint[1] ? pUint[1] : 1;
        double hVal = 20.0 * std::log10(static_cast<double>(H)) + correction;
        double lVal = 20.0 * std::log10(static_cast<double>(L)) + correction;
        for (int i = 0; i < SetPoints; ++i)
        {
            pOut[i * 2] = hVal;
            pOut[i * 2 + 1] = lVal;
        }
    }
    else
    {
        return false;
    }

    return true;
}
