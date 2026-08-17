#pragma once
#include <queue>
#include <mutex>
#include "PDW_Config.h"
#include "MultiIQ.h"


namespace MULTI
{
	//public enum class DataType {
	//	IQ,
	//	PDW,
	//	IQPDW
	//};

	//struct NvmeIoArgs {
	//	DataType Type;          
	//	std::string FileName;   
	//	uint32_t Span;          
	//	uint64_t CenterFreq;    
	//	std::string FilePath;   
	//	float Rate;             
	//	bool flag;               
	//};

	class Multi
	{
	public:
		Multi();
		~Multi();
		static Multi* getInstance() {
			if (instance == nullptr)
			{
				instance = new Multi();
			}
			return instance;
		};
		static Multi* instance;
		void SetChannelNum(uint32_t num);
		void SetChannelFreq(int64_t* Nfreq);
		void PDW_Enable(bool enable);
		void PDW_Threshold(uint32_t channel_num, float* threshold);
		void PDW_DDC_Config();
		void SetBandwidthGain(uint32_t bandwidth, uint32_t gain);
		void PDW_DDC_FIR_COEF(uint32_t* value, uint32_t length);
		void fir_config_axis_reset();
		void fir_config_txdata_fifo_reset();
		void Start_PDW();
		void Stop_PDW();
		int DequeueQueuePDW(unsigned char* buffer);
		std::vector<std::vector<uint8_t>> arr_native;


		void Start_IQ();
		void Stop_IQ();
		void iq_switch(uint32_t flag);
		//bool DequeueQueue(int queueIndex, uint8_t* buffer);
		bool DequeueQueue(int queueIndex, std::vector<uint8_t>& outData);
		void ClearQueue();
		void Start_Stream(PDWCONFIG::Nvme_io_argus argus);
		void Stop_Stream();
		void Start_playback(PDWCONFIG::Nvme_io_argus argus);
		void Stop_playback();
		double get_pulse_speed();
		double get_iq_speed();
		double get_speed();
		std::queue<std::vector<unsigned char>>& GetQueueByIndex(int index);

		MULTIIQ::MIQStream* miq;

	private:
		PDWCONFIG::Nvme_io_argus streamargs;
		PDWCONFIG::Nvme_io_argus playbackargs;
		PDWCONFIG::PDWCtrl* pdwctrl;
		PDWCONFIG::PDWStream* pdwstream;
		//MULTIIQ::MIQStream* miq;
		
	};
}
