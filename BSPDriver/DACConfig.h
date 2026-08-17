#pragma once
#include <cstdint>
#include <memory>

namespace Common {

    class DACConfig {
    public:
        DACConfig() noexcept;
        ~DACConfig() noexcept;

        void ad9175_init(uint64_t dac_clkin_Hz);
        void set_dac_clkin(uint64_t dac_clk_freq_khz, uint64_t dac_clkin);
        void set_interpolation(uint8_t channel_interpolatin, uint8_t main_interpolatin);
        void dac_jesd_init();

        DACConfig(const DACConfig&) = delete;
        DACConfig& operator=(const DACConfig&) = delete;
        DACConfig(DACConfig&&) = default;
        DACConfig& operator=(DACConfig&&) = default;

    private:
        struct Impl;
        std::unique_ptr<Impl> impl_;
    };

}  
