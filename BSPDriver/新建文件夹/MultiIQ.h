#pragma once
#include <stdint.h>
#include <queue>
#include <thread>
#include <array>
#include <mutex>
#include <condition_variable>
#include "Device_MEM32.h"
#include "Device_Data.h"
#include <string>
#include <vector>
#include <Windows.h>
#include <sstream>
#include <filesystem>
#include <fstream>
#include "liquid.h"
#include <atomic>
#include "PDW_Config.h"

#define create_async_read_file(name)  CreateFileA(name, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS,FILE_FLAG_NO_BUFFERING | FILE_FLAG_OVERLAPPED, NULL);
#define create_async_write_file(name) CreateFileA(name,  GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_FLAG_NO_BUFFERING | FILE_FLAG_OVERLAPPED, NULL);
#define create_async_write_file_slow(name) CreateFileA(name,  GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS,  FILE_FLAG_OVERLAPPED, NULL);
#define create_sync_read_file(name)  CreateFileA(name, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS,FILE_FLAG_NO_BUFFERING , NULL);
#define create_sync_read_file_slow(name)  CreateFileA(name, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS,NULL , NULL);
#define create_sync_write_file(name) CreateFileA(name,  GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_FLAG_NO_BUFFERING  , NULL);
#define create_sync_write_file_slow(name) CreateFileA(name,  GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, NULL  , NULL);
#define create_fpga_read(name) CreateFileA(name, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, NULL, NULL);
#define create_fpga_write(name) CreateFileA(name, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, NULL, NULL);

namespace MULTIIQ
{
	/*   struct Nvme_io_argus {
		   string file_name;
		   double duration;
		   uint32_t span;
		   uint64_t centerfreq;
		   string file_path;
	   };*/

	enum Run_status
	{
		idle,
		onAcquisition,
		onPlayback,
		complete
	};

	template<typename T>
	class ThreadSafeQueue {
	public:
		explicit ThreadSafeQueue(size_t max_size = 10000)
			: max_size_(max_size) {
		}

		// Push 数据，如果队列已满，则阻塞等待，直到有空间可用
		void Push(const T& item) {
			std::lock_guard<std::mutex> lock(mutex_);
			if (queue_.size() >= max_size_) {
				return; // 队列满了，直接丢弃新数据
			}
			queue_.push(item);
			cond_.notify_one();
		}

		// Pop 数据，如果队列为空且未停止，则等待
		bool Pop(T& item) {
			std::unique_lock<std::mutex> lock(mutex_);
			// 等待直到队列非空 或 停止
			//cond_.wait(lock, [this]() { return !queue_.empty() || stop_; });
			//if (stop_ && queue_.empty()) return false; // 停止了且队列为空，返回失败

			if (!queue_.empty()) {
				item = queue_.front();
				queue_.pop();
				cond_not_full_.notify_one();  // 通知可能正在等待 Push 的线程
				return true;
			}
			return false;
		}

		// 停止队列，唤醒所有等待的线程
		void Stop() {
			std::lock_guard<std::mutex> lock(mutex_);
			stop_ = true;
			cond_.notify_all();      // 唤醒所有等待 Pop 的线程
			cond_not_full_.notify_all(); // 唤醒所有等待 Push 的线程
		}

		// 判断队列是否为空
		bool Empty() const {
			std::lock_guard<std::mutex> lock(mutex_);
			return queue_.empty();
		}

		// 可选：获取当前队列大小
		size_t Size() const {
			std::lock_guard<std::mutex> lock(mutex_);
			return queue_.size();
		}

		void Clear() {
			std::lock_guard<std::mutex> lock(mutex_);
			// 清空队列
			while (!queue_.empty()) {
				queue_.pop();
			}
			// 唤醒可能正在等待 Push 的线程（因为现在有空位了！）
			cond_not_full_.notify_all();
		}

	private:
		mutable std::mutex mutex_;
		std::queue<T> queue_;
		std::condition_variable cond_;
		std::condition_variable cond_not_full_;
		bool stop_ = false;
		size_t max_size_;
	};

	class TokenBucket {
	public:
		TokenBucket(double rate, double burst = 0.0)
			: rate_(rate), capacity_(burst > 0.0 ? burst : rate), tokens_(capacity_), last_time_(std::chrono::steady_clock::now()) {
		}

		bool consume(double tokens) {
			std::unique_lock<std::mutex> lock(mutex_);
			// 1. 根据当前时间，计算应该新产生多少令牌
			auto now = std::chrono::steady_clock::now();
			std::chrono::duration<double> elapsed = now - last_time_;
			last_time_ = now;

			tokens_ += elapsed.count() * rate_; // 每秒产生 rate_ 个令牌
			if (tokens_ > capacity_) {
				tokens_ = capacity_; // 令牌不超过桶容量
			}

			// 2. 检查令牌是否足够
			if (tokens_ >= tokens) {
				tokens_ -= tokens;
				return true; // 成功消费，无需等待
			}
			else {
				// 计算需要等待的时间
				double deficit = tokens - tokens_;
				tokens_ = 0.0;
				// 注意：这里释放锁后再等待，避免持有锁时阻塞
				lock.unlock();
				// 精确等待所需的时间
				std::this_thread::sleep_for(std::chrono::duration<double>(deficit / rate_));
				return false; // 消费成功但经历了等待
			}
		}

	private:
		double rate_; // 令牌产生速率 (bytes per second)
		double capacity_; // 桶的容量 (bytes)
		double tokens_; // 当前令牌数量
		std::chrono::time_point<std::chrono::steady_clock> last_time_; // 上次更新时间
		std::mutex mutex_;
	};


	class MIQStream
	{
	public:
		static MIQStream* getInstance() {
			if (instance == nullptr)
			{
				instance = new MIQStream();
			}
			return instance;
		};
		static MIQStream* instance;
		MIQStream();
		~MIQStream();
		void iq_switch(uint32_t flag);
		void process_iqiq();
		void process_playback_iq();
		void ReadData();
		void Start_IQ();
		void Stop_IQ();
		bool enqueue_iq(std::queue<std::vector<unsigned char>>& queue, const std::vector<unsigned char>& data_frame);
		void Start_acquisition_IQ(PDWCONFIG::Nvme_io_argus argus);
		void Stop_acquisition_IQ();
		int acquisition_multiiq(PDWCONFIG::Nvme_io_argus argus);
		int playback_iq(PDWCONFIG::Nvme_io_argus argus);
		void start_playback_iq(PDWCONFIG::Nvme_io_argus argus);
		int stop_playback_iq();
		float speed;

		// 新增辅助函数
		void set_queue_count(int count);
		int get_queue_count() const { return queue_count; }
		// 清空所有队列（替代原来的clear_queue）
		void clear_queues();
		// 统一入队接口（新增）
		bool enqueue_iq(int channel_id, const std::vector<unsigned char>& data_frame);

		// 动态队列管理
		std::vector<std::queue<std::vector<unsigned char>>> channel_queues;
		std::mutex mtx;

	private:
		double desired_bandwidth_mbps = 1.0; // 期望带宽：1 MB/s
		double desired_bandwidth_bps = desired_bandwidth_mbps * 1024 * 1024; // 转换为 bytes per second
		Run_status status = idle;
		uint32_t data_len = 16 * 512; //需要是4的整数倍,512的整数倍
		uint32_t channel_count = 4;    //4个通道
		uint32_t bytes_per_frame = 512;  //512字节
		uint32_t frames_per_block = 16;   //16个帧
		int sync_nvme_write(HANDLE nvme_file, unsigned char* data, uint32_t len);
		int sync_nvme_read(HANDLE nvme_file, char* data, uint32_t len);
		uint64_t duration_to_byte(double duration, uint32_t span);

		bool state_process_iq;
		bool state_read_iq;
		bool state_playback_process_iq;
		std::thread* t;
		Device::Device_Data_Multi* device;
		Device::Device_MEM32* pcie_mem;
		int max_len = 10;
		ThreadSafeQueue<std::vector<unsigned char>> data_queue{ 10000 };

		int queue_count = 16;  // 默认队列数量

	};
}


