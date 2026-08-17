#ifndef PSCAN_DRIVER_H
#define PSCAN_DRIVER_H

// PscanDriver - 动态加载TransComApi.dll的驱动包装器
// 不依赖BSPDriver头文件，完全自包含

#include <Windows.h>
#include <cstdint>
#include <vector>
#include <functional>

// BSPDriver导出函数类型定义（与PscanApi.h一致）
typedef int (*PFN_PscanApi_Init)(double cf, double span, uint32_t rbw);
typedef int (*PFN_PscanApi_SetRBW)(uint32_t rbw);
typedef int (*PFN_PscanApi_SetPara)(double centerFreq, double span, double step, double refLevel);

typedef int (*PFN_PscanApi_SetConfig)(double centerFreq, double span, uint32_t rbw, double step, double refLevel);
typedef int (*PFN_PscanApi_Start)(void);
typedef int (*PFN_PscanApi_Stop)(void);
typedef int (*PFN_PscanApi_RunSingle)(void);
typedef int (*PFN_PscanApi_SetRunMode)(int mode);
typedef int (*PFN_PscanApi_GetRunMode)(void);
typedef int (*PFN_PscanApi_GetSpectrumData)(double* outFreqs, double* outAmps, uint32_t* outSize);
typedef double (*PFN_PscanApi_ConvertAmplitude)(double ampDbm, int targetUnit);

class PscanDriver
{
public:
    PscanDriver() = default;
    ~PscanDriver() { Unload(); }

    // 禁止拷贝
    PscanDriver(const PscanDriver&) = delete;
    PscanDriver& operator=(const PscanDriver&) = delete;

    // 加载TransComApi.dll
    bool Load(const wchar_t* dllPath = L"BSPDriver.dll")
    {
        hModule_ = LoadLibraryW(dllPath);
        if (!hModule_) return false;

        // 加载所有导出函数
        pfnInit_ = reinterpret_cast<PFN_PscanApi_Init>(GetProcAddress(hModule_, "PscanApi_Init"));
        pfnSetRBW_ = reinterpret_cast<PFN_PscanApi_SetRBW>(GetProcAddress(hModule_, "PscanApi_SetRBW"));
        pfnSetPara_ = reinterpret_cast<PFN_PscanApi_SetPara>(GetProcAddress(hModule_, "PscanApi_SetPara"));
        pfnSetConfig_ = reinterpret_cast<PFN_PscanApi_SetConfig>(GetProcAddress(hModule_, "PscanApi_SetConfig"));
        pfnStart_ = reinterpret_cast<PFN_PscanApi_Start>(GetProcAddress(hModule_, "PscanApi_Start"));
        pfnStop_ = reinterpret_cast<PFN_PscanApi_Stop>(GetProcAddress(hModule_, "PscanApi_Stop"));
        pfnRunSingle_ = reinterpret_cast<PFN_PscanApi_RunSingle>(GetProcAddress(hModule_, "PscanApi_RunSingle"));
        pfnSetRunMode_ = reinterpret_cast<PFN_PscanApi_SetRunMode>(GetProcAddress(hModule_, "PscanApi_SetRunMode"));
        pfnGetRunMode_ = reinterpret_cast<PFN_PscanApi_GetRunMode>(GetProcAddress(hModule_, "PscanApi_GetRunMode"));
        pfnGetSpectrumData_ = reinterpret_cast<PFN_PscanApi_GetSpectrumData>(GetProcAddress(hModule_, "PscanApi_GetSpectrumData"));
        pfnConvertAmplitude_ = reinterpret_cast<PFN_PscanApi_ConvertAmplitude>(GetProcAddress(hModule_, "PscanApi_ConvertAmplitude"));

        return pfnInit_ && pfnSetRBW_ && pfnSetPara_ && pfnStart_ && pfnStop_ &&
               pfnGetSpectrumData_ && pfnGetRunMode_ && pfnSetRunMode_ && pfnSetConfig_;
    }

    void Unload()
    {
        if (hModule_)
        {
            FreeLibrary(hModule_);
            hModule_ = nullptr;
        }
    }

    bool IsLoaded() const { return hModule_ != nullptr; }

    // The test client runs a potentially blocking hardware initialization on a
    // worker thread, so it needs a stable copy of the exported entry point.
    PFN_PscanApi_Init GetInitFunction() const { return pfnInit_; }

    // Do not unload a DLL while one of its exported functions is still running.
    // This is only used during application shutdown with an in-flight Init call;
    // Windows will reclaim the module when the process exits.
    void DetachModuleForInFlightCall()
    {
        hModule_ = nullptr;
    }

    // 包装接口（与PscanMock相同的接口）
    bool Init(double cf, double span, uint32_t rbw)
    {
        return pfnInit_ && pfnInit_(cf, span, rbw) != 0;
    }

    bool SetRBW(uint32_t rbw)
    {
        return pfnSetRBW_ && pfnSetRBW_(rbw) != 0;
    }

    bool SetPara(double centerFreq, double span, double step, double refLevel)
    {
        return pfnSetPara_ && pfnSetPara_(centerFreq, span, step, refLevel) != 0;
    }

    bool SetConfig(double centerFreq, double span, uint32_t rbw, double step, double refLevel)
    {
        return pfnSetConfig_ && pfnSetConfig_(centerFreq, span, rbw, step, refLevel) != 0;
    }

    bool Start() { return pfnStart_ && pfnStart_() != 0; }
    bool Stop() { return pfnStop_ && pfnStop_() != 0; }
    bool RunSingle() { return pfnRunSingle_ && pfnRunSingle_() != 0; }

    bool SetRunMode(int mode) { return pfnSetRunMode_ && pfnSetRunMode_(mode) != 0; }
    int GetRunMode() { return pfnGetRunMode_ ? pfnGetRunMode_() : 0; }

    bool GetSpectrumData(std::vector<double>& freqs, std::vector<double>& amps)
    {
        if (!pfnGetSpectrumData_) return false;

        // 直接分配足够大的缓冲区获取数据
        // (DLL侧API不支持nullptr查询模式，传nullptr会返回0)
        static constexpr uint32_t kMaxPoints = 65536;
        freqs.resize(kMaxPoints);
        amps.resize(kMaxPoints);
        uint32_t actualSize = kMaxPoints;
        if (!pfnGetSpectrumData_(freqs.data(), amps.data(), &actualSize) || actualSize == 0)
            return false;

        freqs.resize(actualSize);
        amps.resize(actualSize);
        return true;
    }

    double ConvertAmplitude(double ampDbm, int targetUnit)
    {
        return pfnConvertAmplitude_ ? pfnConvertAmplitude_(ampDbm, targetUnit) : ampDbm;
    }

private:
    HMODULE hModule_ = nullptr;
    PFN_PscanApi_Init pfnInit_ = nullptr;
    PFN_PscanApi_SetRBW pfnSetRBW_ = nullptr;
    PFN_PscanApi_SetPara pfnSetPara_ = nullptr;

    PFN_PscanApi_SetConfig pfnSetConfig_ = nullptr;
    PFN_PscanApi_Start pfnStart_ = nullptr;
    PFN_PscanApi_Stop pfnStop_ = nullptr;
    PFN_PscanApi_RunSingle pfnRunSingle_ = nullptr;
    PFN_PscanApi_SetRunMode pfnSetRunMode_ = nullptr;
    PFN_PscanApi_GetRunMode pfnGetRunMode_ = nullptr;
    PFN_PscanApi_GetSpectrumData pfnGetSpectrumData_ = nullptr;
    PFN_PscanApi_ConvertAmplitude pfnConvertAmplitude_ = nullptr;
};

#endif // PSCAN_DRIVER_H
