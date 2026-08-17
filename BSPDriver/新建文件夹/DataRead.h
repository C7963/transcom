#pragma once
#include <stdint.h>
#include <queue>
#include <thread>
#include <array>
#include <mutex>
#include <vector>
#include <condition_variable>
#include "Device_MEM32.h"
#include "Device_Data.h"


namespace RTSAControl
{
	template<typename T>
	class ThreadSafeQueue {
	public:
		explicit ThreadSafeQueue(size_t max_size = 10)
			: max_size_(max_size) {}

		// Push 数据，如果队列已满，则阻塞等待，直到有空间可用
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
			std::unique_lock<std::mutex> lock(mutex_);
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


	private:
		mutable std::mutex mutex_;
		std::queue<T> queue_;
		std::condition_variable cond_;
		std::condition_variable cond_not_full_;
		bool stop_ = false;
		size_t max_size_;
	};

	class FreqAmpData
	{
	public:
		double Freq;
		double Amp;

		FreqAmpData(double freq, double amp)
			: Freq(freq), Amp(amp) {}
	};

	class SpectrumData
	{
	public:
		static SpectrumData* instance;
		static SpectrumData* getInstance() {
			if (instance == nullptr)
			{
				instance = new SpectrumData();
			}
			return instance;
		};
		SpectrumData();
		~SpectrumData();
		void read_spectrum();
		std::vector<unsigned char> read_spectrum_one();
		int read_spectrum_one(unsigned char* external_buffer, int buffer_size);
		void process_spectrum();
		void Start_Spectrum();
		void Stop_Spectrum();
		std::vector<FreqAmpData> GetSpectrumDataSnapshot();
		std::mutex mtx;
		std::vector<FreqAmpData> sptrmPointList;
		std::vector<unsigned char> buffer_spectrum;
	private:
		bool state_spectrum;
		Device::Device_Data_RTSA* device;
		int max_len = 10;
		ThreadSafeQueue<std::vector<unsigned char>> data_queue{ 10 };
	};

	class IQData
	{
	public:
		static IQData* instance;
		static IQData* getInstance() {
			if (instance == nullptr)
			{
				instance = new IQData();
			}
			return instance;
		};
		IQData();
		~IQData();
		void read_iq();
		int read_iq_one(unsigned char* external_buffer, int buffer_size);
		std::vector<unsigned char> read_iq_one();
		std::vector<std::pair<std::vector<int16_t>, std::vector<int16_t>>> process_iq();
		void Start_IQ();
		void Stop_IQ();
		std::vector<std::pair<std::vector<int16_t>, std::vector<int16_t>>> getiq();
		std::mutex mtx;
		std::vector<std::pair<std::vector<int16_t>, std::vector<int16_t>>> all_data;
		std::vector<unsigned char> buffer_iq;
	private:

		bool state_iq;
		Device::Device_Data_RTSA* device;
		int max_len = 10;
		ThreadSafeQueue<std::vector<unsigned char>> data_queue{ 10 };
	};

	class PersistenceData
	{
	public:
		static PersistenceData* instance;
		static PersistenceData* getInstance() {
			if (instance == nullptr)
			{
				instance = new PersistenceData();
			}
			return instance;
		};
		PersistenceData();
		~PersistenceData();
		void read_persistence();
		int read_persistence_one(unsigned char* external_buffer, int buffer_size);
		std::vector<unsigned char> read_persistence_one();
		void process_persistence();
		void Start_Persistence();
		void Stop_Persistence();
		std::vector<float> getfloatdata();
		void HandlePersistence(const std::vector<uint8_t>& PersistenceData);
		std::vector<float> floatData;
		std::mutex mtx;
		std::vector<unsigned char> buffer_persistence;
	private:
		bool state_persistence;
		Device::Device_Data_RTSA* device;
		int max_len = 10;
		ThreadSafeQueue<std::vector<unsigned char>> data_queue{ 10 };
	};

	class DMAData
	{
	public:
		static DMAData* instance;
		static DMAData* getInstance() {
			if (instance == nullptr)
			{
				instance = new DMAData();
			}
			return instance;
		};
		DMAData();
		~DMAData();
		uint64_t CalculateBasePosition(unsigned long long dataPosition);
		void GetRawTriggerData(unsigned long long DataPosition, uint64_t OneDataByteNum, std::vector<uint8_t>& outData);

		void GetProcessedTriggerData(unsigned long long dataPosition, uint64_t oneDataByteNum, std::vector<int16_t>& outI, std::vector<int16_t>& outQ);
 
		std::mutex mtx;
		int TrigOffsetAmount;

	private: 
		std::vector<uint8_t> ReadBackData(uint32_t add, uint32_t size);
		// 成员变量，用于复用内存，避免频繁申请
		std::vector<uint8_t> m_internalBuffer;
		uint64_t IQStartAddress = 0x80000000;
		uint64_t IQEndAddress = 0x1FFEF7C00;
		double SetTriggerOffset = 0.02;
		bool state_dma;
		Device::Device_Data_RTSA* device;
		Device::Device_MEM32* pcie_mem;
		int max_len = 10;
		std::pair<std::vector<uint8_t>, std::vector<uint8_t>> SaveTriggerIQ;
		std::vector<uint8_t> SaveIQData;
		std::vector<uint8_t> Dmadata;
	};
}


