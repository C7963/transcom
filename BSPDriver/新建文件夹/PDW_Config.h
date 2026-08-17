#pragma once
#include <stdint.h>
#include <queue>
#include <thread>
#include <array>
#include <mutex>
#include <condition_variable>
#include "Device_MEM32.h"
#include "Device_Data.h"
#include <sstream>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <Windows.h>
#include <atomic>



#define REG_PDW_ENABLE     0x00080002
#define REG_THRESHOLD      0x00080007
#define REG_DDC_CONFIG     0x0008000B
#define TDFR               0x00081008
#define TDFD               0x00081010
#define TLR                0x00081014
#define SRR                0x00081028
#define REG_DDC_DECIM      0x0008001B
#define REG_DDC_DECIM2     0x000800FF

#define create_async_read_file(name)  CreateFileA(name, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS,FILE_FLAG_NO_BUFFERING | FILE_FLAG_OVERLAPPED, NULL);
#define create_async_write_file(name) CreateFileA(name,  GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_FLAG_NO_BUFFERING | FILE_FLAG_OVERLAPPED, NULL);
#define create_async_write_file_slow(name) CreateFileA(name,  GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS,  FILE_FLAG_OVERLAPPED, NULL);
#define create_sync_read_file(name)  CreateFileA(name, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS,FILE_FLAG_NO_BUFFERING , NULL);
#define create_sync_read_file_slow(name)  CreateFileA(name, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS,NULL , NULL);
#define create_sync_write_file(name) CreateFileA(name,  GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_FLAG_NO_BUFFERING  , NULL);
#define create_sync_write_file_slow(name) CreateFileA(name,  GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, NULL  , NULL);
#define create_fpga_read(name) CreateFileA(name, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, NULL, NULL);
#define create_fpga_write(name) CreateFileA(name, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, NULL, NULL);

namespace PDWCONFIG
{
	template<typename T>
	class ThreadSafeQueue {
	public:
		explicit ThreadSafeQueue(size_t max_size = 10000)
			: max_size_(max_size) {
		}

		// Push 数据，如果队列已满，则阻塞等待，直到有空间可用
	/*	void Push(T&& item) {
			std::unique_lock<std::mutex> lock(mutex_);
			cond_.wait(lock, [this]() { return queue_.size() < max_size_; });
			queue_.push(std::move(item));
			cond_.notify_one();
		}*/
		void Push(const T& item) {
			std::lock_guard<std::mutex> lock(mutex_);
			if (queue_.size() >= max_size_) {
				return; // 队列满了，直接丢弃新数据
			}
			queue_.push(item);
			//cond_.notify_one();
		}

		// Pop 数据，如果队列为空且未停止，则等待
		bool Pop(T& item) {
			std::lock_guard<std::mutex> lock(mutex_);
			// 等待直到队列非空 或 停止
			//cond_.wait(lock, [this]() { return !queue_.empty() || stop_; });
			//if (stop_ && queue_.empty()) return false; // 停止了且队列为空，返回失败
			
			if (!queue_.empty()) {
				item = queue_.front();
				queue_.pop();
				//cond_not_full_.notify_one();  // 通知可能正在等待 Push 的线程
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
			//cond_not_full_.notify_all();
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


	struct pulse_frame
	{
		uint32_t frame_header;
		uint32_t pulse_width;
		uint32_t phase;
		uint64_t amplitude;
		uint8_t channel_num;
		uint64_t time;
		char frame_tail[3];
	};

	enum class DataType {
		IQ,
		PDW,
		IQPDW
	};

	struct Nvme_io_argus {
		DataType type;
		string file_name;
		uint32_t span;
		uint64_t centerfreq;
		string file_path;
		float rate;
		bool flag;
	};

	enum Run_status
	{
		idle,
		onAcquisition,
		onPlayback,
		complete
	};

	class PDWCtrl
	{
	public:
		static PDWCtrl* getInstance() {
			if (instance == nullptr)
			{
				instance = new PDWCtrl();
			}
			return instance;
		};
		static PDWCtrl* instance;
		PDWCtrl();
		~PDWCtrl();
		void SetChannelFreq(int64_t* Nfreq);
		void PDW_Enable(bool enable);
		void PDW_Threshold(uint32_t channel_num, float* threshold);
		void PDW_Threshold(uint32_t channel_num, uint32_t threshold);
		void PDW_DDC_Config();
		void Set_BandWidth_Gain(uint32_t decim);
		void PDW_DDC_FIR_COEF(uint32_t* value, uint32_t length);
		void fir_config_axis_reset();
		void fir_config_txdata_fifo_reset();
		void Set_ChannelNum(uint32_t num);
		void SetBandWidthGain(uint32_t bandwidth, uint32_t gain);

	private:
	/*	double FS = 614.4e6;
		static constexpr uint32_t N = 4;
		static constexpr uint32_t M = 2;
		double F_IFs[N];
		double K[N];
		double D[N];
		double Pinc[N * M];
		double Poff[N * M];
		uint32_t Ftw[N * M * 2];*/

		double FS = 614.4e6;
		uint32_t N = 4;
		uint32_t M = 2;
		std::vector<double> F_IFs;
		std::vector<double> K;
		std::vector<double> D;
		std::vector<double> Pinc;
		std::vector<double> Poff;
		std::vector<uint32_t> Ftw;

	};

	class PDWStream
	{
	public:
		static PDWStream* getInstance() {
			if (instance == nullptr)
			{
				instance = new PDWStream();
			}
			return instance;
		};
		PDWStream();
		~PDWStream();
		void ReadData();
		void process();
		void Start();
		void Stop();
		bool enqueue(const std::array<unsigned char, 32>& data_frame);
		bool enqueue(const std::vector<unsigned char>& data_frame);
		int acquisition_pulse(Nvme_io_argus argus);
		void stop_acquisition_pulse();
		void start_acquisition_pulse(Nvme_io_argus argus);
		int playback_pulse(Nvme_io_argus argus);
		void start_playback_pulse(Nvme_io_argus argus);
		int stop_playback_pulse();
		std::queue<std::array<unsigned char, 32>> queue;
		std::queue<std::vector<unsigned char>> queue1;
		static PDWStream* instance;
		std::mutex mtx;
		std::queue<std::vector<unsigned char>> all_data;
		float speed;
		ThreadSafeQueue<std::vector<unsigned char>> data_queue{ 200 };
		bool state_read;

	private:
		double desired_bandwidth_mbps = 1.0; // 期望带宽：1 MB/s
		double desired_bandwidth_bps = desired_bandwidth_mbps * 1024 * 1024; // 转换为 bytes per second
		uint32_t MAX_ALL_DATA_SIZE = 100;
		uint32_t data_len = 16 * 512; //需要是4的整数倍,512的整数倍
		Run_status status = idle;
		int max_len = 32 * 1024;
		int max_len_iq = 2000 * 1024;
	
		bool state_process;
		std::thread* t;
		std::condition_variable cv;
		PDWCtrl* pdwctrl;
		Device::Device_Data_Multi* device;

		int sync_nvme_write(HANDLE nvme_file, unsigned char* data, uint32_t len);
		int sync_nvme_read(HANDLE nvme_file, char* data, uint32_t len);

	};
}


