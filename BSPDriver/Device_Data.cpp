#include "pch.h"
#include "Device_Data.h"
#include <exception>
#include <iostream>
#include <memory>
#include "Device_Address.h"
#include "Device_MEM32.h"
#include "XillyFile.h"

namespace Device {
namespace {
// Opens a FIFO lazily, reads one block, then restores the file offset.
// read6 is retained for the future multi-channel FFM path.
// No spectrum-specific device is owned here.
// EOF closes the FIFO so the next call starts cleanly.
bool ReadData(CommBus::XillyFile* device, unsigned char* buffer,
              unsigned int length, bool closeOnEof)
{
    if (device == nullptr || buffer == nullptr || length == 0) {
        return false;
    }

    try {
        if (!device->is_opened() &&
            !device->open_file(CommBus::XillyFile::e_ReadOnly)) {
            return false;
        }

        if (device->read_data(buffer, length) == 0) {
            return false;
        }

        device->set_offset(0);
        if (closeOnEof && device->isReadEOF()) {
            device->close_file();
        }
        return true;
    }
    catch (const std::exception& ex) {
        device->close_file();
        std::cout << "Read Error: " << ex.what() << std::endl;
        return false;
    }
}
} // namespace
class DeviceWorkMode {
public:
    virtual ~DeviceWorkMode() = default;
    virtual int Open() = 0;
    virtual int Close() = 0;
};
class Multi_Impl final : public DeviceWorkMode {
public:
    Multi_Impl()
        : pulseDevice_(std::make_unique<CommBus::XillyFile>(Device_Address::Xillybus_Read5_32)),
          channelsDevice_(std::make_unique<CommBus::XillyFile>(Device_Address::Xillybus_Read6_32))
    {
    }
    int Open() override
    {
        try {
            return pulseDevice_->open_file(CommBus::XillyFile::e_ReadOnly) ? 0 : -1;
        }
        catch (...) {
            return -1;
        }
    }
    int Close() override
    {
        try {
            if (pulseDevice_ && pulseDevice_->is_opened()) {
                pulseDevice_->close_file();
            }
            if (channelsDevice_ && channelsDevice_->is_opened()) {
                channelsDevice_->close_file();
            }
            return 0;
        }
        catch (...) {
            return -1;
        }
    }
    CommBus::XillyFile* pulse() const { return pulseDevice_.get(); }
    CommBus::XillyFile* channels() const { return channelsDevice_.get(); }

private:
    std::unique_ptr<CommBus::XillyFile> pulseDevice_;
    std::unique_ptr<CommBus::XillyFile> channelsDevice_;
};
Device_Data_Multi* Device_Data_Multi::instance = nullptr;
Device_Data_Multi::Device_Data_Multi()
    : pcie_mem(Device_MEM32::getInstance()),
      m_impl(std::make_unique<Multi_Impl>())
{
}
Device_Data_Multi::~Device_Data_Multi() = default;
Device_Data_Multi* Device_Data_Multi::getInstance()
{
    if (instance == nullptr) {
        instance = new Device_Data_Multi();
    }
    return instance;
}
int Device_Data_Multi::Device_OpenDevice()
{
    return m_impl->Open();
}
int Device_Data_Multi::Device_CloseDevice()
{
    return m_impl->Close();
}

bool Device_Data_Multi::getEOF() const
{
    return privateEOF;
}

void Device_Data_Multi::setEOF(const bool& value)
{
    privateEOF = value;
}

bool Device_Data_Multi::ReadPulseData(unsigned char* res, unsigned int readlen)
{
    const auto* impl = static_cast<Multi_Impl*>(m_impl.get());
    return ReadData(impl->pulse(), res, readlen, true);
}

bool Device_Data_Multi::ReadChannelsData(unsigned char* res, unsigned int readlen)
{
    const auto* impl = static_cast<Multi_Impl*>(m_impl.get());
    return ReadData(impl->channels(), res, readlen, true);
}
} // namespace Device
