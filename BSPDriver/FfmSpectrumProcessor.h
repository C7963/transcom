#pragma once

#include "FfmDefs.h"

#include <cstdint>
#include <vector>

namespace FFM {
class FfmSpectrumProcessor {
public:
    bool Process(const std::vector<uint8_t>& bytes, const FfmConfig& config,
                 uint32_t decim, SpectrumFrame& frame) const;
};
} // namespace FFM
