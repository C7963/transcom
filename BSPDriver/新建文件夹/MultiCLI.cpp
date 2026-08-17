#include "MultiCLI.h"


using namespace System;
using namespace System::Collections::Generic;
using namespace PDWCONFIG;
using namespace MULTICLI;
using namespace MULTIIQ;
using namespace MULTI;

MultiCli::MultiCli()
{
    multi = Multi::getInstance();
}

MultiCli::~MultiCli()
{

}

void MultiCli::SetChannelNum(uint32_t num)
{
    multi->SetChannelNum(num);
}

void MultiCli::SetChannelFreq(cli::array<int64_t>^ Nfreq)
{
    pin_ptr<int64_t> pinNfreq = &Nfreq[0];
    multi->SetChannelFreq(pinNfreq);
}

void MultiCli::PDW_Enable(bool enable)
{
    multi->PDW_Enable(enable);
}

void MultiCli::PDW_Threshold(uint32_t channel_num, cli::array<float>^ threshold)
{
    pin_ptr<float> pinthreshold = &threshold[0];
    multi->PDW_Threshold(channel_num, pinthreshold);
}

void MultiCli::PDW_DDC_Config()
{
    multi->PDW_DDC_Config();
}

void MultiCli::SetBandwidthGain(uint32_t bandwidth, uint32_t gain)
{
    multi->SetBandwidthGain(bandwidth, gain);
}

void MultiCli::PDW_DDC_FIR_COEF(cli::array<uint32_t>^ value, uint32_t length)
{
    cli::pin_ptr<uint32_t> filterptr = &value[0];
    multi->PDW_DDC_FIR_COEF(filterptr, length);
}


void MultiCli::fir_config_axis_reset()
{
    multi->fir_config_axis_reset();
}

void  MultiCli::fir_config_txdata_fifo_reset()
{
    multi->fir_config_txdata_fifo_reset();
}

void MultiCli::Start_PDW()
{
    multi->Start_PDW();
}

void MultiCli::Stop_PDW()
{
    multi->Stop_PDW();
}

bool MultiCli::DequeueQueuePDW([Out]cli::array<unsigned char>^% buffer)
{
    try {
        
        buffer = gcnew cli::array<unsigned char>(32);

        pin_ptr<unsigned char> pBuffer = &buffer[0];
        int readBytes = multi->DequeueQueuePDW(static_cast<unsigned char*>(pBuffer));

        return readBytes > 0;
    }
    catch (const std::exception& ex) {
        return false;
    }
    return true;
}


PDWCONFIG::Nvme_io_argus ConvertNvmeIoArgs(NvmeIoArgsCli^ managedArgs)
{
    Nvme_io_argus nativeArgs;
    msclr::interop::marshal_context context;
    nativeArgs.file_name = context.marshal_as<std::string>(managedArgs->FileName);
    if (!System::String::IsNullOrEmpty(managedArgs->FilePath))
    {
        nativeArgs.file_path = context.marshal_as<std::string>(managedArgs->FilePath);
    }
    nativeArgs.span = managedArgs->Span;
    nativeArgs.centerfreq = managedArgs->CenterFreq;
    nativeArgs.rate = managedArgs->Rate;
    nativeArgs.flag = managedArgs->flag;
    return nativeArgs;
}

void MultiCli::Start_Stream(NvmeIoArgsCli^ args)
{
    if (args == nullptr)
    {
        return;
    }
    Nvme_io_argus nativeArgs = ConvertNvmeIoArgs(args);
    multi->Start_Stream(nativeArgs);
}

void MultiCli::Stop_Stream()
{
    multi->Stop_Stream();
}


void MultiCli::Start_playback(NvmeIoArgsCli^ args)
{
    if (args == nullptr)
    {
        return;
    }
    Nvme_io_argus nativeArgs = ConvertNvmeIoArgs(args);
    multi->Start_playback(nativeArgs);
}

void MultiCli::Stop_playback()
{
    multi->Stop_playback();
}



double MultiCli::get_pulse_speed()
{
    return multi->get_pulse_speed();
}

double MultiCli::get_iq_speed()
{
    return multi->get_iq_speed();
}

double MultiCli::get_speed()
{
    return multi->get_speed();
}

void MultiCli::Start_IQ()
{
    multi->Start_IQ();
} 

void MultiCli::Stop_IQ()
{
    multi->Stop_IQ();
}

void MultiCli::iq_switch(uint32_t flag)
{
    multi->iq_switch(flag);        //0-脉冲iq,1-原始iq
}

std::queue<std::vector<unsigned char>>& MultiCli::GetQueueByIndex(int index)
{
    return multi->GetQueueByIndex(index);
}

bool MultiCli::DequeueQueue(int queueIndex, [Out]cli::array<unsigned char>^% buffer)
{
    try
    {
        // 1. 准备一个原生的临时容器
        std::vector<uint8_t> tempNativeData;

        // 2. 调用原生核心逻辑 (内部处理了 mutex 锁)
        // 数据会被 move 到 tempNativeData 中
        if (!multi->DequeueQueue(queueIndex, tempNativeData))
        {
            buffer = nullptr;
            return false;
        }

        // 3. 根据拿到数据的实际大小分配托管数组
        int actualSize = static_cast<int>(tempNativeData.size());
        buffer = gcnew cli::array<unsigned char>(actualSize);

        // 4. 将数据从原生容器拷贝到托管数组
        if (actualSize > 0)
        {
            pin_ptr<unsigned char> pPinned = &buffer[0];
            memcpy(pPinned, tempNativeData.data(), actualSize);
        }

        return true;
    }
    catch (const std::exception& ex)
    {
        return false;
    }
}

void MultiCli::ClearQueue()
{
    multi->ClearQueue();
}


