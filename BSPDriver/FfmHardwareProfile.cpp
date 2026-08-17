#include "FfmHardwareProfile.h"

#include <array>

namespace FFM {
namespace {
constexpr std::array<SpanProfile, 18> kSpans{{
    {480000000U, 0}, {240000000U, 1}, {120000000U, 2}, {60000000U, 3},
    {30000000U, 4}, {15000000U, 5}, {7500000U, 6}, {3000000U, 7},
    {1500000U, 8}, {750000U, 9}, {300000U, 10}, {150000U, 11},
    {75000U, 12}, {30000U, 13}, {15000U, 14}, {7500U, 15},
    {3000U, 16}, {1500U, 17}
}};
constexpr std::array<SpanProfile, 11> kIfBandwidths{{
    {3000000U, 0}, {1500000U, 1}, {750000U, 2}, {300000U, 3},
    {150000U, 4}, {75000U, 5}, {30000U, 6}, {15000U, 7},
    {7500U, 8}, {3000U, 9}, {1500U, 10}
}};
}

std::optional<SpanProfile> FfmHardwareProfile::FindSpan(uint32_t spanHz)
{
    for (const auto& item : kSpans) if (item.spanHz == spanHz) return item;
    return std::nullopt;
}

std::optional<uint32_t> FfmHardwareProfile::FindMultiDecim(uint32_t ifBandwidthHz)
{
    for (auto it = kIfBandwidths.rbegin(); it != kIfBandwidths.rend(); ++it) {
        if (it->spanHz >= ifBandwidthHz) return it->decim;
    }
    return std::nullopt;
}
} // namespace FFM
