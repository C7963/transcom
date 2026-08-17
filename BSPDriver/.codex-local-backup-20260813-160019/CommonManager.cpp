#include "CommonManager.h"
#include <thread>
#include <chrono>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <windows.h>
#include "pugixml.hpp"  
#if 0
// Former C# PSCAN clock tables retained for rollback only.
#include "PscanClockTables.h"
#endif

using namespace std::chrono_literals;

// ---- 硬件初始化调试断点 ----
// 仅在 _DEBUG 构建下生效：F5 调试（已附加调试器）运行到每个阶段会自动中断
// （相当于预先打好的断点），便于逐段检查 PCIe 寄存器写入结果与初始化进度。
// 无调试器直接运行时自动跳过中断（仅输出日志），方便完整跑完初始化流程验证。
// Release 构建下为空操作，不影响性能。
#ifdef _DEBUG
#define BSP_INIT_BREAK(stage) do { \
    OutputDebugStringA("[//BSP_INIT_BREAK] " stage "\n"); \
    if (IsDebuggerPresent()) { DebugBreak(); } \
} while (0)
#else
#define //BSP_INIT_BREAK(stage) do { } while (0)
#endif

namespace Common {

    namespace {
        std::string GetCommonManagerDllDir()
        {
            char path[MAX_PATH] = { 0 };
            HMODULE module = nullptr;
            if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                reinterpret_cast<LPCSTR>(&GetCommonManagerDllDir), &module)) {
                GetModuleFileNameA(module, path, MAX_PATH);
            }
            std::string fullPath(path);
            const auto pos = fullPath.find_last_of("\\/");
            return pos == std::string::npos ? "." : fullPath.substr(0, pos);
        }

        void LogCommonManagerStartup(const char* message) noexcept
        {
            try {
                std::ofstream log(GetCommonManagerDllDir() + "\\bsdriver_diag.log",
                    std::ios::app);
                if (log.is_open()) {
                    log << "[COMMON_MANAGER_CTOR] " << message << std::endl;
                }
            }
            catch (...) {
            }
        }
    }

#if 0
    // Preserve the former C# PSCAN-specific initialization and diagnostics for rollback.
    // The active hardware initialization now uses only the Win_API InitDevice path.
    namespace {
        struct RegisterWrite {
            uint32_t offset;
            uint32_t value;
        };

        // TransComReceiver/DataProvider.cs loads ADC9695.xlsx sheet "config2".
        // Keep this table embedded so the C++ Pscan path does not depend on Excel.
        constexpr RegisterWrite kMZ121AdcConfig2[] = {
            { 0x00, 0x81 }, { 0x01, 0x02 }, { 0x08, 0x03 },
            { 0x1A4C, 0x0E }, { 0x1A4D, 0x0E }, { 0x1B03, 0x00 },
            { 0x1B08, 0x00 }, { 0x1B10, 0x00 }, { 0x1910, 0x0D },
            { 0x120, 0x00 }, { 0x1FF, 0x00 }, { 0x040, 0x00 },
            { 0x200, 0x02 }, { 0x201, 0x01 }, { 0x26F, 0x00 },
            { 0x311, 0x00 }, { 0x314, 0x00 }, { 0x315, 0x00 },
            { 0x320, 0x00 }, { 0x321, 0x00 }, { 0x327, 0x00 },
            { 0x331, 0x05 }, { 0x334, 0x00 }, { 0x335, 0x00 },
            { 0x340, 0x00 }, { 0x341, 0x00 }, { 0x347, 0x00 },
            { 0x300, 0x10 }, { 0x300, 0x00 }, { 0x571, 0x94 },
            { 0x572, 0x80 }, { 0x56E, 0x00 }, { 0x570, 0x91 },
            { 0x58B, 0x83 }, { 0x58F, 0x0D }, { 0x590, 0x0F },
            { 0x5B0, 0x00 }, { 0x5B2, 0x03 }, { 0x5B3, 0x02 },
            { 0x5B5, 0x01 }, { 0x5B6, 0x00 }, { 0x5BF, 0x00 },
            { 0x5C0, 0x02 }, { 0x5C1, 0x02 }, { 0x5C2, 0x02 },
            { 0x5C3, 0x02 }, { 0x5C4, 0x00 }, { 0x5C6, 0x00 },
            { 0x5C8, 0x00 }, { 0x5CA, 0x00 }, { 0x1228, 0x4F },
            { 0x1228, 0x0F }, { 0x1222, 0x00 }, { 0x1222, 0x40 },
            { 0x1222, 0x00 }, { 0x1262, 0x08 }, { 0x1262, 0x00 },
            { 0xDF8, 0x00 }, { 0xDF9, 0x00 }, { 0x701, 0x00 },
            { 0x73B, 0x80 }, { 0x18A6, 0x00 }, { 0x18E0, 0x00 },
            { 0x18E1, 0x00 }, { 0x18E2, 0x00 },
        };

        // TransComReceiver/DataProvider.cs loads ADC9695.xlsx sheet "jesd".
        constexpr RegisterWrite kMZ121JesdConfig[] = {
            { 0x20, 0x1 }, { 0x20, 0x0 }, { 0x34, 0x0 }, { 0x3C, 0x03031F01 },
        };

        std::string GetDriverDirectory()
        {
            char path[MAX_PATH] = { 0 };
            HMODULE module = nullptr;
            if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                reinterpret_cast<LPCSTR>(&GetDriverDirectory), &module))
            {
                GetModuleFileNameA(module, path, MAX_PATH);
            }
            std::string fullPath(path);
            const auto slash = fullPath.find_last_of("\\\\/");
            return slash == std::string::npos ? "." : fullPath.substr(0, slash);
        }

        void LogPscanAdcInit(const std::string& message)
        {
            std::ofstream log(GetDriverDirectory() + "\\\\bsdriver_diag.log", std::ios::app);
            if (log.is_open()) {
                log << "[PSCAN_ADC_INIT] " << message << std::endl;
            }
        }

        void LogPscanClockInit(const std::string& message)
        {
            std::ofstream log(GetDriverDirectory() + "\\\\bsdriver_diag.log", std::ios::app);
            if (log.is_open()) {
                log << "[PSCAN_CLOCK_INIT] " << message << std::endl;
            }
        }

        uint32_t ReadClockStatus(Device::Device_MEM32* mem, const char* stage)
        {
            unsigned char bytes[4] = { 0 };
            const bool ok = mem != nullptr && mem->ReadBackData(0x000B02FE, 1, bytes);
            uint32_t value = 0;
            std::memcpy(&value, bytes, sizeof(value));
            std::ostringstream oss;
            oss << "stage=" << stage << " address=0x000B02FE value=0x"
                << std::hex << std::uppercase << std::setw(8) << std::setfill('0') << value
                << " result=" << (ok ? "OK" : "FAILED");
            LogPscanClockInit(oss.str());
            return value;
        }

        bool WritePscanClockRegister(Device::Device_MEM32* mem, uint32_t address,
            uint32_t value, const char* stage)
        {
            const bool ok = mem != nullptr && mem->SendData(address, value);
            if (!ok) {
                std::ostringstream oss;
                oss << "stage=" << stage << " address=0x" << std::hex
                    << std::uppercase << std::setw(8) << std::setfill('0') << address
                    << " value=0x" << std::setw(8) << value << " result=FAILED";
                LogPscanClockInit(oss.str());
            }
            return ok;
        }

        bool ConfigureMZ121PscanClock(Device::Device_MEM32* mem)
        {
            bool ok = true;
            LogPscanClockInit("begin profile=CSharp_HMC703_config2_Si5386_config4");

            // DataProvider() and HMC703Config() both select the internal
            // reference before writing HMC703 config2.
            ok = WritePscanClockRegister(mem, 0x000B0000, 0x00000001,
                "reference_before_hmc") && ok;
            ok = WritePscanClockRegister(mem, 0x000B0000, 0x00000001,
                "hmc_reference") && ok;

            size_t hmcFailed = 0;
            for (const auto& reg : kMZ121Hmc703Config2) {
                if (!WritePscanClockRegister(mem, reg.address, reg.value,
                    "hmc703_config2")) {
                    ++hmcFailed;
                }
                std::this_thread::sleep_for(1ms);
            }
            ok = hmcFailed == 0 && ok;
            {
                std::ostringstream oss;
                oss << "stage=hmc703_config2 writes=" << kMZ121Hmc703Config2Count
                    << " failed_writes=" << hmcFailed
                    << " result=" << (hmcFailed == 0 ? "OK" : "FAILED");
                LogPscanClockInit(oss.str());
            }
            std::this_thread::sleep_for(1ms);

            const RegisterWrite siPreamble[] = {
                { 0x000B0201, 0x0B }, { 0x000B0224, 0xC0 },
                { 0x000B0225, 0x00 }, { 0x000B0201, 0x05 },
                { 0x000B0240, 0x01 },
            };
            for (const auto& reg : siPreamble) {
                ok = WritePscanClockRegister(mem, reg.offset, reg.value,
                    "si5386_preamble") && ok;
            }

            size_t siFailed = 0;
            for (const auto& reg : kMZ121Si5386Config4) {
                if (!WritePscanClockRegister(mem, 0x000B0201, reg.page,
                    "si5386_page")) {
                    ++siFailed;
                }
                if (!WritePscanClockRegister(mem, reg.address, reg.value,
                    "si5386_config4")) {
                    ++siFailed;
                }
                std::this_thread::sleep_for(1ms);
            }
            ok = siFailed == 0 && ok;
            {
                std::ostringstream oss;
                oss << "stage=si5386_config4 table_entries=" << kMZ121Si5386Config4Count
                    << " bus_writes=" << (kMZ121Si5386Config4Count * 2)
                    << " failed_writes=" << siFailed
                    << " result=" << (siFailed == 0 ? "OK" : "FAILED");
                LogPscanClockInit(oss.str());
            }

            const RegisterWrite siPostamble[] = {
                { 0x000B0201, 0x05 }, { 0x000B0214, 0x01 },
                { 0x000B0201, 0x00 }, { 0x000B021C, 0x01 },
                { 0x000B0201, 0x05 }, { 0x000B0240, 0x00 },
                { 0x000B0201, 0x0B }, { 0x000B0224, 0xC3 },
                { 0x000B0225, 0x02 },
            };
            for (const auto& reg : siPostamble) {
                ok = WritePscanClockRegister(mem, reg.offset, reg.value,
                    "si5386_postamble") && ok;
            }
            ok = WritePscanClockRegister(mem, 0x000B0201, 0x00,
                "si5386_calibration_page") && ok;
            ok = WritePscanClockRegister(mem, 0x000B02E3, 0xC7,
                "si5386_calibration_start") && ok;
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            const uint32_t status1 = ReadClockStatus(mem, "si5386_calibration_read1");
            ok = WritePscanClockRegister(mem, 0x000B02E4, 0x01,
                "si5386_calibration_step2") && ok;
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            const uint32_t status2 = ReadClockStatus(mem, "si5386_calibration_read2");
            {
                std::ostringstream oss;
                oss << "stage=si5386_calibration_status"
                    << " read1=0x" << std::hex << std::uppercase << status1
                    << " read2=0x" << status2 << std::dec
                    << " expected=0xF action=DIAGNOSTIC_ONLY";
                LogPscanClockInit(oss.str());
            }

            LogPscanClockInit(std::string("complete result=") + (ok ? "OK" : "FAILED"));
            return ok;
        }

        bool WritePscanAdcRegister(Device::Device_MEM32* mem, uint32_t address,
            uint32_t value, const char* stage)
        {
            const bool ok = mem != nullptr && mem->SendData(address, value);
            if (!ok) {
                std::ostringstream oss;
                oss << "stage=" << stage << " address=0x" << std::hex
                    << std::uppercase << std::setw(8) << std::setfill('0') << address
                    << " value=0x" << std::setw(8) << value << " result=FAILED";
                LogPscanAdcInit(oss.str());
            }
            return ok;
        }

        template <size_t N>
        bool WritePscanAdcTable(Device::Device_MEM32* mem, uint32_t baseAddress,
            const RegisterWrite (&table)[N], const char* stage)
        {
            bool ok = true;
            for (size_t index = 0; index < N; ++index) {
                ok = WritePscanAdcRegister(mem, baseAddress + table[index].offset,
                    table[index].value, stage) && ok;
                std::this_thread::sleep_for(1ms);
                if (index == 0) {
                    std::this_thread::sleep_for(10ms);
                }
            }

            std::ostringstream oss;
            oss << "stage=" << stage << " writes=" << N
                << " first=0x" << std::hex << std::uppercase << table[0].offset
                << " last=0x" << table[N - 1].offset
                << std::dec << " result=" << (ok ? "OK" : "FAILED");
            LogPscanAdcInit(oss.str());
            return ok;
        }

        bool ConfigureMZ121PscanAdc(Device::Device_MEM32* mem)
        {
            bool ok = true;
            LogPscanAdcInit("begin profile=CSharp_ADCConfig_config2");
            ok = WritePscanAdcRegister(mem, 0x000B0000, 0x00000001, "reference") && ok;
            ok = WritePscanAdcRegister(mem, 0x000C0004, 0x00000300, "adc_reset_assert") && ok;

            const int gainResult = ADC::ADCApi::set_rf_gain(30, 0);
            // ADCApi::set_rf_gain() returns true (1) when channel 0 is written.
            ok = gainResult != 0 && ok;
            {
                std::ostringstream oss;
                oss << "stage=rf_gain gain=30 channel=0 result=" << gainResult;
                LogPscanAdcInit(oss.str());
            }
            std::this_thread::sleep_for(1ms);

            ok = WritePscanAdcTable(mem, 0x000C2000, kMZ121AdcConfig2,
                "adc9695_config2") && ok;
            ok = WritePscanAdcTable(mem, 0x000C6000, kMZ121JesdConfig,
                "jesd") && ok;
            ok = WritePscanAdcRegister(mem, 0x00005001, 0x00000002, "adc_decimation") && ok;
            ok = WritePscanAdcRegister(mem, 0x00005000, 0x00000004, "pscan_channel") && ok;
            ok = WritePscanAdcRegister(mem, 0x000C0004, 0x00000000, "adc_reset_release") && ok;
            std::this_thread::sleep_for(1ms);
            ok = WritePscanAdcRegister(mem, 0x000C2572, 0x000000C0, "adc0_sync") && ok;
            ok = WritePscanAdcRegister(mem, 0x000C4572, 0x000000C0, "adc1_sync") && ok;
            ok = WritePscanAdcRegister(mem, 0x00001040, 0x00000000, "dma_reset") && ok;
            ok = WritePscanAdcRegister(mem, 0x00001040, 0x00000001, "dma_enable") && ok;
            LogPscanAdcInit(std::string("complete result=") + (ok ? "OK" : "FAILED"));
            return ok;
        }
    }

#endif

    uint64_t CommonManager::ADCSampleClock = 1.2288e9;
    uint32_t CommonManager::ReferenceClock = 100e6;
    uint32_t CommonManager::Out_ReferenceClock = 10e6;
    uint64_t CommonManager::ADCSpan = 0;
    RFCONTROL::RFType RFType = RFCONTROL::RFType::MZ121;

     // 方法1: 使用 std::array (推荐)
    std::vector<unsigned int>  FilterCoe_ = {
        0xffaf, 0xfff8, 0xff68, 0xffec, 0xfeeb, 0xffd9, 0xfe3b, 0xffc3,
        0xfd56, 0xffaf, 0xfc38, 0xffab, 0xfadf, 0xffce, 0xf944, 0x003f,
        0xf747, 0x0150, 0xf48a, 0x03cc, 0xefaf, 0x0ae4, 0xdfb7, 0x4c1f,
        0x4c1f, 0xdfb7, 0x0ae4, 0xefaf, 0x03cc, 0xf48a, 0x0150, 0xf747,
        0x003f, 0xf944, 0xffce, 0xfadf, 0xffab, 0xfc38, 0xffaf, 0xfd56,
        0xffc3, 0xfe3b, 0xffd9, 0xfeeb, 0xffec, 0xff68, 0xfff8, 0xffaf
    };

    std::vector<unsigned int>  FilterLowCoe_ = {
        0xfff8, 0x000b, 0xffee, 0x001c, 0xffd6, 0x003d, 0xffab, 0x0073,
        0xff66, 0x00ca, 0xfefc, 0x014c, 0xfe5d, 0x020d, 0xfd72, 0x032d,
        0xfc0c, 0x04f1, 0xf9bf, 0x0816, 0xf520, 0x0fbe, 0xe52d, 0x5160,
        0x5160, 0xe52d, 0x0fbe, 0xf520, 0x0816, 0xf9bf, 0x04f1, 0xfc0c,
        0x032d, 0xfd72, 0x020d, 0xfe5d, 0x014c, 0xfefc, 0x00ca, 0xff66,
        0x0073, 0xffab, 0x003d, 0xffd6, 0x001c, 0xffee, 0x000b, 0xfff8
    };

    CommonManager& CommonManager::Instance() noexcept {
        static CommonManager inst;
        return inst;
    }
    CommonManager* CommonManager::getInstance() {
        return &Instance();
    }
    CommonManager::CommonManager() noexcept
        : pcie_mem_(Device::Device_MEM32::getInstance()),
        aux_(std::make_unique<PLAYBACK::AUX_CTRL>()),
        adc_(std::make_unique<ADC::ADCApi>()),
        LogicAPI_(std::make_unique<LOGIC::LOGICApi>()),
        rfControl_(std::make_unique<RFCONTROL::RFControl>()),
        adcconfig_(std::make_unique<Common::ADCConfig>()),
        dacconfig_(std::make_unique<Common::DACConfig>()) 
    {
        LogCommonManagerStartup("stage=members_constructed");
        LogCommonManagerStartup("stage=SetRFType begin");
        SetRFType(RFType);
        LogCommonManagerStartup("stage=SetRFType done");
        LogCommonManagerStartup("stage=LoadFreqErrorData begin");
        LoadFreqErrorData();
        LogCommonManagerStartup("stage=LoadFreqErrorData done");
        //BSP_INIT_BREAK("CommonManager ctor done (RFType + freq-error data loaded)");
    }

    CommonManager::~CommonManager() noexcept = default;

    void CommonManager::InitDevice()
    { 
        SetADCClock();
        //BSP_INIT_BREAK("InitDevice: SetADCClock done (Logic FS/FFT_BW set)");
        pcie_mem_->SendData(0x00100000, 0x0000);
        //BSP_INIT_BREAK("InitDevice: preamble write 0x00100000=0x0000 done");
        aux_->interpolation_fir_dout_shift(0xA);
        set_reference_mode(0x2);
        //BSP_INIT_BREAK("InitDevice: fir_shift + reference_mode(0x2) done");
        ADCInit();
        //BSP_INIT_BREAK("InitDevice: ADCInit done");
        std::this_thread::sleep_for(10ms);
        /*DACInit();*/
        //BSP_INIT_BREAK("InitDevice: DACInit done");
        std::this_thread::sleep_for(1ms); 
        std::this_thread::sleep_for(1ms);
        Device::Device_Data_RTSA::getInstance()->set_trigger_source(0);
        pcie_mem_->SendData(0x000D1000, 1);
        //BSP_INIT_BREAK("InitDevice: trigger source + 0x000D1000=1 done (InitDevice end)");
    }
/**
 * @brief 初始化 PSCAN 设备，仅针对 MZ121/MZ121B 执行C#接收链配置
 * @return bool 初始化成功返回true；型号不支持或时钟/ADC配置失败返回false
 * @note 线程安全：内部持有互斥锁，防止多线程重复初始化
 * @details
 * 1. 如果已经完成初始化，直接跳过，返回true
 * 2. 非 MZ121 / MZ121B 硬件：明确返回失败，不回落到通用InitDevice()
 * 3. MZ121 / MZ121B 硬件：
 *    - 设置ADC基础时钟
 *    - 配置PSCAN系统时钟
 *    - 配置PSCAN ADC参数
 *    - 两次配置之间插入短暂延时，等待硬件寄存器生效
 *    - 时钟、ADC全部配置成功才置位初始化标记
 * 4. 输出完整初始化日志，记录时钟、ADC状态；跳过Win_API的PREAMBLE_DAC_RTSA_D1000流程
 */
#if 0
    bool CommonManager::InitPscanDevice()
    {
    // 加锁保护，避免多线程并发调用造成重复初始化
    std::lock_guard<std::mutex> lock(pscanInitMutex_);

    // 已经初始化完成，直接跳过初始化流程
    if (pscanInitialized_) {
        LogPscanClockInit("pscan_init action=SKIP reason=already_initialized");
        return true;
    }

    // 非MZ121/MZ121B型号不属于当前C# PSCAN接收初始化范围
    if (RFCONTROL::RFControl::RF_SELECT != RFCONTROL::RFType::MZ121 &&
        RFCONTROL::RFControl::RF_SELECT != RFCONTROL::RFType::MZ121B) {
        LogPscanClockInit(
            "pscan_init result=FAILED reason=unsupported_rf_type");
        return false;
    }

    // MZ121 / MZ121B 专项初始化流程
    LogPscanClockInit("pscan_init begin path=CSharp_receive_only");
    SetADCClock();        // 设置ADC基础时钟

    const bool clockOk = ConfigureMZ121PscanClock(pcie_mem_.get());
    std::this_thread::sleep_for(1ms);   // 延时等待时钟寄存器硬件生效

    const bool adcOk = ConfigureMZ121PscanAdc(pcie_mem_.get());
    std::this_thread::sleep_for(1ms);    // 延时等待ADC寄存器硬件生效

    // 时钟与ADC配置全部成功才算初始化完成
    const bool ok = clockOk && adcOk;
    pscanInitialized_ = ok;

    // 组装初始化结果日志，标记已跳过旧版Windows API动作
    std::ostringstream oss;
    oss << "pscan_init complete clock=" << (clockOk ? "OK" : "FAILED")
        << " adc=" << (adcOk ? "OK" : "FAILED")
        << " result=" << (ok ? "OK" : "FAILED")
        << " skipped_win_api_actions=PREAMBLE_DAC_RTSA_D1000";
    LogPscanClockInit(oss.str());

    return ok;
}
#endif

    void CommonManager::ADCInit()
    {
        //BSP_INIT_BREAK("ADCInit begin (adc_status check + si5386 init_clock)");
        if (!adc_->get_adc_status()) {
            init_clock(ADCSampleClock, ReferenceClock, Out_ReferenceClock);
        }
        //BSP_INIT_BREAK("ADCInit: si5386 clock init done");
        adc_->set_rf_gain(30, 0);
        std::this_thread::sleep_for(1ms);
        adcconfig_->adc_init(ADCSampleClock);
        std::this_thread::sleep_for(1ms);
        //BSP_INIT_BREAK("ADCInit: adc_init done");
        adcconfig_->set_adc_channel(0 << 8);
        std::this_thread::sleep_for(1ms);
        adcconfig_->adc_jesd_init();
        std::this_thread::sleep_for(1ms); 
        //BSP_INIT_BREAK("ADCInit: adc_jesd_init done");
    }

    void CommonManager::DACInit()
    {
        //BSP_INIT_BREAK("DACInit begin");
        dacconfig_->dac_jesd_init();
        std::this_thread::sleep_for(10ms);
        dacconfig_->ad9175_init(1228.8e6 / 4);
        //BSP_INIT_BREAK("DACInit: dac_jesd_init + ad9175_init done");
    }

    void CommonManager::set_reference_mode(uint8_t mode) {
        pcie_mem_->SendData(REFERENCE_CLOCK_REG, mode);
    }

    float CommonManager::get_fpga_temp() {
        unsigned char temp[4] = { 0 };
        pcie_mem_->ReadBackData(FPGA_TEMP_REG, 1, temp);
        unsigned int number = 0;
        std::memcpy(&number, temp, sizeof(unsigned int));
        return number * 501.3743f / 65536 - 273.6777f;
    }

    float CommonManager::get_fpga_volt()
    {
        unsigned char temp[4] = { 0 };
        pcie_mem_->ReadBackData(FPGA_VOLTAGE_REG, 1, temp);
        unsigned int number = 0;
        std::memcpy(&number, temp, sizeof(unsigned int));
        return number / 65536.0f * 3;
    }

    bool CommonManager::get_ddr_status(uint8_t ddrNo) {
        unsigned char temp[4] = { 0 };
        pcie_mem_->ReadBackData(DDR_STATUS_REG, 1, temp);
        return (temp[0] & ddrNo) == ddrNo;
    }

    void CommonManager::init_clock(uint64_t adc_clk, uint32_t ref_clk, uint32_t ref_out_clk)
    {
        int regNum = 0;
        if (adc_clk == 1228800000)
        {
            if (ref_clk == 10E6)
            {
                si5386 = si5386a_reve_registers_cfg1;
                regNum = SI5386A_1D2288G_REG_CONFIG_NUM_10M;
            }
            else if (ref_clk == 100E6)
            {
                if (ref_out_clk == 10E6)
                {
                    si5386 = si5386a_reve_registers_cfg4;
                    regNum = SI5386A_1D2288G_REG_CONFIG_NUM_IN100M_OUT10M;
                }
                else
                {
                    si5386 = si5386a_reve_registers_cfg2;
                    regNum = SI5386A_1D2288G_REG_CONFIG_NUM_IN100M_OUT100M;
                }

            }
        }
        else if (adc_clk == 819200000)
        {
            si5386 = si5386a_reve_registers_cfg3;
            regNum = SI5386A_819D2M_REG_CONFIG_NUM_10M;
        }

#if 0
        // Retain the former write-result diagnostics, but do not execute them.
        // Disable the active Win_API loop below before restoring this block.
        int failedWrites = 0;
        for (int i = 0; i < regNum; i++)
        {
            if (si5386[i].address == 0x0006) {
                Sleep(625);
            }
            if (!pcie_mem_->SendData(CLOCK_BASE_PAGE_REG, (si5386[i].address) >> 8 & 0xF)) {
                ++failedWrites;
            }
            if (!pcie_mem_->SendData(CLOCK_BASE_REG + (uint32_t)(si5386[i].address & 0xFF), (unsigned int)si5386[i].value)) {
                ++failedWrites;
            }
            Sleep(1);
        }

        std::ostringstream oss;
        oss << "si5386 table_entries=" << regNum
            << " bus_writes=" << (regNum * 2)
            << " failed_writes=" << failedWrites
            << " result=" << (regNum > 0 && failedWrites == 0 ? "OK" : "FAILED");
        LogPscanClockInit(oss.str());
#endif

        // Exact Win_API register order and delays; no logging in this timing path.
        for (int i = 0; i < regNum; i++)
        {
            if (si5386[i].address == 0x0006) {
                Sleep(625);
            }
            pcie_mem_->SendData(CLOCK_BASE_PAGE_REG, (si5386[i].address) >> 8 & 0xF);
            pcie_mem_->SendData(CLOCK_BASE_REG + (uint32_t)(si5386[i].address & 0xFF), (unsigned int)si5386[i].value);
            Sleep(1);
        }
    }

    void CommonManager::CloseDevice()
    {
        Device::Device_Data_RTSA::getInstance()->Device_CloseDevice();
        Device::Device_Data_Multi::getInstance()->Device_CloseDevice();
    }
    int CommonManager::get_system_status() {
        unsigned char temp[4] = { 0 };
        pcie_mem_->ReadBackData(SYSTEM_STATUS_REG, 1, temp);
        int number = 0;
        std::memcpy(&number, temp, sizeof(int));
        return number;
    }
    void CommonManager::update_si5386_firmware(uint64_t adc_clk, uint32_t ref_clk)
        {
        	int regNum = 0;
        	if (adc_clk == 1228800000)
        	{
        		if (ref_clk == 10E6)
        		{
                    si5386 = si5386a_reve_registers_cfg1;
        			regNum = SI5386A_1D2288G_REG_CONFIG_NUM_10M;
        		}
        		else if (ref_clk == 100E6)
        		{
        			si5386 = si5386a_reve_registers_cfg2;
        			regNum = SI5386A_1D2288G_REG_CONFIG_NUM_IN100M_OUT100M;
        		}
        	}
        	else if (adc_clk == 819200000)
        	{
        		si5386 = si5386a_reve_registers_cfg3;
        		regNum = SI5386A_819D2M_REG_CONFIG_NUM_10M;
        	}
        
        	for (int i = 0; i < regNum; i++)
        	{
        		if (si5386[i].address == 0x0006) {
        			Sleep(625);
        		}
        		pcie_mem_->SendData(CLOCK_BASE_PAGE_REG, (si5386[i].address) >> 8 & 0xF);
        		pcie_mem_->SendData(CLOCK_BASE_REG + (uint32_t)(si5386[i].address & 0xFF), (unsigned int)si5386[i].value);
        		//Sleep(1);
        	}
        	pcie_mem_->SendData(0x000B0201, 0x00);
        	pcie_mem_->SendData(0x000B02E3, 0xC7);
        	Sleep(1000);
        	unsigned char temp[4] = { 0 };
        	pcie_mem_->ReadBackData(0x000B02FE, 1, temp);
        	pcie_mem_->SendData(0x000B02E4, 0x1);
        	Sleep(1000);
        	unsigned char temp2[4] = { 0 };
        	pcie_mem_->ReadBackData(0x000B02FE, 1, temp2);
        }
        
    void CommonManager::SetADCClock() {
        ADCSpan = ADCSampleClock / 2;
        LogicAPI_->set_FS(ADCSpan);
        LogicAPI_->set_FFT_BW(ADCSpan);
    }

    void CommonManager::SetRFType(RFCONTROL::RFType rf_select) {
        rfControl_->SetRFCard(rf_select);
        pcie_mem_->SendData(0x00010005, 0x80048); //?
        //BSP_INIT_BREAK("SetRFType: first PCIe write done (0x00010005=0x00080048)");
    }

    void CommonManager::SetFilterCoe(const std::vector<unsigned int>& filterLowCoe,
        const std::vector<unsigned int>& filterCoe) {
        FilterLowCoe_ = filterLowCoe;
        FilterCoe_ = filterCoe;
    }
    void CommonManager::SetADCChannel(uint32_t channel)
    {
        adcconfig_->set_adc_channel(channel);
    }
    void CommonManager::SetWorkMode(Global::WorkMode workmode) {
        if (workmode == Global::WorkMode::SWEEP) {
            adcconfig_->set_adc_nco(ADCSampleClock / 4);
            int SpanRate = 8;
            adcconfig_->set_adc_dcm((uint8_t)SpanRate);
            adcconfig_->set_adc_filter(FilterCoe_, 0x0);
        }
        else if (workmode == Global::WorkMode::RTSA) {
           // RTSA 模式配置占位（与原逻辑保持对应）
        }
        else if (workmode == Global::WorkMode::STREAM) {
            // stream 模式配置占位
        }
    }

    uint32_t CommonManager::GetIFATT() {
        return rfControl_->GetIFATT();
    }
    uint32_t CommonManager::GetRFATT() {
        return rfControl_->GetRFATT();
    }
    void CommonManager::SetADCFilter(uint64_t centerfreq)
    {  
        if (centerfreq < 20E6) {
            adcconfig_->set_adc_nco(centerfreq);
            adcconfig_->set_adc_filter(FilterLowCoe_, 0x2);
        }
        else {
            double frequency2 = std::floor(centerfreq / 10000.0);
            int Lo_compensation2 = static_cast<int>(centerfreq - frequency2 * 10000.0);
            adcconfig_->set_adc_nco(static_cast<uint64_t>(1228.8e6 / 4 + Lo_compensation2));
            adcconfig_->set_adc_filter(FilterCoe_, 0x2);
        }
    }

  // 辅助函数：获取可执行文件所在目录
    std::string GetExePath() {
        // 实际实现取决于具体平台（Windows/Linux）
        // 这里是一个简化版本，需要根据实际情况实现
        return "./";  // 返回当前目录
    }

    double CommonManager::GetFreqErrorData(double freq) {
        try {
            double errorvalue = 0;
            double BeginFreq = 0;
            double EndFreq = 0;

             // 使用std::map的有序特性替代LINQ查询[6](@ref)
            auto& FreqErrorValue = Global::FreqErrorValue;

            /// 查找小于等于freq的最大键（BeginFreq）
            auto it_lower = FreqErrorValue.lower_bound(freq);
            if (it_lower != FreqErrorValue.end() && it_lower->first == freq) {
                // 如果正好找到freq，BeginFreq就是freq
                BeginFreq = freq;
            }
            else {
                // 否则找前一个元素（小于freq的最大值）
                if (it_lower != FreqErrorValue.begin()) {
                    --it_lower;
                    BeginFreq = it_lower->first;
                }
                else {
                    // û��С��freq�ļ�
                    BeginFreq = NAN; // ʹ��NAN��ʾ��Ч
                }
            }

            // ���Ҵ��ڵ���freq����С����EndFreq��
            auto it_upper = FreqErrorValue.upper_bound(freq);
            if (it_upper != FreqErrorValue.end()) {
                EndFreq = it_upper->first;
            }
            else {
                // û�д���freq�ļ�
                EndFreq = NAN; // ʹ��NAN��ʾ��Ч
            }

            // ѡ��Ҫʹ�õ�ErrorValueӳ�䣨����ԭ�����߼�������ֱ��ʹ��FreqErrorValue��
            std::map<double, double>* ErrorValue = nullptr;
            switch (RFCONTROL::RFControl::RF_SELECT) {
            case RFCONTROL::RFType::MZ116:
            case RFCONTROL::RFType::MZ121:
            case RFCONTROL::RFType::MZ121B:
            case RFCONTROL::RFType::CM18:
                ErrorValue = &FreqErrorValue;
                break;
            default:
                // ��������Ĭ�ϴ����򱣳�nullptr
                break;
            }

            if (!ErrorValue) {
                return 0;
            }

            // ����Ƿ���ھ�ȷƥ��
            auto exact_it = ErrorValue->find(freq);
            if (exact_it != ErrorValue->end()) {
                return exact_it->second;
            }
            else {
                // ���BeginFreq��EndFreq�Ƿ���Ч
                if (!std::isnan(BeginFreq) && !std::isnan(EndFreq) &&
                    ErrorValue->find(BeginFreq) != ErrorValue->end() &&
                    ErrorValue->find(EndFreq) != ErrorValue->end()) {

                    // ���Բ�ֵ����
                    double valueBegin = (*ErrorValue)[BeginFreq];
                    double valueEnd = (*ErrorValue)[EndFreq];

                    errorvalue = valueBegin + ((valueEnd - valueBegin) / (EndFreq - BeginFreq)) * (freq - BeginFreq);
                    return errorvalue;
                }
                else {
                    return 0;
                }
            }
        }
        catch (...) {
            // 捕获所有异常，返回0[9,10](@ref)
            return 0;
        }
    }
    void CommonManager::LoadFreqErrorData() {
        switch (RFCONTROL::RFControl::RF_SELECT) {
        case RFCONTROL::RFType::RF12: {
            std::string fullFileName = GetExePath() + "Calibration/Spectrum-loss-configs.xml";

            pugi::xml_document doc;
            pugi::xml_parse_result result = doc.load_file(fullFileName.c_str());

            if (result) {
                pugi::xml_node root = doc.child("Spectrum-loss-configs");
                for (pugi::xml_node node : root.children()) {
                    double freq = std::stod(node.attribute("freq").value()) * 1E6;
                    double powervalue = std::stod(node.attribute("ErrorPower").value());
                    Global::FreqErrorValue[freq] = powervalue;
                }
            }

            std::string AttFileName = GetExePath() + "Calibration/Spectrum-loss-configs-10dBAtt.xml";
            pugi::xml_document attDoc;
            if (attDoc.load_file(AttFileName.c_str())) {
                pugi::xml_node attRoot = attDoc.child("Spectrum-loss-configs");
                for (pugi::xml_node node : attRoot.children()) {
                    double freq = std::stod(node.attribute("freq").value()) * 1E6;
                    double powervalue = std::stod(node.attribute("ErrorPower").value());
                    Global::FreqATTErrorValue[freq] = powervalue;
                }
            }

            std::string PreampFileName = GetExePath() + "Calibration/Spectrum-loss-configs-Preamp.xml";
            pugi::xml_document preampDoc;
            if (preampDoc.load_file(PreampFileName.c_str())) {
                pugi::xml_node preampRoot = preampDoc.child("Spectrum-loss-configs");
                for (pugi::xml_node node : preampRoot.children()) {
                    double freq = std::stod(node.attribute("freq").value()) * 1E6;
                    double powervalue = std::stod(node.attribute("ErrorPower").value());
                    Global::FreqPreampErrorValue[freq] = powervalue;
                }
            }
            break;
        }
        case RFCONTROL::RFType::MZ116:
        case RFCONTROL::RFType::CM18: {
            std::string fullFileNameMz116 = GetExePath() + "Calibration/Spectrum-loss-configs-MZ116.xml";

            pugi::xml_document docMz116;
            if (docMz116.load_file(fullFileNameMz116.c_str())) {
                pugi::xml_node lossItems = docMz116.child("Spectrum-loss-configs").child("lossitems");
                for (pugi::xml_node node : lossItems.children()) {
                    double freq = std::stod(node.attribute("freq").value()) * 1E6;
                    double powervalue = std::stod(node.attribute("ErrorPower").value());
                    Global::FreqErrorValue[freq] = powervalue;
                } 

                pugi::xml_node errBase116 = docMz116.child("Spectrum-loss-configs").child("ErrBase");
                Global::BaseErrorValue = std::stof(errBase116.attribute("ErrBase").value());
            }
            break;
        }
        case RFCONTROL::RFType::MZ121:   {
            std::string fullFileNameMz121 = GetExePath() + "Calibration/Spectrum-loss-configs-MZ121.xml";

            pugi::xml_document docMz121;
            if (docMz121.load_file(fullFileNameMz121.c_str())) {
                // ��ͬƵ��У׼����
                pugi::xml_node lossItems = docMz121.child("Spectrum-loss-configs").child("lossitems");
                for (pugi::xml_node node : lossItems.children()) {
                    double freq = std::stod(node.attribute("freq").value()) * 1E6;
                    double powervalue = std::stod(node.attribute("ErrorPower").value());
                    Global::FreqErrorValue[freq] = powervalue;
                }

                LoadSweepRbwErrDIC(fullFileNameMz121);

                // ʵʱƵ�׶����������
                pugi::xml_node errBase121 = docMz121.child("Spectrum-loss-configs").child("ErrBase");
                Global::BaseErrorValue = std::stof(errBase121.attribute("ErrBase").value());

                // ɨƵ�����������
                pugi::xml_node sweepErrBase121 = docMz121.child("Spectrum-loss-configs").child("SweepErrBase");
                Global::SweepBaseErrorValue = std::stof(sweepErrBase121.attribute("ErrBase").value());
            }
            break;
        }
        case RFCONTROL::RFType::MZ121B: {
            std::string fullFileNameMz121B = GetExePath() + "Calibration/Spectrum-loss-configs-MZ121B.xml";

            pugi::xml_document docMz121B;
            if (docMz121B.load_file(fullFileNameMz121B.c_str())) {
                // ��ͬƵ��У׼����
                pugi::xml_node lossItems = docMz121B.child("Spectrum-loss-configs").child("lossitems");
                for (pugi::xml_node node : lossItems.children()) {
                    double freq = std::stod(node.attribute("freq").value()) * 1E6;
                    double powervalue = std::stod(node.attribute("ErrorPower").value());
                    Global::FreqErrorValue[freq] = powervalue;
                }

                LoadSweepRbwErrDIC(fullFileNameMz121B);

                // ʵʱƵ�׶����������
                pugi::xml_node errBase121 = docMz121B.child("Spectrum-loss-configs").child("ErrBase");
                Global::BaseErrorValue = std::stof(errBase121.attribute("ErrBase").value());

                // ɨƵ�����������
                pugi::xml_node sweepErrBase121B = docMz121B.child("Spectrum-loss-configs").child("SweepErrBase");
                Global::SweepBaseErrorValue = std::stof(sweepErrBase121B.attribute("ErrBase").value());
            }
            break;
        }
        default:
            break;
        } 
    }
    void CommonManager::LoadSweepRbwErrDIC(std::string fullFileName)  { 

        pugi::xml_document docMz121;
        if (docMz121.load_file(fullFileName.c_str())) {
            // ��ͬRBWУ׼����
            pugi::xml_node rbwItems = docMz121.child("Spectrum-loss-configs").child("rbwitems");
            for (pugi::xml_node node : rbwItems.children()) {
                int rbw = std::stoi(node.attribute("rbw").value());
                double powervalue = std::stod(node.attribute("ErrorPower").value());
                Global::RbwErrDIC[rbw] = powervalue;
            }
        }
    }

    void CommonManager::UpdateErrorValue(uint64_t CF) {
        Global::ErrorValue =  GetFreqErrorData(CF);
    }
    void CommonManager::Interp(const double* dataIn, int N, int n, double span, double* outBuffer)
    { 

        if (N <= 1 || n <= 0 || !dataIn || !outBuffer) return;
  
         
        double h = span / (N - 1);           // ԭʼ����
        double h_new = span / (n - 1);       // �µ���

        // ---------- ��Ȼ��������ϵ������ ----------
        std::vector<double> x(N), y(N);
        for (size_t i = 0; i < N; ++i) {
            x[i] = i * h;
            y[i] = dataIn[i];
        }

        std::vector<double> a = y;                 // a[i] = f(x[i])
        std::vector<double> b(N, 0.0);
        std::vector<double> d(N, 0.0);
        std::vector<double> c(N, 0.0);

        // 三弯矩法求二阶导数（自然样条：c[0]=c[N-1]=0）
        std::vector<double> mu(N, 0.0), lambda(N, 0.0), g(N, 0.0);
        for (size_t i = 1; i < N - 1; ++i) {
            lambda[i] = (i == N - 2) ? 1.0 : (h / (2 * h));
            mu[i] = 1.0 - lambda[i];
            g[i] = 3.0 * ((y[i + 1] - y[i]) / h - (y[i] - y[i - 1]) / h) / (2 * h);
        }

        // 前向消元 + 回代（Thomas 算法）
        std::vector<double> z(N, 0.0);
        std::vector<double> l(N, 0.0), u(N, 0.0), y_temp(N, 0.0);
        l[0] = 1.0; u[0] = 0.0; z[0] = 0.0;
        for (size_t i = 1; i < N - 1; ++i) {
            l[i] = 2.0 * (x[i + 1] - x[i - 1]) / h - h * mu[i - 1] * u[i - 1];
            u[i] = h * lambda[i] / l[i];
            z[i] = (g[i] - h * mu[i - 1] * z[i - 1]) / l[i];
        }
        l[N - 1] = 1.0; z[N - 1] = 0.0; c[N - 1] = 0.0;

        for (int i = N - 2; i >= 0; --i) {
            c[i] = z[i] - u[i] * c[i + 1];
            b[i] = (a[i + 1] - a[i]) / h - h * (c[i + 1] + 2.0 * c[i]) / 3.0;
            d[i] = (c[i + 1] - c[i]) / (3.0 * h);
        }

        // ---------- ��ÿ��Ŀ�����ֵ ----------
        auto eval = [&](double xi) -> double {
            // �ҵ����ڵ�����
            size_t i = 0;
            while (i < N - 1 && xi > x[i + 1]) ++i;

            double dx = xi - x[i];
            return a[i] + b[i] * dx + c[i] * dx * dx + d[i] * dx * dx * dx;
            };

        for (int i = 0; i < n; ++i) {
            double xi = i * h_new;
            outBuffer[i] = eval(xi);
        } 
    }
//#pragma managed(push, off)
//    void CommonManager::Interp(const double* dataIn, int dataSize, int n, double span, double* outBuffer)
//    {
//        if (dataSize <= 1 || n <= 0 || !dataIn || !outBuffer) return;
//
//        double h = span / (dataSize - 1);
//        double h2 = span / (n - 1);
//
//        boost::math::cubic_b_spline<double> spline(dataIn, dataSize, 0.0, h);
//
//        for (int i = 0; i < n; ++i)
//        {
//            outBuffer[i] = spline(i * h2);
//        }
//    }
//#pragma managed(pop)
}  
