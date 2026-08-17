#include "ADCConfig.h"
#include <thread>
#include <chrono>
#include <cstring>
#include "HLJS386_HL.h" // 假设项目已有   
#include "jesd204_api.h"
using namespace JESD;
using namespace ADC;

using namespace std::chrono_literals;

namespace Common {

    struct ADCConfig::Impl {
        // 原来代码中使用的结构体指针
        adc_state* adc_st = nullptr;
        adc_dev* adc_dev_p = nullptr;
        jesd_param_t* jesd204b_param = nullptr;
        uint64_t last_nco_frequency = 0;
        int last_nco_ch0_result = 0;
        int last_nco_ch1_result = 0;
        int last_init_result = 0;
        bool nco_write_attempted = false;

        Impl() {
            adc_st = new adc_state();
            adc_st->adc_h = new adc_handle_t();
            adc_dev_p = new adc_dev();
            adc_dev_p->st = adc_st;
        }
        ~Impl() {
            delete adc_st->adc_h;
            delete adc_st;
            delete adc_dev_p;
            delete jesd204b_param;
        }
    };

    ADCConfig::ADCConfig() noexcept : impl_(std::make_unique<Impl>()) {}

    ADCConfig::~ADCConfig() noexcept = default;

    void ADCConfig::adc_init(uint64_t samplefreq)
    {
        // 与原实现保持一致：构造 ad9208_init_param 并调用 ADCApi::adc_initialize(...)
        ad9208_init_param* param = new ad9208_init_param();
        HLJS386::HLJS386_MZ121A_HL mz121;
        param->sampling_frequency_hz = samplefreq;
        // ... 复制原有字段
        param->input_div = 1;
        param->powerdown_pin_en = 0;
        param->powerdown_mode = 3;
        param->duty_cycle_stabilizer_en = 0;
        param->current_scale = 3;
        param->analog_input_mode = 0;
        param->ext_vref_en = 0;
        param->buff_curr_n = 14;
        param->buff_curr_p = 14;
        param->fc_ch = 2;
        param->ddc_cnt = 2;
        param->ddc_input_format_real_en = 1;
        param->ddc_output_format_real_en = 0;
        param->test_mode_ch0 = 0;
        param->test_mode_ch1 = 0;
        param->sysref_lmfc_offset = 0;
        param->sysref_edge_sel = 0;
        param->sysref_clk_edge_sel = 0;
        param->sysref_neg_window_skew = 0;
        param->sysref_pos_window_skew = 0;
        param->sysref_mode = 0;
        param->sysref_count = 0;
        param->jesd_subclass = 0;

        param->ddc = new adc_ddc[2];
        for (int i = 0; i < 2; ++i) {
            param->ddc[i].gain_db = 1;
            param->ddc[i].decimation = 2;
            param->ddc[i].nco_mode = 0;
            param->ddc[i].carrier_freq_hz = param->sampling_frequency_hz / 4;
            param->ddc[i].po = 0;
        }

        impl_->jesd204b_param = new jesd_param_t();
        impl_->jesd204b_param->jesd_M = 4;
        impl_->jesd204b_param->jesd_L = 4;
        impl_->jesd204b_param->jesd_S = 1;
        impl_->jesd204b_param->jesd_F = 2;
        impl_->jesd204b_param->jesd_K = 32;
        impl_->jesd204b_param->jesd_N = 14;
        impl_->jesd204b_param->jesd_NP = 16;
        impl_->jesd204b_param->jesd_CS = 0;

        param->jesd_param = impl_->jesd204b_param;  
        impl_->adc_st->jesd_param = impl_->jesd204b_param;

        std::this_thread::sleep_for(10ms);
        impl_->last_nco_frequency = param->sampling_frequency_hz / 4;
        impl_->last_init_result = ADCApi::adc_initialize(impl_->adc_dev_p, param);
        std::this_thread::sleep_for(10ms);
        ADCApi::adc_register_write_direct(ADC_FPGA_DCM_CFG, 2);

        // free temporaries
        delete[] param->ddc;
        delete param;
    }

    void ADCConfig::adc_jesd_init()
    {
        jesd_init_param* init_param = new jesd_init_param();
        init_param->jesd_subclass = 0;
        Jesd204Api::adc_jesd_init(init_param);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        ADCApi::adc_register_write_direct(ADC_FPGA_RESET_CFG, 0x0300);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        ADCApi::adc_register_write_direct(ADC_FPGA_RESET_CFG, 0x0000);
        delete init_param;
    }

    void ADCConfig::set_adc_sample(uint64_t sample)
    {
        ADCApi::adc_set_input_clk_cfg(sample, 1);
    }

    void ADCConfig::set_adc_dcm(uint8_t dcm)
    {
        ADCApi::adc_adc_set_ddc_dcm(0, dcm);
        ADCApi::adc_register_write_direct(ADC_FPGA_DCM_CFG, dcm);
    }

    void ADCConfig::set_adc_nco(uint64_t carrierfreq)
    {
        impl_->nco_write_attempted = true;
        impl_->last_nco_frequency = carrierfreq;
        impl_->last_nco_ch0_result = ADCApi::adc_adc_set_ddc_nco(0, carrierfreq);
        impl_->last_nco_ch1_result = ADCApi::adc_adc_set_ddc_nco(1, carrierfreq);
    }

    uint64_t ADCConfig::get_last_nco_frequency() const
    {
        return impl_->last_nco_frequency;
    }

    int ADCConfig::get_last_nco_ch0_result() const
    {
        return impl_->last_nco_ch0_result;
    }

    int ADCConfig::get_last_nco_ch1_result() const
    {
        return impl_->last_nco_ch1_result;
    }

    bool ADCConfig::has_nco_write_attempt() const
    {
        return impl_->nco_write_attempted;
    }

    int ADCConfig::get_last_init_result() const
    {
        return impl_->last_init_result;
    }

    void ADCConfig::set_adc_channel(uint32_t channel)
    {
        ADCApi::adc_set_channel(channel);
    }

    void ADCConfig::set_adc_filter(const std::vector<uint32_t>& filter, uint16_t mode)
    {
        ADCApi::adc_filter_init(const_cast<uint32_t*>(filter.data()), mode);
    }

    bool ADCConfig::get_adc_status()
    {
        return ADCApi::get_adc_status();
    }

    bool ADCConfig::get_adc_clockstate()
    {
        return ADCApi::get_adc_clockstate();
    } 
} 
