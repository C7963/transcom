#pragma once

#include <cstdint>

#ifdef BSPDRIVER_EXPORTS
#define FFM_API __declspec(dllexport)
#else
#define FFM_API __declspec(dllimport)
#endif

extern "C" {
FFM_API int FfmApi_SetConfig(uint64_t centerFreqHz, uint32_t spanHz,
    uint32_t ifBandwidthHz, uint32_t rfAttDb, uint32_t ifAttDb, uint32_t rfMode);
FFM_API int FfmApi_Start();
FFM_API int FfmApi_Stop();
FFM_API int FfmApi_RunSingle();
FFM_API int FfmApi_GetSpectrumData(double* frequenciesHz, double* correctedDbm,
    double* rawFftDb, uint32_t* size);
}
