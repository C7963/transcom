#include "FfmController.h"

#include "FfmHardwareProfile.h"

namespace FFM {
bool FfmController::Configure(const FfmConfig& config)
{
    const auto profile = FfmHardwareProfile::FindSpan(config.spanHz);
    if (!profile || !FfmHardwareProfile::FindMultiDecim(config.ifBandwidthHz)) return false;
    std::lock_guard<std::mutex> lock(mutex_);
    config_ = config;
    decim_ = profile->decim;
    configured_ = true;
    running_ = false;
    latest_ = {};
    return true;
}

bool FfmController::Start()
{
    FfmConfig config;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!configured_) return false;
        config = config_;
    }
    if (!configurator_.Configure(config) || !source_.Open()) return false;
    std::lock_guard<std::mutex> lock(mutex_);
    running_ = true;
    return true;
}

void FfmController::Stop()
{
    configurator_.Stop();
    source_.Close();
    std::lock_guard<std::mutex> lock(mutex_);
    running_ = false;
}

bool FfmController::RunSingle()
{
    FfmConfig config;
    uint32_t decim;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!configured_ || !running_) return false;
        config = config_;
        decim = decim_;
    }
    std::vector<uint8_t> bytes;
    SpectrumFrame frame;
    if (!source_.ReadFrame(bytes) || !processor_.Process(bytes, config, decim, frame)) return false;
    std::lock_guard<std::mutex> lock(mutex_);
    latest_ = std::move(frame);
    return true;
}

bool FfmController::GetLatest(SpectrumFrame& frame) const
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (latest_.correctedDbm.empty()) return false;
    frame = latest_;
    return true;
}
} // namespace FFM
