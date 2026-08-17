#define BSPDRIVER_EXPORTS
#include "PscanApi.h"
#include "Pscan.h"
#include "CommonManager.h"
#include <vector>
#include <cstdint>
#include <fstream>
#include <string>
#include <sstream>
#include <atomic>
#include <windows.h>

// PscanApi内部诊断日志（写到DLL所在目录的bsdriver_diag.log）
static std::string GetApiDllDir()
{
    char path[MAX_PATH] = { 0 };
    HMODULE hMod = nullptr;
    if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        (LPCSTR)&GetApiDllDir, &hMod))
    {
        GetModuleFileNameA(hMod, path, MAX_PATH);
    }
    std::string s(path);
    auto pos = s.find_last_of("\\/");
    return (pos != std::string::npos) ? s.substr(0, pos) : ".";
}

static void ApiLog(const char* msg)
{
    std::string logPath = GetApiDllDir() + "\\bsdriver_diag.log";
    std::ofstream log(logPath, std::ios::app);
    if (log.is_open())
    {
        log << msg << std::endl;
    }
}

// 获取Pscan单例引用
static PSCAN::Pscan& GetPscan()
{
    return PSCAN::Pscan::GetInstance();
}

int PscanApi_Init(double cf, double span, uint32_t rbw)
{
    static std::atomic<uint32_t> initCallCount{ 0 };
    const uint32_t callIndex = ++initCallCount;
    std::ostringstream begin;
    begin << "[PSCAN_API_INIT] begin call_index=" << callIndex
        << " cf=" << cf << " span=" << span << " rbw=" << rbw;
    ApiLog(begin.str().c_str());

    try
    {
        ApiLog("[PSCAN_API_INIT] stage=CommonManager::Instance begin");
        auto& common = Common::CommonManager::Instance();
        ApiLog("[PSCAN_API_INIT] stage=CommonManager::Instance done");

        ApiLog("[PSCAN_API_INIT] stage=InitDevice begin");
        common.InitDevice();

        ApiLog("[PSCAN_API_INIT] stage=InitDevice done");

        ApiLog("[PSCAN_API_INIT] stage=Pscan::GetInstance begin");
        auto& pscan = GetPscan();
        ApiLog("[PSCAN_API_INIT] stage=Pscan::GetInstance done");

        ApiLog("[PSCAN_API_INIT] stage=Pscan::InitializeState begin");
        pscan.InitializeState(cf, span, rbw);
        ApiLog("[PSCAN_API_INIT] success");
        return 1;
    }
    catch (const std::exception& ex)
    {
        ApiLog((std::string("[PSCAN_API_INIT] exception: ") + ex.what()).c_str());
        return 0;
    }
    catch (...)
    {
        ApiLog("[PSCAN_API_INIT] unknown exception");
        return 0;
    }
}
int PscanApi_SetRBW(uint32_t rbw)
{
    try
    {
        auto& pscan = GetPscan();
        auto config = pscan.GetConfig();
        config.rbw = rbw;
        return pscan.SetPara(config) ? 1 : 0;
    }
    catch (...)
    {
        return 0;
    }
}

int PscanApi_SetPara(double centerFreq, double span, double step, double refLevel)
{
    try
    {
        auto& pscan = GetPscan();
        PSCANCONFIG::PscanConfig config;
        config.centerFreq = centerFreq;
        config.span = span;
        config.step = step;
        config.refLevel = refLevel;
        config.rbw = pscan.GetConfig().rbw;  // 保留当前RBW值
        config.traceModels.push_back(
            PSCANCONFIG::TraceModel(PSCANCONFIG::TracesType::Trace1, PSCANCONFIG::DetectorType::RMS));
        config.UpdateDetectorBitmask();
        return pscan.SetPara(config) ? 1 : 0;
    }
    catch (...)
    {
        return 0;
    }
}

int PscanApi_SetConfig(double centerFreq, double span, uint32_t rbw, double step, double refLevel)
{
    try
    {
        auto& pscan = GetPscan();
        auto config = pscan.GetConfig();
        config.centerFreq = centerFreq;
        config.span = span;
        config.rbw = rbw;
        config.step = step;
        config.refLevel = refLevel;
        config.UpdateDetectorBitmask();

        ApiLog("[PSCAN_API_CONFIG] applying complete configuration once");
        return pscan.SetPara(config) ? 1 : 0;
    }
    catch (...)
    {
        ApiLog("[PSCAN_API_CONFIG] failed");
        return 0;
    }
}
int PscanApi_Start(void)
{
    try
    {
        GetPscan().Start();
        return 1;
    }
    catch (...)
    {
        return 0;
    }
}

int PscanApi_Stop(void)
{
    try
    {
        GetPscan().Stop();
        return 1;
    }
    catch (...)
    {
        return 0;
    }
}

int PscanApi_RunSingle(void)
{
    try
    {
        GetPscan().RunSingle();
        return 1;
    }
    catch (...)
    {
        return 0;
    }
}

int PscanApi_SetRunMode(int mode)
{
    try
    {
        auto& pscan = GetPscan();
        switch (mode)
        {
        case 0: pscan.SetRunMode(PSCANCONFIG::RunMode::Stop); break;
        case 1: pscan.SetRunMode(PSCANCONFIG::RunMode::RunSingle); break;
        case 2: pscan.SetRunMode(PSCANCONFIG::RunMode::RunContinue); break;
        default: return 0;
        }
        return 1;
    }
    catch (...)
    {
        return 0;
    }
}

int PscanApi_GetRunMode(void)
{
    try
    {
        auto mode = GetPscan().GetRunMode();
        switch (mode)
        {
        case PSCANCONFIG::RunMode::Stop: return 0;
        case PSCANCONFIG::RunMode::RunSingle: return 1;
        case PSCANCONFIG::RunMode::RunContinue: return 2;
        default: return 0;
        }
    }
    catch (...)
    {
        return 0;
    }
}

int PscanApi_GetSpectrumData(double* outFreqs, double* outAmps, uint32_t* outSize)
{
    if (!outFreqs || !outAmps || !outSize) return 0;

    try
    {
        auto& pscan = GetPscan();
        std::vector<std::pair<PSCANCONFIG::DetectorType, std::vector<PSCANCONFIG::FreqAmpData>>> dataPairs;

        if (!pscan.GetSpectrumData(dataPairs) || dataPairs.empty())
        {
            return 0;
        }

        // 取第一个检波类型的数据
        const auto& freqAmpData = dataPairs[0].second;
        uint32_t reqSize = *outSize;
        uint32_t actualSize = static_cast<uint32_t>(freqAmpData.size());

        if (actualSize > reqSize)
        {
            actualSize = reqSize;
        }

        // 查找峰值数据点用于日志
        double maxAmp = -999.0;
        double minAmp = 999.0;
        double peakFreq = 0;
        double firstFreq = 0;
        double lastFreq = 0;
        uint32_t peakIdx = 0;
        for (uint32_t i = 0; i < actualSize; ++i)
        {
            outFreqs[i] = freqAmpData[i].frequency;
            outAmps[i] = freqAmpData[i].amplitude;
            if (i == 0) firstFreq = freqAmpData[i].frequency;
            if (i == actualSize - 1) lastFreq = freqAmpData[i].frequency;
            if (freqAmpData[i].amplitude > maxAmp)
            {
                maxAmp = freqAmpData[i].amplitude;
                peakFreq = freqAmpData[i].frequency;
                peakIdx = i;
            }
            if (freqAmpData[i].amplitude < minAmp)
            {
                minAmp = freqAmpData[i].amplitude;
            }
        }

        *outSize = actualSize;
        
        // 日志: 输出层数据验证
        static int g_apiCount = 0;
        if (g_apiCount < 10 || g_apiCount % 20 == 0)
        {
            std::ostringstream oss;
            oss << "[API_OUT][" << g_apiCount << "] DATA_VERIFY"
                << " points=" << actualSize
                << " freq_range=[" << firstFreq << " -> " << lastFreq << "] Hz"
                << " span_calc=" << (lastFreq - firstFreq) << " Hz"
                << " amp_range=[" << minAmp << ", " << maxAmp << "]"
                << " peak[" << peakIdx << "]=" << peakFreq << "Hz/" << maxAmp << "dBm"
                << " peak_ratio=" << (actualSize > 1 ? (double)peakIdx / (double)(actualSize - 1) : 0.0);
            ApiLog(oss.str().c_str());
        }
        g_apiCount++;

        return 1;
    }
    catch (...)
    {
        return 0;
    }
}

double PscanApi_ConvertAmplitude(double ampDbm, int targetUnit)
{
    try
    {
        auto& pscan = GetPscan();
        return pscan.ConvertAmplitude(ampDbm, static_cast<PSCANCONFIG::UnitType>(targetUnit));
    }
    catch (...)
    {
        return ampDbm;
    }
}
