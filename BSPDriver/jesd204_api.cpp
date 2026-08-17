#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "api_def.h"
#include "Device_MEM32.h"
#include "jesd204_api.h"
#include <Windows.h>

using namespace JESD;

int32_t Jesd204Api::adc_jesd_init(jesd_init_param* init_param)
{
	int32_t ret;

	ret = Jesd204Api::adc_jesd_reset(1);
	if (ret < 0) {
		printf("adc jesd reset failed (%d)\n", ret);
		ret = -ENODEV;
		goto error;
	}

	ret = Jesd204Api::adc_jesd_reset(0);
	if (ret < 0) {
		printf("adc jesd reset failed (%d)\n", ret);
		ret = -ENODEV;
		goto error;
	}

	ret = Jesd204Api::adc_jesd_subclass_set(init_param->jesd_subclass);
	if (ret < 0) {
		printf("Failed to set subclass (%d)\n", ret);
		goto error;
	}

	ret = Jesd204Api::adc_jesd_8B10B_set();
	if (ret < 0) {
		printf("Failed to set jesd 8B10B (%d)\n", ret);
		goto error;
	}

error:
	return API_ERROR_INVALID_PARAM;
}


int32_t Jesd204Api::dac_jesd_init(jesd_init_param* init_param)
{
	int32_t ret;


	ret = Jesd204Api::dac_jesd_reset();
	if (ret < 0) {
		printf("adc jesd reset failed (%d)\n", ret);
		ret = -ENODEV;
		goto error;
	}

	ret = Jesd204Api::dac_jesd_8B10B_set();
	if (ret < 0) {
		printf("Failed to set jesd 8B10B (%d)\n", ret);
		goto error;
	}

	ret = Jesd204Api::dac_jesd_tx_ila_cfg0_set();
	if (ret < 0) {
		printf("Failed to set adc jesd tx ila cfg0 (%d)\n", ret);
		goto error;
	}

	ret = Jesd204Api::dac_jesd_tx_ila_cfg1_set(init_param->jesd_param);
	if (ret < 0) {
		printf("Failed to set adc jesd tx ila cfg1 (%d)\n", ret);
		goto error;
	}

	ret = Jesd204Api::dac_jesd_tx_ila_cfg2_set(init_param->jesd_param);
	if (ret < 0) {
		printf("Failed to set adc jesd tx ila cfg2 (%d)\n", ret);
		goto error;
	}

	ret = Jesd204Api::dac_jesd_subclass_set(init_param->jesd_subclass);
	if (ret < 0) {
		printf("Failed to set subclass (%d)\n", ret);
		goto error;
	}

	ret = Jesd204Api::dac_jesd_reset1(1);
	if (ret < 0) {
		printf("adc jesd reset failed (%d)\n", ret);
		ret = -ENODEV;
		goto error;
	}

	ret = Jesd204Api::dac_jesd_reset1(0);
	if (ret < 0) {
		printf("adc jesd reset failed (%d)\n", ret);
		ret = -ENODEV;
		goto error;
	}

error:
	return API_ERROR_INVALID_PARAM;
}


int Jesd204Api::adc_jesd_reset(uint8_t hw_reset)
{
	int err;

	err = adc_jesd_register_write(XLNX_JESD204_REG_RESET, hw_reset);
	if (err != API_ERROR_OK)
		return err;
	
	return API_ERROR_OK;
}

int Jesd204Api::adc_jesd_subclass_set(uint8_t subclass)
{
	int err;
	uint32_t tmp_reg;

	err = adc_jesd_register_write(XLNX_JESD204_REG_CTRL_SUB_CLASS, subclass);
	if (err != API_ERROR_OK)
		return err;

	return API_ERROR_OK;
}

int Jesd204Api::adc_jesd_8B10B_set()
{
	int err;
	uint32_t tmp_reg = 0x3031f01;

	err = adc_jesd_register_write(XLNX_JESD204_REG_CTRL_8B10B_CFG, tmp_reg);
	if (err != API_ERROR_OK)
		return err;

	return API_ERROR_OK;
}

int Jesd204Api::adc_jesd_register_write(const uint16_t address, const uint32_t data)
{
	auto pcie_mem = Device::Device_MEM32::getInstance();
	uint32_t BASE = 0x000C6000;
	pcie_mem->SendData(BASE + address, data);
	return API_ERROR_OK;
}

int Jesd204Api::adc_jesd_register_read(const uint16_t address, uint32_t* data)
{
	auto pcie_mem = Device::Device_MEM32::getInstance();
	uint32_t BASE = 0x000C6000;
	union uint32_chararray* uint32_char = (union uint32_chararray*)(data);
	pcie_mem->ReadBackData(BASE + address, 1, (unsigned char*)(uint32_char->c));
	return API_ERROR_OK;
}

int Jesd204Api::dac_jesd_reset()
{
	int err;

	err = dac_jesd_register_write(0x1000, 0x0);
	if (err != API_ERROR_OK)
		return err;
	err = dac_jesd_register_write(0x1000, 0x80000000);
	if (err != API_ERROR_OK)
		return err;
	err = dac_jesd_register_write(0x1000, 0x0);
	if (err != API_ERROR_OK)
		return err;

	return API_ERROR_OK;
}

int Jesd204Api::dac_jesd_reset1(uint8_t hw_reset)
{
	int err;

	err = dac_jesd_register_write(XLNX_JESD204_REG_RESET, hw_reset);
	if (err != API_ERROR_OK)
		return err;

	return API_ERROR_OK;
}

int Jesd204Api::dac_jesd_8B10B_set()
{
	int err;
	uint32_t tmp_reg = 0x3031f01;

	err = dac_jesd_register_write(XLNX_JESD204_REG_CTRL_8B10B_CFG, tmp_reg);
	if (err != API_ERROR_OK)
		return err;

	return API_ERROR_OK;
}

int Jesd204Api::dac_jesd_subclass_set(uint8_t subclass)
{
	int err;
	uint32_t tmp_reg;

	err = dac_jesd_register_write(XLNX_JESD204_REG_CTRL_SUB_CLASS, subclass);
	if (err != API_ERROR_OK)
		return err;

	return API_ERROR_OK;
}

int Jesd204Api::dac_jesd_tx_ila_cfg0_set()
{
	int err;
	uint32_t tmp_reg = 0;

	err = dac_jesd_register_write(XLNX_JESD204_CTRL_TX_ILA_CFG0, tmp_reg);
	if (err != API_ERROR_OK)
		return err;

	return API_ERROR_OK;
}

int Jesd204Api::dac_jesd_tx_ila_cfg1_set(jesd_param_t* jesd_param)
{
	int err;
	uint32_t tmp_reg = 0;
	tmp_reg &= (~XLNX_JESD204_LN_EVEN(ALL));             //清除低八位
	tmp_reg |= XLNX_JESD204_LN_EVEN(jesd_param->jesd_M);      //设置低八位为jesd_M
	tmp_reg &= (~XLNX_JESD204_LN_EVEN1(ALL));             //清除8-12位
	tmp_reg |= XLNX_JESD204_LN_EVEN1(jesd_param->jesd_N);      //设置8-12位为jesd_N
	tmp_reg &= (~XLNX_JESD204_LN_EVEN2(ALL));             //清除16-20位
	tmp_reg |= XLNX_JESD204_LN_EVEN2(jesd_param->jesd_NP);      //设置16-20位为jesd_NP
	tmp_reg &= (~XLNX_JESD204_LN_EVEN3(ALL));             //清除24-25位
	tmp_reg |= XLNX_JESD204_LN_EVEN3(jesd_param->jesd_CS);      //设置24-25位为jesd_CS

	err = dac_jesd_register_write(XLNX_JESD204_CTRL_TX_ILA_CFG1, tmp_reg);
	if (err != API_ERROR_OK)
		return err;

	return API_ERROR_OK;
}

int Jesd204Api::dac_jesd_tx_ila_cfg2_set(jesd_param_t* jesd_param)
{
	int err;
	uint32_t tmp_reg = 0;
	tmp_reg &= (~XLNX_JESD204_LN_EVEN4(ALL));             //清除8-12位
	tmp_reg |= XLNX_JESD204_LN_EVEN4(jesd_param->jesd_S);      //设置8-12位为jesd_S
	tmp_reg &= (~XLNX_JESD204_LN_EVEN5(ALL));             //清除16位
	tmp_reg |= XLNX_JESD204_LN_EVEN5(jesd_param->jesd_HD);      //设置16位为jesd_HD
	tmp_reg &= (~XLNX_JESD204_LN_EVEN6(ALL));             //清除24-28位
	tmp_reg |= XLNX_JESD204_LN_EVEN6(jesd_param->jesd_CF);      //设置24-28位为jesd_CF
	          

	err = dac_jesd_register_write(XLNX_JESD204_CTRL_TX_ILA_CFG2, tmp_reg);
	if (err != API_ERROR_OK)
		return err;

	return API_ERROR_OK;
}

int Jesd204Api::dac_jesd_ctrl_sysref_set()
{
	int err;
	uint32_t tmp_reg = 0x00000000;

	err = dac_jesd_register_write(XLNX_JESD204_CTRL_SYSREF, tmp_reg);
	if (err != API_ERROR_OK)
		return err;

	return API_ERROR_OK;
}


int Jesd204Api::dac_jesd_register_write(const uint16_t address, const uint32_t data)
{
	auto pcie_mem = Device::Device_MEM32::getInstance();
	uint32_t BASE = 0x001C1000;
	pcie_mem->SendData(BASE + address, data);
	Sleep(1);
	return API_ERROR_OK;
}

int Jesd204Api::dac_jesd_register_read(const uint16_t address, uint32_t* data)
{
	auto pcie_mem = Device::Device_MEM32::getInstance();
	uint32_t BASE = 0x001C1000;
	union uint32_chararray* uint32_char = (union uint32_chararray*)(data);
	pcie_mem->ReadBackData(BASE + address, 1, (unsigned char*)(uint32_char->c));
	return API_ERROR_OK;
}