// pscan_test_main.cpp
// 用法：InitDevice 硬件初始化 -> PscanOrder::Handle 下发任务 -> ReadOneSpectrumFrame 读一帧
#include "CommonManager.h"
#include "PscanOrder.h"
#include "Device_Address.h"
#include "XillyFile.h"

#include <windows.h>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>

namespace {

    // 按 PscanParameter(vector<uint8_t>&) 解析格式拼装 body：
    //   [0..35]  Taskid (36字节)
    //   [36] antNo  [37] ploarzation  [38] Antgainswitch
    //   [39..46] Startfreq(u64)  [47..54] Endfreq(u64)  [55..58] Step(u32)
    //   [59] Rfatt  [60] Agc  [61] Ifatt  [62] Rfmode  [63..64] DetectorMode(u16)
    std::vector<uint8_t> BuildPscanBody(uint64_t startFreq, uint64_t endFreq,
        uint32_t step, uint8_t rfatt,
        uint8_t ifatt, uint8_t rfmode,
        uint16_t detectorMode)
    {
        std::vector<uint8_t> body(65, 0);
        const std::string taskId = "PSCAN_TEST";
        std::memcpy(body.data(), taskId.data(), taskId.size());
        // body[36] antNo=0, body[37] ploarzation=0, body[38] Antgainswitch=0
        std::memcpy(body.data() + 39, &startFreq, 8);
        std::memcpy(body.data() + 47, &endFreq, 8);
        std::memcpy(body.data() + 55, &step, 4);
        body[59] = rfatt;   // <= 30
        body[60] = 0;       // Agc
        body[61] = ifatt;   // <= 30
        body[62] = rfmode;  // 1=LOWD 2=LOWN 3=NORM
        std::memcpy(body.data() + 63, &detectorMode, 2);
        return body;
    }

    void DumpAmplitudeData(const std::vector<uint8_t>& data)
    {
        if (data.size() % sizeof(int16_t) != 0) {
            std::cout << "[FAIL] 幅度数据长度异常："
                << data.size() << " 字节" << std::endl;
            return;
        }

        const size_t count = data.size() / sizeof(int16_t);
        std::cout << "幅度数据大小=" << data.size() << " 字节" << std::endl;
        std::cout << "幅度点数=" << count << std::endl;
        for (size_t i = 0; i < count; ++i) {
            int16_t amp = 0;
            std::memcpy(&amp, data.data() + i * sizeof(int16_t), sizeof(amp));
            std::cout << "    amp[" << i << "]=" << (amp / 10.0) << " dB" << std::endl;
        }
    }

} // namespace

int main()
{
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    try {
        // 1) 硬件初始化（DAC/ADC/JESD/时钟）
        std::cout << "[1] InitDevice..." << std::endl;
        Common::CommonManager& common = Common::CommonManager::Instance();
        common.InitDevice();

        // 2) PScan 上下文（按工程实际值填写）
        DATASERVICE::PscanOrderContext ctx;
        ctx.RFModule = "mz121";      // 走 0x000C0007=3 + ADC 0x310/0x311 配置
        ctx.ADC0_Address = 0x000C2000;
        ctx.Pscan_BW = 614.4e6;      // 频谱带宽（与上位机一致）
        ctx.Fs_ADC = 1228.8e6;       // 采样率，与 CommonManager::ADCSampleClock 一致
        ctx.Rf_Sub = 180.0e6;        // 中频，按实际 RF 参数
        ctx.RFRBW = 1000;
        ctx.PscanErrorValue = 0.0;
        ctx.RFModeError = 0.0;
        ctx.Lown = 0.0;
        ctx.Lowd = 0.0;

        // 3) 下发 PScan 任务（参数必须通过 IsValid 校验）
        //    Start/End ∈ [9k, 18G]；Step ∈ {3125,6250,12500,25000,50000,100000,200000}
        //    Rfatt/Ifatt ≤ 30；Rfmode ∈ 1..3
        const std::vector<uint8_t> body = BuildPscanBody(
            911600000ULL,       // Startfreq 911.6 MHz
            931600000ULL,       // Endfreq 931.6 MHz
            200000,             // Step 200 kHz
            0,                  // Rfatt
            0,                  // Ifatt
            3,                  // Rfmode = NORM
            0);                 // DetectorMode = AutoPeak  改成88m 到108m
        DATASERVICE::PscanOrder order;
        if (!order.Handle(body, ctx)) {
            std::cout << "[FAIL] PscanOrder::Handle 失败（参数非法或 End<Start）" << std::endl;
            return 1;
        }
        std::cout << "[2] Pscan 配置成功，频谱点数=" << order.PscanSpectrumNum() << std::endl;

        // 4) 读一帧频谱（ReadOneSpectrumFrame 阻塞读，验证完整链路）
        std::cout << "[3] 读取一帧..." << std::endl;
        std::vector<uint8_t> amplitudeData = order.ReadOneSpectrumFrame();
        if (amplitudeData.empty()) {
            std::cout << "[FAIL] 未读到幅度数据" << std::endl;
            return 2;
        }
        DumpAmplitudeData(amplitudeData);
        std::cout << "[OK] 读取幅度数据成功" << std::endl;
    }
    catch (const std::exception& ex) {
        std::cerr << "异常: " << ex.what() << std::endl;
        return 3;
    }
    return 0;
}
