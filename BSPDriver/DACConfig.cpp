#include "DACConfig.h"
#include <thread>
#include <chrono> 
#include "DAC_Api.h"
#include "jesd204_api.h"

using namespace JESD;
using namespace DAC;
using namespace std::chrono_literals;

namespace Common {

    struct DACConfig::Impl {
        dac_state* ad9175_state = nullptr;
        dac_dev* ad9175_dev_p = nullptr;
        Impl() {
            ad9175_state = new dac_state();
            ad9175_dev_p = new dac_dev();
            ad9175_dev_p->st = ad9175_state;
        }
        ~Impl() {
            delete ad9175_state;
            delete ad9175_dev_p;
        }
    };

    DACConfig::DACConfig() noexcept : impl_(std::make_unique<Impl>()) {}
    DACConfig::~DACConfig() noexcept = default;

    void DACConfig::ad9175_init(uint64_t dac_clkin_Hz)
    {
        dac_init_param* param = new dac_init_param();
        param->dac_clkin_Hz = dac_clkin_Hz;
        if (dac_clkin_Hz == 204800000) {
            param->dac_rate_khz = 9830400;
            param->clock_output_config = 0;
            param->channel_interpolation = 3;
            param->dac_interpolation = 8;
            param->dac_mask = AD917X_DACALL;
        }
        else if (dac_clkin_Hz == 307200000) {
            param->dac_rate_khz = 11059200;
            param->clock_output_config = 3;
            param->channel_interpolation = 3;
            param->dac_interpolation = 6;
            param->dac_mask = AD917X_DACALL;
        }
        param->link_mode = 0;
        param->jesd_mode = 3;
        param->jesd_subclass = 1;
        param->syncoutb_type = SIGNAL_LVDS;
        param->sysref_coupling = COUPLING_AC;
        param->logic_lane = 1;
        param->physical_lane = 1;

        jesd_param_t jesd204b_param;
        jesd204b_param.jesd_M = 2;
        jesd204b_param.jesd_L = 2;
        jesd204b_param.jesd_S = 1;
        jesd204b_param.jesd_F = 2;
        jesd204b_param.jesd_K = 32;
        jesd204b_param.jesd_N = 16;
        jesd204b_param.jesd_NP = 16;

        impl_->ad9175_state->appJesdConfig = jesd204b_param;
        DACApi::dac_init(impl_->ad9175_dev_p, param);
        delete param;
    }

    void DACConfig::set_dac_clkin(uint64_t dac_clk_freq_khz, uint64_t dac_clkin)
    {
        uint64_t pll_mult = NO_OS_DIV_ROUND_CLOSEST(dac_clk_freq_khz, dac_clkin / 1000);
        DACApi::dac_set_dac_clk((uint64_t)dac_clkin * pll_mult, 1, dac_clkin);
    }

    void DACConfig::set_interpolation(uint8_t channel_interpolatin, uint8_t main_interpolatin)
    {
        DACApi::dac_jesd_config_datapath(0, 3, channel_interpolatin, main_interpolatin);
    }

    void DACConfig::dac_jesd_init()
    {
        jesd_init_param* init_param = new jesd_init_param();
        init_param->jesd_subclass = 1;
        jesd_param_t* jesd204_param = new jesd_param_t();
        jesd204_param->jesd_CS = 0;
        jesd204_param->jesd_N = 15;
        jesd204_param->jesd_M = 1;
        jesd204_param->jesd_CF = 0;
        jesd204_param->jesd_HD = 1;
        jesd204_param->jesd_S = 0;
        jesd204_param->jesd_NP = 15;
        init_param->jesd_param = jesd204_param;
        Jesd204Api::dac_jesd_init(init_param);
        delete init_param;
    }

} 
