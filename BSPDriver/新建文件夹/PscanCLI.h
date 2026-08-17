#pragma once
#include "Pscan.h"
#include "PscanDefs.h"
#include <msclr\marshal_cppstd.h>

using namespace System;
using namespace System::Collections::Generic;
using namespace System::Runtime::InteropServices;

ref class PscanCli
{
public:
    PscanCli();
    ~PscanCli();

    // 初始化
    void Init(double cf, double span, uint32_t rbw);

    // 设置参数
    void SetPara(
        double centerFreq,
        double span,
        uint32_t rbw,
        double step,
        double refLevel,
        int unitType,
        uint32_t detectorBitmask);

    // 添加或更新迹线
    void SetTrace(int traceType, int detectorType, bool enabled);

    // 启动/停止
    void Start();
    void Stop();
    void RunSingle();

    // 获取频谱数据
    bool GetSpectrumData(
        Dictionary<int, List<double>^>^% dataDict,
        Dictionary<int, List<double>^>^% freqDict);

    // 单位转换
    static double ConvertAmplitude(double ampDbm, int targetUnit);

    // 检波类型位掩码工具函数
    static uint32_t GetDetectorBitmask(int detectorType);

    // 数据事件（用于UI更新）
    event EventHandler^ DataArrived;

private:
    PSCAN::Pscan* pscan_;
    PSCANCONFIG::PscanConfig config_;
    System::Collections::Generic::List<PSCANCONFIG::TraceModel>^ traceModels_;

    // 内部数据回调
    void OnNativeDataArrived(
        const std::vector<std::pair<PSCANCONFIG::DetectorType, std::vector<PSCANCONFIG::FreqAmpData>>>& dataPairs);

    // 检波类型转换
    static PSCANCONFIG::DetectorType IntToDetectorType(int type);
    static PSCANCONFIG::TracesType IntToTracesType(int type);
    static PSCANCONFIG::UnitType IntToUnitType(int type);
};
