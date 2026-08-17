#include "FfmDataSource.h"

#include "Device_Address.h"
#include "FfmHardwareProfile.h"
#include "XillyFile.h"

namespace FFM {
FfmDataSource::FfmDataSource()
    : device_(std::make_unique<CommBus::XillyFile>(Device::Device_Address::Xillybus_Read3_32)) {}
FfmDataSource::~FfmDataSource() { Close(); }

bool FfmDataSource::Open()
{
    return device_ && (device_->is_opened() || device_->open_file(CommBus::XillyFile::e_ReadOnly));
}

void FfmDataSource::Close()
{
    if (device_ && device_->is_opened()) device_->close_file();
}

bool FfmDataSource::ReadFrame(std::vector<uint8_t>& bytes)
{
    bytes.assign(FfmHardwareProfile::RawFrameBytes, 0U);
    if (!Open()) return false;
    device_->set_offset(0);
    const uint32_t actual = device_->read_data(bytes.data(), static_cast<uint32_t>(bytes.size()));
    return actual == bytes.size();
}
} // namespace FFM
