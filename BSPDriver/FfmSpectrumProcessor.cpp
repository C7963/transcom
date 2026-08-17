#include "FfmSpectrumProcessor.h"

#include <algorithm>
#include <cmath>
#include <complex>
#include <cstring>
#include <limits>

#include "FfmHardwareProfile.h"

namespace FFM {
namespace {
constexpr double kPi = 3.14159265358979323846;

template <typename T> T ReadLe(const uint8_t* p)
{
    T value{};
    std::memcpy(&value, p, sizeof(value));
    return value;
}

void Fft(std::vector<std::complex<double>>& values)
{
    const size_t n = values.size();
    for (size_t i = 1, j = 0; i < n; ++i) {
        size_t bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) std::swap(values[i], values[j]);
    }
    for (size_t len = 2; len <= n; len <<= 1) {
        const std::complex<double> step = std::polar(1.0, -2.0 * kPi / static_cast<double>(len));
        for (size_t i = 0; i < n; i += len) {
            std::complex<double> w(1.0, 0.0);
            for (size_t j = 0; j < len / 2; ++j) {
                const auto u = values[i + j];
                const auto v = values[i + j + len / 2] * w;
                values[i + j] = u + v;
                values[i + j + len / 2] = u - v;
                w *= step;
            }
        }
    }
}
}

bool FfmSpectrumProcessor::Process(const std::vector<uint8_t>& bytes,
                                   const FfmConfig& config, uint32_t decim,
                                   SpectrumFrame& frame) const
{
    constexpr size_t n = FfmHardwareProfile::FftSize;
    if (bytes.size() != FfmHardwareProfile::RawFrameBytes) return false;
    std::vector<std::complex<double>> iq(n);
    if (decim == 0) {
        for (size_t pair = 0; pair < n / 2; ++pair) {
            const uint8_t* p = bytes.data() + pair * 8;
            const std::complex<double> first(ReadLe<int16_t>(p), ReadLe<int16_t>(p + 2));
            const std::complex<double> second(ReadLe<int16_t>(p + 4), ReadLe<int16_t>(p + 6));
            iq[pair * 2] = second;
            iq[pair * 2 + 1] = first;
        }
    } else {
        for (size_t i = 0; i < n; ++i) {
            const uint8_t* p = bytes.data() + i * 8;
            iq[i] = {static_cast<double>(ReadLe<int32_t>(p)), static_cast<double>(ReadLe<int32_t>(p + 4))};
        }
    }

    for (size_t i = 0; i < n; ++i) {
        const double x = static_cast<double>(i) / static_cast<double>(n - 1);
        const double window = 0.35875 - 0.48829 * std::cos(2 * kPi * x) +
                              0.14128 * std::cos(4 * kPi * x) - 0.01168 * std::cos(6 * kPi * x);
        iq[i] *= window;
    }
    Fft(iq);

    std::vector<double> shifted(n);
    const double offset = decim == 0 ? 6.5 : -88.0;
    for (size_t i = 0; i < n; ++i) {
        const auto& bin = iq[(i + n / 2) % n];
        shifted[i] = 10.0 * std::log10(std::max(std::norm(bin), std::numeric_limits<double>::min())) + offset;
    }

    frame = {};
    frame.frequenciesHz.resize(FfmHardwareProfile::OutputPoints);
    frame.rawFftDb.resize(FfmHardwareProfile::OutputPoints);
    frame.correctedDbm.resize(FfmHardwareProfile::OutputPoints);
    frame.displayDbuv.resize(FfmHardwareProfile::OutputPoints);
    const double start = static_cast<double>(config.centerFreqHz) - config.spanHz / 2.0;
    const double rbw = config.spanHz * 1.28 / FfmHardwareProfile::FftSize;
    for (size_t i = 0; i < FfmHardwareProfile::OutputPoints; ++i) {
        const double raw = shifted[224 + i];
        const double corrected = raw - config.baseErrorDb + config.spanErrorDb +
            config.rfModeErrorDb + config.frequencyErrorDb + config.rfAttDb + config.ifAttDb;
        frame.frequenciesHz[i] = start + i * rbw;
        frame.rawFftDb[i] = raw;
        frame.correctedDbm[i] = corrected;
        frame.displayDbuv[i] = corrected + 107.0;
    }
    return true;
}
} // namespace FFM
