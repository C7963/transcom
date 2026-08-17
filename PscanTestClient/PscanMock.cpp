#include "PscanMock.h"
#include <cmath>
#include <random>
#include <chrono>

namespace PSCAN
{
    Pscan::Pscan()
    {
        config_.centerFreq = 2e9;
        config_.span = 100e6;
        config_.rbw = 200000;
        config_.step = 100e3;
        config_.refLevel = -30;
        config_.unitType = PSCANCONFIG::UnitType::dBm;
        config_.runMode = PSCANCONFIG::RunMode::Stop;

        // 创建默认数据接收器
        dataSink_ = std::make_shared<CallbackPscanDataSink>();
    }

    Pscan::~Pscan()
    {
        Stop();
    }

    Pscan& Pscan::GetInstance()
    {
        static Pscan instance;
        return instance;
    }

    void Pscan::Init(double cf, double span, uint32_t rbw)
    {
        config_.centerFreq = cf;
        config_.span = span;
        config_.rbw = rbw;
    }

    void Pscan::SetPara(const PSCANCONFIG::PscanConfig& config)
    {
        std::lock_guard<std::mutex> lock(dataMutex_);
        config_ = config;
    }

    void Pscan::Start()
    {
        if (isRunning_) return;

        runMode_ = PSCANCONFIG::RunMode::RunContinue;
        isRunning_ = true;
        acquireThread_ = std::thread(&Pscan::AcquisitionThread, this);
    }

    void Pscan::Stop()
    {
        runMode_ = PSCANCONFIG::RunMode::Stop;
        isRunning_ = false;

        if (acquireThread_.joinable())
        {
            acquireThread_.join();
        }
    }

    void Pscan::RunSingle()
    {
        if (isRunning_) return;

        runMode_ = PSCANCONFIG::RunMode::RunSingle;
        isRunning_ = true;
        acquireThread_ = std::thread(&Pscan::AcquisitionThread, this);
    }

    void Pscan::SetRunMode(PSCANCONFIG::RunMode mode)
    {
        if (mode == PSCANCONFIG::RunMode::RunContinue)
            Start();
        else if (mode == PSCANCONFIG::RunMode::RunSingle)
            RunSingle();
        else if (mode == PSCANCONFIG::RunMode::Stop)
            Stop();
    }

    PSCANCONFIG::RunMode Pscan::GetRunMode() const
    {
        return runMode_;
    }

    void Pscan::AcquisitionThread()
    {
        while (isRunning_)
        {
            // 生成模拟数据
            auto data = GenerateMockData();

            // 通过数据接收器上传
            if (dataSink_)
            {
                std::vector<std::pair<PSCANCONFIG::DetectorType, std::vector<PSCANCONFIG::FreqAmpData>>> dataPairs;
                dataPairs.push_back({ PSCANCONFIG::DetectorType::PositivePeak, data });
                dataSink_->OnDataArrived(dataPairs);

                // 如果是单次扫描模式，完成一次后停止
                if (runMode_ == PSCANCONFIG::RunMode::RunSingle)
                {
                    isRunning_ = false;
                    runMode_ = PSCANCONFIG::RunMode::Stop;
                    break;
                }
            }

            // 模拟扫描时间
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    std::vector<PSCANCONFIG::FreqAmpData> Pscan::GenerateMockData()
    {
        // 在锁内仅拷贝配置，释放锁后再生成数据，减少持锁时间
        double centerFreq, span, step, refLevel;
        {
            std::lock_guard<std::mutex> lock(dataMutex_);
            centerFreq = config_.centerFreq;
            span = config_.span;
            step = config_.step;
            refLevel = config_.refLevel;
        }

        std::vector<PSCANCONFIG::FreqAmpData> data;

        double startFreq = centerFreq - span / 2.0;
        double stopFreq = centerFreq + span / 2.0;

        if (step <= 0) step = span / 500.0;

        // 随机数生成器
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::normal_distribution<> noise(0, 2.0);

        // 生成模拟频谱数据 - 带有几个信号峰
        for (double freq = startFreq; freq <= stopFreq; freq += step)
        {
            double amp = refLevel - 40 + noise(gen); // 基础噪声

            // 添加几个模拟信号峰
            double normFreq = (freq - startFreq) / span;

            // 信号峰1
            double peak1Freq = 0.2;
            double peak1Amp = 20.0;
            amp += peak1Amp * std::exp(-std::pow((normFreq - peak1Freq) * 10, 2));

            // 信号峰2
            double peak2Freq = 0.5;
            double peak2Amp = 30.0;
            amp += peak2Amp * std::exp(-std::pow((normFreq - peak2Freq) * 15, 2));

            // 信号峰3
            double peak3Freq = 0.8;
            double peak3Amp = 15.0;
            amp += peak3Amp * std::exp(-std::pow((normFreq - peak3Freq) * 12, 2));

            data.push_back(PSCANCONFIG::FreqAmpData(freq, amp));
        }

        return data;
    }
}
