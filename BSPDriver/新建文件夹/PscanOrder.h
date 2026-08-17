#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "SpectrumSweepFft.h"

namespace Device
{
    class Device_MEM32;
}

namespace CommBus
{
    class XillyFile;
}

namespace DATASERVICE
{
    enum class RFModeType : uint8_t
    {
        LOWD = 1,
        LOWN = 2,
        NORM = 3
    };

    struct PscanParameter
    {
        std::string Taskid;
        uint8_t antNo = 0;
        uint8_t ploarzation = 0;
        uint8_t Antgainswitch = 0;
        uint64_t Startfreq = 0;
        uint64_t Endfreq = 0;
        uint32_t Step = 0;
        uint8_t Rfatt = 0;
        uint8_t Agc = 0;
        uint8_t Ifatt = 0;
        uint8_t Rfmode = 0;
        uint16_t DetectorMode = 0;

        explicit PscanParameter(const std::vector<uint8_t>& data);
    };

    struct PscanOrderContext
    {
        std::string RFModule;
        uint32_t ADC0_Address = 0x000C2000;
        double Pscan_BW = 0.0;
        double Fs_ADC = 0.0;
        double Rf_Sub = 0.0;
        uint64_t RFRBW = 0;
        double PscanErrorValue = 179;
        double RFModeError = 0.0;
        double Lown = 0.0;
        double Lowd = 0.0;
    };

    class PscanDataModel
    {
    public:
        uint8_t Segmentid = 1;
        uint32_t Total = 1;
        uint64_t Startfreq = 0;
        uint64_t Stopfreq = 0;
        uint32_t RBW = 0;

        std::vector<uint8_t> GetData(
            const std::vector<uint8_t>& data,
            const PscanOrderContext& context,
            int pscanRFValue,
            int pscanIFValue) const;
    };

    class PscanPcieData
    {
    public:
        PscanPcieData();
        ~PscanPcieData();

        void OpenDevice();
        void CloseDevice();
        std::vector<uint8_t> ReadSpectrumData(uint32_t length);

    private:
        CommBus::XillyFile* device_ = nullptr;
    };

    class PscanOrder
    {
    public:
        PscanOrder();

        // Equivalent to the body of case OrderType.PScan after socket parsing.
        bool Handle(
            const std::vector<uint8_t>& body,
            const PscanOrderContext& context);

        // Equivalent to one iteration of StartNewPscanTask's reader/formatter.
        std::vector<uint8_t> ReadOneSpectrumFrame();

        const PscanParameter& Parameter() const { return pscanParameter_; }
        const PscanDataModel& DataModel() const { return pscanDataModel_; }
        const SWEEPCONFIG::SpectrumSweepFft& SpectrumSweepConfig() const
        {
            return spectrumSweepConfig_;
        }
        SWEEPCONFIG::SpectrumSweepFft& SpectrumSweepConfig()
        {
            return spectrumSweepConfig_;
        }

        uint32_t PscanSpectrumNum() const { return PscanSpectrumNum_; }
        int PscanRFValue() const { return pscanRFValue_; }
        int PscanIFValue() const { return pscanIFValue_; }

    private:
        bool IsValid(const PscanParameter& parameter) const;
        void RFATTFun(uint8_t rfgaindata);
        void IFATTFun(uint8_t ifAttdata, RFModeType mode);
        void ChangeRFMode(RFModeType modeType, const PscanOrderContext& context);
        void SetOrder(uint32_t orderLowdata, uint32_t orderHighData, uint32_t orderCodeType);

        Device::Device_MEM32* pcieMem_ = nullptr;
        PscanPcieData pscanDevice_;
        PscanParameter pscanParameter_{ std::vector<uint8_t>() };
        PscanDataModel pscanDataModel_;
        SWEEPCONFIG::SpectrumSweepFft spectrumSweepConfig_;
        PscanOrderContext context_;
        uint32_t PscanSpectrumNum_ = 1001;
        int pscanRFValue_ = 0;
        int pscanIFValue_ = 0;
        bool configured_ = false;
    };
}
