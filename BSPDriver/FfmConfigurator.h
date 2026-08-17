#pragma once

#include "FfmDefs.h"

namespace Device { class Device_MEM32; }

namespace FFM {
class FfmConfigurator {
public:
    FfmConfigurator();
    bool Configure(const FfmConfig& config);
    bool Stop();
private:
    bool Write(uint32_t address, uint32_t value);
    bool SetOrder(uint32_t low, uint32_t high, uint32_t code);
    Device::Device_MEM32* mem_;
};
} // namespace FFM
