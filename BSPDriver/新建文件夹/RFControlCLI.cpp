#include "RFControlCLI.h"
#include <iostream>

using namespace RFControlCLI;
using namespace RFCONTROL;

RFControlCli::RFControlCli()
{
	rfctrl = new RFControl();
}

void RFControlCli::SetRFCard(RFCONTROL::RF_CLASS rf_select)
{
	rfctrl->SetRFCard(rf_select);
}

void RFControlCli::SetCenterFreq(uint64_t centerfreq)
{
	rfctrl->SetCenterFreq(centerfreq);
}

uint32_t RFControlCli::SetRefLevel(int reflevel)
{
	return rfctrl->SetRefLevel(reflevel);
}
uint32_t RFControlCli::SetRefLevel(int reflevel,double cf)
{
	return rfctrl->SetRefLevel(reflevel,cf);
}
void RFControlCLI::RFControlCli::SetIFATT(uint32_t ifatt)
{
	return rfctrl->SetIFATT(ifatt);
}
uint32_t RFControlCLI::RFControlCli::GetIFATT()
{
	return rfctrl->GetIFATT();
}
void RFControlCli::SetCenterFreq_RPU44(uint64_t centerfreq) {
	rfctrl->SetCenterFreq_RPU44(centerfreq);
}

/// <summary>
/// 传入参考电平，返回实际RF ATT
/// </summary>
/// <param name="reflevel"></param>
/// <returns></returns>
uint32_t RFControlCli::SetRefLevel_RPU44(int reflevel)
{
	return rfctrl->SetRefLevel_RPU44(reflevel);
}

/// <summary>
/// 0 Off 1 On
/// </summary>
/// <param name="flag"></param>
void RFControlCli::SetPowerOnOff_RPU44(uint32_t flag) {
	rfctrl->SetPowerOnOff_RPU44(flag);
}


/// <summary>
/// 设置会凌MZ116模块的中心频点
/// </summary>
/// <param name="centerfreq"></param>
void RFControlCli::SetCenterFreq_MZ116(uint64_t centerfreq) {
	rfctrl->SetCenterFreq_MZ116(centerfreq);
}

/// <summary>
/// 设置会凌MZ116模块的参考电平
/// </summary>
/// <param name="reflevel"></param>
/// <returns></returns>
uint32_t RFControlCli::SetRefLevel_MZ116(int reflevel) {
	return rfctrl->SetRefLevel_MZ116(reflevel);
}

/// <summary>
/// 设置创赫CM18模块的中心频点
/// </summary>
/// <param name="centerfreq"></param>
void RFControlCli::SetCenterFreq_CM18(uint64_t centerfreq)
{
	rfctrl->SetCenterFreq_CM18(centerfreq);
}

/// <summary>
/// 设置创赫CM18模块的参考电平
/// </summary>
/// <param name="reflevel"></param>
/// <returns></returns>
uint32_t RFControlCli::SetRefLevel_CM18(int reflevel) {
	return rfctrl->SetRefLevel_CM18(reflevel);
}

/// <summary>
/// 设置创赫CM18模块的参考电平
/// </summary>
/// <param name="reflevel"></param>
/// <returns></returns>
/// 
void RFControlCli::SetOutBW_CM18(int ifbw) {
	rfctrl->SetOutBW_CM18(ifbw);
}

void RFControlCli::Init_RF12()
{
	rfctrl->Init_RF12();
}

void RFControlCli::SetCenterFreq_RF12(uint64_t centerfreq)
{
	rfctrl->SetCenterFreq_RF12(centerfreq);
}

void RFControlCli::SetRefLevel_RF12(int reflevel)
{
	rfctrl->SetRefLevel_RF12(reflevel);
}

int RFControlCli::GetRF12AttValue()
{
	return rfctrl->GetRF12AttValue();
}

RFControlCli::~RFControlCli()
{

}

uint32_t RFControlCli::Get_Temperature()
{
	return rfctrl->Get_Temperature();
}

uint32_t RFControlCli::Get_Status()
{
	return rfctrl->Get_Status();
}