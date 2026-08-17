#include "PWD_Config.h"
#include <Windows.h>
#include <chrono>
#include <iomanip>
#include <sstream>
#include "HLJS386_HL.h"

using namespace PWDCONFIG;
using namespace DeviceDataConfig;


PWDCONFIG::PWDStream* PWDCONFIG::PWDStream::instance = nullptr;
PWDCONFIG::PWDCtrl* PWDCONFIG::PWDCtrl::instance = nullptr;

PWDCtrl::PWDCtrl()
{
	F_IFs.resize(N);
	K.resize(N);
	D.resize(N);
	Pinc.resize(N * M);
	Poff.resize(N * M);
	Ftw.resize(N * M * 2);
}

PWDCtrl::~PWDCtrl()
{

}

void PWDCtrl::Set_ChannelNum(uint32_t num)
{
	Device::Device_MEM32* pcie_mem = Device::Device_MEM32::getInstance();
	N = num;
	F_IFs.resize(N);
	K.resize(N);
	D.resize(N);
	Pinc.resize(N * M);
	Poff.resize(N * M);
	Ftw.resize(N * M * 2);
	for (int i = 0; i < N; i++)
	{
		pcie_mem->SendData(REG_PDW_ENABLE + i + 1, i);
	}
}

void PWDCtrl::Caculate_F_IFs(int64_t* Nfreq)
{
	for (int i = 0; i < N; i++)
	{
		/*if (Nfreq[i] <= multiParameter2.Freq)
		{
			F_IFs[i] = (multiParameter2.Freq - Nfreq[i]);
		}
		else if (multiParameter2.Parameters[i].Nfreq > multiParameter2.Freq)
		{
			F_IFs[i] = 614.4e6 - (multiParameter2.Parameters[i].Nfreq - multiParameter2.Freq);
		}*/
		F_IFs[i] = Nfreq[i];
	}
}

void PWDCtrl::PWD_Enable(bool enable)
{
	Device::Device_MEM32* pcie_mem = Device::Device_MEM32::getInstance();
	pcie_mem->SendData(REG_PDW_ENABLE, enable);
}

void PWDCtrl::PWD_Threshold(uint32_t channel_num, float* threshold)
{
	Device::Device_MEM32* pcie_mem = Device::Device_MEM32::getInstance();
	pcie_mem->SendData(0x00080000, 0);
	//pcie_mem->SendData(0x000C0004, 0);//ADC前置开关
	for (int i = 0; i < N; i++)
	{
		float val = (threshold[i] + 100 - global_ifatt - global_rfatt) * 1.0; //4通道106 为子通道PDW基础功率误差值 16通道101
		val /= 20;
		uint32_t result = std::pow(10, val);
		pcie_mem->SendData(REG_PDW_ENABLE + N + 1 + i, result);
	}
}

void PWDCtrl::PWD_Threshold(uint32_t channel_num, uint32_t threshold)
{
	Device::Device_MEM32* pcie_mem = Device::Device_MEM32::getInstance();
	pcie_mem->SendData(REG_THRESHOLD + channel_num, threshold);
}

void PWDCtrl::PWD_DDC_Config()
{
	Device::Device_MEM32* pcie_mem = Device::Device_MEM32::getInstance();
	for (uint32_t j = 0; j < N; j++)
	{
		K[j] = FS / F_IFs[j];
		D[j] = M / K[j];
		for (uint32_t i = 0; i < M; i++)
		{
			if (D[j] < 1)
				Pinc[j * M + i] = std::round(D[j] * std::pow(2, 32));
			else
				Pinc[j * M + i] = std::round((D[j] - 1) * std::pow(2, 32));
			if (i < K[j])
				Poff[j * M + i] = std::round(i / K[j] * std::pow(2, 32));
			else
				Poff[j * M + i] = std::round((i / K[j] - 1) * std::pow(2, 32));
		}
	}
	for (uint32_t i = 0; i < N * M; i++)
	{
		Ftw[i * 2] = (uint32_t)Pinc[i];
		Ftw[i * 2 + 1] = (uint32_t)Poff[i];
	}
	pcie_mem->SendData(REG_PDW_ENABLE + 2 * N + 1, Ftw.data(), 2 * N * M);
}

void PWDCtrl::PWD_DDC_FIR_COEF(uint32_t* value, uint32_t length)
{
	Device::Device_MEM32* pcie_mem = Device::Device_MEM32::getInstance();
	for (int i = 0; i < length; i++)
	{
		pcie_mem->SendData(TDFD, (unsigned int)value[i]);

	}
	// This register is used to store packet length values (the number of bytes in the packet)
	// corresponding to valid packets ready for transmit.
	pcie_mem->SendData(TLR, (unsigned int)length * 4);
}

void PWDCtrl::PWD_DDC_Decim(uint32_t decim)
{
	Device::Device_MEM32* pcie_mem = Device::Device_MEM32::getInstance();
	pcie_mem->SendData(REG_PDW_ENABLE + 2 * N + 1 + 2 * M * N, decim);
	//pcie_mem->SendData(REG_DDC_DECIM2, decim);
}

void PWDCtrl::fir_config_axis_reset()
{
	Device::Device_MEM32* pcie_mem = Device::Device_MEM32::getInstance();
	pcie_mem->SendData(SRR, (unsigned int)0xa5);
}
void PWDCtrl::fir_config_txdata_fifo_reset()
{
	Device::Device_MEM32* pcie_mem = Device::Device_MEM32::getInstance();
	pcie_mem->SendData(TDFR, (unsigned int)0xa5);
}

PWDStream::PWDStream()
{
	pwdctrl = new PWDCtrl();
	device = new DeviceDataAPI();
}

PWDStream::~PWDStream()
{
	delete pwdctrl;
	delete device;
}

void PWDStream::ReadData() {
	while (state_read) {
		std::vector<unsigned char> data(32);
		device->get_pulse_data(data.data(), 32);
		data_queue.Push(std::move(data));
		//bool res = enqueue(data);
	}
}


void PWDStream::process() {
	while (state_process) {
		std::vector<unsigned char> data;
		if (data_queue.Pop(data)) {
			if (all_data.size() < MAX_ALL_DATA_SIZE) {
				std::lock_guard<std::mutex> lock(mtx);
				all_data.push(data);
			}
			else
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
		}
		else
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
	}
}

void PWDStream::Start() {
	if (state_read) return;
	state_read = true;
	state_process = true;
	pwdctrl->PWD_Enable(1);
	std::thread* read = new std::thread(&PWDStream::ReadData, this);
	read->detach();
	/*std::thread* t = new std::thread(&PWDStream::process, this);
	t->detach();*/
}

void PWDStream::Stop() {
	if (!state_read) return;
	pwdctrl->PWD_Enable(0);
	state_read = false;
}

bool PWDStream::enqueue(const std::array<unsigned char, 32>& data_frame)
{
	std::lock_guard<std::mutex> lock(mtx);
	if (queue.size() >= max_len) {
		queue.pop(); // 队列满时移除最旧的数据
	}
	queue.push(data_frame);
	return true;
}

bool PWDStream::enqueue(const std::vector<unsigned char>& data_frame)
{
	std::lock_guard<std::mutex> lock(mtx);
	if (queue1.size() >= max_len) {
		queue1.pop(); // 队列满时移除最旧的数据
	}
	queue1.push(data_frame);
	return true;
}

int PWDStream::sync_nvme_write(HANDLE nvme_file, unsigned char* data, uint32_t len)
{
	if (!WriteFile(nvme_file, data, len, (LPDWORD)&len, NULL)) {
		return -2;
	}
	return 1;
}

int PWDStream::sync_nvme_read(HANDLE nvme_file, char* data, uint32_t len) {
	DWORD bytesRead = 0;
	if (ReadFile(nvme_file, data, len, &bytesRead, NULL)) {
		//return -2;
		if (bytesRead < len) {
			return -1;
		}
	}
	return 1;
}


//int PWDStream::acquisition_pulse(Nvme_io_argus argus)
//{
//	int t = 0;
//	status = onAcquisition;
//	std::string final_file_name;
//	auto now = std::chrono::system_clock::now();
//	auto now_time_t = std::chrono::system_clock::to_time_t(now);
//	auto now_tm = *std::localtime(&now_time_t);
//
//	if (!argus.file_path.empty() && !argus.file_name.empty())
//	{
//		std::ostringstream time_stream;
//		time_stream << std::put_time(&now_tm, "%H%M%S");
//		std::string time_str = time_stream.str();
//		final_file_name = argus.file_path + "\\" + argus.file_name + "@" + "PDW" + "@" + time_str + ".dat";
//	}
//	else
//	{
//		std::ostringstream time_stream;
//		time_stream << std::put_time(&now_tm, "%Y%m%d_%H%M%S");  // 如：202508029_143022
//		std::string current_time_str = time_stream.str();
//		std::ostringstream span_stream;
//		span_stream << argus.span;
//		std::string span_str = span_stream.str();
//		std::ostringstream cf_stream;
//		cf_stream << argus.centerfreq;
//		std::string cf_str = cf_stream.str();
//		// 拼接完整文件名，如：20240601_143022@1000@2400
//		final_file_name = argus.file_path + "\\" + current_time_str + "@" + "Span" + span_str + "@" + "CF" + cf_str + "@" + "PDW" + ".dat";
//	}
//
//	HANDLE file_pulse = create_sync_write_file_slow(final_file_name.c_str());
//	if (file_pulse == INVALID_HANDLE_VALUE) {
//		status = idle;
//		return -1; // 文件创建失败
//	}
//	//unsigned char* pulse_data = new unsigned char[data_len];
//
//	std::vector<unsigned char> raw_data;
//
//	const uint32_t channel_count = 1;
//	std::vector<size_t> channel_sizes(channel_count, 0);  
//	std::vector<unsigned char> buffer;
//
//	buffer.reserve(5120);
//
//	auto time2 = std::chrono::high_resolution_clock::now();
//	auto time1 = std::chrono::high_resolution_clock::now();
//	while (status == onAcquisition)
//	{
//		if (data_queue.Pop(raw_data))
//		{
//			uint32_t bytes_to_copy = static_cast<uint32_t>(raw_data.size());
//			auto& size = channel_sizes[0];
//			if (size + bytes_to_copy > buffer.capacity())
//			{
//				sync_nvme_write(file_pulse, buffer.data(), 5120);
//				size = 0;
//				buffer.clear();
//			}
//			if (++t % 500 == 0)
//			{
//				time2 = std::chrono::high_resolution_clock::now();
//				auto duration_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(time2 - time1).count();
//				if (duration_ns == 0) {
//					duration_ns = 1; // 最小1ns
//				}
//				speed = 1e9 * 500 * 32.0 / duration_ns;
//				time1 = std::chrono::high_resolution_clock::now();
//				t = 0;
//			}
//			memcpy(buffer.data() + size, raw_data.data(), 32);
//			size += 32;
//		}
//		else {
//			std::this_thread::sleep_for(std::chrono::milliseconds(1));
//		}
//	}
//	if (!buffer.empty())
//	{
//		sync_nvme_write(file_pulse, buffer.data(), buffer.size());
//		buffer.clear();
//	}
//	CloseHandle(file_pulse);
//	//delete[] pulse_data;
//	status = idle;
//	return 0;
//}

int PWDStream::acquisition_pulse(Nvme_io_argus argus)
{
	int t = 0;
	status = onAcquisition;
	std::string final_file_name;
	auto now = std::chrono::system_clock::now();
	auto now_time_t = std::chrono::system_clock::to_time_t(now);
	auto now_tm = *std::localtime(&now_time_t);

	if (!argus.file_path.empty() && !argus.file_name.empty())
	{
		std::ostringstream time_stream;
		time_stream << std::put_time(&now_tm, "%H%M%S");
		std::string time_str = time_stream.str();
		final_file_name = argus.file_path + "\\" + argus.file_name + "@" + time_str + "@" + "PDW" + ".dat";
	}
	else
	{
		std::ostringstream time_stream;
		time_stream << std::put_time(&now_tm, "%Y%m%d_%H%M%S");  // 如：202508029_143022
		std::string current_time_str = time_stream.str();
		std::ostringstream span_stream;
		span_stream << argus.span / 1e6;
		std::string span_str = span_stream.str();
		std::ostringstream cf_stream;
		cf_stream << argus.centerfreq;
		std::string cf_str = cf_stream.str();
		// 拼接完整文件名，如：20240601_143022@1000@2400
		final_file_name = argus.file_path + "\\" + current_time_str + "@" + "Span" + span_str + "MHz" + "@" + "CF" + cf_str + "@" + "PDW" + ".dat";
	}

	HANDLE file_pulse = create_sync_write_file_slow(final_file_name.c_str());
	if (file_pulse == INVALID_HANDLE_VALUE) {
		status = idle;
		return -1; // 文件创建失败
	}
	//unsigned char* pulse_data = new unsigned char[data_len];

	std::vector<unsigned char> raw_data;

	const uint32_t channel_count = 1;
	std::vector<size_t> channel_sizes(channel_count, 0);
	std::vector<unsigned char> buffer;

	buffer.reserve(512);
	std::vector<unsigned char> data(64);


	auto time2 = std::chrono::high_resolution_clock::now();
	auto time1 = std::chrono::high_resolution_clock::now();
	while (status == onAcquisition)
	{
		device->get_pulse_data(data.data(), 64);
		sync_nvme_write(file_pulse, data.data(), 64);

		if (++t % 50 == 0)
		{
			time2 = std::chrono::high_resolution_clock::now();
			auto duration_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(time2 - time1).count();
			if (duration_ns == 0) {
				duration_ns = 1; // 最小1ns
			}
			speed = 1e9 * 50 * 64.0 / duration_ns;
			time1 = std::chrono::high_resolution_clock::now();
			t = 0;
		}

	}
	CloseHandle(file_pulse);

	status = idle;
	return 0;
}


void PWDStream::stop_acquisition_pulse()
{
	status = idle;
	state_process = true;
	state_read = true;
	speed = 0;
	std::thread* read = new std::thread(&PWDStream::ReadData, this);
	read->detach();
	//std::thread* t = new std::thread(&PWDStream::process, this);
	//t->detach();
}

void PWDStream::start_acquisition_pulse(Nvme_io_argus argus)
{
	state_read = false;
	state_process = false;
	data_queue.Clear();
	status = onAcquisition;
	std::thread* t = new std::thread(&PWDStream::acquisition_pulse, this, argus);
	t->detach();
}

int PWDStream::playback_pulse(Nvme_io_argus argus)
{
	uint32_t delay = 0;
	HANDLE file_pulse;
	file_pulse = create_sync_read_file_slow(argus.file_name.c_str());
	char* pulse_data = new char[32];
	sync_nvme_read(file_pulse, pulse_data, 32);
	delay = (static_cast<uint8_t>(pulse_data[19]) << 24) |
		(static_cast<uint8_t>(pulse_data[18]) << 16) |
		(static_cast<uint8_t>(pulse_data[17]) << 8) |
		(static_cast<uint8_t>(pulse_data[16]));
	uint32_t us_delay = delay / 38.4e6 * 1000 * 1000;
	desired_bandwidth_bps = argus.rate * 1e6 / us_delay * 32;
	TokenBucket bucket(desired_bandwidth_bps, desired_bandwidth_bps);
	std::vector<unsigned char> data_buffer(32);

	while (status == onPlayback) {
		bucket.consume(32);
		if (sync_nvme_read(file_pulse, pulse_data, 32) == -1) {
			if (argus.flag) {
				CloseHandle(file_pulse);
				file_pulse = create_sync_read_file_slow(argus.file_name.c_str());
				continue;
			}
			else {
				break;
			}
		}
		std::memcpy(data_buffer.data(), pulse_data, 32);
		data_queue.Push(data_buffer);
	}
	delete[] pulse_data;
	CloseHandle(file_pulse);
	return 1;
}

void PWDStream::start_playback_pulse(Nvme_io_argus argus)
{
	status = onPlayback;
	state_read = false;
	//state_process = true;
	data_queue.Clear();
	all_data = std::queue<std::vector<unsigned char>>();
	std::thread* t = new std::thread(&PWDStream::playback_pulse, this, argus);
	t->detach();
}


int PWDStream::stop_playback_pulse()
{
	//std::lock_guard<std::mutex> lock(mtx);
	status = idle;
	data_queue.Clear();
	all_data = std::queue<std::vector<unsigned char>>();
	state_process = true;
	state_read = true;

	std::thread* read = new std::thread(&PWDStream::ReadData, this);
	read->detach();
	return 1;
}