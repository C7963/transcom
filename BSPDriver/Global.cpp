#include "Global.h"

namespace Global {
    PdwParameter PdwParameters = {
        1000000,     // RBW
        2.4e9,  // CenterFrequency
        1,           // DecimateFactor
        0.0,         // RefLevel
        false,       // IsPreAmplifier
        false,       // IsTrigger
        1.0f,        // DenominatorNum
        0,           // RFChannelType 
        0.0,         // ATT 
        0.0,         // IQCorrectValue
        614.4e6    // Span 
    };
     double AmpAppend = 0;			//  Span��RBW�Ĺ��ʲ���
     uint32_t FFTGainOffset = 0;  //���汶��
     double ErrorValue = 0;			//  CF��ز�����CF�仯ʱ��ȡ  
     double IQFFTBaseError = 0;
     double BaseErrorValue = 0;		//  �����������ӱ����?
     int CorrectValue = 0;      //����Reflevel����Ĳ���?
     int IQPowerBaseError = -102;
     std::map<double, double> FreqErrorValue = {}; //Ƶ��-����ֵ�?
     std::map<double, double> FreqPreampErrorValue = {};
     std::map<double, double> FreqATTErrorValue = {};
     double SweepBaseErrorValue = 147;  // 系统基础校准偏移量（dB），将原始ADC�?0*log10值偏移到合理dBm范围
                                         // 典型噪声底：20*log10(1000)-147 = -87 dBm    
                                         // 如果有校准文件加载，会被覆盖
     std::map<double, double> RbwErrDIC = {};
     uint32_t SweepSpectrumPointCount = 1001;
     uint32_t SweepSpectrumPointCountSet = 1001;
     double RefLevel = 0;
     uint32_t SubChannelGain = 0;
    //Compensation Compensations {
    //    0.0  ,// AmpAppend;			//  Span��RBW�Ĺ��ʲ���
    //    0  ,// FFTGainOffset;  //���汶��
    //    0.0  ,// ErrorValue;			//  CF��ز�����CF�仯ʱ��ȡ  
    //    0.0  ,// BaseErrorValue;		//  �����������ӱ����?
    //    0.0  ,// CorrectValue;      //����Reflevel����Ĳ���?

    //    {},   // FreqErrorValue; //Ƶ��-����ֵ�?
    //    {},   // FreqPreampErrorValue;
    //    {},   // FreqATTErrorValue;
    //    0.0 , // SweepBaseErrorValue;
    //    {} // RbwErrDIC;
    //};
}