#include <windows.h>
#include <cmath>
#include <memory>
#include "Logic_Api.h"
#include "Device_MEM32.h"
#include "liquid.h"

#define M_PI  3.1415926535897931
#define M_E 2.7182818284590451

using namespace LOGIC;

double LOGICApi::FS = 0;
double LOGICApi::fft_bw = 0;

void LOGICApi::CalcParameter()
{
    fft_hb_sel = 0;
    //fft_hb_decim = 1;
    fft_bw = FS / (double)fft_hb_decim;
    fft_bin = fft_bw / (double)fft_len;
    switch (FFTwindow)
    {
    case FFTWindow::Blackman:
        win_k = 1.913;
        break;
    case FFTWindow::Flattop:
        win_k = 3.751;
        break;
    case FFTWindow::Gaussian:
        win_k = 1.4;
        break;
    case FFTWindow::Hanning:
        win_k = 1.451;
        break;
    case FFTWindow::Hamming:
        win_k = 1.309;
        break;
    case FFTWindow::Rectangle:
        win_k = 0.886;
        break;
    case FFTWindow::Kaiser:
        win_k = 1.4;
        break;
    }
    fft_rbw_len = (uint32_t)std::round(fft_bw / RBW * win_k);
    if (fft_rbw_len % 2 == 1)
        fft_rbw_len += 1;
    if (fft_rbw_len <= 1024)
    {
        fft_sel = 0;
        fft_len = 1024;
        det_num = 0;
    }
    else if (fft_rbw_len <= 4096)
    {
        fft_sel = 1;
        fft_len = 4096;
        det_num = 1;
    }
    else if (fft_rbw_len <= 16384)
    {
        fft_sel = 2;
        fft_len = 16384;
        det_num = 4;
    }
    else
    {
        fft_sel = 2;
        fft_len = 16384;
        det_num = 4;
    }
    trace_num = (uint32_t)std::round(FS * SweepTime / fft_len / fft_hb_decim);
    if (trace_num < 80)
        trace_num = 80;
    SweepTimeBack = (double)fft_len * (double)fft_hb_decim * (double)trace_num / FS;
    trace_shr = static_cast<uint32_t>(std::floor(std::log2(static_cast<double>(trace_num))));
    det_shr = fft_sel * 2;

    double fftfactor = 0;
    for (uint32_t i = 0; i < fft_rbw_len / 2; i++)
    {
        switch (FFTwindow)
        {
        case FFTWindow::Blackman:
            //fftfactor[i] = 0.35875 - 0.48829 * Math.Cos(2 * Math.PI * i / (fft_rbw_len - 1)) + 0.14128 * Math.Cos(4 * Math.PI * i / (fft_rbw_len - 1)) - 0.01168 * Math.Cos(6 * Math.PI * i / (fft_rbw_len - 1));
            fftfactor = liquid_blackmanharris(i, fft_rbw_len);
            break;
        case FFTWindow::Flattop:
            fftfactor = 0.21557895 - 0.41663158 * std::cos(2.0 * M_PI * i / (fft_rbw_len - 1)) +
                0.277263158 * std::cos(4 * M_PI * i / (fft_rbw_len - 1)) -
                0.083578947 * std::cos(6 * M_PI * i / (fft_rbw_len - 1)) +
                0.006947368 * std::cos(8 * M_PI * i / (fft_rbw_len - 1));
            //fftfactor[i] = FFTWin.flattop(i, fft_rbw_len);
            break;
        case FFTWindow::Rectangle:
            fftfactor = 1.0;
            break;
        case FFTWindow::Gaussian:

            fftfactor = std::pow(M_E, -std::pow(i, 2) / (2.0 * std::pow((fft_rbw_len - 1) / 5.0, 2)));//alpha 2.5
            break;
        case FFTWindow::Hanning:
            //blackman = 0.5 * (1 - Math.Cos(2 * Math.PI * i / (fft_rbw_len - 1)));
            fftfactor = liquid_hann(i, fft_rbw_len);
            break;
        case FFTWindow::Hamming:
            fftfactor = liquid_hamming(i, fft_rbw_len);
            break;
        case FFTWindow::Kaiser:
            fftfactor = liquid_kaiser(i, fft_rbw_len, 5.0);
            break;
        }
        //blackman = 0.42 - 0.5 * Math.Cos(2 * Math.PI * i / (fft_rbw_len - 1)) + 0.08 * Math.Cos(4 * Math.PI * i / (fft_rbw_len - 1));
        if (FFTwindow != FFTWindow::Gaussian)
        {
            blackman_win[i] = (uint32_t)std::round(fftfactor * (std::pow(2, win_lsb) - 1));
            blackman_win[fft_rbw_len - 1 - i] = blackman_win[i];
        }
        else
        {
            blackman_win[fft_rbw_len / 2 + i] = (uint32_t)std::round(fftfactor * (std::pow(2, win_lsb) - 1));
            blackman_win[fft_rbw_len / 2 - i] = blackman_win[fft_rbw_len / 2 + i];
        }
    }

    for (uint32_t i = fft_rbw_len; i < fft_len; i++)
    {
        blackman_win[i] = 0;
    }
    if (ValueScale == 0) ValueScale = 1;
    DenominatorNum = (float)PersistenceNum * (float)GraunityNum / (float)fft_len * 4 / (float)fft_hb_decim / ValueScale;
}

void LOGICApi::ConfigSweepParameter()
{
    uint32_t BaseAddress = 0x10010000;
    auto pcie_mem = Device::Device_MEM32::getInstance();
    pcie_mem->SendData(BaseAddress, 0);
    pcie_mem->SendData(BaseAddress + 1, (uint32_t)((fft_len / N) * std::pow(2, 16) + (fft_len / N)));
    pcie_mem->SendData(BaseAddress + 2, fft_hb_decim);
    pcie_mem->SendData(BaseAddress + 3, fft_sort * 32 + fft_sel * 4 + ovl_sel);
    //PCIE_MEM32.Instance.SendData(BaseAddress + 4, 0);
    pcie_mem->SendData(BaseAddress + 5, trace_num);
    pcie_mem->SendData(BaseAddress + 6, (uint32_t)(det_num * std::pow(2, 21) + det_shr * std::pow(2, 16) + trace_len * std::pow(2, 7) + trace_shr * 4 + DetectorType));
    pcie_mem->SendData(BaseAddress + 7, (uint32_t)GraunityNum);
    pcie_mem->SendData(BaseAddress + 8, (uint32_t)PersistenceNum);
    pcie_mem->SendData(BaseAddress + 9, 0);//DenominatorNum  0:Log 1:Num
    pcie_mem->SendData(BaseAddress + 10, ZoomFactor);
    pcie_mem->SendData(BaseAddress + 11, Offset);
    pcie_mem->SendData(BaseAddress + 20, 0x000003E8);
    pcie_mem->SendData(BaseAddress + 21, 0x800001F4);//1F4
    pcie_mem->SendData(BaseAddress + 21, 0x000001F4);
    uint32_t* data = new uint32_t[fft_len / 2];
    for (uint32_t i = 0; i < fft_len / 2; i++)
    {
        data[i] = (uint32_t)(blackman_win[i * 2 + 1] * std::pow(2, 16) + blackman_win[i * 2]);
    }
    pcie_mem->SendData(unsigned int(0x10018000), data, fft_len / 2);

    pcie_mem->SendData(BaseAddress, 3);
}
 

void LOGICApi::Config()
{
    CalcParameter();
    ConfigSweepParameter();
}

void LOGICApi::set_FS(double fs)
{
	FS = fs;
}
void LOGICApi::set_FFT_BW(double bw)
{
	fft_bw = bw;
}

void LOGICApi::set_RBW(uint32_t rbw)
{
	RBW = rbw;
}

void LOGICApi::set_DetectorType(uint32_t detector_type)
{
	DetectorType = detector_type;
}


void LOGICApi::set_fft_sort(uint32_t fft_sort_value)
{
    fft_sort = fft_sort_value;
}

void LOGICApi::set_fft_len(uint32_t fft_len_value)
{
    fft_len = fft_len_value;
}

void LOGICApi::set_fft_hb_decim(uint32_t fft_hb_decim_value)
{
    fft_hb_decim = fft_hb_decim_value;
}

void LOGICApi::set_SweepTime(double SweepTime_value)
{
    SweepTime = SweepTime_value;
}

void LOGICApi::set_PersistenceNum(uint32_t PersistenceNum_value)
{
    PersistenceNum = PersistenceNum_value;
}

void LOGICApi::set_GraunityNum(uint32_t GraunityNum_value)
{
    GraunityNum = GraunityNum_value;
}

void LOGICApi::set_DenominatorNum(float DenominatorNum_value)
{
    DenominatorNum = DenominatorNum_value;
}

void LOGICApi::set_offset(float offset_value)
{
    Offset = offset_value;
}

void LOGICApi::set_ZoomFactor(float ZoomFactor_value)
{
    ZoomFactor = ZoomFactor_value;
}

void LOGICApi::set_ValueScale(float ValueScale_value)
{
    ValueScale = ValueScale_value;
}

void LOGICApi::set_Ovl_sel(uint32_t Ovl_sel_value)
{
	ovl_sel = Ovl_sel_value;
}

void LOGICApi::set_FFTWindow(enum FFTWindow fftwindow)
{
    FFTwindow = fftwindow;
}

void LOGICApi::set_Trace_num(uint32_t trace_num_value)
{
	trace_num = trace_num_value;
}

uint32_t LOGICApi::Get_FFT_Sort()
{
	return fft_sort;
}

float LOGICApi::Get_DenominatorNum()
{
	return DenominatorNum;
}

double LOGICApi::Get_SweepTimeBack()
{
	return SweepTimeBack;
}