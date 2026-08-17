#include "MultiChannel.h"
#include "PDW_Config.h"
#include "MultiIQ.h"

using namespace MULTI;
using namespace PDWCONFIG;
using namespace MULTIIQ;

MULTI::Multi* MULTI::Multi::instance = nullptr;

Multi::Multi()
{
	pdwctrl = PDWCtrl::getInstance();
	pdwstream = PDWStream::getInstance();
	miq = MIQStream::getInstance();
}

Multi::~Multi()
{
	
}

void Multi::SetChannelNum(uint32_t num)
{
	pdwctrl->Set_ChannelNum(num);
	miq->set_queue_count(num);
}

void Multi::SetChannelFreq(int64_t* Nfreq)
{
	pdwctrl->SetChannelFreq(Nfreq);
}

void Multi::PDW_Enable(bool enable)
{
	pdwctrl->PDW_Enable(enable);
}

void Multi::PDW_Threshold(uint32_t channel_num, float* threshold)
{
	pdwctrl->PDW_Threshold(channel_num, threshold);
}

void Multi::PDW_DDC_Config()
{
	pdwctrl->PDW_DDC_Config();
}

void Multi::SetBandwidthGain(uint32_t bandwidth, uint32_t gain)
{
	pdwctrl->SetBandWidthGain(bandwidth, gain);
}

void Multi::PDW_DDC_FIR_COEF(uint32_t* value, uint32_t length)
{
	pdwctrl->PDW_DDC_FIR_COEF(value, length);
}

void Multi::fir_config_axis_reset()
{
	pdwctrl->fir_config_axis_reset();
}

void Multi::fir_config_txdata_fifo_reset()
{
	pdwctrl->fir_config_txdata_fifo_reset();
}

void Multi::Start_PDW()
{
	pdwstream->Start();
}

void Multi::Stop_PDW()
{
	pdwstream->Stop();
}

int Multi::DequeueQueuePDW(unsigned char* buffer) {
	if (pdwstream->data_queue.Empty()) {
		return 0;
	}

	std::vector<unsigned char> data;
	if (pdwstream->data_queue.Pop(data)) {
	
		std::copy(data.begin(), data.end(), buffer);
		return static_cast<int>(data.size()); // 返回实际长度
	}
	return 0;
}


void Multi::Start_IQ()
{
	miq->Start_IQ();
}

void Multi::Stop_IQ()
{
	miq->Stop_IQ();
}

void Multi::iq_switch(uint32_t flag)
{
	miq->iq_switch(flag);
}

std::queue<std::vector<unsigned char>>& Multi::GetQueueByIndex(int index)
{
	return miq->channel_queues[index];
}

//bool Multi::DequeueQueue(int queueIndex, uint8_t* buffer)
//{
//	std::unique_lock<std::mutex> lock(miq->mtx);
//	auto& queue = GetQueueByIndex(queueIndex);
//	if (queue.empty())
//		return false;
//	auto& frontItem = queue.front();
//	std::copy(frontItem.begin(), frontItem.end(), buffer);
//	queue.pop();
//	return true;
//}

bool Multi::DequeueQueue(int queueIndex, std::vector<uint8_t>& outData)
{
	std::unique_lock<std::mutex> lock(miq->mtx); // 确保线程安全
	auto& queue = GetQueueByIndex(queueIndex);

	if (queue.empty())
		return false;

	// 使用 move 语义将数据从队列移动到临时容器，几乎零开销
	outData = std::move(queue.front());
	queue.pop();

	return true; 
}

void Multi::ClearQueue()
{
	miq->clear_queues();
}

void Multi::Start_Stream(PDWCONFIG::Nvme_io_argus argus)
{
	streamargs = argus;
	switch (argus.type)
	{
	case DataType::IQ:
		miq->Start_acquisition_IQ(argus);
		break;
	case DataType::PDW:
		pdwstream->start_acquisition_pulse(argus);
		break;
	case DataType::IQPDW:
		miq->Start_acquisition_IQ(argus);
		pdwstream->start_acquisition_pulse(argus);
		break;
	default:
		break;
	}
}

void Multi::Stop_Stream()
{
	switch (streamargs.type)
	{
	case DataType::IQ:
		miq->Stop_acquisition_IQ();
		break;
	case DataType::PDW:
		pdwstream->stop_acquisition_pulse();
		break;
	case DataType::IQPDW:
		miq->Stop_acquisition_IQ();
		pdwstream->stop_acquisition_pulse();
		break;
	default:
		break;
	}
}

void Multi::Start_playback(PDWCONFIG::Nvme_io_argus argus)
{
	playbackargs = argus;

	if (argus.file_name.find("@IQ") != std::string::npos)
	{
		miq->start_playback_iq(argus);
	}
	else if ((argus.file_name.find("@PDW") != std::string::npos))
	{
		pdwstream->start_playback_pulse(argus);
	}
}

void Multi::Stop_playback()
{
	
	if (playbackargs.file_name.find("@IQ") != std::string::npos)
	{
		miq->stop_playback_iq();
	}
	else if (playbackargs.file_name.find("@PDW") != std::string::npos)
	{
		pdwstream->stop_playback_pulse();
	}
}


double Multi::get_pulse_speed()
{
	return pdwstream->speed;
}

double Multi::get_iq_speed()
{
	return miq->speed;
} 
double Multi::get_speed()
{
	switch (streamargs.type)
	{
	case DataType::IQ:
	{
		auto t = miq->speed;
		return t;
	}
	case DataType::PDW:
	{
		auto t = pdwstream->speed;
		return t;
	}
	case DataType::IQPDW:
	{
		auto t1 = miq->speed;
		auto t2 = pdwstream->speed;
		return t1 + t2;
	}
	default:
		break;
	}
	return 0;
}
