#include "FfmApi.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

int RunFfmSmokeTest()
{
    constexpr uint64_t center = 921600000ULL;
    constexpr uint32_t span = 30000000U;
    if (!FfmApi_SetConfig(center, span, 3000000U, 0U, 0U, 0U)) {
        std::cout << "[FFM FAIL] configuration failed\n";
        return 1;
    }
    if (!FfmApi_Start() || !FfmApi_RunSingle()) {
        std::cout << "[FFM FAIL] read3 did not return a complete 32768-byte frame\n";
        FfmApi_Stop();
        return 2;
    }
    uint32_t size = 1601;
    std::vector<double> freq(size), corrected(size), raw(size);
    if (!FfmApi_GetSpectrumData(freq.data(), corrected.data(), raw.data(), &size) || size != 1601) {
        std::cout << "[FFM FAIL] spectrum output is not 1601 points\n";
        FfmApi_Stop();
        return 3;
    }
    const auto peak = std::max_element(corrected.begin(), corrected.end());
    const size_t peakIndex = static_cast<size_t>(std::distance(corrected.begin(), peak));
    std::cout << "[FFM OK] points=" << size << " peakIndex=" << peakIndex
              << " peakFreq=" << freq[peakIndex] << " peakDbm=" << *peak << '\n';
    FfmApi_Stop();
    return 0;
}

#ifdef FFM_SMOKE_TEST_MAIN
int main() { return RunFfmSmokeTest(); }
#endif
