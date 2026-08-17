#pragma once
#include <cstdint>
#include <vector>
#include <string>

namespace PSCANCONFIG
{
    // 检波类型枚举
    enum class DetectorType : uint32_t
    {
        AutoPeak = 0,      // 双通 High + Low
        PositivePeak = 1,  // 正峰值
        NegativePeak = 2,  // 负峰值
        RMS = 3,           // 有效值
        Average = 4,       // 平均值
        Sample = 5         // 采样
    };

    // 迹线类型
    enum class TracesType : uint32_t
    {
        Blank = 0,
        Trace1 = 1,
        Trace2 = 2,
        Trace3 = 3,
        Trace4 = 4
    };

    // 迹线模式（帧间累积，对齐C# SpectrumDataHandle）
    enum class TraceMode : uint32_t
    {
        ClearWrite = 0,  // 每帧覆盖（实时）
        MaxHold = 1,     // 最大值保持
        MinHold = 2,     // 最小值保持
        Average = 3,     // 平均值
        View = 4         // 保持（冻结）
    };

    // 单位类型
    enum class UnitType : uint32_t
    {
        dBm = 0,
        dBmV = 1,
        dBmuV = 2,
        V = 3,
        W = 4,
        A = 5
    };

    // 运行模式
    enum class RunMode : uint32_t
    {
        Stop = 0,
        RunSingle = 1,
        RunContinue = 2
    };

    // 检波类型位掩码（用于硬件配置）
    enum DetectorBitmask : uint32_t
    {
        Detector_PositivePeak = 1,  // bit 0
        Detector_NegativePeak = 2,  // bit 1
        Detector_Average = 4,       // bit 2
        Detector_Sample = 8,        // bit 3
        Detector_RMS = 16           // bit 4
    };

    // 频率-幅度数据点
    struct FreqAmpData
    {
        double frequency;  // 频率 (Hz)
        double amplitude;  // 幅度 (根据单位类型)

        FreqAmpData() : frequency(0), amplitude(0) {}
        FreqAmpData(double freq, double amp) : frequency(freq), amplitude(amp) {}
    };

    // 迹线配置
    struct TraceModel
    {
        TracesType traceType = TracesType::Trace1;
        DetectorType detectorType = DetectorType::PositivePeak;
        TraceMode traceMode = TraceMode::ClearWrite;  // 帧间累积模式
        bool isEnabled = true;

        TraceModel() = default;
        TraceModel(TracesType tt, DetectorType dt, TraceMode tm = TraceMode::ClearWrite)
            : traceType(tt), detectorType(dt), traceMode(tm), isEnabled(true) {}
    };

    // PSCAN配置参数
    struct PscanConfig
    {
        double centerFreq = 2e9;       // 中心频率 (Hz)
        double span = 50e6;            // 跨度 (Hz)
        uint32_t rbw = 200000;         // 分辨率带宽 (Hz)
        double step = 100e3;           // 步进 (Hz)
        double refLevel = -30;         // 参考电平 (dBm)
        UnitType unitType = UnitType::dBm;
        RunMode runMode = RunMode::Stop;
        uint32_t detectorBitmask = Detector_PositivePeak;  // 默认正峰值
        std::vector<TraceModel> traceModels;

        PscanConfig()
        {
            // 默认添加一个正峰值迹线
            traceModels.emplace_back(TracesType::Trace1, DetectorType::PositivePeak);
        }

        // 获取起始频率
        double GetStartFreq() const { return centerFreq - span / 2.0; }

        // 获取结束频率
        double GetStopFreq() const { return centerFreq + span / 2.0; }

        // 获取数据点数量
        uint32_t GetPointCount() const
        {
            if (step <= 0) return 1;
            return static_cast<uint32_t>(span / step) + 1;
        }

        // 根据检波类型获取位掩码
        static uint32_t GetDetectorBitmask(DetectorType type)
        {
            switch (type)
            {
            case DetectorType::PositivePeak: return Detector_PositivePeak;
            case DetectorType::NegativePeak: return Detector_NegativePeak;
            case DetectorType::Average: return Detector_Average;
            case DetectorType::Sample: return Detector_Sample;
            case DetectorType::RMS: return Detector_RMS;
            default: return 0;
            }
        }

        // 从迹线模型更新检波位掩码
        void UpdateDetectorBitmask()
        {
            detectorBitmask = 0;
            for (const auto& trace : traceModels)
            {
                if (trace.isEnabled && trace.traceType != TracesType::Blank)
                {
                    detectorBitmask |= GetDetectorBitmask(trace.detectorType);
                }
            }
        }
    };
}
