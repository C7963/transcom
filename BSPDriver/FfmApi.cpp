#include "FfmApi.h"

#include <algorithm>
#include <mutex>

#include "FfmController.h"

namespace {
FFM::FfmController gController;
std::mutex gApiMutex;
}

int FfmApi_SetConfig(uint64_t centerFreqHz, uint32_t spanHz,
    uint32_t ifBandwidthHz, uint32_t rfAttDb, uint32_t ifAttDb, uint32_t rfMode)
{
    if (rfMode > 2U) return 0;
    FFM::FfmConfig config;
    config.centerFreqHz = centerFreqHz;
    config.spanHz = spanHz;
    config.ifBandwidthHz = ifBandwidthHz;
    config.rfAttDb = rfAttDb;
    config.ifAttDb = ifAttDb;
    config.rfMode = static_cast<FFM::RfMode>(rfMode);
    std::lock_guard<std::mutex> lock(gApiMutex);
    return gController.Configure(config) ? 1 : 0;
}

int FfmApi_Start() { std::lock_guard<std::mutex> lock(gApiMutex); return gController.Start() ? 1 : 0; }
int FfmApi_Stop() { std::lock_guard<std::mutex> lock(gApiMutex); gController.Stop(); return 1; }
int FfmApi_RunSingle() { std::lock_guard<std::mutex> lock(gApiMutex); return gController.RunSingle() ? 1 : 0; }

int FfmApi_GetSpectrumData(double* frequenciesHz, double* correctedDbm,
                           double* rawFftDb, uint32_t* size)
{
    if (!size) return 0;
    FFM::SpectrumFrame frame;
    {
        std::lock_guard<std::mutex> lock(gApiMutex);
        if (!gController.GetLatest(frame)) return 0;
    }
    const uint32_t required = static_cast<uint32_t>(frame.correctedDbm.size());
    if (!frequenciesHz || !correctedDbm || *size < required) { *size = required; return 0; }
    std::copy(frame.frequenciesHz.begin(), frame.frequenciesHz.end(), frequenciesHz);
    std::copy(frame.correctedDbm.begin(), frame.correctedDbm.end(), correctedDbm);
    if (rawFftDb) std::copy(frame.rawFftDb.begin(), frame.rawFftDb.end(), rawFftDb);
    *size = required;
    return 1;
}
