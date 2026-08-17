#include "IPscanDataSource.h"
#include "PscanDefs.h"
#include "PCIE_SweepData.h"
#include <cstring>

namespace PSCAN
{
    // PCIE数据源实现
    uint32_t PciePscanDataSource::ReadData(uint32_t detectorBitmask, uint8_t* buffer, uint32_t pointCount)
    {
        if (!buffer || pointCount == 0) return 0;

        // 根据位掩码确定需要读取的检波类型
        uint32_t totalBytes = 0;
        uint32_t offset = 0;

        // 正峰值
        if (detectorBitmask & PSCANCONFIG::Detector_PositivePeak)
        {
            uint32_t bytes = HardwareWrappers::ReadPos(buffer + offset, pointCount);
            if (bytes == 0) return 0;
            offset += bytes;
            totalBytes += bytes;
        }

        // 负峰值
        if (detectorBitmask & PSCANCONFIG::Detector_NegativePeak)
        {
            uint32_t bytes = HardwareWrappers::ReadNeg(buffer + offset, pointCount);
            if (bytes == 0) return 0;
            offset += bytes;
            totalBytes += bytes;
        }

        // 平均值
        if (detectorBitmask & PSCANCONFIG::Detector_Average)
        {
            uint32_t bytes = HardwareWrappers::ReadAvg(buffer + offset, pointCount);
            if (bytes == 0) return 0;
            offset += bytes;
            totalBytes += bytes;
        }

        // 采样
        if (detectorBitmask & PSCANCONFIG::Detector_Sample)
        {
            uint32_t bytes = HardwareWrappers::ReadSmp(buffer + offset, pointCount);
            if (bytes == 0) return 0;
            offset += bytes;
            totalBytes += bytes;
        }

        // 有效值
        if (detectorBitmask & PSCANCONFIG::Detector_RMS)
        {
            uint32_t bytes = HardwareWrappers::ReadRMS(buffer + offset, pointCount);
            if (bytes == 0) return 0;
            offset += bytes;
            totalBytes += bytes;
        }

        return totalBytes;
    }

    // Socket数据源实现（占位实现）
    uint32_t SocketPscanDataSource::ReadData(uint32_t detectorBitmask, uint8_t* buffer, uint32_t pointCount)
    {
        // Socket实现需要在实际网络环境中实现
        // 这里返回0表示未实现
        return 0;
    }

    void SocketPscanDataSource::SetSocketEndpoint(const std::string& ip, uint16_t port)
    {
        ip_ = ip;
        port_ = port;
    }
}
