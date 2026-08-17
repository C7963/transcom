#ifndef PSCAN_MOCK_H
#define PSCAN_MOCK_H

#include <memory>
#include <mutex>
#include <thread>
#include <atomic>
#include <vector>
#include <functional>
#include "PscanDefs.h"
#include "IPscanDataSink.h"

namespace PSCAN
{
    // Mock Pscan class for test client - 不依赖完整BSPDriver
    class Pscan
    {
    public:
        Pscan();
        ~Pscan();

        static Pscan& GetInstance();

        // 初始化
        void Init(double cf, double span, uint32_t rbw);

        // 设置参数
        void SetPara(const PSCANCONFIG::PscanConfig& config);

        // 获取当前配置
        const PSCANCONFIG::PscanConfig& GetConfig() const { return config_; }

        // 启动/停止数据采集
        void Start();
        void Stop();
        void RunSingle();

        // 运行模式控制
        void SetRunMode(PSCANCONFIG::RunMode mode);
        PSCANCONFIG::RunMode GetRunMode() const;

        // 获取数据接收器
        std::shared_ptr<IPscanDataSink> GetDataSink() const { return dataSink_; }

        // 设置数据接收器
        void SetDataSink(std::shared_ptr<IPscanDataSink> dataSink) { dataSink_ = dataSink; }

    private:
        // 模拟采集线程
        void AcquisitionThread();

        // 生成模拟数据
        std::vector<PSCANCONFIG::FreqAmpData> GenerateMockData();

        PSCANCONFIG::PscanConfig config_;
        std::shared_ptr<IPscanDataSink> dataSink_;

        std::thread acquireThread_;
        std::atomic<bool> isRunning_{ false };
        std::atomic<PSCANCONFIG::RunMode> runMode_{ PSCANCONFIG::RunMode::Stop };
        std::mutex dataMutex_;
    };
}

#endif // PSCAN_MOCK_H
