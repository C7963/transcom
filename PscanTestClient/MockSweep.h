#pragma once
#include <vector>
#include <cmath>
#include <random>
#include <algorithm>
#include "PscanDefs.h"

// Mock Sweep - 模拟硬件Sweep数据源，用于测试Pscan模块
class MockSweep
{
public:
    static MockSweep& Instance()
    {
        static MockSweep instance;
        return instance;
    }

    // 模拟初始化参数
    void InitPara(double cf, double span, double rbw)
    {
        centerFreq_ = cf;
        span_ = span;
        rbw_ = rbw;
        pointCount_ = static_cast<uint32_t>(span / 100e3) + 1; // 100kHz步进
    }

    void SetCFSpan(double cf, double span)
    {
        centerFreq_ = cf;
        span_ = span;
        pointCount_ = static_cast<uint32_t>(span / 100e3) + 1;
    }

    void SetRBW(uint32_t rbw) { rbw_ = rbw; }
    void Config() {}

    uint32_t GetDataLen() const { return pointCount_; }

    // 获取模拟频谱数据（正峰值检波）
    bool GetPositivePeakData(std::vector<double>& buffer)
    {
        GenerateSpectrumData(buffer, PeakType::Positive);
        return true;
    }

    // 获取模拟频谱数据（负峰值检波）
    bool GetNegativeData(std::vector<double>& buffer)
    {
        GenerateSpectrumData(buffer, PeakType::Negative);
        return true;
    }

    // 获取模拟频谱数据（平均值检波）
    bool GetAverageData(std::vector<double>& buffer)
    {
        GenerateSpectrumData(buffer, PeakType::Average);
        return true;
    }

    // 获取模拟频谱数据（采样检波）
    bool GetSampleData(std::vector<double>& buffer)
    {
        GenerateSpectrumData(buffer, PeakType::Sample);
        return true;
    }

    // 获取模拟频谱数据（RMS检波）
    bool GetRMSData(std::vector<double>& buffer)
    {
        GenerateSpectrumData(buffer, PeakType::RMS);
        return true;
    }

    // 获取模拟频谱数据（AutoPeak检波）
    void GetAutoPeakData(std::vector<double>& highData, std::vector<double>& lowData)
    {
        GenerateSpectrumData(highData, PeakType::Positive);
        GenerateSpectrumData(lowData, PeakType::Negative);
    }

    // 设置模拟信号参数（用于测试）
    void SetSimSignal(double freq, double amplitude, double bandwidth = 1e6)
    {
        simFreq_ = freq;
        simAmplitude_ = amplitude;
        simBandwidth_ = bandwidth;
    }

    // 添加噪声底
    void SetNoiseFloor(double noiseFloor) { noiseFloor_ = noiseFloor; }

private:
    enum class PeakType { Positive, Negative, Average, Sample, RMS };

    MockSweep()
        : rng_(std::random_device{}())
        , dist_(0.0, 1.0)
    {
        // 默认模拟参数
        centerFreq_ = 2e9;
        span_ = 50e6;
        rbw_ = 200000;
        pointCount_ = 501;
        simFreq_ = 2e9;
        simAmplitude_ = -30.0;
        simBandwidth_ = 1e6;
        noiseFloor_ = -80.0;
    }

    void GenerateSpectrumData(std::vector<double>& buffer, PeakType peakType)
    {
        buffer.resize(pointCount_);
        double startFreq = centerFreq_ - span_ / 2.0;
        double step = (pointCount_ > 1) ? span_ / (pointCount_ - 1) : 0;

        for (uint32_t i = 0; i < pointCount_; ++i)
        {
            double freq = startFreq + step * i;

            // 基础噪声底
            double amp = noiseFloor_ + dist_(rng_) * 3.0;

            // 模拟信号：高斯形状的信号峰
            double freqDiff = freq - simFreq_;
            double gaussianAmp = simAmplitude_ * std::exp(-(freqDiff * freqDiff) / (2.0 * simBandwidth_ * simBandwidth_));
            amp += gaussianAmp;

            // 根据检波类型添加变化
            switch (peakType)
            {
            case PeakType::Positive:
                amp += dist_(rng_) * 2.0; // 正峰值略高
                break;
            case PeakType::Negative:
                amp -= dist_(rng_) * 2.0; // 负峰值略低
                break;
            case PeakType::Average:
                // 平均值保持不变
                break;
            case PeakType::Sample:
                amp += (dist_(rng_) - 0.5) * 5.0; // 采样波动更大
                break;
            case PeakType::RMS:
                amp -= 1.0; // RMS通常比峰值低
                break;
            }

            buffer[i] = amp;
        }
    }

    double centerFreq_;
    double span_;
    double rbw_;
    uint32_t pointCount_;

    // 模拟信号参数
    double simFreq_;
    double simAmplitude_;
    double simBandwidth_;
    double noiseFloor_;

    // 随机数生成器
    std::mt19937 rng_;
    std::uniform_real_distribution<double> dist_;
};
