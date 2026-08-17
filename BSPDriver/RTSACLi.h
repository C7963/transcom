#pragma once  
#include "RTSA.h"
#include "DataRead.h"
#include <vector>
#include <array> 
#include "CommonCLI.h"

using namespace System;
using namespace System::Collections::Generic;
using namespace System::Runtime::InteropServices;
using namespace RTSAControl; 
using namespace CommonControlCLI;

namespace RTSAControlCLI {
    public value struct ManagedFreqAmpData
    {
        double Frequency;
        double Amplitude;
    }; 

    public ref class RTSACli
    {
    public:
        RTSACli();
        ~RTSACli(); 
        CommonCli::RefLevelResultsCli SetPara(uint64_t CF, double Span, double rbw);
        CommonCli::RefLevelResultsCli SetCF(uint64_t centerfreq);
        void SetSpan(double Span);
        void SetRBW(uint32_t rbw);
        void SetSpanRBW(double Span, uint32_t rbw);
        CommonCli::RefLevelResultsCli SetRefLevel(int reflevel);
        void SetPowerOnOff(uint32_t flag);
        void SetOutBW(int ifbw);  
        void Config();
        void SetDetectorType(uint32_t detector_type);  
        void SetFFTLen(uint32_t fft_len_value); 
        void SetSweepTime(double SweepTime_value);
        void SetPersistenceNum(uint32_t PersistenceNum_value);
        void SetGraunityNum(uint32_t GraunityNum_value);
        void SetDenominatorNum(float DenominatorNum_value);
        void SetOffset(float offset_value);
        int GetAtt();
        void SetZoomFactor(float ZoomFactor_value);
        void SetValueScale(float ValueScale_value);
        void SetOvlSel(uint32_t ovl_sel);
        void SetFFTWindow(uint32_t fftwindow);
        void SetTraceNum(uint32_t trace_num);
        float GetDenominatorNum();
        double GetSweepTimeBack();
        void SetFfthbdecim(uint32_t fft_hb_decim_value);
        void SetTriggerSource(uint32_t source);
        void SetTriggerPosttime(double time);
        void SetTriggerThreshold_dBm(float threshold);
        void SetTriggerThreshold_v(float threshold);
      
        int GetCorrectValue();
        double GetErrorValue();
        double GetAmpAppend();
        double GetBaseErrorValue();
        double GetIQCorrectValue();

        //数据读取
        void ReadSpectrum();
        void ProcessSpectrum();
        int GetSpectrumData(cli::array<Byte>^ managedBuffer);
        List<ManagedFreqAmpData>^ GetSpectrumDataSnapshot();
        cli::array<Byte>^ ReadRawSpectrumOne();
        cli::array<double>^ ReadProcessedSpectrumOne();
        int ReadProcessedSpectrumOne(cli::array<double>^% outputBuffer);

        //void ReadIQ();
        List<Tuple<cli::array<short>^, cli::array<short>^>^>^ ProcessIQ();
        cli::array<Byte>^ ReadIQOne();
        Tuple<cli::array<short>^, cli::array<short>^>^ ReadProcessedIQOne();
        int ReadProcessedIQOne(cli::array<short>^% iBuffer, cli::array<short>^% qBuffer);

        //void ReadPersistence();
        //void ProcessPersistence();
        cli::array<Byte>^ ReadPersistenceOne();
        int ReadPersistenceOne(cli::array<float>^% rawData);
        List<float>^ GetFloatData(); 
        uint64_t GetTriggerDataAddress();
        bool GetInterrupt();
        void ReadRawTriggerData(UInt64 dataPosition,UInt64 oneDataByteNum, cli::array<Byte>^% outputBuffer );
        void ReadProcessedTriggerData(UInt64 dataPosition, UInt64 oneDataByteNum, cli::array<Int16>^% outI, cli::array<Int16>^% outQ );
        void GetDmaAddrData(UInt64 address, UInt32 dataLength, UInt64% baseAddress, cli::array<System::Byte>^% outputBuffer);
        void GetDmaData(UInt64 address, UInt32 dataLength,cli::array<System::Byte>^% outputBuffer);
        void CloseDevice();
        cli::array<Byte>^ SpectrumBuffer;
        cli::array<double>^ SpectrumAmpBuffer;
        cli::array<Byte>^ IQBuffer;
        cli::array<Byte>^ PersistenceBuffer;
        cli::array<short>^ IBuffer = nullptr;   // 分离后的 I
        cli::array<short>^ QBuffer = nullptr;   // 分离后的 Q
    private:
        RTSA::RTSA* rtsa;
        //RTSAControl::SpectrumData* spectrum;
        //RTSAControl::IQData* iq;
        //RTSAControl::PersistenceData* persistence;
    };
}