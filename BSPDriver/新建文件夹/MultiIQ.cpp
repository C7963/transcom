#include <Windows.h>
#include "MultiIQ.h"
#include <chrono>
#include <iomanip>
#include <sstream>

using namespace MULTIIQ;
using namespace Device;
using namespace PDWCONFIG;

constexpr size_t FRAME_SIZE = 512;  // 每帧大小（字节）
constexpr size_t FRAME_COUNT = 16;  // 帧数量

MULTIIQ::MIQStream* MULTIIQ::MIQStream::instance = nullptr;

MIQStream::MIQStream() : queue_count(16)
{
	device = Device_Data_Multi::getInstance();
    pcie_mem = Device::Device_MEM32::getInstance();
	channel_queues.resize(queue_count);
}

MIQStream::~MIQStream()
{

}

void MIQStream::set_queue_count(int count)
{
	std::lock_guard<std::mutex> lock(mtx);
	if (count > 0 && count <= 16) {
		queue_count = count;
		channel_count = count;
		channel_queues.resize(count);
	}
}

void MIQStream::clear_queues()
{
	//std::lock_guard<std::mutex> lock(mtx); 
	for (auto& queue : channel_queues) {
		queue = std::queue<std::vector<unsigned char>>();
	}
}


int MIQStream::sync_nvme_write(HANDLE nvme_file, unsigned char* data, uint32_t len)
{
	if (!WriteFile(nvme_file, data, len, (LPDWORD)&len, NULL)) {
		return -2;
	}
	return 1;
}

int MIQStream::sync_nvme_read(HANDLE nvme_file, char* data, uint32_t len) {
	DWORD bytesRead = 0;
	if (ReadFile(nvme_file, data, len, &bytesRead, NULL)) {
		//return -2;
		if (bytesRead < len) {
			return -1;
		}
	}
	return 1;
}

uint64_t MIQStream::duration_to_byte(double duration, uint32_t span)
{
	return 4.0 * span * duration;
}

void MIQStream::iq_switch(uint32_t flag)
{  
	std::lock_guard<std::mutex> lock(mtx); 
	pcie_mem->SendData(0x00080001, flag);				 //0-脉冲iq,1-原始iq
	clear_queues();
	//device->Device_CloseMultiDevice();
}

void MIQStream::ReadData()
{
	std::vector<unsigned char> raw_data(FRAME_SIZE * FRAME_COUNT);
	while (state_read_iq) {
		device->ReadChannelsData(raw_data.data(), FRAME_SIZE * FRAME_COUNT);
		data_queue.Push(raw_data);
	}
}

void MIQStream::process_iqiq()
{
	uint64_t channel_timestamps[16] = { 0 };
	bool first_frame = true;
	std::vector<std::vector<unsigned char>> channel_buffers(16);
	size_t data_start_pos = 0;

	while (state_process_iq) {
		std::vector<unsigned char> raw_data;
		if (!data_queue.Pop(raw_data)) {
			std::this_thread::sleep_for(std::chrono::milliseconds(2));
			continue;
		}
		for (uint64_t frame_idx = 0; frame_idx < FRAME_COUNT; ++frame_idx) {
			size_t frame_offset = data_start_pos + frame_idx * FRAME_SIZE;
			auto frame_start = raw_data.begin() + frame_offset;

			unsigned char channel_id = *(frame_start + 16);
			if (channel_id < 16) {
				uint64_t timestamp;
				std::memcpy(&timestamp, &(*(frame_start + 8)), sizeof(timestamp));
				if (!first_frame && timestamp != channel_timestamps[channel_id]) {

					if (!channel_buffers[channel_id].empty())
					{
						enqueue_iq(channel_id, channel_buffers[channel_id]);
						channel_buffers[channel_id].clear();
					}
				}
				channel_timestamps[channel_id] = timestamp;
				first_frame = false;

				std::vector<unsigned char> data(FRAME_SIZE - 64);
				std::copy(frame_start + 32, frame_start + FRAME_SIZE - 32, data.begin());


				channel_buffers[channel_id].insert(channel_buffers[channel_id].end(),
					data.begin(), data.end());
			}
			else {
				std::cout << "Channel_Id Error" << channel_id << endl;
			}
		}
	}
}


void MIQStream::process_playback_iq()
{
	uint64_t channel_timestamps[4] = { 0 };
	bool first_frame = true;
	std::vector<std::vector<unsigned char>> channel_buffers(4);
	size_t data_start_pos = 0;

	while (state_playback_process_iq) {
		std::vector<unsigned char> raw_data;
		if (!data_queue.Pop(raw_data)) {
			std::this_thread::sleep_for(std::chrono::milliseconds(2));
			continue;
		}
		for (uint64_t frame_idx = 0; frame_idx < 16 * 1280; ++frame_idx) {
			size_t frame_offset = data_start_pos + frame_idx * FRAME_SIZE;
			auto frame_start = raw_data.begin() + frame_offset;

			unsigned char channel_id = *(frame_start + 16);
			if (channel_id < 4) {
				uint64_t timestamp;
				std::memcpy(&timestamp, &(*(frame_start + 8)), sizeof(timestamp));
				if (!first_frame && timestamp != channel_timestamps[channel_id]) {

					if (!channel_buffers[channel_id].empty())
					{
						enqueue_iq(channel_id, channel_buffers[channel_id]);
						channel_buffers[channel_id].clear();
					}
				}
				channel_timestamps[channel_id] = timestamp;
				first_frame = false;

				std::vector<unsigned char> data(FRAME_SIZE - 64);
				std::copy(frame_start + 32, frame_start + FRAME_SIZE - 32, data.begin());


				channel_buffers[channel_id].insert(channel_buffers[channel_id].end(),
					data.begin(), data.end());
			}
			else {
				std::cout << "Channel_Id Error" << channel_id << endl;
			}
		}
	}
}

int MIQStream::acquisition_multiiq(PDWCONFIG::Nvme_io_argus argus)
{
	int t = 0;
	const uint32_t frame_size = frames_per_block * bytes_per_frame;
	std::string base_filename;
	auto now = std::chrono::system_clock::now();
	auto now_time_t = std::chrono::system_clock::to_time_t(now);
	auto now_tm = *std::localtime(&now_time_t);
	std::ostringstream span_stream;
	span_stream << argus.span / 1e6;
	std::string span_str = span_stream.str();

	if (!argus.file_path.empty() && !argus.file_name.empty())
	{
		std::ostringstream time_stream;
		time_stream << std::put_time(&now_tm, "%H%M%S");
		std::string time_str = time_stream.str();
		base_filename = argus.file_path + "\\" + argus.file_name + "@" + time_str + "@Span" + span_str + "MHz";
	}
	else
	{
		std::ostringstream time_stream;
		time_stream << std::put_time(&now_tm, "%Y%m%d_%H%M%S");  // 如：20240601_143022
		std::string current_time_str = time_stream.str();

		std::ostringstream cf_stream;
		cf_stream << argus.centerfreq;
		std::string cf_str = cf_stream.str();
		// 拼接完整文件名，如：20240601_143022@Span1000@CF2400
		base_filename = argus.file_path + "\\" + current_time_str + "@" + "Span" + span_str + "MHz" + "@" + "CF" + cf_str;
	}

	std::vector<HANDLE> channel_files(channel_count, INVALID_HANDLE_VALUE);
	for (uint32_t ch = 0; ch < channel_count; ++ch)
	{
		std::ostringstream file_stream;
		file_stream << base_filename << "@" << "channel" << ch + 1 << "@" << "IQ" << ".dat";  // 如 20240601_143022@Span1000@CF2400@channel0@IQ.dat
		std::string channel_filename = file_stream.str();

		channel_files[ch] = create_sync_write_file_slow(channel_filename.c_str());
		if (channel_files[ch] == INVALID_HANDLE_VALUE)
		{
			for (uint32_t i = 0; i < ch; ++i)
			{
				if (channel_files[i] != INVALID_HANDLE_VALUE)
					CloseHandle(channel_files[i]);
			}
			return -1;
		}
	}

	const size_t buffer_flush_threshold = 20 * 16 * 512;
	std::vector<size_t> channel_sizes(channel_count, 0);
	std::vector<std::vector<unsigned char>> channel_buffers(channel_count);

	// 预分配大缓冲区
	for (auto& buf : channel_buffers) {
		buf.reserve(buffer_flush_threshold);
	}
	//std::vector<unsigned char> raw_data(FRAME_SIZE * FRAME_COUNT);
	std::vector<unsigned char> raw_data;

	auto time2 = std::chrono::high_resolution_clock::now();
	auto time1 = std::chrono::high_resolution_clock::now();
	while (status == onAcquisition) {
		//std::vector<unsigned char> raw_data;
		if (!data_queue.Pop(raw_data)) {
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			continue;
		}
		//device->ReadChannelsData(raw_data.data(), FRAME_SIZE * FRAME_COUNT);
		for (uint32_t frame_idx = 0; frame_idx < FRAME_COUNT; ++frame_idx) {
			uint32_t frame_offset = frame_idx * bytes_per_frame;
			unsigned char* frame_ptr = raw_data.data() + frame_offset;
			uint8_t channel_id = frame_ptr[16];

			if (channel_id < channel_count) {
				auto& buffer = channel_buffers[channel_id];
				auto& size = channel_sizes[channel_id];

				// 检查是否需要刷新
				if (size + bytes_per_frame > buffer.capacity()) {
					DWORD bytes_written;
					WriteFile(channel_files[channel_id],
						buffer.data(),
						size,
						&bytes_written,
						NULL);
					size = 0;
				}

				// 追加数据
				memcpy(buffer.data() + size, frame_ptr, bytes_per_frame);
				size += bytes_per_frame;
			}
		}
		if (++t % 50000 == 0) {
			time2 = std::chrono::high_resolution_clock::now();
			speed = 1000.0 * 50000 * data_len / std::chrono::duration_cast<std::chrono::milliseconds>(time2 - time1).count();
			time1 = std::chrono::high_resolution_clock::now();
			t = 0;
		}
	}
	for (auto h : channel_files)
	{
		if (h != INVALID_HANDLE_VALUE)
			CloseHandle(h);
	}
	status = idle;
	return 0;
}


void MIQStream::Start_IQ()
{
	if (state_read_iq) return;
	state_read_iq = true;
	state_process_iq = true;
	std::thread* read = new std::thread(&MIQStream::ReadData, this);
	std::thread* t = new std::thread(&MIQStream::process_iqiq, this);
	read->detach();
	t->detach();
}

void MIQStream::Start_acquisition_IQ(PDWCONFIG::Nvme_io_argus argus)
{
	state_process_iq = false;
	//state_read_iq = false;
	data_queue.Clear();
	status = onAcquisition;
	std::thread* t = new std::thread(&MIQStream::acquisition_multiiq, this, argus);
	t->detach();
}

void MIQStream::Stop_acquisition_IQ()
{
	status = idle;
	state_process_iq = true;
	speed = 0;
	std::thread* t = new std::thread(&MIQStream::process_iqiq, this);
	t->detach();
}

void MIQStream::Stop_IQ()
{
	if (!state_read_iq) return;
	state_read_iq = false;
	state_process_iq = false;
	data_queue.Stop();
}

// 统一入队接口
bool MIQStream::enqueue_iq(int channel_id, const std::vector<unsigned char>& data_frame)
{
	try
	{
		std::lock_guard<std::mutex> lock(mtx);
		if (channel_id < 0 || channel_id >= queue_count) {
			return false;
		}

		auto& queue = channel_queues[channel_id];
		if (queue.size() >= max_len) {
			queue.pop();
		}
		queue.push(data_frame);
		return true;
	}
	catch (const std::exception& e)
	{
		std::cerr << "enqueue_iq error: " << e.what() << std::endl;
		return false;
	}
}

// 兼容旧接口的enqueue_iq
bool MIQStream::enqueue_iq(std::queue<std::vector<unsigned char>>& queue, const std::vector<unsigned char>& data_frame)
{
	try
	{
		std::lock_guard<std::mutex> lock(mtx);
		if (queue.size() >= max_len) {
			queue.pop();
		}
		queue.push(data_frame);
		return true;
	}
	catch (const std::exception& e)
	{
		std::cerr << "enqueue_iq error: " << e.what() << std::endl;
		return false;
	}
}

int MIQStream::playback_iq(PDWCONFIG::Nvme_io_argus argus)
{
	size_t span_pos = argus.file_name.find("@Span");
	size_t num_start = span_pos + 5;  // "@Span" 长度为5
	size_t num_end = num_start;
	bool has_dot = false;
	while (num_end < argus.file_name.size()) {
		char c = argus.file_name[num_end];
		if (isdigit(c)) {
			++num_end;
		}
		else if (c == '.' && !has_dot) {  // 允许一个小数点
			has_dot = true;
			++num_end;
		}
		else {
			break;  // 遇到非数字且非小数点，或者第二个小数点，就停止
		}
	}

	// 截取数字部分
	std::string number_str = argus.file_name.substr(num_start, num_end - num_start);
	double span = std::stod(number_str);

	desired_bandwidth_mbps = argus.rate * span * 4 * 8 / 7;
	//desired_bandwidth_mbps = argus.rate * argus.span * 4 * 8 / 7;
	desired_bandwidth_bps = desired_bandwidth_mbps * 1024 * 1024;
	TokenBucket bucket(desired_bandwidth_bps, desired_bandwidth_bps);
	HANDLE file_iq;
	std::vector<unsigned char> data_buffer(data_len);
	char* iq_data = new char[data_len];
	file_iq = create_sync_read_file_slow(argus.file_name.c_str());
	//bool flag = true;
	while (status == onPlayback) {
		bucket.consume(data_len);
		if (data_queue.Size() <= 1000) {
			if (sync_nvme_read(file_iq, iq_data, data_len) == -1) {
				if (argus.flag) {
					CloseHandle(file_iq);
					file_iq = create_sync_read_file_slow(argus.file_name.c_str());
					continue;
				}
				else {
					break;
				}
			};
			std::memcpy(data_buffer.data(), iq_data, data_len);
			data_queue.Push(data_buffer);

		}
		else {
			std::this_thread::sleep_for(std::chrono::nanoseconds(1));
		}
	}
	status = idle;
	delete[] iq_data;
	CloseHandle(file_iq);
	return 1;
}

void MIQStream::start_playback_iq(PDWCONFIG::Nvme_io_argus argus)
{
	status = onPlayback;
	state_read_iq = false;
	//state_process_iq = false;
	state_process_iq = true;
	data_queue.Clear();
	std::thread* t = new std::thread(&MIQStream::playback_iq, this, argus);
	t->detach();
}

int MIQStream::stop_playback_iq()
{
	status = idle;
	data_queue.Clear();
	clear_queues();
	state_read_iq = true;
	std::thread* read = new std::thread(&MIQStream::ReadData, this);
	read->detach();
	return 1;
}

