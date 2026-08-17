#pragma once
#include <boost/math/interpolators/cubic_b_spline.hpp>
#include <boost/math/special_functions/trunc.hpp>
#include <cstdint>
#include <vector>
#include <memory>
#include <utility>
#include "ADCConfig.h"
#include "DACConfig.h"
#include "Device_MEM32.h"
#include "RFControl.h"
#include "Global.h"
#include <stdint.h>
#include <queue>
#include <thread>
#include <array>
#include <mutex>
#include "PlayBack.h"
#include "Device_Data.h"
#include "Logic_Api.h"
#include "ADC_Api.h"
#include "DAC_Api.h"
#include "jesd204_api.h"
#include "SweepLogic.h"
#include "Si5386A_1d2288G_10M.h"
#include "Si5386A_1d2288G_100M.h"
#include "Si5386A_819d2M_10M.h"

namespace Common {

    using namespace Device;

    class CommonManager {
    public:
        static CommonManager& Instance() noexcept;
        static CommonManager* getInstance();

        // 初始化/关闭
        void InitDevice();
        void ADCInit();
        void DACInit();

        // 系统设置
        void set_reference_mode(uint8_t mode);
        float get_fpga_temp();
        float get_fpga_volt();
        bool get_ddr_status(uint8_t ddrNo);
        void CloseDevice();
        void init_clock(uint64_t adc_clk, uint32_t ref_clk, uint32_t ref_out_clk);
        int get_system_status();

        void SetADCClock();
        void SetRFType(RFCONTROL::RFType rf_select);

        void SetFilterCoe(const std::vector<unsigned int>& filterLowCoe,
            const std::vector<unsigned int>& filterCoe);
        void SetADCFilter(uint64_t centerfreq);
        double GetFreqErrorData(double freq);
        void LoadFreqErrorData();
        void LoadSweepRbwErrDIC(std::string filename);
        void UpdateErrorValue(uint64_t CF);
        void Interp(const double* dataIn, int dataSize, int n, double span, double* outBuffer);
        void SetWorkMode(Global::WorkMode workmode);

        uint32_t GetIFATT();
        uint32_t GetRFATT();
        void SetADCChannel(uint32_t channel);
        void update_si5386_firmware(uint64_t adc_clk, uint32_t ref_clk);

        // 禁止拷贝
        CommonManager(const CommonManager&) = delete;
        CommonManager& operator=(const CommonManager&) = delete;

        std::unique_ptr<ADCConfig> adcconfig_;
        std::unique_ptr<RFCONTROL::RFControl> rfControl_;
        CommonManager() noexcept;
        ~CommonManager() noexcept;

        // 全局时钟参数
        static uint64_t ADCSampleClock;
        static uint32_t ReferenceClock;
        static uint32_t Out_ReferenceClock;
        static uint64_t ADCSpan;

    private:
        const si5386a_reve_register_t* si5386;

        // 底层设备智能指针
        std::unique_ptr<Device::Device_MEM32> pcie_mem_;
        std::unique_ptr<PLAYBACK::AUX_CTRL> aux_;
        std::unique_ptr<ADC::ADCApi> adc_;
        std::unique_ptr<LOGIC::LOGICApi> LogicAPI_;

        std::unique_ptr<DACConfig> dacconfig_;
    };

}
