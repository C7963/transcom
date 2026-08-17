#pragma once

#include <cstdint>
#include <optional>

namespace FFM {

struct SpanProfile {
    uint32_t spanHz;
    uint32_t decim;
};

class FfmHardwareProfile {
public:
    static constexpr double AdcSampleRateHz = 1228800000.0;
    static constexpr double FfmSampleRateHz = 614400000.0;
    static constexpr double AdcNcoOffsetHz = 307200000.0;
    static constexpr uint64_t DirectRfLimitHz = 10000000ULL;
    static constexpr uint32_t FftSize = 2048;
    static constexpr uint32_t OutputPoints = 1601;
    static constexpr uint32_t RawFrameBytes = 32768;

    static std::optional<SpanProfile> FindSpan(uint32_t spanHz);
    static std::optional<uint32_t> FindMultiDecim(uint32_t ifBandwidthHz);
};

} // namespace FFM
