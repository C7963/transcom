#pragma once

#include "FfmConfigurator.h"
#include "FfmDataSource.h"
#include "FfmSpectrumProcessor.h"

#include <mutex>

namespace FFM {
class FfmController {
public:
    bool Configure(const FfmConfig& config);
    bool Start();
    void Stop();
    bool RunSingle();
    bool GetLatest(SpectrumFrame& frame) const;
private:
    mutable std::mutex mutex_;
    FfmConfig config_{};
    SpectrumFrame latest_{};
    FfmConfigurator configurator_{};
    FfmDataSource source_{};
    FfmSpectrumProcessor processor_{};
    uint32_t decim_ = 0;
    bool configured_ = false;
    bool running_ = false;
};
} // namespace FFM
