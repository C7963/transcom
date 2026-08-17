#pragma once
#include <cstdint>
#include <memory>
#include <vector>
#include "ADCConfig.h"
#include "CommonManager.h"
#include "RFControl.h"
#include "LOGIC_Api.h"
#include "Device_Data.h"
#include "Global.h" 
#include "DataRead.h"

namespace RTSA
{
	struct IQPair {
		std::vector<short>* I;
		std::vector<short>* Q;
	};


	class RTSA {
	public:

		static RTSA& Instance() noexcept;
		static RTSA* getInstance();

		RTSA();
		~RTSA();

		Global::RefLevelResults SetPara(uint64_t CF, double Span, double RBW);
		Global::RefLevelResults SetCF(uint64_t centerfreq);
		Global::RefLevelResults SetRefLevel(int reflevel);
		Global::RefLevelResults SetCFRefLevel(uint64_t centerfreq, int reflevel);
		void SetSpanRBW(double bw, uint32_t rbw);
		void SetSpan(double bw);
		void SetRBW(uint32_t rbw);

		void Config();
		void SetPowerOnOff(uint32_t flag);
		void SetOutBW(int bw);

		void SetDetectorType(uint32_t detector_type);
		void SetFFTLen(uint32_t fft_len_value);
		void SetFfthbdecim(uint32_t fft_hb_decim_value);
		void SetSweepTime(double SweepTime_value);
		void SetPersistenceNum(uint32_t PersistenceNum_value);
		void SetGraunityNum(uint32_t GraunityNum_value);
		void SetDenominatorNum(float DenominatorNum_value);
		void SetOffset(float offset_value);
		void SetZoomFactor(float ZoomFactor_value);
		void SetValueScale(float ValueScale_value);
		void SetOvlSel(uint32_t ovl_sel);
		void SetFFTWindow(uint32_t fftwindow);
		void SetTraceNum(uint32_t trace_num);
		uint32_t GetAtt();
		float GetDenominatorNum();
		double GetSweepTimeBack();

		void SetADCFilter(uint64_t centerfreq);
		double GetIQCorrectValue();
		void SetTriggerSource(uint32_t source);
		void SetTriggerPosttime(double time);
		void SetTriggerThreshold_dBm(float threshold);
		void SetTriggerThreshold_v(float threshold);
		uint64_t GetTriggerDataAddress();  
		bool GetInterrupt(); 
		void ReadRawTriggerData(uint64_t baseAddress, uint64_t oneDataByteNum, std::vector<uint8_t>& outputBuffer); 
		void ReadProcessedTriggerData(unsigned long long dataPosition, uint64_t oneDataByteNum, std::vector<int16_t>& outI, std::vector<int16_t>& outQ);
		void UpdateCorrectValue(int RFATT);
		void UpdateAmpAppend();
		int GetCorrectValue();
		double GetErrorValue();
		double GetAmpAppend();
		double GetBaseErrorValue();
		//Êý¾Ý¶ÁÈ¡
		void ReadSpectrum();
		void ProcessSpectrum();
		int GetSpectrumData(unsigned char* buffer, int bufferSize);
		std::vector<RTSAControl::FreqAmpData> GetSpectrumSnapshot();
		int ReadProcessedSpectrumOne(unsigned char* buffer, int len);
		std::vector<unsigned char>* ReadRawSpectrumOne();
		std::vector<double>* ReadProcessedSpectrumOne();
		int ReadProcessedSpectrumOne(std::vector<double>& outputBuffer);

		void ReadIQ();
		std::vector<std::pair<std::vector<int16_t>, std::vector<int16_t>>> ProcessIQ();
		std::vector<unsigned char>* ReadIQOne();
		std::pair<std::vector<short>*, std::vector<short>*> ReadProcessedIQOne();
		int ReadProcessedIQOne(std::vector<short>& iBuffer, std::vector<short>& qBuffer);
		int ReadProcessedIQOne(short* pDstI, short* pDstQ);
		int read_iq_one(unsigned char* external_buffer, int buffer_size);

		void ReadPersistence();
		void ProcessPersistence();
		int ReadPersistenceOne(std::vector<float>& rawData);
		int ReadPersistenceOne(float* pDstFloat, int maxFloatCount);
		void GetDmaData(uint64_t baseAddress, uint64_t oneDataByteNum, std::vector<uint8_t>& outputBuffer);
		void GetDmaAddrData(uint64_t address, uint32_t dataLength, uint64_t& baseAddress, std::vector<uint8_t>& outputBuffer);

		std::vector<float> GetFloatData();
	
	
		int read_persistence_one(unsigned char* external_buffer, int buffer_size);

		
		int Persistence_GetData(unsigned char* PersistenceData);
		Global::Parameter* Logic_Get_Parameter();
		void CloseDevice();
		
	private:
		std::unique_ptr<LOGIC::LOGICApi> logicConfig_;
		std::unique_ptr<Device::Device_Data_RTSA> device_;
		std::unique_ptr<Common::CommonManager> common_;
		RTSAControl::SpectrumData* spectrum;
		RTSAControl::IQData* iq;
		RTSAControl::PersistenceData* persistence;
		RTSAControl::DMAData* dmaData;

		std::vector<unsigned int> FilterLowCoe_;
		std::vector<unsigned int> FilterCoe_;
		int AmpAppend_;
		UINT32 ClockSample;
		int spectrumRequiredSize = 4096;
		int SpectrumSize = 1024;
		int IQRequiredSize = 1024 * 2400;
		int PersistenceRequiredSize = 1024 * 512 * 4;

		std::vector<unsigned char> spectrumBuffer;
		std::vector<double> spectrumAmpBuffer;     

		std::vector<unsigned char> m_nativeRawBuffer;
		std::vector<unsigned char> IQBuffer;
		std::vector<short> IBuffer;
		std::vector<short> QBuffer;
		// ½ûÖ¹¿½±´
		RTSA(const RTSA&) = delete;
		RTSA& operator=(const RTSA&) = delete;
	};

}
