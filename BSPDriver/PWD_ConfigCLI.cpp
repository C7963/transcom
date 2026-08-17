#include "PWD_ConfigCLI.h"


using namespace System;
using namespace System::Collections::Generic;
using namespace PWDCONFIG;
using namespace PWDCONFIGCLI;

PWDConfig::PWDConfig()
{
	pwdctrl = PWDCtrl::getInstance();
	pwdstream = PWDStream::getInstance();
}

PWDConfig::~PWDConfig()
{

}

void PWDConfig::SetChannelNum(uint32_t num)
{
	pwdctrl->Set_ChannelNum(num);
}

void PWDConfig::Caculate_F_IFs(cli::array<int64_t>^ Nfreq)
{
	pin_ptr<int64_t> pinNfreq = &Nfreq[0];
	pwdctrl->Caculate_F_IFs(pinNfreq);
}

void PWDConfig::PWD_Enable(bool enable)
{
	pwdctrl->PWD_Enable(enable);
}

void PWDConfig::PWD_Threshold(uint32_t channel_num, cli::array<float>^ threshold)
{
	pin_ptr<float> pinthreshold = &threshold[0];
	pwdctrl->PWD_Threshold(channel_num, pinthreshold);
}

void PWDConfig::PWD_Threshold(uint32_t channel_num, int32_t threshold)
{
	pwdctrl->PWD_Threshold(channel_num, threshold);
}

void PWDConfig::PWD_DDC_Config()
{
	pwdctrl->PWD_DDC_Config();
}

void PWDConfig::PWD_DDC_Decim(uint32_t decim)
{
	pwdctrl->PWD_DDC_Decim(decim);
}

void PWDConfig::PWD_DDC_FIR_COEF(cli::array<uint32_t>^ value, uint32_t length)
{
	cli::pin_ptr<uint32_t> filterptr = &value[0];
	pwdctrl->PWD_DDC_FIR_COEF(filterptr, length);
}


void  PWDConfig::fir_config_axis_reset()
{
	pwdctrl->fir_config_axis_reset();
}
void  PWDConfig::fir_config_txdata_fifo_reset()
{
	pwdctrl->fir_config_txdata_fifo_reset();
}

void PWDConfig::Start()
{
	pwdstream->Start();
}

void PWDConfig::Stop()
{
	pwdstream->Stop();
}

void PWDConfig::Pop()
{
	int len = pwdstream->queue.size();
	arr_cli = gcnew cli::array<cli::array <uint8_t>^>(len);
	for (int i = 0; i < len; i++) {
		if (pwdstream->queue.empty()) {
			break; // 队列已空，提前退出循环
		}
		std::lock_guard<std::mutex> lock(pwdstream->mtx);
		//pwdstream->mtx.lock();
		array<unsigned char, 32> arr;
		arr = pwdstream->queue.front();
		pwdstream->queue.pop();
		//pwdstream->mtx.unlock();
		cli::array <uint8_t>^ array_cli = gcnew cli::array <uint8_t>(32);
		for (int j = 0; j < arr.size(); j++)
		{
			array_cli[j] = arr[j];
		}
		arr_cli[i] = array_cli;
	}
	arrCliLength = arr_cli->Length;
}


//bool PWDConfig::DequeueQueue([Out]cli::array<unsigned char>^% buffer)
//{
//	std::lock_guard<std::mutex> lock(pwdstream->mtx);
//	if (pwdstream->all_data.empty())
//		return false;
//	auto& frontItem = pwdstream->all_data.front();
//	buffer = gcnew cli::array<unsigned char>(static_cast<int>(frontItem.size()));
//	pin_ptr<unsigned char> pinnedBuffer = &buffer[0];
//	unsigned char* nativeBuffer = pinnedBuffer;
//	std::copy(frontItem.begin(), frontItem.end(), nativeBuffer);
//	pwdstream->all_data.pop();
//	return true;
//
//}

bool PWDConfig::DequeueQueue([Out]cli::array<unsigned char>^% buffer)
{
	if (pwdstream->data_queue.Empty())
		return false;
	std::vector<unsigned char> data;
	if (pwdstream->data_queue.Pop(data)) {
		
		buffer = gcnew cli::array<unsigned char>(static_cast<int>(data.size()));
		pin_ptr<unsigned char> pinnedBuffer = &buffer[0];
		unsigned char* nativeBuffer = pinnedBuffer;
		std::copy(data.begin(), data.end(), nativeBuffer);
	}
	return true;
}