#include "RTSA.h"
#include <cmath>
#include <thread>
#include <chrono>  

using namespace std::chrono_literals;
using namespace Device;
using namespace RTSAControl;

namespace RTSA {  
	RTSA& RTSA::Instance() noexcept {
		static RTSA inst;
		return inst;
	}

	RTSA* RTSA::getInstance() {
		return &Instance();
	}

	RTSA::RTSA()
		: logicConfig_(std::make_unique<LOGIC::LOGICApi>()),
		device_(Device::Device_Data_RTSA::getInstance()),
		common_(Common::CommonManager::getInstance())
	{
		// 初始化默认滤波器系数（来自原文件）
		FilterLowCoe_ = { 0xfff8,0x000b,0xffee,0x001c,0xffd6,0x003d,0xffab,0x0073,0xff66,0x00ca,
						  0xfefc,0x014c,0xfe5d,0x020d,0xfd72,0x032d,0xfc0c,0x04f1,0xf9bf,0x0816,
						  0xf520,0x0fbe,0xe52d,0x5160,0x5160,0xe52d,0x0fbe,0xf520,0x0816,0xf9bf,
						  0x04f1,0xfc0c,0x032d,0xfd72,0x020d,0xfe5d,0x014c,0xfefc,0x00ca,0xff66,
						  0x0073,0xffab,0x003d,0xffd6,0x001c,0xffee,0x000b,0xfff8 };
		FilterCoe_ = { 0xffaf,0xfff8,0xff68,0xffec,0xfeeb,0xffd9,0xfe3b,0xffc3,0xfd56,0xffaf,
					   0xfc38,0xffab,0xfadf,0xffce,0xf944,0x003f,0xf747,0x0150,0xf48a,0x03cc,
					   0xefaf,0x0ae4,0xdfb7,0x4c1f,0x4c1f,0xdfb7,0x0ae4,0xefaf,0x03cc,0xf48a,
					   0x0150,0xf747,0x003f,0xf944,0xffce,0xfadf,0xffab,0xfc38,0xffaf,0xfd56,
					   0xffc3,0xfe3b,0xffd9,0xfeeb,0xffec,0xff68,0xfff8,0xffaf };
		device_->Device_OpenDevice();
		spectrum = SpectrumData::getInstance();
		iq = IQData::getInstance();
		persistence = PersistenceData::getInstance();
		dmaData = DMAData::getInstance();
		device_->set_trigger_threshold(8.69);  //初次加载时恢复到 触发初始阈值 0 dBm
	}

	RTSA::~RTSA() = default;

	Global::RefLevelResults RTSA::SetRefLevel(int reflevel)
	{
		auto nativeResult = common_->rfControl_->SetRefLevel(reflevel);
		Global::RefLevelResults result{ 0,0 };
		result.Att = nativeResult.Att;
		result.FFTGainOffset = nativeResult.FFTGainOffset;
		Global::RefLevel = reflevel;
		UpdateCorrectValue(nativeResult.Att);
		return result;
	}

	Global::RefLevelResults RTSA::SetCF(uint64_t centerfreq)
	{
		common_->rfControl_->SetCenterFreq(centerfreq);
		common_->UpdateErrorValue(centerfreq);
		SetADCFilter(centerfreq);
		Global::RTSAParameter.Centerfrquency = centerfreq;
		return SetRefLevel(Global::RefLevel);
	}

	Global::RefLevelResults RTSA::SetCFRefLevel(uint64_t centerfreq, int reflevel)
	{
		common_->rfControl_->SetCenterFreq(centerfreq);
		common_->UpdateErrorValue(centerfreq);
		SetADCFilter(centerfreq);
		Global::RTSAParameter.Centerfrquency = centerfreq;
		Global::RTSAParameter.ATT = reflevel;
		return SetRefLevel(Global::RefLevel);
	}

	Global::RefLevelResults RTSA::SetPara(uint64_t CF, double Span, double RBW)
	{
		auto refLevelResults = SetCF(static_cast<uint64_t>(CF));
		logicConfig_->set_RBW(RBW);
		SetSpan(Span);
		Global::RTSAParameter.Centerfrquency = CF;
		Global::RTSAParameter.Span = Span;
		return refLevelResults;
	}

	void RTSA::Config() {
		logicConfig_->Config();
	}

	void RTSA::SetSpanRBW(double bw, uint32_t rbw) {
		logicConfig_->set_RBW(rbw);
		SetSpan(bw);
		Global::RTSAParameter.RBW = rbw;
	}

	void RTSA::SetSpan(double bw) {
		int num = static_cast<int>(1228.8e6 / bw);
		common_->adcconfig_->set_adc_dcm(static_cast<uint8_t>(num));
		SetFfthbdecim(num / 2);
		Global::RTSAParameter.DecimateFactor = num;
		UpdateAmpAppend();
		ClockSample = bw / 4;
	}

	void RTSA::SetRBW(uint32_t rbw) {
		logicConfig_->set_RBW(rbw);
		logicConfig_->Config();
		UpdateAmpAppend();
	}

	void RTSA::SetPowerOnOff(uint32_t flag) {
		common_->rfControl_->SetPowerOnOff(flag);
	}

	void RTSA::SetOutBW(int bw) {
		common_->rfControl_->SetOutBW(bw);
	}

	void RTSA::SetDetectorType(uint32_t detector_type) {
		logicConfig_->set_DetectorType(detector_type);
		logicConfig_->Config();
	}

	void RTSA::SetFFTLen(uint32_t fft_len_value) {
		logicConfig_->set_fft_len(fft_len_value);
		logicConfig_->Config();
	}

	void RTSA::SetFfthbdecim(uint32_t fft_hb_decim_value) {
		logicConfig_->set_fft_hb_decim(fft_hb_decim_value);
		logicConfig_->Config();
	}

	void RTSA::SetSweepTime(double SweepTime_value) {
		logicConfig_->set_SweepTime(SweepTime_value);
		logicConfig_->Config();
	}

	void RTSA::SetPersistenceNum(uint32_t PersistenceNum_value) {
		logicConfig_->set_PersistenceNum(PersistenceNum_value);
		logicConfig_->Config();
	}

	void RTSA::SetGraunityNum(uint32_t GraunityNum_value) {
		logicConfig_->set_GraunityNum(GraunityNum_value);
		logicConfig_->Config();
	}

	void RTSA::SetDenominatorNum(float DenominatorNum_value) {
		logicConfig_->set_DenominatorNum(DenominatorNum_value);
		logicConfig_->Config();
	}

	void RTSA::SetOffset(float offset_value) {
		logicConfig_->set_offset(offset_value);
		logicConfig_->Config();
	}

	void RTSA::SetZoomFactor(float ZoomFactor_value) {
		logicConfig_->set_ZoomFactor(ZoomFactor_value);
		logicConfig_->Config();
	}

	void RTSA::SetValueScale(float ValueScale_value) {
		logicConfig_->set_ValueScale(ValueScale_value);
		logicConfig_->Config();
	}

	void RTSA::SetOvlSel(uint32_t ovl_sel) {
		logicConfig_->set_Ovl_sel(ovl_sel);
		logicConfig_->Config();
	}

	void RTSA::SetFFTWindow(uint32_t fftwindow) {
		logicConfig_->set_FFTWindow((LOGIC::FFTWindow)fftwindow);
		logicConfig_->Config();
	}

	void RTSA::SetTraceNum(uint32_t trace_num) {
		logicConfig_->set_Trace_num(trace_num);
		logicConfig_->Config();
	}
	uint32_t RTSA::GetAtt() {
		return common_->GetIFATT() + common_->GetRFATT();
	}

	float RTSA::GetDenominatorNum() {
		return logicConfig_->Get_DenominatorNum();
	}

	double RTSA::GetSweepTimeBack() {
		return logicConfig_->Get_SweepTimeBack();
	}

	void RTSA::SetADCFilter(uint64_t centerfreq)
	{
		common_->SetADCFilter(centerfreq);
	}

	int RTSA::Persistence_GetData(unsigned char* PersistenceData)
	{
		try {
			if (!device_->ReadPersistenceData(PersistenceData, 1024 * 512 * 4))
				return -1;
			std::this_thread::sleep_for(1ms);
		}
		catch (std::exception& ex) {
			// 不在库层打印，返回错误
			return -1;
		}
		return 0;
	}

	Global::Parameter* RTSA::Logic_Get_Parameter()
	{
		try {
			return &Global::RTSAParameter;
		}
		catch (std::exception&) {
			return nullptr;
		}
	}

	void RTSA::SetTriggerSource(uint32_t source)
	{
		device_->set_trigger_source(source);
	}

	double RTSA::GetIQCorrectValue()
	{
		return 	Global::IQFFTBaseError + Global::ErrorValue + Global::CorrectValue;
	}

	void RTSA::SetTriggerPosttime(double time)
	{
		int PostTotalNum = static_cast<int>(std::floor(time * ClockSample));
		device_->set_trigger_posttime(PostTotalNum);
	}

	void RTSA::SetTriggerThreshold_dBm(float threshold)
	{
		float lndbm = (float)((threshold - Global::ErrorValue - Global::IQPowerBaseError - Global::CorrectValue) / 20) * 2.3f;
		device_->set_trigger_threshold(lndbm);
	}

	void RTSA::SetTriggerThreshold_v(float threshold)
	{
		float dbmvalue = 10.0f * log10f(powf(threshold, 2.0f) / 50000.0f);
		float vvalue = (float)powf(10, (dbmvalue - Global::ErrorValue - Global::IQPowerBaseError - Global::CorrectValue) / 20);
		device_->set_trigger_threshold(vvalue);
	}

	uint64_t RTSA::GetTriggerDataAddress()
	{
		return device_->get_trigger_dataAddress();
	}

	void  RTSA::GetDmaData(uint64_t baseAddress, uint64_t oneDataByteNum, std::vector<uint8_t>& outputBuffer)
	{
		device_->GetDmaData(baseAddress, oneDataByteNum, outputBuffer);
	}

	void  RTSA::GetDmaAddrData(uint64_t address, uint32_t dataLength, uint64_t& baseAddress, std::vector<uint8_t>& outputBuffer)
	{
		device_->GetDmaAddrData(address, dataLength, baseAddress, outputBuffer);
	}
	void  RTSA::ReadRawTriggerData(uint64_t baseAddress, uint64_t oneDataByteNum, std::vector<uint8_t>& outputBuffer)
	{
		dmaData->GetRawTriggerData(baseAddress, oneDataByteNum, outputBuffer);
	}
	void RTSA::ReadProcessedTriggerData(unsigned long long dataPosition, uint64_t oneDataByteNum, std::vector<int16_t>& outI, std::vector<int16_t>& outQ) {
		dmaData->GetProcessedTriggerData(dataPosition, oneDataByteNum, outI, outQ);
	}

	bool RTSA::GetInterrupt() {
		return device_->GetInterrupt();
	}
	void RTSA::UpdateCorrectValue(int RFATT)
	{
		if (Global::RefLevel >= -50)
		{
			auto IFATT = common_->rfControl_->GetIFATT();
			/*       int delta = (int)IFATT - 10; */
			Global::FFTGainOffset = 0;
			Global::CorrectValue = RFATT + IFATT;
		}
		else
		{
			if (Global::RefLevel >= -60 && Global::RefLevel < -50)
			{
				Global::FFTGainOffset = 1;
			}
			else if (Global::RefLevel >= -70 && Global::RefLevel < -60)
			{
				Global::FFTGainOffset = 2;
			}
			else if (Global::RefLevel >= -80 && Global::RefLevel < -70)
			{
				Global::FFTGainOffset = 3;
			}
			else if (Global::RefLevel >= -90 && Global::RefLevel < -80)
			{
				Global::FFTGainOffset = 4;
			}
			Global::CorrectValue = -15 - 6 * (int)Global::FFTGainOffset; //去除低噪放增益以及DDC增益 
		}
	}

	void RTSA::UpdateAmpAppend()
	{
		int num = static_cast<int>(common_->ADCSampleClock / Global::RTSAParameter.Span);
		unsigned int decimation = static_cast<unsigned int>(num);
		double Compensation = 20 * std::log10(decimation * Global::RTSAParameter.RBW / 200000.0);
		Global::AmpAppend = Compensation;
	}

	int RTSA::GetCorrectValue() {
		return Global::CorrectValue;
	}

	double RTSA::GetErrorValue() {
		return Global::ErrorValue;
	}

	double RTSA::GetAmpAppend() {
		return Global::AmpAppend;
	}

	double RTSA::GetBaseErrorValue() {
		return Global::BaseErrorValue;
	}

	void RTSA::ReadSpectrum()
	{
		return spectrum->read_spectrum();
	}

	void RTSA::ProcessSpectrum()
	{
		return spectrum->process_spectrum();
	}

	int RTSA::GetSpectrumData(unsigned char* buffer, int bufferSize)
	{
		if (buffer == nullptr) {
			throw std::invalid_argument("buffer is null");
		}
		if (bufferSize <= 0) {
			return 0;
		}
		int bytesWritten = spectrum->read_spectrum_one(buffer, bufferSize);
		return bytesWritten;
	}

	std::vector<FreqAmpData> RTSA::GetSpectrumSnapshot()
	{
		return spectrum->GetSpectrumDataSnapshot();
	}

	int RTSA::ReadProcessedSpectrumOne(unsigned char* buffer, int len)
	{
		return spectrum->read_spectrum_one(buffer, len);
	}

	std::vector<unsigned char>* RTSA::ReadRawSpectrumOne()
	{
		try
		{
			// 1. 优化内存分配：仅在大小改变或不足时进行 resize
			if (spectrumBuffer.size() != static_cast<size_t>(spectrumRequiredSize)) {
				spectrumBuffer.resize(spectrumRequiredSize);
			}

			// 2. 获取原生指针
			// 原生 vector 的内存是连续且固定的，直接使用 .data()
			unsigned char* nativeBuffer = spectrumBuffer.data();

			// 3. 调用底层 API 填充数据
			// 直接写入 vector 的内存区域
			int bytesRead = GetSpectrumData(nativeBuffer, (int)spectrumBuffer.size());

			if (bytesRead <= 0) {
				return nullptr;
			}
			return &spectrumBuffer;
		}
		catch (const std::exception& ex)
		{
			throw std::runtime_error(ex.what());
		}
	}

	std::vector<double>* RTSA::ReadProcessedSpectrumOne()
	{
		// 1. 先调用原始数据读取逻辑 
		ReadRawSpectrumOne();
		if (spectrumBuffer.empty()) return nullptr;

		// 2. 优化内存分配
		if (spectrumAmpBuffer.size() != static_cast<size_t>(SpectrumSize)) {
			spectrumAmpBuffer.resize(SpectrumSize);
		}
		// 3. 获取原生指针
		// 注意：原代码将 Byte* 强转为 uint*，即每 4 字节作为一个样本
		const unsigned int* pUint = reinterpret_cast<const unsigned int*>(spectrumBuffer.data());
		double* dst = spectrumAmpBuffer.data();

		// 4. 预计算常数项（移出循环以优化性能）
		const double correction = -Global::BaseErrorValue - Global::ErrorValue + Global::AmpAppend + Global::CorrectValue;;

		// 5. 核心计算循环
		for (int i = 0; i < SpectrumSize; ++i)
		{
			unsigned int v = pUint[i];
			if (v == 0) v = 1; // 防止 log10(0) 导致无穷大

			// 使用标准库的 std::log10
			dst[i] = 20.0 * std::log10(static_cast<double>(v)) + correction;
		}
		return &spectrumAmpBuffer;
	}

	int RTSA::ReadProcessedSpectrumOne(std::vector<double>& outputBuffer)
	{
		// 确保输出缓冲区大小足够
		if (outputBuffer.size() < static_cast<size_t>(SpectrumSize)) {
			outputBuffer.resize(SpectrumSize);
		}
		try
		{
			// 2. 准备原始字节缓冲区 (类成员变量，实现内存复用)
			// SpectrumRequiredSize 通常为 SpectrumSize * sizeof(unsigned int)
			if (spectrumBuffer.size() < static_cast<size_t>(spectrumRequiredSize)) {
				spectrumBuffer.resize(spectrumRequiredSize);
			}

			// 3. 直接调用底层 API 读取原始字节
			unsigned char* pRaw = spectrumBuffer.data();
			int bytesRead = GetSpectrumData(pRaw, (int)spectrumBuffer.size());

			if (bytesRead <= 0) {
				return 0;
			}

			// 4. 将字节指针转换为 32位无符号整数指针
			// 每个点占用 4 字节
			unsigned int* pUint = reinterpret_cast<unsigned int*>(pRaw);

			// 5. 准备校准参数（对应原来的 Global 变量）
			const double correction = -Global::BaseErrorValue
				+ Global::ErrorValue
				+ Global::AmpAppend
				+ Global::CorrectValue;

			// 6. 核心处理循环
			double* pOut = outputBuffer.data();
			for (int i = 0; i < SpectrumSize; ++i)
			{
				unsigned int v = pUint[i];
				if (v == 0) v = 1; // 避免 log10(0)

				// 使用标准库 std::log10，性能通常由于编译器优化非常出色
				pOut[i] = 20.0 * std::log10(static_cast<double>(v)) + correction;
			}

			return SpectrumSize;
		}
		catch (const std::exception& ex) {
			// 原生 C++ 抛出标准异常
			throw std::runtime_error(ex.what());
		}
	}

	void RTSA::ReadIQ()
	{
		return iq->read_iq();
	}

	std::vector<std::pair<std::vector<int16_t>, std::vector<int16_t>>> RTSA::ProcessIQ()
	{
		return iq->process_iq();
	}

	std::vector<unsigned char>* RTSA::ReadIQOne()
	{
		try
		{
			size_t targetSize = 1024 * 2400;
			if (IQBuffer.size() != targetSize) {
				IQBuffer.resize(targetSize);
			}
			unsigned char* nativeBuffer = IQBuffer.data();
			int bytesRead = iq->read_iq_one(nativeBuffer, (int)IQBuffer.size());
			if (bytesRead <= 0) {
				return nullptr;
			}
			return &IQBuffer;
		}
		catch (const std::exception& ex)
		{
			throw std::runtime_error(ex.what());
		}
	}

	std::pair<std::vector<short>*, std::vector<short>*> RTSA::ReadProcessedIQOne()
	{
		try
		{
			// 1. 获取原始字节数据
			std::vector<unsigned char>* raw = ReadIQOne();

			if (raw == nullptr || raw->empty()) {
				return { nullptr, nullptr };
			}

			// 2. 计算采样点数 (每个采样点由 2字节I + 2字节Q = 4字节组成)
			size_t rawLength = raw->size();
			int sampleCount = static_cast<int>(rawLength / 4);

			if (sampleCount <= 0) {
				return { nullptr, nullptr };
			}

			// 3. 优化内存：仅在大小不足时重新分配
			if (IBuffer.size() < (size_t)sampleCount) {
				IBuffer.resize(sampleCount);
				QBuffer.resize(sampleCount);
			}

			// 4. 获取原生指针直接进行分离
			// 在原生 C++ 中不需要 pin_ptr，内存地址在 resize 后是固定的
			const unsigned char* src = raw->data();
			short* dstI = IBuffer.data();
			short* dstQ = QBuffer.data();

			// 5. 拆分交织数据 (I_L, I_H, Q_L, Q_H)
			for (int i = 0; i < sampleCount; ++i)
			{
				int offset = i * 4;
				// 使用小端字节序重组 short (16-bit)
				dstI[i] = static_cast<short>(src[offset] | (src[offset + 1] << 8));
				dstQ[i] = static_cast<short>(src[offset + 2] | (src[offset + 3] << 8));
			}

			// 返回指向成员变量 vector 的指针
			return { &IBuffer, &QBuffer };
		}
		catch (const std::exception& ex)
		{
			throw std::runtime_error(ex.what());
		}
	}

	int RTSA::ReadProcessedIQOne(std::vector<short>& iBuffer, std::vector<short>& qBuffer)
	{
		// 1. 基础检查与内存预备
		int iqRequiredSize = 1024 * 2400;
		int reqSamples = iqRequiredSize / 4;

		if (m_nativeRawBuffer.size() != iqRequiredSize) {
			m_nativeRawBuffer.resize(iqRequiredSize);
		}

		// 确保输出缓冲区大小足够
		if (iBuffer.size() < (size_t)reqSamples) iBuffer.resize(reqSamples);
		if (qBuffer.size() < (size_t)reqSamples) qBuffer.resize(reqSamples);

		// 2. 调用底层 API 读取数据
		int bytesRead = iq->read_iq_one(m_nativeRawBuffer.data(), iqRequiredSize);
		if (bytesRead <= 0) return 0;

		// 3. 极速拆分
		// 在原生 C++ 中，直接使用 data() 获取指针，无需 pin_ptr
		short* pSrc = reinterpret_cast<short*>(m_nativeRawBuffer.data());
		short* pDstI = iBuffer.data();
		short* pDstQ = qBuffer.data();

		// 4. 数据拆分循环
		// I_L, I_H, Q_L, Q_H 排列意味着 pSrc[0]=I, pSrc[1]=Q
		for (int i = 0; i < reqSamples; i++)
		{
			*pDstI++ = *pSrc++;
			*pDstQ++ = *pSrc++;
		}

		return reqSamples;
	}

	int RTSA::ReadProcessedIQOne(short* pDstI, short* pDstQ)
	{
		int iqRequiredSize = 1024 * 2400;
		int reqSamples = iqRequiredSize / 4;
		if (m_nativeRawBuffer.size() != iqRequiredSize) m_nativeRawBuffer.resize(iqRequiredSize);

		int bytesRead = iq->read_iq_one(m_nativeRawBuffer.data(), iqRequiredSize);
		if (bytesRead <= 0) return 0;

		short* pSrc = reinterpret_cast<short*>(m_nativeRawBuffer.data());

		for (int i = 0; i < reqSamples; i++) {
			*pDstI++ = *pSrc++;
			*pDstQ++ = *pSrc++;
		}
		return reqSamples;
	}

	int RTSA::read_iq_one(unsigned char* external_buffer, int buffer_size)
	{
		return iq->read_iq_one(external_buffer, buffer_size);
	}

	void RTSA::ReadPersistence()
	{
		return persistence->read_persistence();
	}

	void RTSA::ProcessPersistence()
	{
		return persistence->process_persistence();
	}

	int RTSA::ReadPersistenceOne(std::vector<float>& rawData)
	{
		try
		{
			if (rawData.empty()) {
				return -1;
			}

			// 1. 计算需要的字节大小
			int requiredByteSize = static_cast<int>(rawData.size() * sizeof(float));

			// 2. 直接获取 float 向量的原始地址
			// 在原生 C++ 中，vector 的内存是连续的，可以直接当作 unsigned char* 使用
			unsigned char* nativeBuffer = reinterpret_cast<unsigned char*>(rawData.data());

			// 3. 调用底层的 C++ API
			// 直接将数据写入 float 数组所在的地址，省去了中间的 Byte 数组和 memcpy
			int bytesRead = persistence->read_persistence_one(
				nativeBuffer,
				requiredByteSize
			);

			if (bytesRead <= 0) {
				return -1;
			}

			// 返回读取到的 float 个数
			return bytesRead / sizeof(float);
		}
		catch (const std::exception& ex)
		{
			// 抛出原生异常
			throw std::runtime_error(ex.what());
		}
	}

	/**
	 * @brief 读取余辉(Persistence)数据到指定的指针位置
	 * @param pDstFloat 指向目标 float 缓冲区的指针
	 * @param maxFloatCount 目标缓冲区能容纳的 float 数量
	 * @return 成功读取的 float 元素数量，失败返回 -1
	 */
	int RTSA::ReadPersistenceOne(float* pDstFloat, int maxFloatCount)
	{
		if (pDstFloat == nullptr || maxFloatCount <= 0) {
			return -1;
		}

		try
		{
			// 1. 计算需要的字节大小
			int requiredByteSize = maxFloatCount * sizeof(float);

			// 2. 强转指针：将 float* 视为 unsigned char* // 这样底层 API 的数据会直接填充到你传入的 float 数组内存空间中
			unsigned char* pNativeBuffer = reinterpret_cast<unsigned char*>(pDstFloat);

			// 3. 调用底层的 C++ API (直接写入目标内存)
			int bytesRead = read_persistence_one(
				pNativeBuffer,
				requiredByteSize
			);

			if (bytesRead <= 0) {
				return -1;
			}

			// 4. 返回读取到的 float 个数
			return requiredByteSize / sizeof(float);
		}
		catch (const std::exception& ex)
		{
			// 原生 C++ 抛出标准异常
			throw std::runtime_error(ex.what());
		}
	}

	int RTSA::read_persistence_one(unsigned char* external_buffer, int buffer_size)
	{
		return persistence->read_persistence_one(external_buffer, buffer_size);
	}

	std::vector<float> RTSA::GetFloatData()
	{
		return persistence->getfloatdata();
	}

	void RTSA::CloseDevice()
	{
		device_->Device_CloseDevice();
	}
}