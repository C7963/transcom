#pragma once
#include <vector>
#include <map>
#include <functional>
#include "PscanDefs.h"

namespace PSCAN
{
    // 数据接收器接口 - 抽象数据上传方式（回调/事件/观察者）
    class IPscanDataSink
    {
    public:
        virtual ~IPscanDataSink() = default;

        // 数据到达回调 - 传递处理后的频谱数据
        // dataPairs: 按检波类型分组的频率-幅度数据
        virtual void OnDataArrived(
            const std::vector<std::pair<PSCANCONFIG::DetectorType, std::vector<PSCANCONFIG::FreqAmpData>>>& dataPairs) = 0;

        // 频谱图数据到达回调（可选）
        virtual void OnSpectrogramDataArrived(const std::vector<PSCANCONFIG::FreqAmpData>& spectrogramData) {}
    };

    // 基于回调函数的数据接收器实现
    class CallbackPscanDataSink : public IPscanDataSink
    {
    public:
        using DataCallback = std::function<void(
            const std::vector<std::pair<PSCANCONFIG::DetectorType, std::vector<PSCANCONFIG::FreqAmpData>>>&)>;

        using SpectrogramCallback = std::function<void(
            const std::vector<PSCANCONFIG::FreqAmpData>&)>;

        CallbackPscanDataSink() = default;

        void SetDataCallback(DataCallback callback)
        {
            dataCallback_ = std::move(callback);
        }

        void SetSpectrogramCallback(SpectrogramCallback callback)
        {
            spectrogramCallback_ = std::move(callback);
        }

        void OnDataArrived(
            const std::vector<std::pair<PSCANCONFIG::DetectorType, std::vector<PSCANCONFIG::FreqAmpData>>>& dataPairs) override
        {
            if (dataCallback_)
            {
                dataCallback_(dataPairs);
            }
        }

        void OnSpectrogramDataArrived(
            const std::vector<PSCANCONFIG::FreqAmpData>& spectrogramData) override
        {
            if (spectrogramCallback_)
            {
                spectrogramCallback_(spectrogramData);
            }
        }

    private:
        DataCallback dataCallback_;
        SpectrogramCallback spectrogramCallback_;
    };

    // 基于观察者模式的数据接收器
    class ObservablePscanDataSink : public IPscanDataSink
    {
    public:
        using ObserverId = uint32_t;

        // 添加观察者
        ObserverId AddObserver(IPscanDataSink* observer)
        {
            ObserverId id = nextId_++;
            observers_[id] = observer;
            return id;
        }

        // 移除观察者
        void RemoveObserver(ObserverId id)
        {
            observers_.erase(id);
        }

        void OnDataArrived(
            const std::vector<std::pair<PSCANCONFIG::DetectorType, std::vector<PSCANCONFIG::FreqAmpData>>>& dataPairs) override
        {
            for (auto& pair : observers_)
            {
                if (pair.second)
                {
                    pair.second->OnDataArrived(dataPairs);
                }
            }
        }

        void OnSpectrogramDataArrived(
            const std::vector<PSCANCONFIG::FreqAmpData>& spectrogramData) override
        {
            for (auto& pair : observers_)
            {
                if (pair.second)
                {
                    pair.second->OnSpectrogramDataArrived(spectrogramData);
                }
            }
        }

    private:
        std::map<ObserverId, IPscanDataSink*> observers_;
        ObserverId nextId_ = 1;
    };
}
