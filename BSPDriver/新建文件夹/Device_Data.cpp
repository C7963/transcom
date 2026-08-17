#include "pch.h"
#include "Device_Data.h"
#include <clocale>
#include "ConvertHelper.h"
#include <iostream>
#include "Device_Address.h"
#include "Device_MEM32.h"
#include "Global.h"

// 假设的外部依赖，请替换为你实际的头文件
// #include "XillyFile.h"
// #include "Device_Address.h"
// #include "Device_MEM32.h"
 
using namespace std;
using namespace CommBus; // 假设 XillyFile 在此命名空间

namespace Device
{
    // =========================================================
    // 1. 辅助工具：提取公共读取逻辑
    // =========================================================
    namespace
    {
        // 核心读取逻辑：封装了 try-catch, is_opened, open, read, eof check
        // device: 具体设备指针
        // autoCloseEOF: 为 true 时，如果读到文件末尾会自动关闭文件 (Spectrum, Pulse, Channels 需要)
        bool Helper_ReadData(XillyFile* device, unsigned char* buffer, unsigned int len, bool autoCloseEOF)
        {
            if (!device) return false;

            try
            {
                if (!device->is_opened())
                {
                    if (!device->open_file(XillyFile::e_ReadOnly)) {
                        return false; // 打开失败
                    }
                }

                if (device->read_data(buffer, len)>0)
                {
                    device->set_offset(0); // 重置偏移量

                    if (autoCloseEOF && device->isReadEOF())
                    {
                        device->close_file();
                    }
                    return true;
                }
                return false; // 读取返回 false
            }
            catch (const std::exception& ex)
            {
                // 异常保护
                device->close_file();
                std::cout << "Read Error: " << ex.what() << std::endl;
                return false;
            }
        }
    }

    // =========================================================
    // [Internal] 2. 实现类定义 (策略模式的具体实现)
    // =========================================================

    // 抽象基类
    class DeviceWorkMode {
    public:
        virtual ~DeviceWorkMode() {}
        virtual int Open() = 0;
        virtual int Close() = 0;
    };

    // RTSA 模式具体实现 (包含6个设备)
    class RTSA_Impl : public DeviceWorkMode {
    public:
        unique_ptr<XillyFile> SpectrumDevice;
        unique_ptr<XillyFile> IQDevice;
        unique_ptr<XillyFile> PersistenceDevice;
        unique_ptr<XillyFile> DmaDevice;  
        RTSA_Impl() {
            SpectrumDevice.reset(new XillyFile(Device_Address::Xillybus_Read0_32));
            IQDevice.reset(new XillyFile(Device_Address::Xillybus_Read1_32));
            PersistenceDevice.reset(new XillyFile(Device_Address::Xillybus_Read2_32));
            DmaDevice.reset(new XillyFile(Device_Address::Xillybus_Dma32));  
        }

        int Open() override {
            try {
                SpectrumDevice->open_file(XillyFile::e_ReadOnly);
                IQDevice->open_file(XillyFile::e_ReadOnly);
                PersistenceDevice->open_file(XillyFile::e_ReadOnly);
                DmaDevice->open_file(XillyFile::e_ReadOnly); 
                return 0;
            }
            catch (...) { return -1; }
        }

        int Close() override {
            try {
                if (SpectrumDevice) SpectrumDevice->close_file();
                if (IQDevice) IQDevice->close_file();
                if (PersistenceDevice) PersistenceDevice->close_file();
                // if (DmaDevice) DmaDevice->close_file(); 
                return 0;
            }
            catch (...) { return -1; }
        }
    };

    // Multi 模式具体实现 (包含2个设备)
    class Multi_Impl : public DeviceWorkMode {
    public:
        unique_ptr<XillyFile> PulseDevice;
        unique_ptr<XillyFile> ChannelsDevice;

        Multi_Impl() {
            PulseDevice.reset(new XillyFile(Device_Address::Xillybus_Read5_32));
            ChannelsDevice.reset(new XillyFile(Device_Address::Xillybus_Read6_32));
        }

        int Open() override {
            try {
                PulseDevice->open_file(XillyFile::e_ReadOnly);
                // ChannelsDevice->open_file... // 根据原逻辑保留注释或开启
                return 0;
            }
            catch (...) { return -1; }
        }

        int Close() override {
            try {
                if (PulseDevice) PulseDevice->close_file();
                // if (ChannelsDevice) ChannelsDevice->close_file();
                return 0;
            }
            catch (...) { return -1; }
        }
    };

    // =========================================================
    // [Class 1] Device_Data_RTSA 实现
    // =========================================================

    Device_Data_RTSA* Device_Data_RTSA::instance = nullptr;

    Device_Data_RTSA::Device_Data_RTSA() : privateEOF(false), interruptFIFO(PCIE_InterruptFIFO::Instance())
    {
        pcie_mem = Device::Device_MEM32::getInstance(); 
        m_impl.reset(new RTSA_Impl());
    }
    bool Device_Data_RTSA::GetInterrupt()   //没有返回或者返回false都是没有中断
    {
        // 1. 获取单例并调用 ReadData 
        interruptFIFO.ReadData(0, 4);

        // 2. 逻辑判断
        if (interruptFIFO.ListInterrupt.empty())  
        {
            return false;
        }
        else if (interruptFIFO.ListInterrupt[0] == 1)
        {
            return true;
        } 
        return false;
    }

    void Device_Data_RTSA::clear_interrupt() {
        pcie_mem->SendData(INTR_CLR, 1);
    }
    Device_Data_RTSA::~Device_Data_RTSA() {} // m_impl 自动释放

    Device_Data_RTSA* Device_Data_RTSA::getInstance()
    {
        if (instance == nullptr) instance = new Device_Data_RTSA();
        return instance;
    }

	int Device_Data_RTSA::Device_OpenDevice() {
		int result = m_impl->Open();
		set_dma_mm2s(0);
		set_dma_mm2s(1);
		return result;
	}

    int Device_Data_RTSA::Device_CloseDevice() {
        clear_interrupt();
        interruptFIFO.Close();
        return m_impl->Close();
    }

    bool Device_Data_RTSA::getEOF() const { return privateEOF; }
    void Device_Data_RTSA::setEOF(const bool& value) { privateEOF = value; }

    // RTSA 特有读取方法
    bool Device_Data_RTSA::ReadSpectrumData(unsigned char* res, unsigned int readlen)
    {
        // 转换基类指针为具体实现类指针
        auto impl = static_cast<RTSA_Impl*>(m_impl.get());
        // Spectrum 需要检测 EOF 并关闭，传入 true
        return Helper_ReadData(impl->SpectrumDevice.get(), res, readlen, true);
    }
    void Device_Data_RTSA::set_dma_mm2s(uint8_t flag) {
        pcie_mem->SendData(DMA_MM2S_REG, flag);
    }
    bool Device_Data_RTSA::ReadIQData(unsigned char* res, unsigned int len)
    {
        auto impl = static_cast<RTSA_Impl*>(m_impl.get());
        // IQ 不需要检测 EOF，传入 false
        return Helper_ReadData(impl->IQDevice.get(), res, len, false);
    }

    bool Device_Data_RTSA::ReadPersistenceData(unsigned char* res, unsigned int len)
    {
        auto impl = static_cast<RTSA_Impl*>(m_impl.get());
        return Helper_ReadData(impl->PersistenceDevice.get(), res, len, false);
    }

    bool Device_Data_RTSA::ReadDmaData(unsigned char* res, unsigned int len)
    {
        auto impl = static_cast<RTSA_Impl*>(m_impl.get());
        return Helper_ReadData(impl->DmaDevice.get(), res, len, false);
    }

    bool Device_Data_RTSA::get_dma_data(unsigned char* res, unsigned int len)
    {
        return ReadDmaData(res, len);
    }
     

    // 寄存器操作 (直接使用 pcie_mem)
    void Device_Data_RTSA::set_trigger_source(uint32_t source) {
        pcie_mem->SendData(TRIGGER_SOURCE_REG, source);
    }
    void Device_Data_RTSA::set_trigger_posttime(uint32_t time) {
        pcie_mem->SendData(TRIGGER_POSTTIME_REG, time);
    }
    void Device_Data_RTSA::set_trigger_threshold(float threshold) {
        pcie_mem->SendData(TRIGGER_THRESHOLD_REG, threshold);
    }
    uint64_t Device_Data_RTSA::get_trigger_dataAddress() {
        unsigned char highAddress[4], lowAddress[4];
        pcie_mem->ReadBackData(TRIGGER_DATA_HIGHADDRESS_REG, 1, highAddress);
        pcie_mem->ReadBackData(TRIGGER_DATA_LOWADDRESS_REG, 1, lowAddress);
        unsigned char Address[8];
        std::copy(highAddress, highAddress + 4, Address);
        std::copy(lowAddress, lowAddress + 4, Address + 4);
        uint64_t dataAddress;
        std::memcpy(&dataAddress, Address, sizeof(dataAddress));
        return dataAddress;
    }
    void Device_Data_RTSA::set_dma_config(uint64_t dataAddress, uint32_t dataLength) {
        pcie_mem->SendData(DMA_LOW_ADDRESS_REG, uint32_t(dataAddress & 0xffffffff));
        pcie_mem->SendData(DMA_HIGH_ADDRESS_REG, uint32_t(dataAddress >> 32));
        pcie_mem->SendData(DMA_LENGTH_REG, dataLength);
    }
   

    /**
     * @brief 获取 DMA 数据 (优化版)
     * * @param baseAddress 基地址
     * @param oneDataByteNum 需要读取的字节数 (即使是 double 也转为整数处理)
     * @param outputBuffer [输出] 数据容器。传入引用以复用内存，避免频繁 malloc。
     */
    void  Device_Data_RTSA::GetDmaData(uint64_t baseAddress, uint64_t oneDataByteNum, std::vector<uint8_t>& outputBuffer)
    {
        // 1. 类型转换与准备
        // 将 double 转为 uint32_t，避免后续重复转换
        const uint32_t reqSize = static_cast<uint32_t>(oneDataByteNum);

        // 2. 内存优化：只有当现有容量不足时才分配内存
        // vector::resize 在 capacity 足够时非常快，只是移动指针
        if (outputBuffer.size() != reqSize) {
            outputBuffer.resize(reqSize);
        }

        // 获取指向 vector 内部数据的原生指针  
        uint8_t* pData = outputBuffer.data();

        uint64_t currentAddress = 0;
        bool isSplitRead = false; // 标记是否需要分两段读取

        // 3. 地址逻辑判断 (保持原 C# 逻辑)
        // 注意：C# 中的 GloblaRunning 是全局访问 

        if (baseAddress < IQEndAddress)
        {
            // 检查是否跨越了结束地址 (Ring Buffer Wrap)
            if (IQEndAddress - baseAddress < reqSize)
            {
                // 需要回卷：一部分在尾部，一部分在头部
                currentAddress = baseAddress;
                isSplitRead = true;
            }
            else
            {
                // 正常连续读取
                currentAddress = baseAddress;
                isSplitRead = false;
            }
        }
        else
        {
            // 如果 baseAddress 已经超出范围，原逻辑进行了偏移修正
            currentAddress = baseAddress - IQEndAddress + IQStartAddress;
            isSplitRead = false;
        }

        // 4. 执行 DMA 读取
        if (!isSplitRead)
        {
            // === 情况 A：单次读取 ===
            set_dma_config(currentAddress, reqSize);
            // 直接写入 outputBuffer 的开始位置，无中间拷贝
            get_dma_data(pData, reqSize);
        }
        else
        {
            // === 情况 B：分段读取  ===

            // 第一段：从 currentAddress 到 EndAddress
            uint32_t firstDataByteNum = static_cast<uint32_t>(IQEndAddress - currentAddress);

            // 配置并读取第一段
            set_dma_config(currentAddress, firstDataByteNum);
            // 直接写入 outputBuffer 的起始位置 [0 ... first-1]
            get_dma_data(pData, firstDataByteNum);

            // 第二段：从 StartAddress 开始，读取剩余长度
            uint32_t secondDataByteNum = reqSize - firstDataByteNum;

            // 配置并读取第二段
            set_dma_config(IQStartAddress, secondDataByteNum);

            // 直接写入 outputBuffer 的后半部分 
            // 指针偏移 pData + firstDataByteNum
            get_dma_data(pData + firstDataByteNum, secondDataByteNum);
        }
    }


    /**
     * @brief 获取 DMA 数据（优化版）
     * @param address 起始地址
     * @param dataLength 读取长度
     * @param baseAddress [输出] 更新后的下次基地址
     * @param outputBuffer [输出] 预分配的缓冲区引用，用于存储读取的数据
     */
    void  Device_Data_RTSA::GetDmaAddrData(uint64_t address, uint32_t dataLength, uint64_t& baseAddress, std::vector<uint8_t>& outputBuffer)
    {
        // 1. 预准备：确保输出缓冲区容量足够，避免在函数内部发生 memory realloc
        if (outputBuffer.size() != dataLength) {
            outputBuffer.resize(dataLength);
        }
        uint8_t* pDest = outputBuffer.data();

        // 缓存全局参数，减少内存寻址开销
        const uint64_t IQ_End = IQEndAddress;
        const uint64_t IQ_Start = IQStartAddress;

        uint64_t endPosition = address + dataLength;

        // 2. 逻辑判断：是否跨越环形缓冲区边界
        if (endPosition > IQ_End)
        {
            // === 情况 A: 跨越边界（需要分两段读取） ===

            // 第一部分：从当前地址读到缓冲区末尾
            uint32_t deltaPosition = static_cast<uint32_t>(IQ_End - address);
            set_dma_config(address, deltaPosition);
            // 直接读入目标缓冲区的前半部分
            get_dma_data(pDest, deltaPosition);

            // 第二部分：从缓冲区开头读取剩余数据
            uint32_t resetPosition = dataLength - deltaPosition;
            // 更新下次基地址 (C# 逻辑: IQStartAddress + 偏移量)
            baseAddress = IQ_Start + resetPosition;

            set_dma_config(IQ_Start, resetPosition);
            // 直接读入目标缓冲区的后半部分（利用指针偏移 pDest + deltaPosition）
            // 彻底消除了 C# 中的 Concat 拷贝开销
            get_dma_data(pDest + deltaPosition, resetPosition);
        }
        else
        {
            // === 情况 B: 连续读取（不跨边界） ===

            // 更新下次基地址
            baseAddress = address + dataLength;

            set_dma_config(address, dataLength);
            // 直接读入目标缓冲区
            get_dma_data(pDest, dataLength);
        }
    }
    // =========================================================
    // [Class 2] Device_Data_Multi 实现
    // =========================================================

    Device_Data_Multi* Device_Data_Multi::instance = nullptr;

    Device_Data_Multi::Device_Data_Multi() : privateEOF(false)
    {
       pcie_mem = Device::Device_MEM32::getInstance();
        // 构造时直接加载 Multi 实现
        m_impl.reset(new Multi_Impl());
    }

    Device_Data_Multi::~Device_Data_Multi() {}

    Device_Data_Multi* Device_Data_Multi::getInstance()
    {
        if (instance == nullptr) instance = new Device_Data_Multi();
        return instance;
    }

    int Device_Data_Multi::Device_OpenDevice() {
        return m_impl->Open();
    }

    int Device_Data_Multi::Device_CloseDevice() {
        return m_impl->Close();
    }

    bool Device_Data_Multi::getEOF() const { return privateEOF; }
    void Device_Data_Multi::setEOF(const bool& value) { privateEOF = value; }

    bool Device_Data_Multi::ReadPulseData(unsigned char* res, unsigned int readlen)
    {
        auto impl = static_cast<Multi_Impl*>(m_impl.get());
        // Pulse 需要 EOF 关闭
        return Helper_ReadData(impl->PulseDevice.get(), res, readlen, true);
    }

    bool Device_Data_Multi::ReadChannelsData(unsigned char* res, unsigned int readlen)
    {
        auto impl = static_cast<Multi_Impl*>(m_impl.get());
        // Channels 需要 EOF 关闭
        return Helper_ReadData(impl->ChannelsDevice.get(), res, readlen, true);
    }
}