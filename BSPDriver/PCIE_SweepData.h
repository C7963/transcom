#pragma once 
#include <string>
#include <mutex>
#include <memory>
#include <iostream>
#include "Device_Address.h"
#include "XillyFile.h"

// =====================================
// ����ģ�壺Positive / Negative / RMS / Average / Sample
// =====================================
template<const std::string* DevicePath>
class PCIE_Spectrum
{
public:
    // 单例模式，确保只初始化一次 instance_ 使用 once_flag
    static PCIE_Spectrum& Instance()
    {
        static PCIE_Spectrum instance;
        return instance;
    }

    // 重置FIFO读指针到起始位置（只应在重试前调用一次）
    void ResetFifo()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        try {
            if (!device_->is_opened()) {
                device_->open_file(CommBus::XillyFile::DeviceFileOpenType::e_ReadOnly);
            }
            device_->set_offset(0);
        }
        catch (...) {
            std::cerr << "ResetFifo failed for: " << *DevicePath << std::endl;
        }
    }

    // 从当前位置读取数据（不再调用set_offset，避免丢弃FIFO数据）
    uint32_t ReadSpectrumData(uint8_t* buffer, int len)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        bool reopenedThisRead = false;
        try {
            if (!device_->is_opened()) {
                if (!device_->open_file(CommBus::XillyFile::DeviceFileOpenType::e_ReadOnly)) {
                    std::cerr << "Open device failed: " << *DevicePath << std::endl;
                    return 0;
                }
                reopenedThisRead = true;
            }
        }
        catch (...) {
            std::cerr << "Open device failed: " << *DevicePath << std::endl;
            return 0;
        }

        uint32_t lenBytes = len * 4;
        try {
            // Match the C# PSCAN reader: reset the read0 file offset per frame.
            device_->set_offset(0);
            const uint32_t readBytes = device_->read_data(buffer, lenBytes);
            if (device_->isReadEOF()) {
                const bool closeOk = device_->close_file();
                std::cerr << "Spectrum FIFO EOF: " << *DevicePath
                    << " readBytes=" << readBytes
                    << " expectedBytes=" << lenBytes
                    << " close=" << (closeOk ? "OK" : "FAILED")
                    << " reopen_on_next_read=YES" << std::endl;
            }
            else if (reopenedThisRead) {
                std::cerr << "Spectrum FIFO reopened: " << *DevicePath << std::endl;
            }
            return readBytes;
        }
        catch (...) {
            std::cerr << "Read failed from: " << *DevicePath << std::endl;
            return 0;
        }
    }

protected:
    // 保护构造，防止外部实例化
    PCIE_Spectrum() : device_(new CommBus::XillyFile(*DevicePath)) {}
    // 禁止拷贝
    PCIE_Spectrum(const PCIE_Spectrum&) = delete;
    PCIE_Spectrum& operator=(const PCIE_Spectrum&) = delete;

private:
    std::unique_ptr<CommBus::XillyFile> device_;
    std::mutex mutex_;
};

// =====================================
// ȫ�ػ�ģ�壺Auto
// =====================================
template<>
class PCIE_Spectrum<&Device::Device_Address::Xillybus_spectrum_Auto>
{
public:
    static PCIE_Spectrum& Instance()
    {
        static PCIE_Spectrum instance;
        return instance;
    }

    bool EOF_Flag() const { return eof_; }

    void ResetFifo()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        openIfNeeded();
        try {
            spectrum_->set_offset(0);
        }
        catch (...) {
            std::cerr << "ResetFifo failed for Auto\n";
        }
    }

    uint32_t ReadSpectrumData(uint8_t* buffer, int maxBytes)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!buffer || maxBytes <= 0) return 0;

        openIfNeeded();
        try {
            uint32_t readbytes = spectrum_->read_data(buffer, maxBytes);
            eof_ = spectrum_->isReadEOF();
            return readbytes;
        }
        catch (...) {
            std::cerr << "ReadSpectrumData failed\n";
            eof_ = true;
            return 0;
        }
    }

    bool ReadRawData(uint8_t* buffer, uint32_t len)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        uint32_t lenBytes = len * 4;
        try {
            raw_->open_file(CommBus::XillyFile::DeviceFileOpenType::e_ReadOnly);
            raw_->read_data(buffer, lenBytes);
            raw_->close_file();
            eof_ = raw_->isReadEOF();
            return true;
        }
        catch (...) {
            std::cerr << "Read Raw Failed\n";
            return false;
        }
    }

    bool DeviceFileStatus() const { return spectrum_->is_opened(); }

private:
    PCIE_Spectrum()
    {
        spectrum_.reset(new CommBus::XillyFile(Device::Device_Address::Xillybus_spectrum_Auto));
        raw_.reset(new CommBus::XillyFile(Device::Device_Address::Xillybus_spectrum_Auto));
    }
    PCIE_Spectrum(const PCIE_Spectrum&) = delete;
    PCIE_Spectrum& operator=(const PCIE_Spectrum&) = delete;

    void openIfNeeded()
    {
        if (!spectrum_->is_opened()) {
            spectrum_->open_file(CommBus::XillyFile::DeviceFileOpenType::e_ReadOnly);
        }
    }

    std::unique_ptr<CommBus::XillyFile> spectrum_;
    std::unique_ptr<CommBus::XillyFile> raw_;
    bool eof_ = false;
    std::mutex mutex_;
};

// =====================================
// ������ Wrapper (���ֲ���)
// =====================================
using PCIE_SPECTRUM_Positive = PCIE_Spectrum<&Device::Device_Address::Xillybus_spectrum_Positive>;
using PCIE_SPECTRUM_Negative = PCIE_Spectrum<&Device::Device_Address::Xillybus_spectrum_Negative>;
using PCIE_SPECTRUM_RMS = PCIE_Spectrum<&Device::Device_Address::Xillybus_spectrum_RMS>;
using PCIE_SPECTRUM_Average = PCIE_Spectrum<&Device::Device_Address::Xillybus_spectrum_Average>;
using PCIE_SPECTRUM_Sample = PCIE_Spectrum<&Device::Device_Address::Xillybus_spectrum_Sample>;
using PCIE_SPECTRUM_Auto = PCIE_Spectrum<&Device::Device_Address::Xillybus_spectrum_Auto>;

namespace HardwareWrappers {
    // 读取函数（不再内部调用set_offset(0)，避免重试时丢弃FIFO数据）
    inline int ReadAuto(unsigned char* p, int len) { return PCIE_SPECTRUM_Auto::Instance().ReadSpectrumData(p, len); }
    inline int ReadPos(unsigned char* p, int len) { return PCIE_SPECTRUM_Positive::Instance().ReadSpectrumData(p, len); }
    inline int ReadNeg(unsigned char* p, int len) { return PCIE_SPECTRUM_Negative::Instance().ReadSpectrumData(p, len); }
    inline int ReadSmp(unsigned char* p, int len) { return PCIE_SPECTRUM_Sample::Instance().ReadSpectrumData(p, len); }
    inline int ReadRMS(unsigned char* p, int len) { return PCIE_SPECTRUM_RMS::Instance().ReadSpectrumData(p, len); }
    inline int ReadAvg(unsigned char* p, int len) { return PCIE_SPECTRUM_Average::Instance().ReadSpectrumData(p, len); }

    // FIFO重置函数（在读取前调用一次，将读指针移到起始位置）
    inline void ResetAuto() { PCIE_SPECTRUM_Auto::Instance().ResetFifo(); }
    inline void ResetPos() { PCIE_SPECTRUM_Positive::Instance().ResetFifo(); }
    inline void ResetNeg() { PCIE_SPECTRUM_Negative::Instance().ResetFifo(); }
    inline void ResetSmp() { PCIE_SPECTRUM_Sample::Instance().ResetFifo(); }
    inline void ResetRMS() { PCIE_SPECTRUM_RMS::Instance().ResetFifo(); }
    inline void ResetAvg() { PCIE_SPECTRUM_Average::Instance().ResetFifo(); }
}
