#pragma once

#include <cstdint>
#include <memory>

namespace Device {
class DeviceWorkMode;
class Device_MEM32;

// Multi-channel FIFO data path.
// This is intentionally independent of the spectrum scan path.
class Device_Data_Multi {
public:
    static Device_Data_Multi* getInstance();

    int Device_OpenDevice();
    int Device_CloseDevice();

    // FIFO reads used by the multi-channel path.
    bool getEOF() const;
    void setEOF(const bool& value);
    bool ReadPulseData(unsigned char* res, unsigned int readlen);
    bool ReadChannelsData(unsigned char* res, unsigned int readlen);

    ~Device_Data_Multi();

private:
    Device_Data_Multi();

    static Device_Data_Multi* instance;
    bool privateEOF = false;
    Device_MEM32* pcie_mem = nullptr;
    std::unique_ptr<DeviceWorkMode> m_impl;
};
} // namespace Device
