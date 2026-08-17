#pragma once

#include <cstdint>
#include <vector>

namespace FFM {

enum class RfMode : uint32_t { Normal = 0, LowNoise = 1, LowDistortion = 2 };

struct FfmConfig {
    uint64_t centerFreqHz = 921600000ULL;
    uint32_t spanHz = 30000000U;
    uint32_t ifBandwidthHz = 3000000U;
    uint32_t rfAttDb = 0;
    uint32_t ifAttDb = 0;
    RfMode rfMode = RfMode::Normal;
    double baseErrorDb = 0.0;
    double spanErrorDb = 0.0;
    double rfModeErrorDb = 0.0;
    double frequencyErrorDb = 0.0;
};

struct SpectrumFrame {
    std::vector<double> frequenciesHz;
    std::vector<double> rawFftDb;
    std::vector<double> correctedDbm;
    std::vector<double> displayDbuv;
};

} // namespace FFM
