#pragma once
#include <memory>
#include <string>
#include <iostream>
#include "PCIE_InterruptFIFO.h"

// 假设这些在其他地方定义了，这里做前置声明或模拟
namespace CommBus { class XillyFile; }
namespace Device { class Device_MEM32; }
#define TRIGGER_DATA_HIGHADDRESS_REG					  0x00001000
#define TRIGGER_DATA_LOWADDRESS_REG						  0x00001001
#define TRIGGER_SOURCE_REG								  0x1001000C
#define TRIGGER_POSTTIME_REG							  0x1001000D
#define TRIGGER_THRESHOLD_REG							  0x1001000E
#define DMA_LOW_ADDRESS_REG							      0x00001058
#define DMA_HIGH_ADDRESS_REG   						      0x0000105C
#define DMA_LENGTH_REG									  0x00001068


namespace Device
{
    class DeviceWorkMode;
   
    class Device_Data_RTSA
    {
    public:
        static Device_Data_RTSA* getInstance(); 
        // 原始业务接口保持不变
        int Device_OpenDevice();
        int Device_CloseDevice();      

        bool getEOF() const;
        void setEOF(const bool& value);

        // RTSA 模式数据读取
        bool ReadSpectrumData(unsigned char* res, unsigned int readlen);
        bool ReadIQData(unsigned char* res, unsigned int len);
        bool ReadPersistenceData(unsigned char* res, unsigned int len);
        bool ReadDmaData(unsigned char* res, unsigned int len);
        void set_dma_mm2s(uint8_t flag);
         
        // 寄存器操作 (通常这些是通用的，保留在主类中)
        void set_trigger_source(uint32_t source);
        void set_trigger_posttime(uint32_t time);
        void set_trigger_threshold(float threshold);
        uint64_t get_trigger_dataAddress();
        void set_dma_config(uint64_t dataAddress, uint32_t dataLength);
        void GetDmaData(uint64_t baseAddress, uint64_t oneDataByteNum, std::vector<uint8_t>& outputBuffer);
        void GetDmaAddrData(uint64_t address, uint32_t dataLength, uint64_t& baseAddress, std::vector<uint8_t>& outputBuffer);
        bool get_dma_data(unsigned char* res, unsigned int len); 
        Device_Data_RTSA();
        bool GetInterrupt();
        void clear_interrupt();
        ~Device_Data_RTSA();
    private:
 
        static Device_Data_RTSA* instance;
        uint32_t IQStartAddress = 0x80000000;
        uint64_t IQEndAddress = 0x1FFEF7C00;
        bool privateEOF;
        Device::Device_MEM32* pcie_mem; 
        PCIE_InterruptFIFO& interruptFIFO;
        // 【新增】多态指针，指向具体的 RTSA 或 Multi 实现
        std::unique_ptr<DeviceWorkMode> m_impl;
    };
    class Device_Data_Multi
    {
    public:
        static Device_Data_Multi* getInstance(); 

        // 原始业务接口保持不变
        int Device_OpenDevice();
        int Device_CloseDevice();      

        bool getEOF() const;
        void setEOF(const bool& value); 
        // Multi 模式数据读取
        bool ReadPulseData(unsigned char* res, unsigned int readlen);
        bool ReadChannelsData(unsigned char* res, unsigned int readlen);
        ~Device_Data_Multi();
        Device_Data_Multi();
    private: 
        static Device_Data_Multi* instance;

        bool privateEOF;
        Device_MEM32* pcie_mem; 

        // 多态指针，指向具体的 RTSA 或 Multi 实现
        std::unique_ptr<DeviceWorkMode> m_impl;
    };
}