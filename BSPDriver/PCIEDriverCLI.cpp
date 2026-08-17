#include <iostream>
#include "PCIEDriverCLI.h"

using namespace PCIEDriverCLI;
PCIEDriver::PCIEDriver()
{
	device = Device::Device_MEM32::getInstance();
}

bool PCIEDriver::SendData(uint32_t address, uint32_t cmd)
{
	return device->SendData(address, cmd);
}

bool PCIEDriver::SendData(uint32_t address, cli::array<float>^ cmd, uint32_t len)
{
	cli::pin_ptr<float> cmdptr = &cmd[0];
	return device->SendData(address, cmdptr, len);
}

bool PCIEDriver::SendData(uint32_t address, float cmd)
{
	return device->SendData(address, cmd);
}

bool PCIEDriver::SendData(uint32_t address, cli::array<uint32_t>^ cmd, uint32_t len)
{
	cli::pin_ptr<uint32_t> cmdptr = &cmd[0];
	return device->SendData(address, cmdptr, len);
}

bool PCIEDriver::SendData(uint32_t address, int cmd)
{
	return device->SendData(address, cmd);
}

void PCIEDriver::ReadDataHDD(uint32_t len, cli::array<uint8_t>^ dataBufOut)
{
	cli::pin_ptr<uint8_t> ptr = &dataBufOut[0];
	device->ReadDataHDD(ptr, len);
}

void PCIEDriver::ReadBackData(uint32_t address, uint32_t len, cli::array<uint8_t>^ dataBufOut)
{
	cli::pin_ptr<uint8_t> ptr = &dataBufOut[0];
	device->ReadBackData(address, len, ptr);
}