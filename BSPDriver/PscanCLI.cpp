#include "PscanCLI.h"

using namespace System;
using namespace System::Collections::Generic;
using namespace System::Runtime::InteropServices;

PscanCli::PscanCli()
{
    pscan_ = &PSCAN::Pscan::GetInstance();
    traceModels_ = gcnew System::Collections::Generic::List<PSCANCONFIG::TraceModel>();
    traceModels_->Add(PSCANCONFIG::TraceModel(PSCANCONFIG::TracesType::Trace1, PSCANCONFIG::DetectorType::PositivePeak));
}

PscanCli::~PscanCli()
{
    // 注意：不删除pscan_，因为它是单例
}

void PscanCli::Init(double cf, double span, uint32_t rbw)
{
    config_.centerFreq = cf;
    config_.span = span;
    config_.rbw = rbw;
    config_.traceModels.clear();
    for (int i = 0; i < traceModels_->Count; ++i)
    {
        config_.traceModels.push_back(traceModels_[i]);
    }
    config_.UpdateDetectorBitmask();

    pscan_->Init(cf, span, rbw);
}

void PscanCli::SetPara(
    double centerFreq,
    double span,
    uint32_t rbw,
    double step,
    double refLevel,
    int unitType,
    uint32_t detectorBitmask)
{
    config_.centerFreq = centerFreq;
    config_.span = span;
    config_.rbw = rbw;
    config_.step = step;
    config_.refLevel = refLevel;
    config_.unitType = IntToUnitType(unitType);
    config_.detectorBitmask = detectorBitmask;
    config_.traceModels.clear();
    for (int i = 0; i < traceModels_->Count; ++i)
    {
        config_.traceModels.push_back(traceModels_[i]);
    }

    pscan_->SetPara(config_);
}

void PscanCli::SetTrace(int traceType, int detectorType, bool enabled)
{
    PSCANCONFIG::TracesType tt = IntToTracesType(traceType);
    PSCANCONFIG::DetectorType dt = IntToDetectorType(detectorType);

    // 查找或创建迹线
    bool found = false;
    for (int i = 0; i < traceModels_->Count; ++i)
    {
        if (traceModels_[i].traceType == tt)
        {
            PSCANCONFIG::TraceModel updated(tt, dt);
            updated.isEnabled = enabled;
            traceModels_[i] = updated;
            found = true;
            break;
        }
    }

    if (!found)
    {
        PSCANCONFIG::TraceModel newTrace(tt, dt);
        newTrace.isEnabled = enabled;
        traceModels_->Add(newTrace);
    }

    // 更新配置
    config_.traceModels.clear();
    for (int i = 0; i < traceModels_->Count; ++i)
    {
        config_.traceModels.push_back(traceModels_[i]);
    }
    config_.UpdateDetectorBitmask();
    pscan_->SetPara(config_);
}

void PscanCli::Start()
{
    pscan_->Start();
}

void PscanCli::Stop()
{
    pscan_->Stop();
}

void PscanCli::RunSingle()
{
    pscan_->RunSingle();
}

bool PscanCli::GetSpectrumData(
    Dictionary<int, List<double>^>^% dataDict,
    Dictionary<int, List<double>^>^% freqDict)
{
    std::vector<std::pair<PSCANCONFIG::DetectorType, std::vector<PSCANCONFIG::FreqAmpData>>> outData;

    if (!pscan_->GetSpectrumData(outData))
    {
        return false;
    }

    // 转换为C#数据结构
    dataDict = gcnew Dictionary<int, List<double>^>();
    freqDict = gcnew Dictionary<int, List<double>^>();

    for (const auto& [detectorType, freqAmpData] : outData)
    {
        int detTypeInt = static_cast<int>(detectorType);

        auto ampList = gcnew List<double>();
        auto freqList = gcnew List<double>();

        for (const auto& point : freqAmpData)
        {
            ampList->Add(point.amplitude);
            freqList->Add(point.frequency);
        }

        dataDict[detTypeInt] = ampList;
        freqDict[detTypeInt] = freqList;
    }

    return true;
}

double PscanCli::ConvertAmplitude(double ampDbm, int targetUnit)
{
    PSCANCONFIG::UnitType unit = IntToUnitType(targetUnit);

    switch (unit)
    {
    case PSCANCONFIG::UnitType::dBm:
        return ampDbm;
    case PSCANCONFIG::UnitType::dBmV:
        return ampDbm + 46.99;
    case PSCANCONFIG::UnitType::dBmμV:
        return ampDbm + 107.0;
    case PSCANCONFIG::UnitType::V:
        {
            double powerMw = Math::Pow(10.0, ampDbm / 10.0);
            double voltageV = Math::Sqrt(powerMw / 1000.0 * 50.0) * 1000.0;
            return voltageV;
        }
    case PSCANCONFIG::UnitType::W:
        {
            double powerMw = Math::Pow(10.0, ampDbm / 10.0);
            return powerMw / 1000.0;
        }
    case PSCANCONFIG::UnitType::A:
        {
            double powerMw = Math::Pow(10.0, ampDbm / 10.0);
            double currentA = Math::Sqrt(powerMw / 1000.0 / 50.0) * 1000.0;
            return currentA;
        }
    default:
        return ampDbm;
    }
}

uint32_t PscanCli::GetDetectorBitmask(int detectorType)
{
    return PSCANCONFIG::PscanConfig::GetDetectorBitmask(IntToDetectorType(detectorType));
}

void PscanCli::OnNativeDataArrived(
    const std::vector<std::pair<PSCANCONFIG::DetectorType, std::vector<PSCANCONFIG::FreqAmpData>>>& dataPairs)
{
    // 触发C#事件
    if (DataArrived)
    {
        DataArrived(this, EventArgs::Empty);
    }
}

PSCANCONFIG::DetectorType PscanCli::IntToDetectorType(int type)
{
    switch (type)
    {
    case 0: return PSCANCONFIG::DetectorType::AutoPeak;
    case 1: return PSCANCONFIG::DetectorType::PositivePeak;
    case 2: return PSCANCONFIG::DetectorType::NegativePeak;
    case 3: return PSCANCONFIG::DetectorType::RMS;
    case 4: return PSCANCONFIG::DetectorType::Average;
    case 5: return PSCANCONFIG::DetectorType::Sample;
    default: return PSCANCONFIG::DetectorType::PositivePeak;
    }
}

PSCANCONFIG::TracesType PscanCli::IntToTracesType(int type)
{
    switch (type)
    {
    case 0: return PSCANCONFIG::TracesType::Blank;
    case 1: return PSCANCONFIG::TracesType::Trace1;
    case 2: return PSCANCONFIG::TracesType::Trace2;
    case 3: return PSCANCONFIG::TracesType::Trace3;
    case 4: return PSCANCONFIG::TracesType::Trace4;
    default: return PSCANCONFIG::TracesType::Trace1;
    }
}

PSCANCONFIG::UnitType PscanCli::IntToUnitType(int type)
{
    switch (type)
    {
    case 0: return PSCANCONFIG::UnitType::dBm;
    case 1: return PSCANCONFIG::UnitType::dBmV;
    case 2: return PSCANCONFIG::UnitType::dBmμV;
    case 3: return PSCANCONFIG::UnitType::V;
    case 4: return PSCANCONFIG::UnitType::W;
    case 5: return PSCANCONFIG::UnitType::A;
    default: return PSCANCONFIG::UnitType::dBm;
    }
}
