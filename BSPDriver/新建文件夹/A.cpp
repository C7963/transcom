// 使用说明（Visual Studio）：
//   1. 选择 Debug|x64，把启动项目设为 BSPDriverConsole（本文件所在工程）。
//   2. 按 F5 调试运行。代码已在硬件初始化每个关键阶段埋好自动断点
//      （BSP_INIT_BREAK，仅 _DEBUG 生效），运行到每个阶段会自动中断，
//      可在 VS 输出窗口看到 "[BSP_INIT_BREAK] <阶段名>"。
//   3. 每到一个断点：检查调用栈、寄存器写地址/值（SendData）、
//      PCIe 打开失败 errno 等，然后按 F5 继续到下一阶段。
//   4. 若某阶段后控制台没有继续打印下一行，说明初始化卡在/失败在该阶段。

#include "CommonManager.h"

#include <windows.h>

#include <cstdint>
#include <exception>
#include <iomanip>
#include <iostream>
#include <string>

namespace {
    void PauseBeforeExit()
    {
        std::cout << "Press Enter to exit...";
        std::cin.get();
    }
}

//int main()
//{
//    // 修复控制台中文乱码：将输出代码页切换为 UTF-8，与源码/编译产物编码一致。
//    SetConsoleOutputCP(CP_UTF8);
//    SetConsoleCP(CP_UTF8);
//
//    try {
//        std::cout << "=== BSP hardware init debug console ===" << std::endl;
//
//        // Stage 1: CommonManager 单例构造。
//        // 构造函数内部执行 SetRFType()（第一次 PCIe 写 0x00010005 = 0x00080048）
//        // 和 LoadFreqErrorData()（加载校准数据）。
//        std::cout << "[1/3] Creating CommonManager (ctor -> SetRFType -> LoadFreqErrorData)..."
//            << std::endl;
//        Common::CommonManager& common = Common::CommonManager::Instance();
//
//        std::cout << "[2/3] CommonManager created. Starting InitDevice..." << std::endl;
//        std::cout << "      InitDevice flow: SetADCClock -> preamble write -> "
//            "fir_shift/ref_mode -> ADCInit -> DACInit -> trigger/0x000D1000"
//            << std::endl;
//
//        // Stage 2: 完整 Win_API 硬件初始化。代码内每个子阶段都有自动断点。
//        common.InitDevice();
//
//        // 与上位机判据一致的回读验证：ReadBackData(0x000C_0004, 1) & 3 == 3。
//        uint8_t adcStatus[4] = { 0 };
//        auto mem = Device::Device_MEM32::getInstance();
//        if (mem->ReadBackData(0x000C0004, 1, adcStatus)) {
//            const bool adcOk = (adcStatus[0] & 3) == 3;
//            std::cout << "ADC status reg 0x000C0004 = 0x"
//                << std::hex << std::uppercase << static_cast<int>(adcStatus[0])
//                << std::dec << " -> "
//                << (adcOk ? "匹配上位机判据: ADC 初始化成功 (bit1:0 == 11)"
//                    : "不满足上位机判据: ADC 状态 bit1:0 != 11")
//                << std::endl;
//        }
//        else {
//            std::cout << "ReadBackData(0x000C0004) failed" << std::endl;
//        }
//
//        std::cout << "[3/3] InitDevice returned OK." << std::endl;
//        PauseBeforeExit();
//        return 0;
//    }
//    catch (const std::exception& ex) {
//        std::cerr << "Hardware initialization exception: "
//            << ex.what() << std::endl;
//        PauseBeforeExit();
//        return 1;
//    }
//    catch (...) {
//        std::cerr << "Hardware initialization failed with an unknown exception."
//            << std::endl;
//        PauseBeforeExit();
//        return 2;
//    }
//}