#include "FfmConfigurator.h"

#include <cmath>
#include <vector>

#include "Device_MEM32.h"
#include "FfmHardwareProfile.h"

namespace FFM {
namespace {
constexpr uint32_t kBase = 0x10011000U;
constexpr uint32_t kAdc0 = 0x000C2000U;
constexpr double kFtwScale48 = 281474976710656.0;
}

FfmConfigurator::FfmConfigurator() : mem_(Device::Device_MEM32::getInstance()) {}
bool FfmConfigurator::Write(uint32_t address, uint32_t value) { return mem_ && mem_->SendData(address, value); }

bool FfmConfigurator::SetOrder(uint32_t low, uint32_t high, uint32_t code)
{
    return Write(0x00010001U, (3U << 16) + 1U) && Write(0x00010003U, low) &&
           Write(0x00010004U, high) && Write(0x00010000U, 0x80000000U + code);
}

bool FfmConfigurator::Configure(const FfmConfig& config)
{
    const auto span = FfmHardwareProfile::FindSpan(config.spanHz);
    const auto multi = FfmHardwareProfile::FindMultiDecim(config.ifBandwidthHz);
    if (!span || !multi || config.centerFreqHz == 0) return false;

    bool ok = Write(kBase + 511U, 0U) && Write(0x00005010U, 4U) &&
              Write(0x000C0007U, 2U) && Write(0x00006000U, 0U) &&
              Write(kAdc0 + 0x310U, 0x43U);

    const double nco = config.centerFreqHz <= FfmHardwareProfile::DirectRfLimitHz
        ? static_cast<double>(config.centerFreqHz) : FfmHardwareProfile::AdcNcoOffsetHz;
    const uint64_t ftw = static_cast<uint64_t>(nco / FfmHardwareProfile::AdcSampleRateHz * kFtwScale48);
    for (uint32_t i = 0; i < 6; ++i) ok = Write(kAdc0 + 0x316U + i, static_cast<uint32_t>((ftw >> (8U * i)) & 0xFFU)) && ok;
    ok = Write(0x00006008U, config.centerFreqHz <= FfmHardwareProfile::DirectRfLimitHz ? 1U : 0U) && ok;

    ok = SetOrder(config.rfAttDb * 4U, 0U, 1U) && ok;
    ok = SetOrder(config.ifAttDb * 4U, 0U, 2U) && ok;
    ok = SetOrder(static_cast<uint32_t>(config.rfMode), 0U, 3U) && ok;

    std::vector<uint32_t> ftwTable(160, 0U);
    ok = mem_->SendData(kBase, ftwTable.data(), static_cast<int>(ftwTable.size())) && ok;
    ok = Write(kBase + 508U, span->decim) && ok;
    ok = Write(kBase + 509U, *multi + 1U) && ok;
    ok = Write(kBase + 510U, (span->decim > 4U ? 0x02000000U : 0xFE000000U) + FfmHardwareProfile::FftSize) && ok;
    ok = Write(kBase + 511U, 1U) && Write(kBase + 511U, 3U) && ok;

    const uint64_t quantized = ((config.centerFreqHz + 5000ULL) / 10000ULL) * 10000ULL;
    return SetOrder(static_cast<uint32_t>(quantized), static_cast<uint32_t>(quantized >> 32), 0U) && ok;
}

bool FfmConfigurator::Stop() { return Write(kBase + 511U, 0U); }
} // namespace FFM
