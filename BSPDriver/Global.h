#ifndef GLOBAL_H  
#define GLOBAL_H  

#include <stdint.h>
#include <string>
#include <Windows.h>
#include <mutex>
#include <queue>
#include <map>

using namespace std;

#define create_async_read_file(name)  CreateFileA(name, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS,FILE_FLAG_NO_BUFFERING | FILE_FLAG_OVERLAPPED, NULL);
#define create_async_write_file(name) CreateFileA(name,  GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_FLAG_NO_BUFFERING | FILE_FLAG_OVERLAPPED, NULL);
#define create_async_write_file_slow(name) CreateFileA(name,  GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS,  FILE_FLAG_OVERLAPPED, NULL);
#define create_sync_read_file(name)  CreateFileA(name, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS,FILE_FLAG_NO_BUFFERING , NULL);
#define create_sync_read_file_slow(name)  CreateFileA(name, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS,NULL , NULL);
#define create_sync_write_file(name) CreateFileA(name,  GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_FLAG_NO_BUFFERING  , NULL);
#define create_sync_write_file_slow(name) CreateFileA(name,  GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, NULL  , NULL);
#define create_fpga_read(name) CreateFileA(name, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, NULL, NULL);
#define create_fpga_write(name) CreateFileA(name, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, NULL, NULL);
#define REFERENCE_CLOCK_REG      0x000B0000
#define FPGA_TEMP_REG            0x000F0440
#define FPGA_VOLTAGE_REG         0x000F0444
#define DDR_STATUS_REG           0x000F0004
#define SYSTEM_STATUS_REG        0x000F0004
#define DMA_MM2S_REG             0x00001040
#define CLOCK_BASE_REG           0x000B0200
#define CLOCK_BASE_PAGE_REG      0x000B0201
#define INTR_CLR                 0x000FFFFF

namespace Global
{
	template<typename T>
	class ThreadSafeQueue {
	public:
		explicit ThreadSafeQueue(size_t max_size = 10000)
			: max_size_(max_size) {
		}

		// Push ���ݣ���������������������ȴ���ֱ���пռ����
		void Push(const T& item) {
			std::lock_guard<std::mutex> lock(mutex_);
			if (queue_.size() >= max_size_) {
				return; // �������ˣ�ֱ�Ӷ���������
			}
			queue_.push(item);
			cond_.notify_one();
		}

		// Pop ���ݣ��������Ϊ����δֹͣ����ȴ�
		bool Pop(T& item) {
			std::unique_lock<std::mutex> lock(mutex_);
			// �ȴ�ֱ�����зǿ� �� ֹͣ
			//cond_.wait(lock, [this]() { return !queue_.empty() || stop_; });
			//if (stop_ && queue_.empty()) return false; // ֹͣ���Ҷ���Ϊ�գ�����ʧ��

			if (!queue_.empty()) {
				item = queue_.front();
				queue_.pop();
				cond_not_full_.notify_one();  // ֪ͨ�������ڵȴ� Push ���߳�
				return true;
			}
			return false;
		}

		// ֹͣ���У��������еȴ����߳�
		void Stop() {
			std::lock_guard<std::mutex> lock(mutex_);
			stop_ = true;
			cond_.notify_all();      // �������еȴ� Pop ���߳�
			cond_not_full_.notify_all(); // �������еȴ� Push ���߳�
		}

		// �ж϶����Ƿ�Ϊ��
		bool Empty() const {
			std::lock_guard<std::mutex> lock(mutex_);
			return queue_.empty();
		}

		// ��ѡ����ȡ��ǰ���д�С
		size_t Size() const {
			std::lock_guard<std::mutex> lock(mutex_);
			return queue_.size();
		}

		void Clear() {
			std::lock_guard<std::mutex> lock(mutex_);
			// ��ն���?
			while (!queue_.empty()) {
				queue_.pop();
			}
			// ���ѿ������ڵȴ� Push ���̣߳���Ϊ�����п�λ�ˣ���
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
			// 1. ���ݵ�ǰʱ�䣬����Ӧ���²�����������
			auto now = std::chrono::steady_clock::now();
			std::chrono::duration<double> elapsed = now - last_time_;
			last_time_ = now;

			tokens_ += elapsed.count() * rate_; // ÿ�����?rate_ ������
			if (tokens_ > capacity_) {
				tokens_ = capacity_; // ���Ʋ�����Ͱ����
			}

			// 2. ��������Ƿ���?
			if (tokens_ >= tokens) {
				tokens_ -= tokens;
				return true; // �ɹ����ѣ�����ȴ�?
			}
			else {
				// ������Ҫ�ȴ���ʱ��
				double deficit = tokens - tokens_;
				tokens_ = 0.0;
				// ע�⣺�����ͷ������ٵȴ������������ʱ����?
				lock.unlock();
				// ��ȷ�ȴ������ʱ��?
				std::this_thread::sleep_for(std::chrono::duration<double>(deficit / rate_));
				return false; // ���ѳɹ��������˵ȴ�
			}
		}

	private:
		double rate_; // ���Ʋ������� (bytes per second)
		double capacity_; // Ͱ������ (bytes)
		double tokens_; // ��ǰ��������
		std::chrono::time_point<std::chrono::steady_clock> last_time_; // �ϴθ���ʱ��
		std::mutex mutex_;
	};

	//������ز���?
	struct Nvme_io_argus {
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

	//PDW ��ز���?
	struct PdwParameter
	{
		uint32_t RBW;			    //	�ֱ��ʴ���
		uint64_t Centerfrquency;	//	����Ƶ��
		uint32_t DecimateFactor;	//	��ȡ
		double RefLevel;            //	�ο���ƽ
		bool IsPreAMPLifier;        //	�Ƿ��ڴ���ģʽ
		bool IsTrigger;             //	�Ƿ��ڴ���ģʽ
		float DenominatorNum;       //	FFT����
		int RFChannelType;          //  ͨ������ 
		double ATT;					//	˥��
	
		double IQCorrectValue;		//	IQ�Ĺ��ʲ���
		double Span; 
	};
	extern PdwParameter PdwParameters;

	struct RefLevelResults {
		int Att;
		int FFTGainOffset;
	};

	
	enum class WorkMode
	{
		SWEEP, STREAM, VSA, MULTI
	};
	struct DDC
	{
		uint32_t decimation;
		uint32_t nco_mode;
		uint64_t carrier_freq_hz;
		uint64_t po;
		bool gain_db;
	};

	/*struct Compensation {*/
	extern double AmpAppend;			//  Span��RBW�Ĺ��ʲ���
	extern uint32_t FFTGainOffset;  //���汶��
	extern double ErrorValue;			//  CF��ز�����CF�仯ʱ��ȡ  
	extern double BaseErrorValue;		//  �����������ӱ����?
	extern int CorrectValue;      //����Reflevel����Ĳ���?
	extern int IQPowerBaseError;
	extern double IQFFTBaseError;
	extern std::map<double, double> FreqErrorValue; //Ƶ��-����ֵ�?
	extern std::map<double, double> FreqPreampErrorValue;
	extern std::map<double, double> FreqATTErrorValue;
	extern double SweepBaseErrorValue;
	extern std::map<double, double> RbwErrDIC;
	extern double RefLevel;
	struct dac_parameter {
		uint64_t rate;
		uint32_t clkin;
		uint32_t link_mode;
		uint32_t subclass;
		uint32_t main_interpolation;
		uint32_t channel_interpolation;
		uint32_t jesd_mode;
		uint32_t logic_lane;
		uint32_t physical_lane;
		uint32_t syncoutb_type;
		uint32_t sysref_coupling;
	};

	struct adc_parameter {
		uint64_t sampling_frequency_hz;
		uint32_t input_div;
		bool powerdown_pin_en;
		uint32_t powerdown_mode;
		bool duty_cycle_stabilizer_en;
		uint8_t current_scale;
		bool analog_input_mode;
		bool ext_vref_en;
		uint32_t buff_curr_n;
		uint32_t buff_curr_p;
		uint8_t fc_ch;
		uint32_t ddc_cnt;
		bool ddc_output_format_real_en;
		bool ddc_input_format_real_en;
		uint32_t test_mode_ch0;
		uint32_t test_mode_ch1;
		uint32_t sysref_lmfc_offset;
		bool sysref_edge_sel;
		bool sysref_clk_edge_sel;
		uint32_t sysref_neg_window_skew;
		uint32_t sysref_pos_window_skew;
		uint32_t sysref_mode;
		uint32_t sysref_count;
		uint32_t jesd_subclass;
		std::vector<DDC> ddc;

	};

	extern uint32_t SweepSpectrumPointCount;
	extern uint32_t SweepSpectrumPointCountSet;
	extern uint32_t SubChannelGain;
}

















#endif // GLOBAL_H
