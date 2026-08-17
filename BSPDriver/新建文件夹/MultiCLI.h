
#pragma once
#include <queue>
#include <mutex>
#include <msclr/marshal_cppstd.h>
#include "MultiChannel.h"

using namespace System;
using namespace System::Runtime::InteropServices;

namespace MULTICLI
{
	enum class DataType {
		IQ,
		PDW,
		IQPDW
	};

	public ref struct NvmeIoArgsCli
	{
		DataType Type;
		String^ FileName;
		UInt32 Span;
		UInt64 CenterFreq;
		String^ FilePath;
		float Rate;
		bool flag;
	};

	public ref class MultiCli
	{
	public:
		MultiCli();
		~MultiCli();

		void SetChannelNum(uint32_t num);
		void SetChannelFreq(cli::array<int64_t>^ Nfreq);
		void PDW_Enable(bool enable);
		void PDW_Threshold(uint32_t channel_num, cli::array<float>^ threshold);
		void PDW_DDC_Config();
		void SetBandwidthGain(uint32_t bandwidth, uint32_t gain);
		void PDW_DDC_FIR_COEF(cli::array<uint32_t>^ value, uint32_t length);
		void fir_config_axis_reset();
		void fir_config_txdata_fifo_reset();
		void Start_PDW();
		void Stop_PDW();
		bool DequeueQueuePDW(cli::array<unsigned char>^% buffer);

		//IQ
		void Start_IQ();
		void Stop_IQ();
		void iq_switch(uint32_t flag);
		bool DequeueQueue(int queueIndex, cli::array<unsigned char>^% buffer);
		void ClearQueue();
		void Start_Stream(NvmeIoArgsCli^ args);
		void Stop_Stream();
		void Start_playback(NvmeIoArgsCli^ args);
		void Stop_playback();
		double get_pulse_speed();
		double get_iq_speed();
		double get_speed(); 


	private:

		MULTI::Multi* multi;
		cli::array <uint8_t>^ array_cli;
		std::queue<std::vector<unsigned char>>& GetQueueByIndex(int index);
	};
}
