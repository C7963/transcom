#pragma once
// PscanApi.h - BSPDriver DLL 导出接口（C链接）
#include <cstdint>
// 供 PscanTestClient 等外部程序动态加载使用

#ifdef BSPDRIVER_EXPORTS
#define BSP_API __declspec(dllexport)
#else
#define BSP_API __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif

// 初始化硬件并启动Pscan
// cf: 中心频率(Hz), span: 跨度(Hz), rbw: 分辨率带宽(Hz)
BSP_API int PscanApi_Init(double cf, double span, uint32_t rbw);

// 设置RBW（分辨率带宽）
// rbw: 分辨率带宽(Hz)
BSP_API int PscanApi_SetRBW(uint32_t rbw);

// 设置扫描参数
// centerFreq: 中心频率(Hz), span: 跨度(Hz), step: 步进(Hz), refLevel: 参考电平(dBm)
BSP_API int PscanApi_SetPara(double centerFreq, double span, double step, double refLevel);

// Apply the complete PSCAN parameter set in one hardware transaction.
BSP_API int PscanApi_SetConfig(double centerFreq, double span, uint32_t rbw, double step, double refLevel);

// 启动连续采集
BSP_API int PscanApi_Start(void);

// 停止采集
BSP_API int PscanApi_Stop(void);

// 单次扫描
BSP_API int PscanApi_RunSingle(void);

// 设置运行模式: 0=停止, 1=单次, 2=连续
BSP_API int PscanApi_SetRunMode(int mode);

// 获取运行模式: 0=停止, 1=单次, 2=连续
BSP_API int PscanApi_GetRunMode(void);

// 获取频谱数据
// outFreqs: 输出频率数组, outAmps: 输出幅度数组, outSize: 输入/输出数组大小
// 返回: 0=失败, 1=成功
BSP_API int PscanApi_GetSpectrumData(double* outFreqs, double* outAmps, uint32_t* outSize);

// 单位转换: 将dBm转换为目标单位
// ampDbm: 输入幅度(dBm), targetUnit: 目标单位(0=dBm,1=dBmV,2=dBuV,3=V,4=W,5=A)
BSP_API double PscanApi_ConvertAmplitude(double ampDbm, int targetUnit);

#ifdef __cplusplus
}
#endif
