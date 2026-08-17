#pragma once
#include <cstdint>
#include <vector>
#include <memory>

namespace Common {

    class ADCConfig {
    public:
        ADCConfig() noexcept;
        ~ADCConfig() noexcept;

        // 不暴露内部指针类型，提供明确方法
        void adc_init(uint64_t samplefreq);
        void adc_jesd_init();
        void set_adc_sample(uint64_t sample);
        void set_adc_dcm(uint8_t dcm);
        void set_adc_nco(uint64_t carrierfreq);
        uint64_t get_last_nco_frequency() const;
        int get_last_nco_ch0_result() const;
        int get_last_nco_ch1_result() const;
        bool has_nco_write_attempt() const;
        int get_last_init_result() const;
        void set_adc_channel(uint32_t channel);
        void set_adc_filter(const std::vector<uint32_t>& filter, uint16_t mode);
        bool get_adc_status();
        bool get_adc_clockstate();

        // 禁止拷贝，允许移动
        ADCConfig(const ADCConfig&) = delete;
        ADCConfig& operator=(const ADCConfig&) = delete;
        ADCConfig(ADCConfig&&) = default;
        ADCConfig& operator=(ADCConfig&&) = default;

    private:
        
        struct Impl;
        std::unique_ptr<Impl> impl_; 
    }; 
} 
