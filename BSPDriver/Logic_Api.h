#pragma once

#ifndef __LOGICAPI_H__
#define __LOGICAPI_H__


namespace LOGIC
{
	union uint32_chararray
	{
		char c[4];
		int i;
	};
	enum FFTWindow
	{
		Blackman,
		Flattop,
		Gaussian,
		Rectangle,
		Hanning,
		Hamming,
		Kaiser
	};


	class LOGICApi
	{
	private:
		uint32_t N = 4;
		uint32_t fft_sel = 1;
		uint32_t fft_hb_sel = 0;
		uint32_t fft_rbw_len = 16384;
		double fft_bin;
		double win_k = 1.913;
		uint32_t trace_shr;
		uint32_t det_num;
		uint32_t trace_len = 256;
		uint32_t det_shr;
		uint64_t BaseAddress=0x10010000;
		double SweepTimeBack;
		uint16_t blackman_win[16384];
		static double FS ;
		uint32_t fft_sort = 1;
		static double fft_bw ;
		uint32_t fft_len = 16384;
		uint32_t fft_hb_decim = 1;
		uint32_t ovl_sel = 1;
		uint32_t win_lsb = 8;
		uint32_t trace_num = 80;
		uint32_t RBW;
		uint32_t DetectorType=0;
		double SweepTime = 80E-6;
		uint32_t PersistenceNum=10;
		uint32_t GraunityNum=15359992;
		float DenominatorNum = 3 * 4 * 512 * 256 / 1024.0f;
		float Offset;
		float ZoomFactor=1;
		float ValueScale=1;
		FFTWindow FFTwindow= FFTWindow::Blackman; 

	public: 
		void Config();
		void CalcParameter();
		void ConfigSweepParameter();
		static void set_FS(double fs);
		static void set_FFT_BW(double bw);
		void set_RBW(uint32_t rbw);
		void set_DetectorType(uint32_t detector_type);
		void set_fft_sort(uint32_t fft_sort_value);
		void set_fft_len(uint32_t fft_len_value);
		void set_fft_hb_decim(uint32_t fft_hb_decim_value);
		void set_SweepTime(double SweepTime_value);
		void set_PersistenceNum(uint32_t PersistenceNum_value);
		void set_GraunityNum(uint32_t GraunityNum_value);
		void set_DenominatorNum(float DenominatorNum_value);
		void set_offset(float offset_value);
		void set_ZoomFactor(float ZoomFactor_value);
		void set_ValueScale(float ValueScale_value);
		void set_FFTWindow(enum FFTWindow fftwindow);
		void set_Trace_num(uint32_t trace_num_value);
		void set_Ovl_sel(uint32_t Ovl_sel_value);
		uint32_t Get_FFT_Sort();
		float Get_DenominatorNum();
		double Get_SweepTimeBack();
	};

}
#endif
/* !__LOGICAPI_H__ */
