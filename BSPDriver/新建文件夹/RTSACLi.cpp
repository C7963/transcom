#include "RTSACli.h"  

using namespace RTSAControlCLI;
using namespace RTSAControl; 
const int SpectrumRequiredSize = 4096;
const int SpectrumSize = SpectrumRequiredSize/4;
const int IQRequiredSize = 1024 * 2400;
const int PersistenceRequiredSize = 1024 * 512 * 4;
std::vector<unsigned char>* m_nativeRawBuffer;

RTSACli::RTSACli()
{
	rtsa = RTSA::RTSA::getInstance();
}

RTSACli::~RTSACli()
{

}

CommonCli::RefLevelResultsCli RTSACli::SetCF(uint64_t centerfreq)
{
	Global::RefLevelResults nativeResult = rtsa->SetCF(centerfreq);
	CommonCli::RefLevelResultsCli cliResult{0,0};
	cliResult.Att = nativeResult.Att;
	cliResult.FFTGainOffset = nativeResult.FFTGainOffset;
	return cliResult; 
}

CommonCli::RefLevelResultsCli RTSACli::SetRefLevel(int reflevel)
{
	Global::RefLevelResults nativeResult = rtsa->SetRefLevel(reflevel);
	CommonCli::RefLevelResultsCli cliResult{ 0,0 };
	cliResult.Att = nativeResult.Att;
	cliResult.FFTGainOffset = nativeResult.FFTGainOffset;
	return cliResult;

}

void RTSACli::SetPowerOnOff(uint32_t flag)
{
	rtsa->SetPowerOnOff(flag);
}

void RTSACli::SetOutBW(int bw)
{
	rtsa->SetOutBW(bw);
}

CommonCli::RefLevelResultsCli RTSACli::SetPara(uint64_t CF, double Span, double RBW)
{
	Global::RefLevelResults nativeResult = rtsa->SetPara(CF, Span, RBW);
	CommonCli::RefLevelResultsCli cliResult{ 0,0 };
	cliResult.Att = nativeResult.Att;
	cliResult.FFTGainOffset = nativeResult.FFTGainOffset;
	return cliResult;
}

void RTSACli::Config()
{
	rtsa->Config();
}

void RTSACli::SetSpanRBW(double bw, uint32_t rbw)
{
	rtsa->SetSpanRBW(bw, rbw);
}

void RTSACli::SetSpan(double bw)
{
	rtsa->SetSpan(bw);
}

void RTSACli::SetRBW(uint32_t rbw)
{
	rtsa->SetRBW(rbw);
}

void RTSACli::SetDetectorType(uint32_t detector_type)
{
	rtsa->SetDetectorType(detector_type);
}

void RTSACli::SetFFTLen(uint32_t fft_len_value)
{
	rtsa->SetFFTLen(fft_len_value);
}

void RTSACli::SetFfthbdecim(uint32_t fft_hb_decim_value)
{
	rtsa->SetFfthbdecim(fft_hb_decim_value);
}

void RTSACli::SetSweepTime(double SweepTime_value)
{
	rtsa->SetSweepTime(SweepTime_value);
}

void RTSACli::SetPersistenceNum(uint32_t PersistenceNum_value)
{
	rtsa->SetPersistenceNum(PersistenceNum_value);
}

void RTSACli::SetGraunityNum(uint32_t GraunityNum_value)
{
	rtsa->SetGraunityNum(GraunityNum_value);
}

void RTSACli::SetDenominatorNum(float DenominatorNum_value)
{
	rtsa->SetDenominatorNum(DenominatorNum_value);
}

void RTSACli::SetOffset(float offset_value)
{
	rtsa->SetOffset(offset_value);
}

int RTSACli::GetAtt()
{
	return rtsa->GetAtt();
}

void RTSACli::SetZoomFactor(float ZoomFactor_value)
{
	rtsa->SetZoomFactor(ZoomFactor_value);
}

void RTSACli::SetValueScale(float ValueScale_value)
{
	rtsa->SetValueScale(ValueScale_value);
}

void RTSACli::SetOvlSel(uint32_t ovl_sel)
{
	rtsa->SetOvlSel(ovl_sel);
}

void RTSACli::SetFFTWindow(uint32_t fftwindow)
{
	rtsa->SetFFTWindow(fftwindow);
}

void RTSACli::SetTraceNum(uint32_t trace_num)
{
	rtsa->SetTraceNum(trace_num);
}

float RTSACli::GetDenominatorNum()
{
	return rtsa->GetDenominatorNum();
}

double RTSACli::GetSweepTimeBack()
{
	return rtsa->GetSweepTimeBack();
}

int RTSACli::GetCorrectValue() {
	return Global::CorrectValue;
}

double RTSACli::GetErrorValue() {
	return Global::ErrorValue;
}

double RTSACli::GetAmpAppend() {
	return Global::AmpAppend;
}

double RTSACli::GetBaseErrorValue() {
	return Global::BaseErrorValue;
}

double  RTSACli::GetIQCorrectValue()
{
	return rtsa->GetIQCorrectValue();
}

void RTSACli::SetTriggerSource(uint32_t source)
{
	rtsa->SetTriggerSource(source);
}

void RTSACli::SetTriggerPosttime(double time)
{
	rtsa->SetTriggerPosttime(time);
}

void RTSACli::SetTriggerThreshold_dBm(float threshold)
{
	rtsa->SetTriggerThreshold_dBm(threshold);
}

void RTSACli::SetTriggerThreshold_v(float threshold)
{
	rtsa->SetTriggerThreshold_v(threshold);
}

uint64_t RTSACli::GetTriggerDataAddress()
{
	return rtsa->GetTriggerDataAddress();
}

bool RTSACli::GetInterrupt() {
	return rtsa->GetInterrupt();
}
 
void RTSACli::ReadSpectrum()
{
	return rtsa->ReadSpectrum();
}

void RTSACli::ProcessSpectrum()
{
	return rtsa->ProcessSpectrum();
}

int RTSACli::GetSpectrumData(cli::array<Byte>^ managedBuffer)
{
	// 1. 安全检查
	if (managedBuffer == nullptr) {
		throw gcnew System::ArgumentNullException("managedBuffer");
	}

	int bufferSize = managedBuffer->Length;
	if (bufferSize == 0) {
		return 0;
	}

	// 2. 固定 (pin) 托管数组内存，防止垃圾回收 (GC) 移动它
	// System::Byte 在 C++/CLI 中直接映射为 unsigned char
	pin_ptr<System::Byte> pPinned = &managedBuffer[0];
	unsigned char* pNative = static_cast<unsigned char*>(pPinned);

	// 3. 调用纯 C++ 层的 RTSA::GetSpectrumData
	// 假设 spectrum 是 RTSA* 类型
	// 我们传递原始指针和缓冲区大小
	int bytesRead = rtsa->GetSpectrumData(pNative, bufferSize);

	// 4. 返回实际读取到的字节数
	// 当 pPinned 离开作用域时，数组会自动解锁 (Unpin)
	return bytesRead;
}

List<ManagedFreqAmpData>^ RTSACli::GetSpectrumDataSnapshot()
{
	//std::vector<RTSAControl::FreqAmpData> nativeSnapshot = rtsa->GetSpectrumSnapshot();
	auto nativeSnapshot = rtsa->GetSpectrumSnapshot();
	List<ManagedFreqAmpData>^ managedList = gcnew List<ManagedFreqAmpData>(nativeSnapshot.size());

	for (const auto& item : nativeSnapshot)
	{
		ManagedFreqAmpData managedItem;
		managedItem.Frequency = item.Freq;
		managedItem.Amplitude = item.Amp;
		managedList->Add(managedItem);
	}
	return managedList;
}

cli::array<Byte>^ RTSACli::ReadRawSpectrumOne()
{
	if (rtsa == nullptr) {
		throw gcnew InvalidOperationException("Native spectrum object is not initialized.");
	}
	try
	{
		// 1. 优化 GC 压力：仅在首次或大小改变时进行分配
		if (SpectrumBuffer == nullptr || SpectrumBuffer->Length < SpectrumRequiredSize)
		{
			SpectrumBuffer = gcnew cli::array<Byte>(SpectrumRequiredSize);
		}

		// 3. 优化拷贝：固定内存并获取原生指针
		pin_ptr<Byte> pinnedBuffer = &SpectrumBuffer[0];
		unsigned char* nativeBuffer = (unsigned char*)pinnedBuffer;

		// 4. 零拷贝调用 C++ API
		// C++ API 直接将数据写入 pinnedBuffer 指向的 C# 内存区域
		int bytesRead = rtsa->GetSpectrumData(nativeBuffer, SpectrumBuffer->Length);

		if (bytesRead <= 0) {
			return nullptr;
		}

		return SpectrumBuffer;
	}
	catch (const std::exception& ex)
	{
		// 捕获 C++ 异常并包装
		throw gcnew Exception(gcnew String(ex.what()));
	}
	catch (Exception^ ex)
	{
		throw ex;
	}
}
////拷贝的方式
//cli::array<Byte>^ RTSACli::ReadRawSpectrumOne()
//{
//	try
//	{
//		// 1. 调用原生函数获取数据
//		std::vector<unsigned char>* nativeVec = rtsa->ReadRawSpectrumOne();
//
//		if (nativeVec == nullptr || nativeVec->empty()) {
//			return nullptr;
//		}
//
//		int dataSize = static_cast<int>(nativeVec->size());
//
//		// 2. 检查并复用托管缓冲区
//		if (this->SpectrumBuffer == nullptr || this->SpectrumBuffer->Length != dataSize) {
//			this->SpectrumBuffer = gcnew cli::array<System::Byte>(dataSize);
//		}
//
//		// 3. 将数据从原生 vector 拷贝到托管数组
//		System::Runtime::InteropServices::Marshal::Copy(
//			IntPtr(nativeVec->data()),
//			this->SpectrumBuffer,
//			0,
//			dataSize
//		);
//
//		return this->SpectrumBuffer;
//	}
//	catch (const std::exception& ex)
//	{
//		throw gcnew System::Exception(gcnew System::String(ex.what()));
//	}
//}

//返回处理后的一帧数据
cli::array<double>^ RTSACli::ReadProcessedSpectrumOne()
{
	ReadRawSpectrumOne();

	// 1. 优化 GC 压力：仅在首次或大小改变时进行分配
	if (SpectrumAmpBuffer == nullptr || SpectrumAmpBuffer->Length < SpectrumSize)
	{
		SpectrumAmpBuffer = gcnew cli::array<double>(SpectrumSize);
	}
	pin_ptr<unsigned char> pRaw = &SpectrumBuffer[0];
	unsigned int* pUint = reinterpret_cast<unsigned int*>(pRaw);
	const double correction = -Global::BaseErrorValue - Global::ErrorValue + Global::AmpAppend + Global::CorrectValue;

	for (int i = 0; i < SpectrumSize; i++)
	{
		unsigned int v = pUint[i];
		if (v == 0) v = 1;
		SpectrumAmpBuffer[i] = 20.0 * Math::Log10((double)v) + correction;
	}
	return SpectrumAmpBuffer;  // 直接返回复用的结果数组
}
//拷贝的方式
//cli::array<double>^ RTSACli::ReadProcessedSpectrumOne()
//{
//	try
//	{
//		// 1. 调用原生逻辑
//		std::vector<double>* nativeData = rtsa->ReadProcessedSpectrumOne();
//
//		if (nativeData == nullptr || nativeData->empty()) return nullptr;
//
//		int count = static_cast<int>(nativeData->size());
//
//		// 2. 复用托管缓冲区
//		if (this->SpectrumAmpBuffer == nullptr || this->SpectrumAmpBuffer->Length != count) {
//			this->SpectrumAmpBuffer = gcnew cli::array<double>(count);
//		}
//
//		// 3. 将 double 数据从原生搬运到托管
//		System::Runtime::InteropServices::Marshal::Copy(
//			IntPtr(const_cast<double*>(nativeData->data())),
//			this->SpectrumAmpBuffer,
//			0,
//			count
//		);
//
//		return this->SpectrumAmpBuffer;
//	}
//	catch (const std::exception& ex) {
//		throw gcnew System::Exception(gcnew System::String(ex.what()));
//	}
//}

//int RTSACli::ReadProcessedSpectrumOne(cli::array<double>^% outputBuffer)
//{
//	if (rtsa == nullptr)
//		throw gcnew InvalidOperationException("Native spectrum object is not initialized.");
//
//	if (outputBuffer == nullptr || outputBuffer->Length < SpectrumSize)
//		throw gcnew ArgumentException("Output buffer is null or too small.");
//
//	try
//	{
//		// 1. 确保原始字节缓冲区足够大（只在首次或尺寸变化时分配）
//		if (SpectrumBuffer == nullptr || SpectrumBuffer->Length < SpectrumRequiredSize)
//		{
//			SpectrumBuffer = gcnew cli::array<Byte>(SpectrumRequiredSize);
//		}
//
//		// 2. 固定托管缓冲区，获取原生指针，实现零拷贝读取
//		pin_ptr<Byte> pinnedRaw = &SpectrumBuffer[0];
//		unsigned char* pRaw = (unsigned char*)pinnedRaw;
//
//		int bytesRead = rtsa->ReadProcessedSpectrumOne(pRaw, SpectrumBuffer->Length);
//		if (bytesRead <= 0)
//		{
//			return 0;   // 读取失败
//		}
//
//		// 3. 原地转换为 unsigned int*（数据是 32bit 整数）
//		unsigned int* pUint = reinterpret_cast<unsigned int*>(pRaw);
//
//		// 4. 计算一次修正系数（避免循环内重复计算）
//		const double correction = -Global::BaseErrorValue
//			- Global::ErrorValue
//			+ Global::AmpAppend
//			+ Global::CorrectValue;
//
//		// 5. 固定输出缓冲区（防止 GC 移动），直接写入用户提供的数组
//		pin_ptr<double> pinnedOut = &outputBuffer[0];
//		double* pOut = pinnedOut;
//
//		for (int i = 0; i < SpectrumSize; ++i)
//		{
//			unsigned int v = pUint[i];
//			if (v == 0) v = 1;                                   // 避免 log(0)
//			pOut[i] = 20.0 * Math::Log10((double)v) + correction;
//		}
//
//		return SpectrumSize;   // 成功写入的点数
//	}
//	catch (const std::exception& ex)
//	{
//		throw gcnew Exception(gcnew String(ex.what()));
//	}
//	catch (Exception^ ex)
//	{
//		throw ex;
//	}
//}

int RTSACli::ReadProcessedSpectrumOne(cli::array<double>^% outputBuffer)
{
	try
	{
		// 1. 准备一个临时的原生 vector
		std::vector<double> tempVec;

		// 2. 调用原生函数
		// 注意：nativeController 是 RTSA 的原生指针
		int count = rtsa->ReadProcessedSpectrumOne(tempVec);

		if (count <= 0) return 0;

		// 3. 检查并准备托管输出缓冲区
		if (outputBuffer == nullptr || outputBuffer->Length < count) {
			outputBuffer = gcnew cli::array<double>(count);
		}

		// 4. 将数据从 std::vector 拷贝到 cli::array
		System::Runtime::InteropServices::Marshal::Copy(
			System::IntPtr(tempVec.data()),
			outputBuffer,
			0,
			count
		);

		return count;
	}
	catch (const std::exception& ex) {
		throw gcnew System::Exception(gcnew System::String(ex.what()));
	}
}

//void RTSACli::ReadIQ()
//{
//	return rtsa->ReadIQ();
//}

List<Tuple<cli::array<short>^, cli::array<short>^>^>^ RTSACli::ProcessIQ()
{

	auto nativeResult = rtsa->ProcessIQ();

	List<Tuple<cli::array<short>^, cli::array<short>^>^>^ result = gcnew List<Tuple<cli::array<short>^, cli::array<short>^>^>(nativeResult.size());

	for (const auto& pair : nativeResult)
	{
		cli::array<short>^ iArray = gcnew cli::array<short>(pair.first.size());
		for (size_t i = 0; i < pair.first.size(); i++)
		{
			iArray[i] = static_cast<short>(pair.first[i]);
		}

		cli::array<short>^ qArray = gcnew cli::array<short>(pair.second.size());
		for (size_t i = 0; i < pair.second.size(); i++)
		{
			qArray[i] = static_cast<short>(pair.second[i]);
		}

		result->Add(gcnew Tuple<cli::array<short>^, cli::array<short>^>(iArray, qArray));
	}

	return result;
}

//cli::array<Byte>^ RTSACli::ReadIQOne()
//{
//	try
//	{// 1. 优化 GC 压力：仅在首次或大小改变时进行分配
//		if (IQBuffer == nullptr)
//		{
//			IQBuffer = gcnew cli::array<Byte>(IQRequiredSize);
//		}
//
//		// 3. 优化拷贝：固定内存并获取原生指针
//		pin_ptr<Byte> pinnedBuffer = &IQBuffer[0];
//		unsigned char* nativeBuffer = (unsigned char*)pinnedBuffer;
//
//		// 4. 零拷贝调用 C++ API
//		// C++ API 直接将数据写入 pinnedBuffer 指向的 C# 内存区域
//		int bytesRead = rtsa->read_iq_one(nativeBuffer, IQBuffer->Length);
//
//		if (bytesRead <= 0) {
//			return nullptr;
//		}
//		return IQBuffer;
//	}
//	catch (const std::exception& ex)
//	{
//		throw gcnew Exception(gcnew String(ex.what()));
//	}
//}
//拷贝的方式
cli::array<System::Byte>^ RTSACli::ReadIQOne()
{
	try
	{
		// 1. 调用原生 C++ 对象的方法，获取 vector 指针
		std::vector<unsigned char>* nativeVec = rtsa->ReadIQOne();

		// 2. 检查返回结果
		if (nativeVec == nullptr || nativeVec->empty()) {
			return nullptr;
		}

		// 3. 准备托管数组 (IQBuffer)
		// 保持复用逻辑，避免频繁触发 GC
		int dataSize = static_cast<int>(nativeVec->size());
		if (this->IQBuffer == nullptr || this->IQBuffer->Length != dataSize) {
			this->IQBuffer = gcnew cli::array<System::Byte>(dataSize);
		}

		// 4. 将数据从原生 vector 拷贝到托管数组
		// 使用 Marshal::Copy 是性能最高且最安全的方法
		System::Runtime::InteropServices::Marshal::Copy(
			IntPtr(nativeVec->data()), // 源：vector 的原始指针
			this->IQBuffer,            // 目的：托管数组
			0,                         // 偏移
			dataSize                   // 长度
		);

		return this->IQBuffer;
	}
	catch (const std::exception& ex)
	{
		// 将原生异常转译为 .NET 异常
		throw gcnew System::Exception(gcnew System::String(ex.what()));
	}
}

Tuple<cli::array<short>^, cli::array<short>^>^ RTSACli::ReadProcessedIQOne()
{
	try
	{
		// 1. 读取原始交织数据 
		cli::array<Byte>^ raw = ReadIQOne();
		if (raw == nullptr)
			return Tuple::Create<cli::array<short>^, cli::array<short>^>(nullptr, nullptr);
		if (raw->Length < IQRequiredSize)
			return nullptr;
		int validBytes = raw->Length - (raw->Length % 4);
		int sampleCount = validBytes / 4;

		// 2. 延迟分配分离后的 short 缓冲区
		if (IBuffer == nullptr || IBuffer->Length < sampleCount)
		{
			IBuffer = gcnew cli::array<short>(sampleCount);
			QBuffer = gcnew cli::array<short>(sampleCount);
		}

		// 3. 固定内存，直接原地拆分（零拷贝）
		pin_ptr<Byte>  pinRaw = &raw[0];
		pin_ptr<short> pinI = &IBuffer[0];
		pin_ptr<short> pinQ = &QBuffer[0];

		unsigned char* src = (unsigned char*)pinRaw;
		short* dstI = (short*)pinI;
		short* dstQ = (short*)pinQ;

		// 交织格式：I_L I_H Q_L Q_H 连续排列
		for (int i = 0; i < sampleCount; ++i)
		{
			int offset = i * 4;
			dstI[i] = (short)(src[offset] | (src[offset + 1] << 8));
			dstQ[i] = (short)(src[offset + 2] | (src[offset + 3] << 8));
		}

		return Tuple::Create(IBuffer, QBuffer);
	}
	catch (const std::exception& ex)
	{
		throw gcnew Exception(gcnew String(ex.what()));
	}
}
////拷贝的方式
//Tuple<cli::array<short>^, cli::array<short>^>^ RTSACli::ReadProcessedIQOne()
//{
//	try
//	{
//		 1. 调用原生 C++ 函数获取数据指针对
//		 假设 nativeController 是指向 RTSA 实例的指针
//		std::pair<std::vector<short>*, std::vector<short>*> iqResult = rtsa->ReadProcessedIQOne();
//
//		std::vector<short>* nativeI = iqResult.first;
//		std::vector<short>* nativeQ = iqResult.second;
//
//		 2. 检查结果是否有效
//		if (nativeI == nullptr || nativeQ == nullptr || nativeI->empty())
//		{
//			return Tuple::Create<cli::array<short>^, cli::array<short>^>(nullptr, nullptr);
//		}
//
//		int sampleCount = static_cast<int>(nativeI->size());
//
//		 3. 分配或复用托管缓冲区（IBuffer 和 QBuffer 为类成员变量）
//		if (this->IBuffer == nullptr || this->IBuffer->Length != sampleCount)
//		{
//			this->IBuffer = gcnew cli::array<short>(sampleCount);
//			this->QBuffer = gcnew cli::array<short>(sampleCount);
//		}
//
//		 4. 将数据从原生 Vector 拷贝到托管 Array
//		 指向第一个元素的地址进行拷贝
//		System::Runtime::InteropServices::Marshal::Copy(
//			IntPtr(nativeI->data()),
//			this->IBuffer,
//			0,
//			sampleCount
//		);
//
//		System::Runtime::InteropServices::Marshal::Copy(
//			IntPtr(nativeQ->data()),
//			this->QBuffer,
//			0,
//			sampleCount
//		);
//
//		 5. 返回封装好的 Tuple
//		return Tuple::Create(this->IBuffer, this->QBuffer);
//	}
//	catch (const std::exception& ex)
//	{
//		 异常翻译：将 C++ 标准异常转为 .NET 异常
//		throw gcnew System::Exception(gcnew System::String(ex.what()));
//	}
//}

//int RTSACli::ReadProcessedIQOne(cli::array<short>^% iBuffer, cli::array<short>^% qBuffer)
//{
//	// 1. 基础检查
//	if (iBuffer == nullptr || qBuffer == nullptr) return 0;
//
//	int reqSamples = IQRequiredSize / 4;
//
//	if (iBuffer->Length < reqSamples || qBuffer->Length < reqSamples) return 0;
//
//	// 2. 确保内部 Native Buffer 准备好 (避免每次分配)
//	if (m_nativeRawBuffer == nullptr) {
//		m_nativeRawBuffer = new std::vector<unsigned char>(IQRequiredSize);
//	}
//
//	// 3. 调用底层 C API 读取数据到 Native 堆内存
//	// .data() 返回的是 unsigned char*
//	int bytesRead = rtsa->read_iq_one(m_nativeRawBuffer->data(), IQRequiredSize);
//
//	// 如果读取失败或数据量不对
//	if (bytesRead <= 0) return 0;
//
//	// 4. 极速拆分 (Zero-Copy form Native to Managed)
//	// 获取 C# 数组的指针
//	pin_ptr<short> pI = &iBuffer[0];
//	pin_ptr<short> pQ = &qBuffer[0];
//
//	// 将 Native 的 byte* 强转为 short*，利用 CPU 一次读2字节的特性
//	// 注意：这里假设系统是 Little Endian (Windows 默认都是)，这比位运算快得多
//	short* pSrc = reinterpret_cast<short*>(m_nativeRawBuffer->data());
//	short* pDstI = pI;
//	short* pDstQ = pQ;
//
//	// 循环展开或直接遍历
//	for (int i = 0; i < reqSamples; i++)
//	{
//		// 原理：源数据是 I(2byte) Q(2byte) I(2byte) Q(2byte)...
//		// pSrc[0] 是 I, pSrc[1] 是 Q
//		*pDstI++ = *pSrc++; // 复制 I，源指针+1，目标指针+1
//		*pDstQ++ = *pSrc++; // 复制 Q，源指针+1，目标指针+1
//	}
//
//	return reqSamples;
//}

int RTSACli::ReadProcessedIQOne(cli::array<short>^% iBuffer, cli::array<short>^% qBuffer)
{
	// 1. 确保托管数组已初始化且大小足够
	int reqSamples = 1024 * 2400 / 4;
	if (iBuffer == nullptr || iBuffer->Length < reqSamples) iBuffer = gcnew cli::array<short>(reqSamples);
	if (qBuffer == nullptr || qBuffer->Length < reqSamples) qBuffer = gcnew cli::array<short>(reqSamples);

	try
	{
		// 2. 固定 (Pin) 托管数组，获取原生指针
		pin_ptr<short> pinI = &iBuffer[0];
		pin_ptr<short> pinQ = &qBuffer[0];

		// 3. 调用原生重载函数 (零拷贝直接写入托管内存)
		return rtsa->ReadProcessedIQOne(
			static_cast<short*>(pinI),
			static_cast<short*>(pinQ)
		);
	}
	catch (const std::exception& ex)
	{
		throw gcnew System::Exception(gcnew System::String(ex.what()));
	}
}

//void RTSACli::ReadPersistence()
//{
//	return rtsa->ReadPersistence();
//}
//
//void RTSACli::ProcessPersistence()
//{
//	return rtsa->ProcessPersistence();
//}

cli::array<Byte>^ RTSACli::ReadPersistenceOne()
{
	try
	{
		// 1. 优化 GC 压力：仅在首次或大小改变时进行分配
		if (PersistenceBuffer == nullptr)
		{
			PersistenceBuffer = gcnew cli::array<Byte>(PersistenceRequiredSize);
		}

		// 3. 优化拷贝：固定内存并获取原生指针
		pin_ptr<Byte> pinnedBuffer = &PersistenceBuffer[0];
		unsigned char* nativeBuffer = (unsigned char*)pinnedBuffer;

		// 4. 零拷贝调用 C++ API
		// C++ API 直接将数据写入 pinnedBuffer 指向的 C# 内存区域
		int bytesRead = rtsa->read_persistence_one(nativeBuffer, PersistenceBuffer->Length);

		if (bytesRead <= 0) {
			return nullptr;
		}
		return PersistenceBuffer;
	}
	catch (const std::exception& ex)
	{
		throw gcnew Exception(gcnew String(ex.what()));
	}
}

 //int RTSACli::ReadPersistenceOne(cli::array<float>^% rawData)
 //{
	// try
	// {
	//	 if (rawData == nullptr || rawData->Length == 0)
	//		 return -1;

	//	 int requiredByteSize = rawData->Length * sizeof(float);

	//	 // 1. 分配或复用 byte[] 缓冲区（只在大小改变时分配）
	//	 if (PersistenceBuffer == nullptr ||
	//		 PersistenceBuffer->Length != requiredByteSize)
	//	 {
	//		 PersistenceBuffer = gcnew cli::array<Byte>(requiredByteSize);
	//	 }

	//	 // 2. pin byte[]
	//	 pin_ptr<Byte> pinnedBuffer = &PersistenceBuffer[0];
	//	 unsigned char* nativeBuffer = pinnedBuffer;

	//	 // 3. 调用 C++ API（零拷贝写入 C# byte[]）
	//	 int bytesRead = rtsa->read_persistence_one(
	//		 nativeBuffer,
	//		 requiredByteSize
	//	 );

	//	 if (bytesRead <= 0)
	//		 return -1;

	//	 // 4. Byte → float（避免 BlockCopy，直接 reinterpret_cast 更快）
	//	 pin_ptr<float> pinnedFloat = &rawData[0];
	//	 float* dst = pinnedFloat;
	//	 float* src = reinterpret_cast<float*>(nativeBuffer);

	//	 int floatCount = requiredByteSize / sizeof(float);

	//	 // 高性能 memcpy（比 Buffer.BlockCopy 还快）
	//	 memcpy(dst, src, requiredByteSize);

	//	 return floatCount;
	// }
	// catch (const std::exception& ex)
	// {
	//	 throw gcnew Exception(gcnew String(ex.what()));
	// }
 //}

 int RTSACli::ReadPersistenceOne(cli::array<float>^% rawData)
 {
	 // 基础检查
	 if (rawData == nullptr || rawData->Length == 0)
		 return -1;

	 try
	 {
		 // 1. 固定托管 float 数组内存，获取原生指针
		 pin_ptr<float> pPinned = &rawData[0];
		 float* pNative = static_cast<float*>(pPinned);

		 // 2. 调用原生函数 (数据直接写入 C# 数组地址)
		 // 移除了 PersistenceBuffer 中转和 memcpy，性能达到理论极限
		 return rtsa->ReadPersistenceOne(pNative, rawData->Length);
	 }
	 catch (const std::exception& ex)
	 {
		 throw gcnew System::Exception(gcnew System::String(ex.what()));
	 }
 }

 List<float>^ RTSACli::GetFloatData()
 {
	 std::vector<float> nativeFloats = rtsa->GetFloatData();

	 List<float>^ managedList = gcnew List<float>(nativeFloats.size());

	 for (float f : nativeFloats)
	 {
		 managedList->Add(f);
	 }

	 return managedList;
 }

 void RTSAControlCLI::RTSACli::ReadRawTriggerData(UInt64 dataPosition, UInt64 oneDataByteNum, cli::array<Byte>^% outputBuffer)
 { 
	 // 使用内部 std::vector 调用原生实现（保持原生逻辑和零拷贝 DMA 不变）
	 std::vector<uint8_t> nativeBuffer;
	 rtsa->ReadRawTriggerData(dataPosition, oneDataByteNum, nativeBuffer);

	 System::UInt64 actualSize = nativeBuffer.size();

	 // 如果调用方传入的数组为空或大小不匹配，重新分配
	 if (outputBuffer == nullptr || outputBuffer->Length != (int)actualSize)
	 {
		 outputBuffer = gcnew cli::array<Byte>((int)actualSize);
	 }

	 // 高效拷贝（仅此一次拷贝，DMA 读取仍是零拷贝）
	 if (actualSize > 0)
	 {
		 Marshal::Copy(IntPtr((void*)nativeBuffer.data()), outputBuffer, 0, (int)actualSize);
	 }
 }

 void RTSACli::ReadProcessedTriggerData(UInt64 dataPosition, UInt64 oneDataByteNum, cli::array<Int16>^% outI, cli::array<Int16>^% outQ)
 {
	 // 使用内部 std::vector 调用原生实现（保持原生解交织逻辑不变）
	 std::vector<int16_t> nativeI;
	 std::vector<int16_t> nativeQ;
	 rtsa->ReadProcessedTriggerData(dataPosition, oneDataByteNum, nativeI, nativeQ);

	 size_t sampleCount = nativeI.size();  // 应该等于 nativeQ.size() 且 = oneDataByteNum / 4

	 // 调整托管数组大小
	 if (outI == nullptr || outI->Length != (int)sampleCount)
	 {
		 outI = gcnew  cli::array<Int16>((int)sampleCount);
	 }
	 if (outQ == nullptr || outQ->Length != (int)sampleCount)
	 {
		 outQ = gcnew  cli::array<Int16>((int)sampleCount);
	 }

	 // 拷贝到托管数组
	 if (sampleCount > 0)
	 {
		 Marshal::Copy(IntPtr((void*)nativeI.data()), outI, 0, (int)sampleCount);
		 Marshal::Copy(IntPtr((void*)nativeQ.data()), outQ, 0, (int)sampleCount);
	 } 
 }

 void RTSACli::GetDmaAddrData(UInt64 address, UInt32 dataLength, UInt64% baseAddress, cli::array<System::Byte>^% outputBuffer)
 {
	 // 1. 安全检查
	 if (outputBuffer == nullptr) {
		 throw gcnew System::ArgumentNullException("outputBuffer");
	 }
	 if (dataLength == 0) return;

	 // 2. 准备原生变量
	 uint64_t nativeBaseAddr = 0;

	 // 3. 创建临时 std::vector (因为 C++ 接口要求这个类型)
	 // 注意：这会产生一次内存分配开销
	 std::vector<uint8_t> nativeVec;

	 // 如果你的 C++ 内部逻辑依赖 outputBuffer 的初始大小，请先 resize
	 // nativeVec.resize(dataLength); 

	 // 4. 调用原生方法
	 // 此时 nativeVec 会在 C++ 内部被填充数据
	 rtsa->GetDmaAddrData(
		 static_cast<uint64_t>(address),
		 static_cast<uint32_t>(dataLength),
		 nativeBaseAddr,
		 nativeVec
	 );

	 // 5. 将结果从 std::vector 拷贝回 cli::array
	 // 这是第二次内存拷贝，大数据量下会影响性能
	 int actualSize = static_cast<int>(nativeVec.size());
	 int targetSize = Math::Min(actualSize, outputBuffer->Length);

	 if (targetSize > 0) {
		 // 使用 Marshal::Copy 进行高速内存拷贝
		 System::Runtime::InteropServices::Marshal::Copy(
			 IntPtr(nativeVec.data()),
			 outputBuffer,
			 0,
			 targetSize
		 );
	 }

	 // 6. 写回更新后的地址
	 baseAddress = nativeBaseAddr;
 }
 void RTSAControlCLI::RTSACli::GetDmaData(UInt64 address, UInt32 dataLength, cli::array<System::Byte>^% outputBuffer)
 {
	 // 1. 安全检查
	 if (outputBuffer == nullptr) {
		 throw gcnew System::ArgumentNullException("outputBuffer");
	 }
	 if (dataLength == 0) return;

	 // 2. 准备原生变量
	 uint64_t nativeBaseAddr = 0;

	 // 3. 创建临时 std::vector (因为 C++ 接口要求这个类型)
	 // 注意：这会产生一次内存分配开销
	 std::vector<uint8_t> nativeVec;

	 // 如果你的 C++ 内部逻辑依赖 outputBuffer 的初始大小，请先 resize
	 // nativeVec.resize(dataLength); 

	 // 4. 调用原生方法
	 // 此时 nativeVec 会在 C++ 内部被填充数据
	 rtsa->GetDmaData(
		 static_cast<uint64_t>(address),
		 static_cast<uint32_t>(dataLength), 
		 nativeVec
	 );

	 // 5. 将结果从 std::vector 拷贝回 cli::array
	 // 这是第二次内存拷贝，大数据量下会影响性能
	 int actualSize = static_cast<int>(nativeVec.size());
	 int targetSize = Math::Min(actualSize, outputBuffer->Length);

	 if (targetSize > 0) {
		 // 使用 Marshal::Copy 进行高速内存拷贝
		 System::Runtime::InteropServices::Marshal::Copy(
			 IntPtr(nativeVec.data()),
			 outputBuffer,
			 0,
			 targetSize
		 );
	 }

 }
 void RTSACli::CloseDevice()
 {
	 rtsa->CloseDevice();
 }
 
 
