#pragma once
#include <memory>
#include <mutex>
#include <vector>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <queue>
#include <map>
#include "PscanDefs.h"
#include "IPscanDataSource.h"
#include "IPscanDataSink.h"
// Legacy Sweep path retained for rollback, but disabled for the RMS PSCAN path.
// #include "Sweep.h"
// #include "SpectrumSweep.h"
#include "PscanOrder.h"
#include "Global.h"

namespace PSCAN
{
    // 单帧数据包（生产者→消费者传递）
    struct FrameData
    {
        std::vector<std::pair<PSCANCONFIG::DetectorType, std::vector<double>>> rawFrames; // 各检波器的原始dB数据
    };

    // PSCAN核心类 - 负责初始化、参数设置、数据处理和上传
    class Pscan
    {
    public:
        Pscan();
        ~Pscan();

        // 单例模式
        static Pscan& GetInstance();

        void InitializeState(double cf, double span, uint32_t rbw);
        bool SetPara(const PSCANCONFIG::PscanConfig& config);
        const PSCANCONFIG::PscanConfig& GetConfig() const { return config_; }
        void SetDataSource(std::shared_ptr<IPscanDataSource> dataSource);
        void SetDataSink(std::shared_ptr<IPscanDataSink> dataSink);
        void Start();
        void Stop();
        void RunSingle();
        void SetRunMode(PSCANCONFIG::RunMode mode);
        PSCANCONFIG::RunMode GetRunMode() const;
        std::shared_ptr<IPscanDataSink> GetDataSink() const { return dataSink_; }
        std::vector<PSCANCONFIG::FreqAmpData> ProcessData(
            const std::vector<double>& dbData,
            PSCANCONFIG::DetectorType detectorType);
        void UploadToUI(
            const std::vector<std::pair<PSCANCONFIG::DetectorType, std::vector<PSCANCONFIG::FreqAmpData>>>& dataPairs);
        bool GetSpectrumData(
            std::vector<std::pair<PSCANCONFIG::DetectorType, std::vector<PSCANCONFIG::FreqAmpData>>>& outData);
        double ConvertAmplitude(double ampDbm, PSCANCONFIG::UnitType targetUnit) const;

    private:
        // 生产者线程：从硬件读取原始dB数据
        void ProducerThread();
        // 消费者线程：处理+迹线累积+上传
        void ConsumerThread();
        // Legacy Sweep reader retained as a comment for rollback:
        // bool ReadFromSweep(std::vector<double>& buffer, PSCANCONFIG::DetectorType detectorType);
        bool ReadRmsFrame(std::vector<double>& buffer);
        bool EnsureConfigured(const char* reason);
        bool ConfigureRmsOrder(const char* reason);

        // Cross-frame trace accumulation.
        void AccumulateTrace(
            std::vector<PSCANCONFIG::FreqAmpData>& currentFrame,
            PSCANCONFIG::DetectorType detectorType);

        // 成员变量
        PSCANCONFIG::PscanConfig config_;
        std::shared_ptr<IPscanDataSource> dataSource_;
        std::shared_ptr<IPscanDataSink> dataSink_;
        // Legacy members retained as comments for rollback:
        // SWEEPCONFIG::Sweep* sweep_;
        // std::unique_ptr<SWEEPCONFIG::SpectrumSweep> spectrumSweep_;
        std::unique_ptr<DATASERVICE::PscanOrder> pscanOrder_;

        // 线程控制
        std::thread producerThread_;
        std::thread consumerThread_;
        std::atomic<bool> isRunning_{ false };
        std::atomic<bool> isStopRequested_{ false };
        std::atomic<PSCANCONFIG::RunMode> runMode_{ PSCANCONFIG::RunMode::Stop };
        std::atomic<uint64_t> configVersion_{ 0 };
        std::atomic<bool> configured_{ false };
        std::mutex dataMutex_;
        std::condition_variable cv_;

        // Frame queue aligned with the C# ConcurrentQueue behavior.
        std::queue<FrameData> frameQueue_;
        std::mutex queueMutex_;
        std::condition_variable queueCV_;
        static const int MAX_QUEUE_SIZE = 10; // C# 默认10帧缓冲
        // Trace accumulation state by detector.
        std::map<PSCANCONFIG::DetectorType, std::vector<PSCANCONFIG::FreqAmpData>> traceAccumulator_;
        std::map<PSCANCONFIG::DetectorType, uint32_t> traceFrameCount_;

        // Working buffers.
        std::vector<uint8_t> rawBuffer_;
        std::vector<double> processedBuffer_;

        // 单次扫描缓存
        std::vector<std::pair<PSCANCONFIG::DetectorType, std::vector<PSCANCONFIG::FreqAmpData>>> cachedData_;
        bool hasCachedData_ = false;
    };
}
