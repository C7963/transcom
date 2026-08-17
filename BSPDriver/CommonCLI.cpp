#include "CommonCLI.h"

using namespace CommonControlCLI; 

CommonCli::CommonCli()
{
    common = &Common::CommonManager::Instance();
}

CommonCli::~CommonCli()
{
   
}
void CommonCli::SetFilterCoe(cli::array<unsigned int>^ filterLowCoe, cli::array<unsigned int>^ filterCoe)
{
    std::vector<unsigned int> nativeFilterLowCoe(filterLowCoe->Length);
    std::vector<unsigned int> nativeFilterCoe(filterCoe->Length);

    for (int i = 0; i < filterLowCoe->Length; i++)
    {
        nativeFilterLowCoe[i] = filterLowCoe[i];
    }

    for (int i = 0; i < filterCoe->Length; i++)
    {
        nativeFilterCoe[i] = filterCoe[i];
    }
    
    common->SetFilterCoe(nativeFilterLowCoe, nativeFilterCoe);
}

uint32_t CommonCli::GetIFATT() {
    return common->GetIFATT();
}

float CommonCli::Get_fpga_temp() {
   return common->get_fpga_temp();
}

float CommonCli::Get_fpga_volt() {
    return common->get_fpga_volt();
}

int CommonCli::Get_system_status() {
    return common->get_system_status();
}

bool CommonCli::Get_ddr_status(uint8_t ddrNo) {
    return common->get_ddr_status(ddrNo);
} 

void CommonCli::CloseDevice()
{
    common->CloseDevice();
}
 
void CommonCli::SetRFType(RFCONTROL::RFType rf_select)
{
	common->SetRFType(rf_select);
}

void CommonCli::InitDevice()
{
	common->InitDevice();
}
 
void CommonCli::SetWorkMode(Global::WorkMode workmode)
{
	common->SetWorkMode(workmode);
}

void CommonCli::SetADCChannel(uint32_t channel)
{
    common->SetADCChannel(channel);
}

void CommonCli::update_si5386_firmware(uint64_t adc_clk, uint32_t ref_clk)
{
    common->update_si5386_firmware(adc_clk, ref_clk);
}

void CommonCli::set_reference_mode(uint8_t mode)
{
    common->set_reference_mode(mode);
}

