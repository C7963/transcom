#include "PscanOrder.h"

#include "Device_Address.h"
#include "Device_MEM32.h"
#include "XillyFile.h"

#include <cmath>
#include <cstring>
#include <limits>

namespace DATASERVICE
{
    namespace
    {
        template <typename T>
        T ReadValue(const std::vector<uint8_t>& data, size_t offset)
        {
            T value{};
            if (offset + sizeof(T) <= data.size())
                std::memcpy(&value, data.data() + offset, sizeof(T));
            return value;
        }

    }

    PscanParameter::PscanParameter(const std::vector<uint8_t>& data)
    {
        const size_t taskLength = data.size() < 36 ? data.size() : 36;
        Taskid.assign(reinterpret_cast<const char*>(data.data()), taskLength);
        if (data.size() > 36) antNo = data[36];
        if (data.size() > 37) ploarzation = data[37];
        if (data.size() > 38) Antgainswitch = data[38];
        Startfreq = ReadValue<uint64_t>(data, 39);
        Endfreq = ReadValue<uint64_t>(data, 47);
        Step = ReadValue<uint32_t>(data, 55);
        if (data.size() > 59) Rfatt = data[59];
        if (data.size() > 60) Agc = data[60];
        if (data.size() > 61) Ifatt = data[61];
        if (data.size() > 62) Rfmode = data[62];
        if (data.size() > 63)
            DetectorMode = ReadValue<uint16_t>(data, 63);
        else
            DetectorMode = 0;
    }

    std::vector<uint8_t> PscanDataModel::GetData(
        const std::vector<uint8_t>& data,
        const PscanOrderContext& context,
        int pscanRFValue,
        int pscanIFValue) const
    {
        const size_t count = data.size() / sizeof(int32_t);
        std::vector<int32_t> value(count);
        std::memcpy(value.data(), data.data(), count * sizeof(int32_t));
        std::vector<int16_t> backData(count);

        for (size_t i = 0; i < count; ++i)
        {
            if (value[i] == 0)
                value[i] = 1;
            const double db =
                20.0 * std::log10(value[i]) - context.PscanErrorValue +
                context.RFModeError + pscanRFValue + pscanIFValue + 107.0;
            backData[i] = static_cast<int16_t>(std::round(db * 10.0));
        }

        const auto* ampBytes = reinterpret_cast<const uint8_t*>(backData.data());
        const size_t backByteLength = backData.size() * sizeof(int16_t);
        return std::vector<uint8_t>(ampBytes, ampBytes + backByteLength);
    }

    PscanPcieData::PscanPcieData()
        : device_(new CommBus::XillyFile(Device::Device_Address::Xillybus_Read0_32))
    {
    }

    PscanPcieData::~PscanPcieData()
    {
        CloseDevice();
        delete device_;
    }

    void PscanPcieData::OpenDevice()
    {
        if (device_ && !device_->is_opened())
            device_->open_file(CommBus::XillyFile::e_ReadOnly);
    }

    void PscanPcieData::CloseDevice()
    {
        if (device_ && device_->is_opened())
            device_->close_file();
    }

    std::vector<uint8_t> PscanPcieData::ReadSpectrumData(uint32_t length)
    {
        std::vector<uint8_t> data(length * 4);
        try
        {
            OpenDevice();
            if (!device_ || !device_->is_opened())
                return {};
            if (device_->read_data(data.data(), static_cast<uint32_t>(data.size())) == 0)
                return {};
            if (device_->isReadEOF())
                CloseDevice();
            return data;
        }
        catch (...)
        {
            CloseDevice();
            return {};
        }
    }

    PscanOrder::PscanOrder()
        : pcieMem_(Device::Device_MEM32::getInstance())
    {
    }

    bool PscanOrder::IsValid(const PscanParameter& parameter) const
    {
        if (parameter.Startfreq < 9000 || parameter.Startfreq > 18000000000ULL) return false;
        if (parameter.Endfreq < 9000 || parameter.Endfreq > 18000000000ULL) return false;
        if (parameter.Step != 3125 && parameter.Step != 6250 && parameter.Step != 12500 &&
            parameter.Step != 25000 && parameter.Step != 50000 && parameter.Step != 100000 &&
            parameter.Step != 200000) return false;
        if (parameter.Rfatt > 30 || parameter.Ifatt > 30) return false;
        if (parameter.Rfmode < 1 || parameter.Rfmode > 3) return false;
        return true;
    }

    void PscanOrder::SetOrder(uint32_t orderLowdata, uint32_t orderHighData, uint32_t orderCodeType)
    {
        pcieMem_->SendData(0x00010001, static_cast<uint32_t>(3 * std::pow(2, 16) + 1));
        pcieMem_->SendData(0x00010003, orderLowdata);
        pcieMem_->SendData(0x00010004, orderHighData);
        pcieMem_->SendData(0x00010000, static_cast<uint32_t>(1 * std::pow(2, 31) + orderCodeType));
    }

    void PscanOrder::RFATTFun(uint8_t rfgaindata)
    {
        SetOrder(static_cast<uint32_t>(rfgaindata) * 4, 0, 1);
    }

    void PscanOrder::IFATTFun(uint8_t ifAttdata, RFModeType)
    {
        SetOrder(static_cast<uint32_t>(ifAttdata) * 4, 0, 2);
    }

    void PscanOrder::ChangeRFMode(RFModeType modeType, const PscanOrderContext& context)
    {
        if (modeType == RFModeType::NORM)
        {
            context_.RFModeError = 0;
            SetOrder(0, 0, 3);
        }
        else if (modeType == RFModeType::LOWN)
        {
            context_.RFModeError = context.Lown;
            SetOrder(1, 0, 3);
        }
        else if (modeType == RFModeType::LOWD)
        {
            context_.RFModeError = context.Lowd;
            SetOrder(2, 0, 3);
        }
    }

    bool PscanOrder::Handle(
        const std::vector<uint8_t>& body,
        const PscanOrderContext& context)
    {
        context_ = context;
        // Same order as case OrderType.PScan in DataService.cs:
        // close devices, select PSCAN logic, configure ADC, then parse body.
        pscanDevice_.CloseDevice();
        pcieMem_->SendData(0x00005010, 1);
        if (context.RFModule == "mz121")
        {
            pcieMem_->SendData(0x000C0007, 3);
            pcieMem_->SendData(context.ADC0_Address + 0x310, 0x47);
            pcieMem_->SendData(context.ADC0_Address + 0x311, 0x70);
        }
        else
        {
            pcieMem_->SendData(0x000C0007, 2);
            pcieMem_->SendData(context.ADC0_Address + 0x310, 0x43);
        }

        pscanParameter_ = PscanParameter(body);
        if (pscanParameter_.Endfreq < pscanParameter_.Startfreq)
            return false;
        if (!IsValid(pscanParameter_))
            return false;

        pcieMem_->SendData(0x00006000, 0);
        IFATTFun(pscanParameter_.Ifatt, static_cast<RFModeType>(pscanParameter_.Rfmode));
        RFATTFun(pscanParameter_.Rfatt);
        ChangeRFMode(static_cast<RFModeType>(pscanParameter_.Rfmode), context);
        pscanRFValue_ = pscanParameter_.Rfatt;
        pscanIFValue_ = pscanParameter_.Ifatt;
        pcieMem_->SendData(0x00006000, 1);

        spectrumSweepConfig_.Mode = 0;
        spectrumSweepConfig_.FS_ADC = context.Fs_ADC;
        spectrumSweepConfig_.Rf_Sub = context.Rf_Sub;
        spectrumSweepConfig_.CenterFrequency = (pscanParameter_.Endfreq + pscanParameter_.Startfreq) / 2.0;
        spectrumSweepConfig_.Rf_Start = pscanParameter_.Startfreq;
        spectrumSweepConfig_.Rf_Stop = pscanParameter_.Endfreq;
        spectrumSweepConfig_.Span = pscanParameter_.Endfreq - pscanParameter_.Startfreq;
        spectrumSweepConfig_.Step = pscanParameter_.Step;
        spectrumSweepConfig_.Det_Len = 1001;
        spectrumSweepConfig_.BW = context.Pscan_BW;
        spectrumSweepConfig_.Config();

        pscanDataModel_.Startfreq = pscanParameter_.Startfreq;
        pscanDataModel_.Stopfreq = pscanParameter_.Endfreq;
        pscanDataModel_.RBW = pscanParameter_.Step;
        PscanSpectrumNum_ = static_cast<uint32_t>(
            (pscanDataModel_.Stopfreq - pscanDataModel_.Startfreq) /
            pscanDataModel_.RBW) + 1;
        configured_ = true;
        return true;
    }

    std::vector<uint8_t> PscanOrder::ReadOneSpectrumFrame()
    {
        if (!configured_)
            return {};
        const uint32_t num = static_cast<uint32_t>(spectrumSweepConfig_.DataNum + 1);
        const std::vector<uint8_t> spectrumData = pscanDevice_.ReadSpectrumData(num);
        if (spectrumData.empty())
            return {};
        std::vector<uint8_t> data = spectrumData;
        if (pscanParameter_.DetectorMode == 0)
            data.resize(static_cast<size_t>(PscanSpectrumNum_) * 4);
        return pscanDataModel_.GetData(data, context_, pscanRFValue_, pscanIFValue_);
    }
}
