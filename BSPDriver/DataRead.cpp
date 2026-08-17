#include <Windows.h>
#include "DataRead.h"
#include "RTSA.h"

using namespace Device;
using namespace RTSAControl;

RTSAControl::SpectrumData* RTSAControl::SpectrumData::instance = nullptr;
RTSAControl::IQData* RTSAControl::IQData::instance = nullptr;
RTSAControl::PersistenceData* RTSAControl::PersistenceData::instance = nullptr;
RTSAControl::DMAData* RTSAControl::DMAData::instance = nullptr;

SpectrumData::SpectrumData()
{
	buffer_spectrum.resize(4 * 1024);
	device = Device::Device_Data_RTSA::getInstance();
	state_spectrum = false;
}

SpectrumData::~SpectrumData()
{

}

void SpectrumData::read_spectrum()
{
	std::vector<unsigned char> buffer(4 * 1024);
	while (state_spectrum)
	{
		device->ReadSpectrumData(buffer.data(), buffer.size());
		data_queue.Push(buffer);
	}
}

std::vector<unsigned char> SpectrumData::read_spectrum_one()
{
	device->ReadSpectrumData(buffer_spectrum.data(), buffer_spectrum.size());
	return buffer_spectrum;
}

int SpectrumData::read_spectrum_one(unsigned char* external_buffer, int buffer_size) {
	if (!device) {
		return -1; // 错误：设备未初始化
	}
	if (!external_buffer) {
		return -2; // 错误：缓冲区为空
	}

	// 关键：调用底层的 ReadSpectrumData，
	// 但是使用 *外部* 传入的缓冲区和大小
	return device->ReadSpectrumData(external_buffer, buffer_size);
}

void SpectrumData::process_spectrum()
{
	std::vector<unsigned char> raw_data(4 * 1024);
	while (state_spectrum) {
		if (!data_queue.Pop(raw_data)) {
			continue;
		}
		sptrmPointList.reserve(1024);
		/*	double Spanstep = Global::RTSAParameter.Span / 1023;
			double Startfreq = Global::RTSAParameter.Centerfrquency - Global::RTSAParameter.Span / 2;*/
		double Spanstep = 614.4e6 / 1023;
		double Startfreq = 2e9 - 614.4e6 / 2;
		std::vector<uint32_t> sptrmData(raw_data.size() / 4);
		std::memcpy(sptrmData.data(), raw_data.data(), raw_data.size());
		for (int i = 0; i < sptrmData.size(); i++)
		{
			double value = Startfreq + Spanstep * i;
			int64_t i_freq = std::round(value);
			double freq = (double)i_freq;
			uint32_t iqData1 = (sptrmData[i] == 0) ? 1 : sptrmData[i];
			double d = static_cast<double>(iqData1);
			double amp = 0;
			//没写完

			amp = 20 * std::log10(d);

			//sptrmPointList.push_back(FreqAmpData(freq, std::sqrt(std::pow(10, amp / 10) / 1000 * 50) * 1000));
			//sptrmPointList.push_back(FreqAmpData(freq, std::pow(10, amp / 10)));
			sptrmPointList.push_back(FreqAmpData(freq, amp));

		}
	}
}

std::vector<FreqAmpData> SpectrumData::GetSpectrumDataSnapshot()
{
	std::lock_guard<std::mutex> lock(mutex);
	return sptrmPointList;
}

//std::vector<FreqAmpData> SpectrumData::GetSpectrumData()
//{
//	std::vector<FreqAmpData> sptrmPointHighList(1001);
//	std::vector<FreqAmpData> sptrmPointLowList(1001);
//	std::vector<FreqAmpData> sptrmPointList(1001 * 2);
//
//	vector<uint8_t> buffer;
//	device->ReadSpectrumData(buffer.data(), 1001 * 2);
//
//	size_t uintCount = buffer.size() / 4;
//	std::vector<uint32_t> rawData(uintCount);
//	std::memcpy(rawData.data(), buffer.data(), buffer.size());
//	size_t datalength = uintCount / 2;
//	std::vector<double> highData(datalength, 0.0);
//	std::vector<double> lowData(datalength, 0.0);
//	for (size_t i = 0; i < datalength; i++) {
//		uint32_t u1 = rawData[i * 2];
//		uint32_t u2 = rawData[i * 2 + 1];
//		double iqData1 = (u1 == 0) ? 1.0 : static_cast<double>(u1);
//		double iqData2 = (u2 == 0) ? 1.0 : static_cast<double>(u2);
//	}
//
//	if (Global::SweepParas.SpectrumPointCountSet < 1001) {
//		highData = Interp(highData, 1001);
//		lowData = Interp(lowData, 1001);
//	}
//
//	int rbw = static_cast<int>(GlobalRunning::SpectrumParams.Rbw);
//	size_t finalPointCount = highData.size();  // 插值后可能是 1001 个点
//
//	for (size_t i = 0; i < finalPointCount; i++) {
//		double freq;
//		if (GlobalRunning::SpectrumParams.Span == 0) {
//			freq = GlobalRunning::startfreq + GlobalRunning::perspan * i;
//		}
//		else {
//			freq = GlobalRunning::startfreq + GlobalRunning::perspan * i;
//		}
//
//		double amp = highData[i]
//			- GlobalRunning::SweepBaseErrorValue
//			+ GlobalRunning::CommonParams.CorrectValue
//			+ GlobalRunning::RbwErrDIC[rbw];
//
//		sptrmPointHighList[i] = FreqAmpData(freq, amp);
//
//		double amp2 = lowData[i]
//			- GlobalRunning::SweepBaseErrorValue
//			+ GlobalRunning::CommonParams.CorrectValue
//			+ GlobalRunning::RbwErrDIC[rbw];
//
//		sptrmPointLowList[i] = FreqAmpData(freq, amp2);
//	}
//	return sptrmPointHighList;
//}

void SpectrumData::Start_Spectrum()
{
	if (state_spectrum) return;
	state_spectrum = true;
	std::thread read(&SpectrumData::read_spectrum, this);
	std::thread t(&SpectrumData::process_spectrum, this);
	read.detach();
	t.detach();
}

void SpectrumData::Stop_Spectrum()
{
	if (!state_spectrum) return;
	state_spectrum = false;
}


IQData::IQData()
{
	buffer_iq.resize(1024 * 2400);
	device = Device::Device_Data_RTSA::getInstance();
	state_iq = false;
}

IQData::~IQData()
{

}

static std::pair<std::vector<uint8_t>, std::vector<uint8_t>> HandleIQData(const std::vector<uint8_t>& IQdata)
{
	size_t validLength = IQdata.size() - (IQdata.size() % 4);
	if (validLength == 0)
		return std::make_pair(std::vector<uint8_t>{}, std::vector<uint8_t>{});

	size_t numSamples = validLength / 4;
	std::vector<uint8_t> IData(numSamples * 2);
	std::vector<uint8_t> QData(numSamples * 2);

	for (size_t s = 0; s < numSamples; ++s)
	{
		size_t base = s * 4;
		IData[s * 2 + 0] = IQdata[base + 0];
		IData[s * 2 + 1] = IQdata[base + 1];
		QData[s * 2 + 0] = IQdata[base + 2];
		QData[s * 2 + 1] = IQdata[base + 3];
	}

	return std::make_pair(IData, QData);
}

static std::vector<int16_t> bytesToHwsToShorts(const std::vector<uint8_t>& buffer)
{
	if (buffer.empty())
		return {};

	size_t shortCount = buffer.size() / 2;
	std::vector<int16_t> backData(shortCount);

	for (size_t i = 0; i < shortCount; ++i)
	{
		uint8_t b1 = buffer[i * 2 + 0];
		uint8_t b2 = buffer[i * 2 + 1];
		int16_t value = static_cast<int16_t>((b2 << 8) | b1);
		backData[i] = value;
	}
	return backData;
}

void IQData::read_iq()
{
	std::vector<unsigned char> buffer(1024 * 2400);
	while (state_iq)
	{
		device->ReadIQData(buffer.data(), buffer.size());
		data_queue.Push(buffer);
	}
}

int IQData::read_iq_one(unsigned char* external_buffer, int buffer_size) {
	if (!device) {
		return -1; // 错误：设备未初始化
	}
	if (!external_buffer) {
		return -2; // 错误：缓冲区为空
	}

	// 关键：调用底层的 ReadSpectrumData，
	// 但是使用 *外部* 传入的缓冲区和大小

	return device->ReadIQData(external_buffer, buffer_size);
}
std::vector<unsigned char> IQData::read_iq_one()
{
	device->ReadIQData(buffer_iq.data(), buffer_iq.size());
	return buffer_iq;
}

//void IQData::process_iq()
//{
//	std::vector<unsigned char> raw_data(2400 * 1024);
//	while (state_iq) {
//		if (!data_queue.Pop(raw_data)) {
//			continue;
//		}
//		auto data = HandleIQData(raw_data);
//		std::vector<int16_t> IData = bytesToHwsToShorts(data.first);
//		std::vector<int16_t> QData = bytesToHwsToShorts(data.second);
//	}
//}

std::vector<std::pair<std::vector<int16_t>, std::vector<int16_t>>> IQData::process_iq()
{
	std::vector<unsigned char> raw_data(2400 * 1024);
	while (state_iq) {
		if (!data_queue.Pop(raw_data)) {
			continue;
		}

		auto data = HandleIQData(raw_data);
		std::vector<int16_t> IData = bytesToHwsToShorts(data.first);
		std::vector<int16_t> QData = bytesToHwsToShorts(data.second);

		all_data.emplace_back(IData, QData);
	}

	return all_data;
}


std::vector<std::pair<std::vector<int16_t>, std::vector<int16_t>>> IQData::getiq()
{
	return IQData::all_data;
}

void IQData::Start_IQ()
{
	if (state_iq) return;
	state_iq = true;
	std::thread read(&IQData::read_iq, this);
	std::thread t(&IQData::process_iq, this);
	read.detach();
	t.detach();
}

void IQData::Stop_IQ()
{
	if (!state_iq) return;
	state_iq = false;
}


PersistenceData::PersistenceData()
{
	buffer_persistence.resize(1024 * 512 * 4);
	device = Device::Device_Data_RTSA::getInstance();
	state_persistence = false;
}

PersistenceData::~PersistenceData()
{

}

void PersistenceData::HandlePersistence(const std::vector<uint8_t>& PersistenceData)
{
	//std::vector<float> floatData = Global::GlobalParameter::GetInstance().GetFloatData();
	size_t expectedFloatCount = PersistenceData.size() / sizeof(float);
	if (floatData.empty() || floatData.size() != expectedFloatCount) {
		floatData.resize(expectedFloatCount);
	}
	if (PersistenceData.size() % sizeof(float) != 0) {
		return;
	}
	std::memcpy(floatData.data(), PersistenceData.data(), PersistenceData.size());
}

void PersistenceData::read_persistence()
{
	std::vector<unsigned char> buffer(1024 * 512 * 4);
	while (state_persistence)
	{
		device->ReadPersistenceData(buffer.data(), buffer.size());
		data_queue.Push(buffer);
	}
}
int PersistenceData::read_persistence_one(unsigned char* external_buffer, int buffer_size) {
	if (!device) {
		return -1; // 错误：设备未初始化
	}
	if (!external_buffer) {
		return -2; // 错误：缓冲区为空
	}

	// 关键：调用底层的 ReadSpectrumData，
	// 但是使用 *外部* 传入的缓冲区和大小

	return device->ReadPersistenceData(external_buffer, buffer_size);
}

std::vector<unsigned char> PersistenceData::read_persistence_one()
{
	device->ReadPersistenceData(buffer_persistence.data(), buffer_persistence.size());
	return buffer_persistence;
}


void PersistenceData::process_persistence()
{
	std::vector<unsigned char> raw_data(1024 * 512 * 4);

	while (state_persistence) {
		if (!data_queue.Pop(raw_data)) {
			continue;
		}
		if (floatData.empty() || floatData.size() != raw_data.size() / 4)
		{
			floatData.resize(raw_data.size() / 4);
		}
		std::memcpy(floatData.data(), raw_data.data(), raw_data.size());
	}
}

void PersistenceData::Start_Persistence()
{
	if (state_persistence) return;
	state_persistence = true;
	std::thread read(&PersistenceData::read_persistence, this);
	std::thread t(&PersistenceData::process_persistence, this);
	read.detach();
	t.detach();
}

void PersistenceData::Stop_Persistence()
{
	if (!state_persistence) return;
	state_persistence = false;
}

std::vector<float> PersistenceData::getfloatdata()
{
	std::lock_guard<std::mutex> lock(mutex);
	return floatData;
}


DMAData::DMAData()
{
	device = Device::Device_Data_RTSA::getInstance();
	pcie_mem = Device::Device_MEM32::getInstance();
}

DMAData::~DMAData()
{

}
 
uint64_t DMAData::CalculateBasePosition(unsigned long long dataPosition) {
	if (dataPosition < IQStartAddress) {
		dataPosition = IQEndAddress + (dataPosition - IQStartAddress);
	}
	return dataPosition;
}

void DMAData::GetRawTriggerData(unsigned long long dataPosition, uint64_t oneDataByteNum, std::vector<uint8_t>& outData) {
	uint64_t basePos = CalculateBasePosition(dataPosition);

	// 优化：确保输出容器大小正确。如果容量已足够，resize 是近乎免费的。
	if (outData.size() != oneDataByteNum) {
		outData.resize(oneDataByteNum);
	}

	// 直接读入外部传入的容器内存，实现零拷贝
	device->GetDmaData(basePos, static_cast<double>(oneDataByteNum), outData);
}
void DMAData::GetProcessedTriggerData(unsigned long long dataPosition, uint64_t oneDataByteNum, std::vector<int16_t>& outI, std::vector<int16_t>& outQ) {
	uint64_t basePos = CalculateBasePosition(dataPosition);

	// 1. 使用内部成员缓冲区读取原始 DMA 数据
	if (m_internalBuffer.size() != oneDataByteNum) {
		m_internalBuffer.resize(oneDataByteNum);
	}
	device->GetDmaData(basePos, static_cast<double>(oneDataByteNum), m_internalBuffer);

	if (m_internalBuffer.empty()) return;

	// 2. 假设数据是 I/Q 交替的 16bit 数据 (这是 PCIE 中断/DMA 常见的格式)
	// 根据具体协议调整转换逻辑，这里演示高效的数据拆分
	size_t sampleCount = oneDataByteNum / 4; // 1个I(2字节) + 1个Q(2字节) = 4字节

	if (outI.size() != sampleCount) outI.resize(sampleCount);
	if (outQ.size() != sampleCount) outQ.resize(sampleCount);

	// 获取原生指针以提高遍历速度
	const int16_t* rawDataPtr = reinterpret_cast<const int16_t*>(m_internalBuffer.data());
	int16_t* pI = outI.data();
	int16_t* pQ = outQ.data();

	// 3. 核心解复用 (De-interleaving) 优化
	// 使用指针直接偏移，编译器可以很好地优化此循环
	for (size_t i = 0; i < sampleCount; ++i) {
		*pI++ = rawDataPtr[2 * i];     // I 数据
		*pQ++ = rawDataPtr[2 * i + 1]; // Q 数据
	}
}

std::vector<uint8_t> DMAData::ReadBackData(uint32_t add, uint32_t size)
{
	try
	{
		std::vector<uint8_t> data(size * 4);
		pcie_mem->ReadBackData(add, size, data.data());
		return data;
	}
	catch (const std::exception&)
	{
		return {};
	}
}
 

