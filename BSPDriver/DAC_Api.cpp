// SPDX-License-Identifier: GPL-2.0
/**
 * \file dac_api.c
 *
 * \brief Contains AD917x APIs for DAC configuration and control
 *
 * release 1.1.x
 *
 * Copyright(c) 2017 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
 */

#include "stddef.h"
#include "DAC_Api.h"
#include "cmath"
#include "Device_MEM32.h"

using namespace DAC;


 /*======================================
  * Revision Data
  *=====================================*/
static uint8_t api_revision[3] = { 1, 1, 1 };
static uint16_t range_boundary[6] = { 2910, 4140, 4370, 6210, 8740, 12420 };
dac_handle_t* DACApi::h = nullptr;


int32_t DACApi::dac_setup(struct dac_state* st)
{
	uint8_t revision[3] = { 0, 0, 0 };
	uint32_t pll_lock_status = 0, dll_lock_stat = 0;
	adi_chip_id_t dac_chip_id;
	int32_t ret;
	uint64_t dac_rate_Hz;
	uint64_t dac_clkin_Hz, lane_rate_kHz;
	dac_jesd_link_stat_t link_status;
	DACApi::h = &st->dac_h;
	uint64_t pll_mult;
	uint8_t dac_mask, chan_mask;

	st->interpolation = st->dac_interpolation * st->channel_interpolation;

	/*Initialise DAC Module*/
	ret = DACApi::dac_init();
	if (ret != 0) {
		printf("dac_init failed %d\n", ret);
		return ret;
	}

	ret = DACApi::dac_reset(0);
	if (ret != 0) {
		printf("dac_reset failed %d\n", ret);
		return ret;
	}

	ret = DACApi::dac_get_chip_id(&dac_chip_id);
	if (ret != 0) {
		printf("dac_get_chip_id failed %d\n", ret);
		return ret;
	}

	ret = DACApi::dac_get_revision(&revision[0], &revision[1], &revision[2]);
	if (ret != 0)
		return ret;

	printf("AD917x DAC Chip ID: %d\n", dac_chip_id.chip_type);
	printf("AD917x DAC Product ID: %x\n", dac_chip_id.prod_id);
	printf("AD917x DAC Product Grade: %d\n", dac_chip_id.prod_grade);
	printf("AD917x DAC Product Revision: %d\n", dac_chip_id.dev_revision);
	printf("AD917x Revision: %d.%d.%d\n", revision[0], revision[1], revision[2]);

	dac_clkin_Hz = st->dac_clkin_Hz;

	printf("PLL Input rate %lld\n", dac_clkin_Hz);

	pll_mult = NO_OS_DIV_ROUND_CLOSEST(st->dac_rate_khz, dac_clkin_Hz / 1000);

	ret = DACApi::dac_set_dac_clk((uint64_t)dac_clkin_Hz * pll_mult, 1, dac_clkin_Hz);
	if (ret != 0) {
		printf("dac_set_dac_clk failed %d\n", ret);
		return ret;
	}

	no_os_delay::no_os_mdelay(100); /* Wait 100 ms for PLL to lock */

	ret = DACApi::dac_get_dac_clk_status(&pll_lock_status, &dll_lock_stat);
	if (ret != 0) {
		printf("dac_get_dac_clk_status failed %d\n", ret);
		return ret;
	}

	printf("PLL lock status %x,  DLL lock status: %x\n", pll_lock_status, dll_lock_stat);

	if (st->clock_output_config) {
		/* DEBUG: route DAC clock to output, so we can meassure it */
		ret = DACApi::dac_set_clkout_config(st->clock_output_config);
		if (ret != 0) {
			printf("dac_set_clkout_config failed %d\n", ret);
			return ret;
		}
	}

	ret = DACApi::dac_jesd_config_datapath(st->link_mode,
		st->jesd_mode,
		st->channel_interpolation,
		st->dac_interpolation);
	if (ret != 0) {
		printf("dac_jesd_config_datapath failed %d\n", ret);
		return ret;
	}

	ret = DACApi::dac_jesd_get_cfg_param(&st->appJesdConfig);
	if (ret != 0) {
		printf("dac_jesd_get_cfg_param failed %d\n", ret);
		return ret;
	}

	ret = DACApi::dac_jesd_set_scrambler_enable(1);
	if (ret != 0) {
		printf("dac_jesd_set_scrambler_enable failed %d\n", ret);
		return ret;
	}

	ret = DACApi::dac_jesd_enable_datapath(0xFF, 0x1, 0x1);
	if (ret != 0) {
		printf("dac_jesd_enable_datapath failed %d\n", ret);
		return ret;
	}

	ret = DACApi::dac_jesd_set_syncoutb_enable(SYNCOUTB_0, 1);
	if (ret != 0) {
		printf("dac_jesd_set_syncoutb_enable failed %d\n", ret);
		return ret;
	}

	no_os_delay::no_os_mdelay(100);

	ret = DACApi::dac_jesd_get_pll_status(&pll_lock_status);
	if (ret != 0) {
		printf("dac_jesd_get_pll_status failed %d\n", ret);
		return ret;
	}

	printf("Serdes PLL %s (stat: %x)\n", ((pll_lock_status & 0x1) == 0x1) ? "Locked" : "Unlocked", pll_lock_status);

	DACApi::dac_get_dac_clk_freq(&dac_rate_Hz);

	lane_rate_kHz = dac_rate_Hz * 20 * st->appJesdConfig.jesd_M;
	no_os_util::no_os_do_div(&lane_rate_kHz, st->appJesdConfig.jesd_L *
		st->interpolation * 1000);


	ret = DACApi::dac_jesd_set_sysref_enable(st->jesd_subclass); /* subclass 0/1 */
	if (ret != 0) {
		printf("DAC:MODE:JESD: ERROR : Sysref enable failed \n");
		return -EIO;
	}

	/*
		Crossbar setup. Program the physical lane value that is providing data (the source)
		for each of the logical lanes.

		R/W Register Bits Value Description
		W 0x308 [7:0]  [5:3] = Logical Lane 1 source, [2:0] = Logical Lane 0 source.
		W 0x309 [7:0]  [5:3] = Logical Lane 3 source, [2:0] = Logical Lane 2 source.
		W 0x30A [7:0]  [5:3] = Logical Lane 5 source, [2:0] = Logical Lane 4 source.
		W 0x30B [7:0]  [5:3] = Logical Lane 7 source, [2:0] = Logical Lane 6 source.
	*/
	ret = DACApi::dac_jesd_set_lane_xbar(0, 4);
	if (ret != 0) {
		printf("dac:mode:jesd: error : Set Lane Xbar failed\n");
		return -EIO;
	}
	ret = DACApi::dac_jesd_set_lane_xbar(1, 5);
	if (ret != 0) {
		printf("dac:mode:jesd: error : Set Lane Xbar failed\n");
		return -EIO;
	}
	ret = DACApi::dac_jesd_set_lane_xbar(2, 0);
	if (ret != 0) {
		printf("dac:mode:jesd: error : Set Lane Xbar failed\n");
		return -EIO;
	}
	ret = DACApi::dac_jesd_set_lane_xbar(3, 1);
	if (ret != 0) {
		printf("dac:mode:jesd: error : Set Lane Xbar failed\n");
		return -EIO;
	}
	ret = DACApi::dac_jesd_set_lane_xbar(4, 2);
	if (ret != 0) {
		printf("dac:mode:jesd: error : Set Lane Xbar failed\n");
		return -EIO;
	}
	ret = DACApi::dac_jesd_set_lane_xbar(5, 3);
	if (ret != 0) {
		printf("dac:mode:jesd: error : Set Lane Xbar failed\n");
		return -EIO;
	}
	ret = DACApi::dac_jesd_set_lane_xbar(6, 6);
	if (ret != 0) {
		printf("dac:mode:jesd: error : Set Lane Xbar failed\n");
		return -EIO;
	}
	ret = DACApi::dac_jesd_set_lane_xbar(7, 7);
	if (ret != 0) {
		printf("dac:mode:jesd: error : Set Lane Xbar failed\n");
		return -EIO;
	}

	/*Enable Link*/
	ret = DACApi::dac_jesd_enable_link((jesd_link_t)(st->link_mode), 0x1);
	if (ret != 0) {
		printf("DAC:MODE:JESD: ERROR : Enable Link failed\n");
		return -EIO;
	}

	no_os_delay::no_os_mdelay(100);

	ret = DACApi::dac_jesd_get_link_status((jesd_link_t)(st->link_mode), &link_status);
	if (ret != 0) {
		printf("DAC:MODE:JESD: ERROR : Get Link status failed \r\n");
		return -EIO;
	}

	printf("code_grp_sync: %x\n", link_status.code_grp_sync_stat);
	printf("frame_sync_stat: %x\n", link_status.frame_sync_stat);
	printf("good_checksum_stat: %x\n", link_status.good_checksum_stat);
	printf("init_lane_sync_stat: %x\n", link_status.init_lane_sync_stat);
	printf("%d lanes @ % kBps %lld\n", st->appJesdConfig.jesd_L, lane_rate_kHz);

	dac_mask = st->dac_mask;
	
	if (st->interpolation > 1) {
		chan_mask = NO_OS_GENMASK(st->appJesdConfig.jesd_M / 2, 0);
		ret = DACApi::dac_set_page_idx(AD917X_DAC_NONE, chan_mask);
		if (ret != 0)
			return -EIO;
		ret = DACApi::dac_set_channel_gain(2048); /* GAIN = 1 */
		if (ret != 0)
			return -EIO;

		st->nco_main_enable = dac_mask;

		DACApi::dac_nco_enable((dac_dac_select_t)(st->nco_main_enable), (dac_channel_select_t)0);
	}

	ret = DACApi::dac_set_page_idx(dac_mask, AD917X_CH_NONE);
	if (ret != 0)
		return -EIO;

	DACApi::dac_register_write(0x596, 0x1c); /* SPI turn on TXENx feature */

	return 0;
}

/**
 * Delay microseconds, compatible with AD917x API
 * Performs a blocking or sleep delay for the specified time in microseconds
 * @param user_data
 * @param us - time to delay/sleep in microseconds.
 * @return 0 in case of success, negative error code otherwise.
 */
static int32_t ad9172_delay_us(void* user_data, uint32_t us)
{
	no_os_delay::no_os_udelay(us);
	return 0;
}

/**
 * Spi write and read compatible with
 * @param user_data
 * @param wbuf Pointer to array with the data to be sent on the SPI
 * @param rbuf Pointer to array where the data to which the SPI will be written
 * @param len The size in bytes allocated for each of the indata and outdata arrays.
 * @return 0 for success, any non-zero value indicates an error
 */
static int32_t ad9172_spi_xfer(void* user_data, uint8_t* wbuf, uint8_t* rbuf, int32_t len)
{
	int32_t ret;
	struct no_os_spi_desc* spi = (struct no_os_spi_desc*)user_data;
	uint8_t* buffer = (uint8_t*)no_os_alloc::no_os_malloc(len);

	memcpy(buffer, wbuf, 3);
	ret = no_os_spi::no_os_spi_write_and_read(spi, buffer, len);
	if (ret < 0) {
		printf("Read Error %d", ret);
	}
	else {
		memcpy(rbuf, buffer, len);
	}
	no_os_alloc::no_os_free(buffer);

	return ret;
}


/**
 * Initialize the device.
 * @param device - The device structure.
 * @param init_param - The structure that contains the device initial
 * 		       parameters.
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t DACApi::dac_init(dac_dev* dev, dac_init_param* init_param)
{
	int32_t ret;

	if (!dev)
		return -ENOMEM;

	struct dac_state* st = dev->st;

	if (!st) {
		ret = -ENOMEM;
		goto error_1;
	}

	goto myneed;
	/* SPI */
	ret = no_os_spi::no_os_spi_init(&dev->spi_desc, init_param->spi_init);
	if (ret < 0)
		goto error_2;

	/* GPIO */
	ret |= no_os_gpio::no_os_gpio_get(&dev->gpio_reset, &init_param->gpio_reset);
	ret |= no_os_gpio::no_os_gpio_get(&dev->gpio_txen0, &init_param->gpio_txen0);
	ret |= no_os_gpio::no_os_gpio_get(&dev->gpio_txen1, &init_param->gpio_txen1);

	ret |= no_os_gpio::no_os_gpio_direction_output(dev->gpio_reset, NO_OS_GPIO_HIGH);
	ret |= no_os_gpio::no_os_gpio_direction_output(dev->gpio_txen0, NO_OS_GPIO_HIGH);
	ret |= no_os_gpio::no_os_gpio_direction_output(dev->gpio_txen1, NO_OS_GPIO_HIGH);

	ret |= no_os_gpio::no_os_gpio_set_value(dev->gpio_reset, NO_OS_GPIO_HIGH);
	ret |= no_os_gpio::no_os_gpio_set_value(dev->gpio_txen0, NO_OS_GPIO_HIGH);
	ret |= no_os_gpio::no_os_gpio_set_value(dev->gpio_txen1, NO_OS_GPIO_HIGH);

	if (ret < 0)
		goto error_3;

myneed:
	st->dac_rate_khz = init_param->dac_rate_khz;
	st->dac_clkin_Hz = init_param->dac_clkin_Hz;
	st->link_mode = init_param->link_mode;
	st->jesd_subclass = init_param->jesd_subclass;
	st->dac_interpolation = init_param->dac_interpolation;
	st->channel_interpolation = init_param->channel_interpolation;
	st->syncoutb_type = init_param->syncoutb_type;
	st->sysref_coupling = init_param->sysref_coupling;
	st->jesd_mode = init_param->jesd_mode;
	st->logic_lane = init_param->logic_lane;
	st->physical_lane = init_param->physical_lane;
	st->clock_output_config = init_param->clock_output_config;
	st->dac_mask = init_param->dac_mask;

	st->dac_h.sdo = SPI_SDIO;
	st->dac_h.dev_xfer = ad9172_spi_xfer;
	st->dac_h.delay_us = ad9172_delay_us;
	st->dac_h.tx_en_pin_ctrl = NULL;
	st->dac_h.reset_pin_ctrl = NULL;
	st->dac_h.syncoutb = st->syncoutb_type;
	st->dac_h.sysref = st->sysref_coupling;

	ret = DACApi::dac_setup(st);
	if (ret < 0) {
		printf("Failed to setup device\n");
		goto error_3;
	}

	printf("%s : DAC Rev %d successfully initialized\n", __func__, 1);

	return 0;

error_3:
	no_os_spi::no_os_spi_remove(dev->spi_desc);
error_2:
	no_os_alloc::no_os_free(st);
error_1:
	no_os_alloc::no_os_free(dev);

	return ret;
}

/**
 * Remove the device - release resources.
 * @param device - The device structure.
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t DACApi::dac_remove(dac_dev* device)
{
	int32_t ret;

	ret = no_os_spi::no_os_spi_remove(device->spi_desc);
	ret += no_os_gpio::no_os_gpio_remove(device->gpio_reset);
	ret += no_os_gpio::no_os_gpio_remove(device->gpio_txen0);
	ret += no_os_gpio::no_os_gpio_remove(device->gpio_txen1);

	no_os_alloc::no_os_free(device->st);

	no_os_alloc::no_os_free(device);

	return ret;
}

static int32_t check_dac_clk_freq_range(uint64_t dac_clk_freq_hz)
{
	adi_chip_id_t adi_chip_id;
	int32_t err;

	if (DACApi::h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;

	err = DACApi::dac_get_chip_id(&adi_chip_id);
	if (err != API_ERROR_OK)
		return err;
	if (adi_chip_id.prod_id == AD9171_ID) {
		if ((dac_clk_freq_hz > DAC_9171_CLK_FREQ_MAX_HZ) ||
			(dac_clk_freq_hz < DAC_CLK_FREQ_MIN_HZ))
			return API_ERROR_INVALID_PARAM;
	}
	else {
		if ((dac_clk_freq_hz > DAC_CLK_FREQ_MAX_HZ) ||
			(dac_clk_freq_hz < DAC_CLK_FREQ_MIN_HZ))
			return API_ERROR_INVALID_PARAM;
	}
	return API_ERROR_OK;
}

static int32_t spi_configure()
{
	switch (DACApi::h->sdo) {
	case SPI_SDO:
		return DACApi::dac_register_write(AD917X_IF_CFG_A_REG, 0x18);
	case SPI_SDIO:
		return DACApi::dac_register_write(AD917X_IF_CFG_A_REG, 0x00);
	default:
		return API_ERROR_SPI_SDO;
		break;
	}
	return API_ERROR_SPI_SDO;
}

static int32_t sysref_configure()
{
	uint32_t tmp_reg;
	int32_t err;

	err = DACApi::dac_register_read(AD917X_SYSREF_CTRL_REG, &tmp_reg);
	if (err != API_ERROR_OK)
		return err;
	tmp_reg &= ~(AD917X_SYSREF_PD);
	if (DACApi::h->sysref == COUPLING_DC)
		tmp_reg |= AD917X_SYSREF_PD;
	err = DACApi::dac_register_write(AD917X_SYSREF_CTRL_REG, tmp_reg);
	return err;
}

static int32_t syncoutb_configure()
{
	if (DACApi::h->syncoutb == SIGNAL_LVDS) {
		return DACApi::dac_register_write(AD917X_SYNCOUTB_CTRL_0_REG, AD917X_SYNCOUTB_MODE(0x1));

	}
	else {
		return DACApi::dac_register_write(AD917X_SYNCOUTB_CTRL_0_REG, AD917X_SYNCOUTB_MODE(0x0));
	}
}


static int32_t dac_init_sequence()
{
	int32_t err;
	uint32_t tmp_reg = 0x0;

	if (DACApi::h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;
	/*Boot from NVRAM & Check Boot Status*/
	err = DACApi::dac_register_write_tbl(&ADI_RECOMMENDED_BOOT_TBL[0],
		NO_OS_ARRAY_SIZE(ADI_RECOMMENDED_BOOT_TBL));
	if (err != API_ERROR_OK)
		return err;

	if (DACApi::h->delay_us != NULL) {
		err = DACApi::h->delay_us(DACApi::h->user_data, NVRAM_RESET_PERIOD_US);
		if (err != 0)
			return API_ERROR_US_DELAY;
	}
	err = DACApi::dac_register_read(AD917X_NVM_LOADER_REG, &tmp_reg);
	if (err != API_ERROR_OK)
		return err;
	if (!(tmp_reg & AD917X_NVM_BLR_DONE))
		return API_ERROR_INIT_SEQ_FAIL;

	return API_ERROR_OK;
}

int32_t DACApi::dac_init()
{
	int32_t err;
	if (DACApi::h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;
	if (DACApi::h->dev_xfer == NULL)
		return API_ERROR_INVALID_XFER_PTR;
	if (DACApi::h->delay_us == NULL)
		return API_ERROR_INVALID_DELAYUS_PTR;
	if (DACApi::h->sdo >= SPI_CONFIG_MAX)
		return API_ERROR_SPI_SDO;
	if (DACApi::h->hw_open != NULL) {
		err = DACApi::h->fd;
		if (err < 0)
			return API_ERROR_HW_OPEN;
	}

	err = spi_configure();
	if (err != API_ERROR_OK)
		return err;

	err = syncoutb_configure();
	if (err != API_ERROR_OK)
		return err;

	err = sysref_configure();
	if (err != API_ERROR_OK)
		return err;

	return API_ERROR_OK;
}

int32_t DACApi::dac_deinit()
{
	int32_t err;
	if (DACApi::h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;
	if (DACApi::h->hw_close != NULL) {
		err = DACApi::h->hw_close((int)(DACApi::h->user_data));
		if (err != 0)
			return API_ERROR_HW_CLOSE;
	}
	return API_ERROR_OK;
}

int32_t DACApi::dac_get_chip_id(adi_chip_id_t* chip_id)
{
	int32_t err;
	uint32_t tmp_reg;
	if (DACApi::h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;
	if (chip_id == NULL)
		return API_ERROR_INVALID_PARAM;
	err = DACApi::dac_register_read(AD917X_CHIP_TYPE_REG, &tmp_reg);
	if (err != API_ERROR_OK)
		return err;
	chip_id->chip_type = tmp_reg;
	
	err = DACApi::dac_register_read(AD917X_PROD_ID_MSB_REG, &tmp_reg);
	if (err != API_ERROR_OK)
		return err;
	chip_id->prod_id = tmp_reg;
	chip_id->prod_id <<= 8;
	
	err = DACApi::dac_register_read(AD917X_PROD_ID_LSB_REG, &tmp_reg);
	if (err != API_ERROR_OK)
		return err;
	chip_id->prod_id |= tmp_reg;

	err = DACApi::dac_register_read(AD917X_CHIP_GRADE_REG, &tmp_reg);
	if (err != API_ERROR_OK)
		return err;
	chip_id->prod_grade = (tmp_reg >> 4);
	chip_id->dev_revision = (tmp_reg & 0x0F);

	return API_ERROR_OK;
}

int32_t DACApi::dac_reset(uint8_t hw_reset)
{
	int32_t err;
	if (DACApi::h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;

	if (hw_reset > 1)
		return API_ERROR_INVALID_PARAM;
	if (hw_reset) {
		if (DACApi::h->reset_pin_ctrl == NULL)
			return API_ERROR_INVALID_PARAM;
		err = DACApi::h->reset_pin_ctrl(DACApi::h->user_data, 0x1);
		if (err != 0)
			return API_ERROR_RESET_PIN_CTRL;
		if (DACApi::h->delay_us != NULL) {
			err = DACApi::h->delay_us(DACApi::h->user_data, HW_RESET_PERIOD_US);
			if (err != 0)
				return API_ERROR_US_DELAY;
		}
		err = DACApi::h->reset_pin_ctrl(DACApi::h->user_data, 0x0);
		if (err != 0)
			return API_ERROR_RESET_PIN_CTRL;
	}

	switch (DACApi::h->sdo) {
	case SPI_SDO:
		err = DACApi::dac_register_write(AD917X_IF_CFG_A_REG, 0x99);
		if (err != API_ERROR_OK)
			return err;
		break;
	case SPI_SDIO:
		err = DACApi::dac_register_write(AD917X_IF_CFG_A_REG, 0x81);
		if (err != API_ERROR_OK)
			return err;
		break;
	default:
		return API_ERROR_SPI_SDO;
		break;
	}

	if (DACApi::h->delay_us != NULL) {
		err = DACApi::h->delay_us(DACApi::h->user_data, SPI_RESET_PERIOD_US);
		if (err != 0)
			return API_ERROR_US_DELAY;
	}

	err = syncoutb_configure();
	if (err != API_ERROR_OK)
		return err;

	err = sysref_configure();
	if (err != API_ERROR_OK)
		return err;

	return dac_init_sequence();

}

int32_t DACApi::dac_get_revision(uint8_t* rev_major, uint8_t* rev_minor, uint8_t* rev_rc)
{
	int32_t err = API_ERROR_OK;

	if (rev_major != NULL)
		*rev_major = api_revision[0];
	else
		err = API_ERROR_INVALID_PARAM;
	if (rev_minor != NULL)
		*rev_minor = api_revision[1];
	else
		err = API_ERROR_INVALID_PARAM;
	if (rev_rc != NULL)
		*rev_rc = api_revision[2];
	else
		err = API_ERROR_INVALID_PARAM;

	return err;
}


int32_t DACApi::dac_set_dac_pll_config(uint8_t dac_pll_en, uint8_t m_div, uint8_t n_div, uint8_t vco_div)
{

	int32_t err;
	uint32_t tmp_reg;

	if (DACApi::h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;

	/*check Parameter valid Ranges */
	if ((dac_pll_en > 1) || (m_div > 4) || (m_div < 1) || (n_div < 2) ||
		(n_div > 50) || (vco_div < 1) || (vco_div > 3))
		return API_ERROR_INVALID_PARAM;
	err = DACApi::dac_register_write(AD917X_PLL_BYPASS_REG, AD917X_PLL_BYPASS(!dac_pll_en));
	if (err != API_ERROR_OK)
		return err;
	if (!dac_pll_en) {
		err = DACApi::dac_register_write(AD917X_DACPLL_CTRLX_REG, 0xFF); /*ADI REC WRITE*/
		if (err != API_ERROR_OK)
			return err;
		err = DACApi::dac_register_write(AD917X_DACPLL_CTRLY_REG, 0xFF); /*ADI REC WRITE*/
		if (err != API_ERROR_OK)
			return err;
		return API_ERROR_OK;
	}
	else {
		err = DACApi::dac_register_write(AD917X_DACPLL_CTRLX_REG, 0x00); /*ADI REC WRITE*/
		if (err != API_ERROR_OK)
			return err;
		err = DACApi::dac_register_write(AD917X_DACPLL_CTRLY_REG, 0x00); /*ADI REC WRITE*/
		if (err != API_ERROR_OK)
			return err;
	}

	/*Configure On-Chip PLL*/
	/*Initialise PLL*/
	/*Call PLL Recommended Init Sequence*/
	err = DACApi::dac_register_write_tbl(&ADI_RECOMMENDED_PLL_TBL_1[0],
		NO_OS_ARRAY_SIZE(ADI_RECOMMENDED_PLL_TBL_1));
	if (err != API_ERROR_OK)
		return err;
	if (DACApi::h->delay_us != NULL)
		DACApi::h->delay_us(DACApi::h->user_data, 100000);

	/*Set  M Divider*/
	err = DACApi::dac_register_read(AD917X_DACPLL_CTRL1_REG, &tmp_reg);
	if (err != API_ERROR_OK)
		return err;
	tmp_reg &= ~AD917X_M_DIV(ALL);
	tmp_reg |= AD917X_M_DIV((m_div));
	//tmp_reg |= AD917X_M_DIV((m_div - 1));
	err = DACApi::dac_register_write(AD917X_DACPLL_CTRL1_REG, tmp_reg);
	if (err != API_ERROR_OK)
		return err;

	/* Set N Divider */
	err = DACApi::dac_register_read(AD917X_DACPLL_CTRL7_REG, &tmp_reg);
	if (err != API_ERROR_OK)
		return err;
	tmp_reg &= ~AD917X_N_DIV(ALL);
	tmp_reg |= AD917X_N_DIV(n_div);
	err = DACApi::dac_register_write(AD917X_DACPLL_CTRL7_REG, tmp_reg);
	if (err != API_ERROR_OK)
		return err;

	/*Set PLL VCO DIV */
	tmp_reg = AD917X_PLL_VCO_DIV_EN(vco_div - 1);
	err = DACApi::dac_register_write(AD917X_PLL_VCO_CTRL_REG, tmp_reg);
	if (err != API_ERROR_OK)
		return err;

	/*Reset VCO*/
	if (DACApi::h->delay_us != NULL)
		DACApi::h->delay_us(DACApi::h->user_data, 1000);

	err = DACApi::dac_register_read(AD917X_DACPLL_CTRL0_REG, &tmp_reg);
	if (err != API_ERROR_OK)
		return err;
	tmp_reg &= AD917X_RESET_VCO_DIV;          //将|修改为&
	err = DACApi::dac_register_write(AD917X_DACPLL_CTRL0_REG, tmp_reg);
	if (err != API_ERROR_OK)
		return err;
	tmp_reg &= ~AD917X_RESET_VCO_DIV;
	err = DACApi::dac_register_write(AD917X_DACPLL_CTRL0_REG, tmp_reg); 
	if (err != API_ERROR_OK)
		return err;

	return API_ERROR_OK;

}


int32_t DACApi::dac_set_dac_clk_freq(uint64_t dac_clk_freq_hz)
{
	int32_t err;
	uint32_t tmp_reg = 0x0;
	if (DACApi::h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;
	err = check_dac_clk_freq_range(dac_clk_freq_hz);
	if (err != API_ERROR_OK)
		return err;
	DACApi::h->dac_freq_hz = (uint64_t)(dac_clk_freq_hz);
	/*Call DLL Recommended Init Sequence*/
	err = DACApi::dac_register_write_tbl(&ADI_RECOMMENDED_DLL_TBL[0],
		NO_OS_ARRAY_SIZE(ADI_RECOMMENDED_DLL_TBL));
	if (err != API_ERROR_OK)
		return err;
	/*Reset DLL*/
	if (dac_clk_freq_hz < DLL_CLK_FREQ_THRES_HZ)
		tmp_reg = 0x48;
	else
		tmp_reg = 0x68;
	err = DACApi::dac_register_write(AD917X_DLL_CTRL0_REG, tmp_reg);
	if (err != API_ERROR_OK)
		return err;
	tmp_reg |= AD917X_DLL_RST;
	err = DACApi::dac_register_write(AD917X_DLL_CTRL0_REG, tmp_reg);
	if (err != API_ERROR_OK)
		return err;

	/*Call DAC Calibration Recommended Init Sequence*/
	err = DACApi::dac_register_write_tbl(&ADI_RECOMMENDED_DAC_CAL_TBL[0],
		NO_OS_ARRAY_SIZE(ADI_RECOMMENDED_DAC_CAL_TBL));
	if (err != API_ERROR_OK)
		return err;
	return API_ERROR_OK;
}


int32_t DACApi::dac_get_dac_clk_freq(uint64_t* dac_clk_freq_hz)
{
	if (DACApi::h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;
	if (dac_clk_freq_hz == NULL)
		return API_ERROR_INVALID_PARAM;
	*dac_clk_freq_hz = DACApi::h->dac_freq_hz;
	return API_ERROR_OK;
}

int32_t DACApi::dac_get_dac_clk_status(uint32_t* pll_lock_stat, uint32_t* dll_lock_stat)
{
	int32_t err;
	uint32_t tmp_reg;

	if (DACApi::h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;
	if (pll_lock_stat != NULL) {

		err = DACApi::dac_register_read(AD917X_DACPLL_STATUS_REG, &tmp_reg);
		if (err != API_ERROR_OK)
			return err;
		*pll_lock_stat = tmp_reg & AD917X_DACPLL_LOCK;
	}
	if (dll_lock_stat != NULL) {

		err = DACApi::dac_register_read(AD917X_DLL_STATUS_REG, &tmp_reg);
		if (err != API_ERROR_OK)
			return err;
		*dll_lock_stat = tmp_reg & AD917X_DLL_LOCK;
	}
	return API_ERROR_OK;
}

int32_t DACApi::dac_set_clkout_config(uint8_t l_div)
{
	int32_t err;
	uint32_t tmp_reg;

	if (DACApi::h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;
	if ((l_div < 1) || (l_div > 4))
		return API_ERROR_INVALID_PARAM;

	err = DACApi::dac_register_read(AD917X_DACPLL_CTRL7_REG, &tmp_reg);
	if (err != API_ERROR_OK)
		return err;
	tmp_reg &= ~AD917X_L_DIV(ALL);
	tmp_reg |= AD917X_L_DIV((l_div));
	err = DACApi::dac_register_write(AD917X_DACPLL_CTRL7_REG, tmp_reg);
	if (err != API_ERROR_OK)
		return err;
	return API_ERROR_OK;
}

int32_t DACApi::dac_set_dac_clk(uint64_t dac_clk_freq_hz, uint8_t dac_pll_en, uint64_t ref_clk_freq_hz)
{
	int32_t err;
	uint8_t m_div, n_div, pll_vco_div;
	uint16_t n_div_tmp, ref_clk_freq_mhz, pfd_clk_freq_mhz, target_pfd, fvco_freq_mhz = 0x0;
	uint64_t dac_clk_freq_mhz;

	if (h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;
	if (dac_pll_en > 1)
		return API_ERROR_INVALID_PARAM;

	err = check_dac_clk_freq_range(dac_clk_freq_hz);
	if (err != API_ERROR_OK)
		return err;

	err = DACApi::dac_register_write(AD917X_PLL_BYPASS_REG, AD917X_PLL_BYPASS(!dac_pll_en));
	if (err != API_ERROR_OK)
		return err;
	dac_clk_freq_mhz = NO_OS_DIV_U64(dac_clk_freq_hz, 1000000);
	if (dac_pll_en) {
		/*Generate On chip PLL Configuration*/
		/*Check REF CLK within 30MHz to 2GHz Range*/
		ref_clk_freq_mhz = NO_OS_DIV_U64(ref_clk_freq_hz, 1000000);
		if ((ref_clk_freq_mhz < REF_CLK_FREQ_MHZ_MIN) ||
			(ref_clk_freq_mhz > REF_CLK_FREQ_MHZ_MAX))
			return API_ERROR_INVALID_PARAM;

		/*Determine FVCO Frequency and Check DAC CLK withing PLL Range*/
		if ((dac_clk_freq_mhz > range_boundary[0]) &&
			(dac_clk_freq_mhz < range_boundary[1])) {
			fvco_freq_mhz = (dac_clk_freq_mhz * 3);
			pll_vco_div = 3;
		}
		else if ((dac_clk_freq_mhz > range_boundary[2]) &&
			(dac_clk_freq_mhz < range_boundary[3])) {
			fvco_freq_mhz = (dac_clk_freq_mhz * 2);
			pll_vco_div = 2;
		}
		else if ((dac_clk_freq_mhz > range_boundary[4]) &&
			(dac_clk_freq_mhz < range_boundary[5])) {
			fvco_freq_mhz = (dac_clk_freq_mhz * 1);
			pll_vco_div = 1;
		}
		else
			return API_ERROR_INVALID_PARAM;

		if ((fvco_freq_mhz >= 9960) && (fvco_freq_mhz <= 10870))
			target_pfd = 220; /* less than 225 */
		else
			target_pfd = 500; /* less than 770 */

		/*Determine divM and check PFD Frequency within Range*/
		m_div = ref_clk_freq_mhz / target_pfd;
		if (ref_clk_freq_mhz % target_pfd)
			m_div++;

		pfd_clk_freq_mhz = ref_clk_freq_mhz / m_div;

		if ((pfd_clk_freq_mhz < PFD_CLK_FREQ_MHZ_MIN) ||
			(pfd_clk_freq_mhz > PFD_CLK_FREQ_MHZ_MAX))
			return API_ERROR_INVALID_PARAM;

		/*Calculate N Divider using FVCO Frequency*/
		n_div_tmp = (fvco_freq_mhz * (m_div));
		n_div = ceil(((n_div_tmp*1000.0* (m_div+1.0))/(8.0* ref_clk_freq_hz/1000)));
		//n_div_tmp = NO_OS_DIV_ROUND_CLOSEST(n_div_tmp * 1000, (uint32_t)NO_OS_DIV_U64(ref_clk_freq_hz, 1000));
		//n_div = NO_OS_DIV_ROUND_CLOSEST(n_div_tmp, 8);

		/*Initialise PLL*/
		/*Call PLL Recommended Init Sequence*/
		err = DACApi::dac_register_write_tbl(&ADI_RECOMMENDED_PLL_TBL_1[0],
			NO_OS_ARRAY_SIZE(ADI_RECOMMENDED_PLL_TBL_1));
		if (err != API_ERROR_OK)
			return err;
		if (h->delay_us != NULL)
			h->delay_us(h->user_data, 100000);

		/*Set the PLL Dividers */
		err = dac_set_dac_pll_config(1, m_div, n_div, pll_vco_div);
		if (err != API_ERROR_OK)
			return err;

	}
	/*Call Set Dac Frequency Sequence*/
	err = dac_set_dac_clk_freq(dac_clk_freq_hz);
	if (err != API_ERROR_OK)
		return err;
	return API_ERROR_OK;

}

int32_t DACApi::dac_set_page_idx(const uint32_t dac, const uint32_t channel)
{
	if (DACApi::h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;

	return DACApi::dac_register_write(AD917X_SPI_PAGEINDX_REG,
		(uint8_t)((dac << 6) | channel));
}

int32_t DACApi::dac_get_page_idx(int32_t* dac, int32_t* channel)
{
	int32_t err;
	uint32_t tmp;
	if (DACApi::h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;
	if ((dac == NULL) || (channel == NULL))
		return API_ERROR_INVALID_PARAM;

	err = DACApi::dac_register_read(AD917X_SPI_PAGEINDX_REG, &tmp);
	if (err != API_ERROR_OK)
		return err;
	*dac = tmp >> 6;
	*channel = tmp & 0x3F;
	return API_ERROR_OK;
}


int32_t DACApi::dac_register_write(const uint16_t address, uint32_t data)
{
	auto pcie_mem = Device::Device_MEM32::getInstance();
	uint32_t BASE = 0x001C0000;
	pcie_mem->SendData(BASE + address, data);
	return API_ERROR_OK;
}


int32_t DACApi::dac_register_read(const uint16_t address, uint32_t* data)
{
	auto pcie_mem = Device::Device_MEM32::getInstance();
	uint32_t BASE = 0x001C0000;
	union uint32_chararray* uint32_char = (union uint32_chararray*)(data);
	pcie_mem->ReadBackData(BASE + address, 1, (unsigned char*)(uint32_char->c));
	return API_ERROR_OK;
}

int32_t DACApi::dac_register_read_block(const uint16_t address, uint32_t* data, uint32_t count)
{
	int32_t err;
	uint16_t i = 0;

	for (i = 0; i < count; i++) {
		err = dac_register_read((address + i), &data[i]);
		if (err != API_ERROR_OK)
			return err;
	}

	return API_ERROR_OK;
}

int32_t DACApi::dac_register_write_tbl(struct adi_reg_data* tbl, uint32_t count)
{
	uint16_t i = 0;
	int32_t err;

	for (i = 0; i < count; i++) {
		err = dac_register_write(tbl[i].reg, tbl[i].val);
		if (err != API_ERROR_OK)
			return err;
	}

	return API_ERROR_OK;
}




int32_t DACApi::jesd_get_link_count(uint8_t* link_count)
{
	int32_t err;
	uint32_t tmp_reg;

	if (DACApi::h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;

	if (link_count == NULL)
		return API_ERROR_INVALID_PARAM;

	err = DACApi::dac_register_read(AD917X_JESD_MODE_REG, &tmp_reg);
	if (err != API_ERROR_OK)
		return err;
	tmp_reg &= ~(AD917X_LINK_PAGE(ALL));
	*link_count = (tmp_reg & AD917X_LINK_MODE) ? 2 : 1;
	return API_ERROR_OK;
}

int32_t DACApi::jesd_set_link(int32_t link)
{
	int32_t err;
	uint32_t tmp_reg;

	if (DACApi::h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;

	if (link > 1)
		return API_ERROR_INVALID_PARAM;

	err = DACApi::dac_register_read(AD917X_JESD_RX_CTL_REG, &tmp_reg);
	if (err != API_ERROR_OK)
		return err;
	tmp_reg &= ~(AD917X_LINK_PAGE(ALL));
	tmp_reg |= AD917X_LINK_PAGE(link);
	return DACApi::dac_register_write(AD917X_JESD_RX_CTL_REG, tmp_reg);
}


int32_t DACApi::dac_jesd_config_datapath(uint8_t dual_en, uint8_t jesd_mode, uint8_t ch_intpl, uint8_t dp_intpl)
{
	int32_t err;
	uint32_t tmp_reg;

	if (DACApi::h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;

	if (dual_en > 1)
		return API_ERROR_INVALID_PARAM;

	if (jesd_mode >= JESD_MODE_INVALID)
		return API_ERROR_INVALID_PARAM;
	if ((ch_intpl < INTERPOLATION_MIN) ||
		(ch_intpl > CH_INTERPOLATION_MAX))
		return API_ERROR_INVALID_PARAM;
	if ((dp_intpl < INTERPOLATION_MIN) ||
		(dp_intpl > DP_INTERPOLATION_MAX))
		return API_ERROR_INVALID_PARAM;

	/*Disable Links Prior to configuration*/
	err = DACApi::dac_register_read(AD917X_JESD_RX_CTL_REG, &tmp_reg);
	if (err != API_ERROR_OK)
		return err;
	tmp_reg &= ~(AD917X_LINK_0_EN | AD917X_LINK_1_EN);
	err = DACApi::dac_register_write(AD917X_JESD_RX_CTL_REG, tmp_reg);
	if (err != API_ERROR_OK)
		return err;
	/*Configure JESD Mode */
	err = DACApi::dac_register_read(AD917X_JESD_MODE_REG, &tmp_reg);
	if (err != API_ERROR_OK)
		return err;
	tmp_reg &= ~(AD917X_JESD_MODE(ALL) | AD917X_LINK_MODE);
	tmp_reg |= AD917X_JESD_MODE(jesd_mode);
	tmp_reg |= ((dual_en == 1) ? AD917X_LINK_MODE : 0);
	err = DACApi::dac_register_write(AD917X_JESD_MODE_REG, tmp_reg);
	if (err != API_ERROR_OK)
		return err;

	/*Configure Interpolation Mode */
	tmp_reg = 0x0;
	tmp_reg |= AD917X_CH_INTERP_MODE(ch_intpl) | AD917X_DP_INTERP_MODE(dp_intpl);
	err = DACApi::dac_register_write(AD917X_INTERP_MODE_REG, tmp_reg);
	if (err != API_ERROR_OK)
		return err;

	/*Check Readback for Valid Configuration*/
	err = DACApi::dac_register_read(AD917X_JESD_MODE_REG, &tmp_reg);
	if (err != API_ERROR_OK)
		return err;
	if (tmp_reg & AD917X_JESD_MODE_INVALID)
		return API_ERROR_INVALID_PARAM;

	return API_ERROR_OK;
}

int32_t DACApi::dac_jesd_get_cfg_status(uint8_t* cfg_valid)
{
	int32_t err;
	uint32_t tmp_reg;

	if (DACApi::h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;

	if (cfg_valid == NULL)
		return API_ERROR_INVALID_PARAM;

	err = DACApi::dac_register_read(AD917X_JESD_MODE_REG, &tmp_reg);
	if (err != API_ERROR_OK)
		return err;
	*cfg_valid = (tmp_reg & AD917X_JESD_MODE_INVALID) ? 0 : 1;

	return API_ERROR_OK;
}

int32_t DACApi::dac_jesd_get_cfg_param(jesd_param_t* jesd_param)
{
	int32_t err;
	uint32_t tmp_reg[AD917X_JESD_PARAM_REG_LEN];

	if (DACApi::h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;

	if (jesd_param == NULL)
		return API_ERROR_INVALID_PARAM;

	err = DACApi::dac_register_read_block(AD917X_JESD_PARAM_REG_BASE, &tmp_reg[0], AD917X_JESD_PARAM_REG_LEN);
	if (err != API_ERROR_OK)
		return err;
	jesd_param->jesd_L = AD917X_JESD_L_GET(tmp_reg);
	jesd_param->jesd_F = AD917X_JESD_F_GET(tmp_reg);
	jesd_param->jesd_K = AD917X_JESD_K_GET(tmp_reg);
	jesd_param->jesd_M = AD917X_JESD_M_GET(tmp_reg);
	jesd_param->jesd_N = AD917X_JESD_N_GET(tmp_reg);
	jesd_param->jesd_NP = AD917X_JESD_NP_GET(tmp_reg);
	jesd_param->jesd_S = AD917X_JESD_S_GET(tmp_reg);
	jesd_param->jesd_JESDV = AD917X_JESD_V_GET(tmp_reg);
	jesd_param->jesd_HD = AD917X_JESD_HD_GET(tmp_reg);
	jesd_param->jesd_DID = AD917X_JESD_DID_GET(tmp_reg);
	jesd_param->jesd_BID = AD917X_JESD_BID_GET(tmp_reg);

	/*Fixed Parameters*/
	jesd_param->jesd_CF = CF_DEFAULT;
	jesd_param->jesd_CS = CS_DEFAULT;

	return API_ERROR_OK;
}

int32_t DACApi::dac_jesd_set_sysref_enable(uint8_t en)
{
	int32_t err;
	uint32_t tmp_reg = 0x0;

	if (DACApi::h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;

	if (en > 1)
		return API_ERROR_INVALID_PARAM;
	err = DACApi::dac_register_read(AD917X_SYSREF_CTRL_REG, &tmp_reg);
	if (err != API_ERROR_OK)
		return err;
	tmp_reg &= ~(AD917X_SYSREF_PD);
	tmp_reg |= ((en == 1) ? 0 : AD917X_SYSREF_PD);
	err = DACApi::dac_register_write(AD917X_SYSREF_CTRL_REG, tmp_reg);
	if (err != API_ERROR_OK)
		return err;
	/*Enable Periodic Rst_en for Subclass 0 and Subclass 1*/
	err = DACApi::dac_register_read(AD917X_SYSREF_ROTATION_REG, &tmp_reg);
	if (err != API_ERROR_OK)
		return err;
	err = DACApi::dac_register_write(AD917X_SYSREF_ROTATION_REG,
		(tmp_reg | AD917X_SYNC_RSV_EN | AD917X_PERIODIC_RST_EN | AD917X_ROTATION_MODE(0x1)));
	if (err != API_ERROR_OK)
		return err;
	/*Set JESD  Subclass 0 and Subclass 1 setting*/
	err = DACApi::dac_register_read(AD917X_JESD_ILS_NP_REG, &tmp_reg);
	if (err != API_ERROR_OK)
		return err;
	tmp_reg = ((en == 1) ? (tmp_reg | AD917X_JESD_JESDV) :
		(tmp_reg & (~AD917X_JESD_JESDV)));
	err = DACApi::dac_register_write(AD917X_JESD_ILS_NP_REG, tmp_reg);
	if (err != API_ERROR_OK)
		return err;

	return API_ERROR_OK;
}

int32_t DACApi::dac_jesd_get_sysref_enable(uint8_t* en)
{
	int32_t err;
	uint32_t tmp_reg;

	if (DACApi::h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;

	if (en == NULL)
		return API_ERROR_INVALID_PARAM;

	err = DACApi::dac_register_read(AD917X_SYSREF_CTRL_REG, &tmp_reg);
	if (err != API_ERROR_OK)
		return err;
	*en = !(tmp_reg & AD917X_SYSREF_PD);

	return API_ERROR_OK;
}

int32_t DACApi::dac_jesd_set_scrambler_enable(uint8_t en)
{
	int32_t err;
	uint32_t tmp_reg;
	uint8_t i, link_count;

	if (DACApi::h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;

	if (en > 0x1)
		return API_ERROR_INVALID_PARAM;
	err = jesd_get_link_count(&link_count);
	if (err != API_ERROR_OK)
		return err;
	for (i = 0; i < link_count; i++) {
		err = jesd_set_link(i);
		if (err != API_ERROR_OK)
			return err;

		err = DACApi::dac_register_read(AD917X_JESD_ILS_SCR_L_REG, &tmp_reg);
		if (err != API_ERROR_OK)
			return err;
		tmp_reg &= ~AD917X_JESD_SCR;
		tmp_reg |= ((en == 1) ? AD917X_JESD_SCR : 0);
		err = DACApi::dac_register_write(AD917X_JESD_ILS_SCR_L_REG, tmp_reg);
		if (err != API_ERROR_OK)
			return err;
	}
	err = jesd_set_link(0);
	if (err != API_ERROR_OK)
		return err;
	return API_ERROR_OK;
}

int32_t DACApi::dac_jesd_enable_datapath(uint8_t lanes_msk, uint8_t run_cal, uint8_t en)
{
	int32_t err;
	uint32_t tmp_reg;

	if (DACApi::h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;
	if (DACApi::h->delay_us == NULL)
		return API_ERROR_INVALID_DELAYUS_PTR;

	if ((en > 1) || (run_cal > 1))
		return API_ERROR_INVALID_PARAM;

	if (en == 0x1) {
		/*Enable JESD Block*/
		err = DACApi::dac_register_write(AD917X_DIG_RESET_REG, AD917X_DIG_PATH_PDN(0));
		if (err != API_ERROR_OK)
			return err;

		if (run_cal == 0x1) {
			/*Calibrate SERDES PHY Termination Block*/
			/*Initialise Equaliser*/
			DACApi::dac_register_write_tbl(&ADI_REC_EQ_INIT_TBL[0],
				NO_OS_ARRAY_SIZE(ADI_REC_EQ_INIT_TBL));
		}
		/*Power Down Any unused Lanes*/
		tmp_reg = ~lanes_msk;
		err = DACApi::dac_register_write(AD917X_PHY_PD_REG, tmp_reg);
		if (err != API_ERROR_OK)
			return err;



		/*Engineering Sample Silicon Configuration*/
		/*Config SERDES PLL with ADI RECOMMENDED Settings*/
		err = DACApi::dac_register_write_tbl(&ADI_REC_ES_SERDES_INIT_TBL_1[0],
			NO_OS_ARRAY_SIZE(ADI_REC_ES_SERDES_INIT_TBL_1));
		if (err != API_ERROR_OK)
			return err;
		err = DACApi::h->delay_us(DACApi::h->user_data, SERDES_PWRUP_DELAY);
		if (err != API_ERROR_OK)
			return err;
		/*Power Up Serdes*/
		err = DACApi::dac_register_write(AD917X_MASTER_PD_REG, AD917X_SERDES_PDN(0));
		if (err != API_ERROR_OK)
			return err;;

		/*Config SERDES PLL with ADI RECOMMENDED Settings*/
		err = DACApi::dac_register_write_tbl(&ADI_REC_ES_SERDES_INIT_TBL_2[0],
			NO_OS_ARRAY_SIZE(ADI_REC_ES_SERDES_INIT_TBL_2));
		if (err != API_ERROR_OK)
			return err;

		/*Enable SERDES PLL*/
		err = DACApi::dac_register_read(AD917X_PLL_EN_CTRL_REG, &tmp_reg);
		if (err != API_ERROR_OK)
			return err;
		tmp_reg |= AD917X_SERDES_PLL_STARTUP;
		err = DACApi::dac_register_write(AD917X_PLL_EN_CTRL_REG, tmp_reg);
		if (err != API_ERROR_OK)
			return err;
	}
	else {

		/*TODO: Procedure to disable Reset JESD IF*/
		/*Disable SERDES PLL*/
		err = DACApi::dac_register_read(AD917X_PLL_EN_CTRL_REG, &tmp_reg);
		if (err != API_ERROR_OK)
			return err;
		tmp_reg &= ~(AD917X_SERDES_PLL_STARTUP);
		err = DACApi::dac_register_write(AD917X_PLL_EN_CTRL_REG, tmp_reg);
		if (err != API_ERROR_OK)
			return err;

		/*Power DOWN Serdes*/
		err = DACApi::dac_register_write(AD917X_MASTER_PD_REG, AD917X_SERDES_PDN(1));
		if (err != API_ERROR_OK)
			return err;;
		/*Power Down ALL Lanes*/
		err = DACApi::dac_register_write(AD917X_PHY_PD_REG, 0xFF);
		if (err != API_ERROR_OK)
			return err;

		/*Disable JESD Block*/
		err = DACApi::dac_register_write(AD917X_DIG_RESET_REG, AD917X_DIG_PATH_PDN(1));
		if (err != API_ERROR_OK)
			return err;
	}

	return API_ERROR_OK;
}

int32_t DACApi::dac_jesd_set_syncoutb_enable(jesd_syncoutb_t syncoutb, uint8_t en)
{
	int32_t err;
	uint32_t tmp_reg;

	if (DACApi::h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;

	if ((en > 1) ||
		((syncoutb > SYNCOUTB_INDEX_MAX) && (syncoutb != SYNCOUTB_ALL)))
		return API_ERROR_INVALID_PARAM;
	err = DACApi::dac_register_read(AD917X_GEN_PD_REG, &tmp_reg);
	if (err != API_ERROR_OK)
		return err;

	switch (syncoutb) {
	case SYNCOUTB_0:
		tmp_reg = ((en == 1) ? (tmp_reg & (~AD917X_SYNCOUTB_0_PD)) :
			(tmp_reg | (AD917X_SYNCOUTB_0_PD)));
		break;
	case SYNCOUTB_1:
		tmp_reg = ((en == 1) ? (tmp_reg & (~AD917X_SYNCOUTB_1_PD)) :
			(tmp_reg | (AD917X_SYNCOUTB_1_PD)));
		break;
	case SYNCOUTB_ALL:
		tmp_reg = ((en == 1) ? (~(SYNCOUTB_INDEX(syncoutb))) :
			SYNCOUTB_INDEX(syncoutb));
		break;
	default:
		return API_ERROR_INVALID_PARAM;
	}

	return DACApi::dac_register_write(AD917X_GEN_PD_REG, tmp_reg);
}

int32_t DACApi::dac_jesd_get_pll_status(uint32_t* pll_status)
{
	int32_t err;

	if (DACApi::h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;
	if (pll_status == NULL)
		return API_ERROR_INVALID_PARAM;

	err = DACApi::dac_register_read(AD917X_PLL_STATUS_REG, pll_status);
	if (err != API_ERROR_OK)
		return err;
	return API_ERROR_OK;
}

int32_t DACApi::dac_jesd_set_lane_xbar(uint8_t logical_lane, uint8_t physical_lane)
{
	int32_t err;
	uint32_t tmp_reg_val;
	uint16_t tmp_reg_addr;
	uint8_t tmp_nibble;

	if (DACApi::h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;

	if ((physical_lane > (LANE_INDEX_MAX)) || (logical_lane > LANE_INDEX_MAX))
		return API_ERROR_INVALID_PARAM;

	tmp_reg_addr = (AD917X_JESD_XBAR_LANE_REG + (logical_lane / 2));
	tmp_nibble = ((logical_lane % 2 > 0) ? 1 : 0);

	err = DACApi::dac_register_read(tmp_reg_addr, &tmp_reg_val);
	if (err != API_ERROR_OK)
		return err;
	if (tmp_nibble == 0) {
		tmp_reg_val &= ~AD917X_XBAR_LANE_EVEN(ALL);
		tmp_reg_val |= AD917X_XBAR_LANE_EVEN(physical_lane);
	}
	else {
		tmp_reg_val &= ~AD917X_XBAR_LANE_ODD(ALL);
		tmp_reg_val |= AD917X_XBAR_LANE_ODD(physical_lane);
	}
	err = DACApi::dac_register_write(tmp_reg_addr, tmp_reg_val);
	if (err != API_ERROR_OK)
		return err;

	return API_ERROR_OK;
}

int32_t DACApi::dac_jesd_invert_lane(uint8_t logical_lane, uint8_t invert)
{
	int32_t err;
	uint32_t tmp_reg;

	if (DACApi::h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;

	if ((logical_lane > LANE_INDEX_MAX) || (invert > 1))
		return API_ERROR_INVALID_PARAM;

	err = DACApi::dac_register_read(AD917X_JESD_INVERT_LANE_REG, &tmp_reg);
	if (err != API_ERROR_OK)
		return err;

	if (invert)
		tmp_reg |= AD917X_JESD_INVERT_LANE(logical_lane);
	else
		tmp_reg &= ~AD917X_JESD_INVERT_LANE(logical_lane);

	err = DACApi::dac_register_write(AD917X_JESD_INVERT_LANE_REG, tmp_reg);
	if (err != API_ERROR_OK)
		return err;

	return API_ERROR_OK;
}

int32_t DACApi::dac_jesd_get_lane_xbar(uint8_t* phy_log_map)
{
	int32_t i, err;
	uint32_t tmp_reg;

	if (DACApi::h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;

	if (phy_log_map == NULL)
		return API_ERROR_INVALID_PARAM;

	for (i = 0; i < AD917X_JESD_NOF_LANES / 2; i++) {

		err = DACApi::dac_register_read(AD917X_JESD_XBAR_LANE_REG + i, &tmp_reg);
		if (err != API_ERROR_OK)
			return err;
		phy_log_map[(2 * i)] = tmp_reg & AD917X_XBAR_LANE_EVEN(ALL);
		phy_log_map[(2 * i) + 1] = ((tmp_reg & AD917X_XBAR_LANE_ODD(ALL)) >> 3);
	}

	return API_ERROR_OK;
}

int32_t DACApi::dac_jesd_enable_link(jesd_link_t link, uint8_t en)
{
	int32_t err;
	uint32_t tmp_reg;

	if (DACApi::h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;

	if ((en > 0x1) || ((link > (LINK_INDEX_MAX) && (link != JESD_LINK_ALL))))
		return API_ERROR_INVALID_PARAM;

	err = DACApi::dac_register_read(AD917X_JESD_RX_CTL_REG, &tmp_reg);
	if (err != API_ERROR_OK)
		return err;
	if (en)
		tmp_reg |= AD917X_LINK_EN(LINK_INDEX(link));
	else
		tmp_reg &= ~AD917X_LINK_EN(LINK_INDEX(link));
	err = DACApi::dac_register_write(AD917X_JESD_RX_CTL_REG, tmp_reg);
	if (err != API_ERROR_OK)
		return err;

	return API_ERROR_OK;
}

int32_t DACApi::dac_jesd_get_link_status(jesd_link_t link, dac_jesd_link_stat_t* link_status)
{
	int32_t err;
	uint32_t tmp_reg;

	if (DACApi::h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;
	if (link_status == NULL)
		return API_ERROR_INVALID_PARAM;

	err = jesd_set_link(link);
	if (err != API_ERROR_OK)
		return err;

	err = DACApi::dac_register_read(AD917X_JESD_CODE_GRP_SYNC_REG, &tmp_reg);
	if (err != API_ERROR_OK)
		return err;
	link_status->code_grp_sync_stat = tmp_reg;

	err = DACApi::dac_register_read(AD917X_JESD_FRAME_SYNC_REG, &tmp_reg);
	if (err != API_ERROR_OK)
		return err;
	link_status->frame_sync_stat = tmp_reg;

	err = DACApi::dac_register_read(AD917X_JESD_GOOD_CHECKSUM_REG, &tmp_reg);
	if (err != API_ERROR_OK)
		return err;
	link_status->good_checksum_stat = tmp_reg;

	err = DACApi::dac_register_read(AD917X_JESD_INIT_LANE_SYNC_REG, &tmp_reg);
	if (err != API_ERROR_OK)
		return err;
	link_status->init_lane_sync_stat = tmp_reg;

	return API_ERROR_OK;
}

int32_t DACApi::dac_jesd_set_lmfc_delay(jesd_link_t link, uint8_t delay, uint8_t var)
{
	int32_t err;

	if (DACApi::h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;
	if ((delay > AD917X_JESD_LMFC_DELAY(ALL)) ||
		(var > AD917X_LMFC_VAR_MAX))
		return API_ERROR_INVALID_PARAM;
	if ((link > (LINK_INDEX_MAX)) && (link != JESD_LINK_ALL))
		return API_ERROR_INVALID_PARAM;

	if ((link == JESD_LINK_0) || (link == JESD_LINK_ALL)) {
		err = DACApi::dac_register_write(AD917X_JESD_LMFC_DELAY0_REG,
			AD917X_JESD_LMFC_DELAY(delay));
		if (err != API_ERROR_OK)
			return err;
		err = DACApi::dac_register_write(AD917X_JESD_LMFC_VAR0_REG,
			AD917X_JESD_LMFC_VAR(var));
		if (err != API_ERROR_OK)
			return err;

	}

	if ((link == JESD_LINK_1) || (link == JESD_LINK_ALL)) {
		err = DACApi::dac_register_write(AD917X_JESD_LMFC_DELAY1_REG,
			AD917X_JESD_LMFC_DELAY(delay));
		if (err != API_ERROR_OK)
			return err;
		err = DACApi::dac_register_write(AD917X_JESD_LMFC_VAR1_REG,
			AD917X_JESD_LMFC_VAR(var));
		if (err != API_ERROR_OK)
			return err;
	}

	return API_ERROR_OK;
}




// SPDX-License-Identifier: GPL-2.0
/**
 * \file dac_nco_api.c
 *
 * \brief Contains AD917x APIs for NCO configuration and control
 *
 * Release 1.1.X
 *
 * Copyright(c) 2017 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
 */

static int32_t adi_api_utils_gcd(int32_t u, int32_t v)
{
	int32_t t;
	while (v != 0) {
		t = u;
		u = v;
		v = t % v;
	}
	return u < 0 ? -u : u; /* abs(u) */
}

static void adi_api_utils_subt_128(uint64_t ah, uint64_t al,
	uint64_t bh, uint64_t bl,
	uint64_t* hi, uint64_t* lo)
{
	/* r = a - b*/
	uint64_t rl, rh;
	if (bl <= al) {
		rl = al - bl;
		rh = ah - bh;
	}
	else {
		rl = bl - al - 1;
		rl = 0xFFFFFFFFFFFFFFFFll - rl;
		ah--;
		rh = ah - bh;
	}

	*lo = rl;
	*hi = rh;
}

static void adi_api_utils_add_128(uint64_t ah, uint64_t al,
	uint64_t bh, uint64_t bl,
	uint64_t* hi, uint64_t* lo)
{
	/* r = a - b*/
	uint64_t rl, rh;
	rl = al + bl;
	rh = ah + bh;

	if (rl < al)
		rh++;

	*lo = rl;
	*hi = rh;
}

static void adi_api_utils_lshift_128(uint64_t* hi, uint64_t* lo)
{
	*hi <<= 1;
	if (*lo & U64MSB)
		*hi |= 1ul;
	*lo <<= 1;
}

static void adi_api_utils_rshift_128(uint64_t* hi, uint64_t* lo)
{
	*lo >>= 1;
	if (*hi & 1u)
		*lo |= U64MSB;
	*hi >>= 1;
}

static void adi_api_utils_mult_128(uint64_t a, uint64_t b, uint64_t* hi,
	uint64_t* lo)
{
	uint64_t    ah = (a >> 32), al = a & 0xffffffff,
		bh = (b >> 32), bl = b & 0xffffffff,
		rh = (ah * bh), rl = (al * bl),
		rm1 = ah * bl, rm2 = al * bh,
		rm1h = rm1 >> 32, rm2h = rm2 >> 32,
		rm1l = rm1 & 0xffffffff, rm2l = rm2 & 0xffffffff,
		rmh = rm1h + rm2h, rml = rm1l + rm2l,
		c = ((rl >> 32) + rml) >> 32;

	rl = rl + (rml << 32);
	rh = rh + rmh + c;
	*lo = rl;
	*hi = rh;
}

static void adi_api_utils_div_128(uint64_t a_hi, uint64_t a_lo,
	uint64_t b_hi, uint64_t b_lo,
	uint64_t* hi, uint64_t* lo)
{
	uint64_t remain_lo =
		a_lo; /* The left-hand side of division, i.e. what is being divided */
	uint64_t remain_hi =
		a_hi; /* The left-hand side of division, i.e. what is being divided */
	uint64_t part1_lo = b_lo; /* The right-hand side of division */
	uint64_t part1_hi = b_hi; /* The right-hand side of division */
	uint64_t result_lo = 0;
	uint64_t result_hi = 0;
	uint64_t mask_lo = 1;
	uint64_t mask_hi = 0;

	if ((part1_lo == 0) && (part1_hi == 0)) {
		/* Do whatever should happen when dividing by zero. */
		return;
	}

	/* while(part1_lo < remain_lo)
	 * Alternative: while(!(part1 & 0x8000)) - For 16-bit, test highest order bit.
	 * Alternative: while(not_signed(part1)) - Same as above: As long as sign bit is not set in part1. */
	while (!(part1_hi & U64MSB)) {
		adi_api_utils_lshift_128(&part1_hi, &part1_lo);
		adi_api_utils_lshift_128(&mask_hi, &mask_lo);
	}

	do {
		if ((remain_hi > part1_hi) || ((remain_hi == part1_hi) &&
			(remain_lo >= part1_lo))) {
			/* remain_lo = remain_lo - part1_lo; */
			adi_api_utils_subt_128(remain_hi, remain_lo, part1_hi, part1_lo, &remain_hi,
				&remain_lo);
			/* result = result + mask; */
			adi_api_utils_add_128(result_hi, result_lo, mask_hi, mask_lo, &result_hi,
				&result_lo);
		}
		/* part1 = part1 >> 1; */
		adi_api_utils_rshift_128(&part1_hi, &part1_lo);
		/* mask  = mask  >> 1; */
		adi_api_utils_rshift_128(&mask_hi, &mask_lo);
	} while ((mask_hi != 0) || (mask_lo != 0));

	/* Now: result = division result (quotient)
	 *      remain_lo = division remain_loder (modulo) */
	*lo = result_lo;
	*hi = result_hi;
}


static int32_t dac_nco_calc_freq_int_main(uint64_t int_part, int64_t* carrier_freq_hz)
{
	uint64_t tmpa_lo, tmpa_hi;
	adi_api_utils_mult_128(int_part, DACApi::h->dac_freq_hz, &tmpa_hi, &tmpa_lo);
	adi_api_utils_div_128(tmpa_hi, tmpa_lo, 0, ADI_POW2_48, &tmpa_hi, &tmpa_lo);
	*carrier_freq_hz = tmpa_lo;
	return API_ERROR_OK;
}

static int32_t dac_nco_calc_freq_fract_main(uint64_t int_part, uint64_t frac_part_a,
	uint64_t frac_part_b, int64_t* carrier_freq_hz)
{
	uint64_t tmpa_lo, tmpa_hi;
	uint64_t tmpb_lo, tmpb_hi;
	adi_api_utils_mult_128(int_part, DACApi::h->dac_freq_hz, &tmpa_hi, &tmpa_lo);
	adi_api_utils_mult_128(frac_part_a, DACApi::h->dac_freq_hz, &tmpb_hi, &tmpb_lo);
	adi_api_utils_div_128(tmpb_hi, tmpb_lo, 0, frac_part_b, &tmpb_hi, &tmpb_lo);
	adi_api_utils_add_128(tmpa_hi, tmpa_lo, tmpb_hi, tmpb_lo, &tmpa_hi, &tmpa_lo);
	adi_api_utils_div_128(tmpa_hi, tmpa_lo, 0, ADI_POW2_48, &tmpa_hi, &tmpa_lo);
	*carrier_freq_hz = tmpa_lo;
	return API_ERROR_OK;
}


static int32_t dac_nco_get_freq(const dac_dds_select_t nco, dac_dac_select_t dac,
	dac_channel_select_t channel, int64_t* carrier_freq_hz)
{
	int32_t err, mod_en = 0;
	uint32_t tmp_reg;

	/* Check if modulus is used */
	switch (nco) {
	case AD917X_DDSM:
		err = DACApi::dac_register_read(AD917X_DDSM_DATAPATH_CFG_REG, &tmp_reg);
		if (err != API_ERROR_OK)
			return err;
		if (tmp_reg & AD917X_DDSM_MODULUS_EN)
			mod_en = 1;
		/* DAC PAGE */
		tmp_reg = AD917X_CHANNEL_PAGE_0;
		switch (dac) {
		case AD917X_DAC0:
			tmp_reg |= AD917X_MAINDAC_PAGE_0;
			break;
		case AD917X_DAC1:
			tmp_reg |= AD917X_MAINDAC_PAGE_1;
			break;
		default:
			return API_ERROR_INVALID_PARAM;
		}
		err = DACApi::dac_register_write(AD917X_SPI_PAGEINDX_REG, tmp_reg);
		if (err != API_ERROR_OK)
			return err;
		break;
	case AD917X_DDSC:
		err = DACApi::dac_register_read(AD917X_DDSC_DATAPATH_CFG_REG, &tmp_reg);
		if (err != API_ERROR_OK)
			return err;
		if (tmp_reg & AD917X_DDSC_MODULUS_EN)
			mod_en = 1;
		/* Channel PAGE */
		err = DACApi::dac_register_write(AD917X_SPI_PAGEINDX_REG, (channel & 0x3F));
		if (err != API_ERROR_OK)
			return err;
		break;
	default:
		return API_ERROR_INVALID_PARAM;
	}

	if (mod_en != 0) {
		/* modulus enabled */
		uint64_t int_part;
		uint64_t frac_part_a;
		uint64_t frac_part_b;
		err = DACApi::dac_nco_get_ftw(nco, &int_part, &frac_part_a, &frac_part_b);
		if (err != API_ERROR_OK)
			return err;
		if ((frac_part_a == 0) || (frac_part_b == 0)) {
			/* Division by 0 = Modulus is off. */
			err = dac_nco_calc_freq_int_main(int_part, carrier_freq_hz);
			if (err != API_ERROR_OK)
				return err;
			return API_ERROR_OK;
		}
		err = dac_nco_calc_freq_fract_main(int_part, frac_part_a,
			frac_part_b, carrier_freq_hz);
		if (err != API_ERROR_OK)
			return err;
	}
	else {
		/* No modulus used */
		/* Get FTW */
		uint64_t int_part;
		err = DACApi::dac_nco_get_ftw(nco, &int_part, NULL, NULL);
		if (err != API_ERROR_OK)
			return err;
		err = dac_nco_calc_freq_int_main(int_part, carrier_freq_hz);
		if (err != API_ERROR_OK)
			return err;
	}
	return API_ERROR_OK;
}


int32_t DACApi::dac_nco_set_phase_offset(const dac_dac_select_t dacs, const uint16_t dacs_po,
	const dac_channel_select_t channels, const uint16_t ch_po)
{
	int32_t err;
	uint32_t tmp_reg = 0;

	if (DACApi::h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;
	if (dacs != 0) {
		/* DAC PAGE */
		if (dacs & AD917X_DAC0)
			tmp_reg |= AD917X_MAINDAC_PAGE_0;
		if (dacs & AD917X_DAC1)
			tmp_reg |= AD917X_MAINDAC_PAGE_1;
	}
	if (channels != 0) {
		/* Channel PAGE */
		tmp_reg |= (channels & 0x3F);
	}
	err = DACApi::dac_register_write(AD917X_SPI_PAGEINDX_REG, tmp_reg);
	if (err != API_ERROR_OK)
		return err;
	if (dacs != 0) {
		/* DAC PAGE */
		err = DACApi::dac_register_write(AD917X_DDSM_PHASE_OFFSET0_REG,
			ADI_GET_BYTE(dacs_po, 0));
		if (err != API_ERROR_OK)
			return err;
		err = DACApi::dac_register_write(AD917X_DDSM_PHASE_OFFSET1_REG,
			ADI_GET_BYTE(dacs_po, 8));
		if (err != API_ERROR_OK)
			return err;
	}
	if (channels != 0) {
		/* Channel PAGE */
		err = DACApi::dac_register_write(AD917X_DDSC_PHASE_OFFSET0_REG,
			ADI_GET_BYTE(ch_po, 0));
		if (err != API_ERROR_OK)
			return err;
		err = DACApi::dac_register_write(AD917X_DDSC_PHASE_OFFSET1_REG,
			ADI_GET_BYTE(ch_po, 8));
		if (err != API_ERROR_OK)
			return err;
	}

	return API_ERROR_OK;
}

int32_t DACApi::dac_nco_get_phase_offset(
	const dac_dac_select_t dacs, uint16_t* dacs_po,
	const dac_channel_select_t channels, uint16_t* ch_po)
{
	int32_t err;
	uint32_t tmp_reg = 0;
	if (DACApi::h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;
	if ((dacs_po == NULL) || (ch_po == NULL))
		return API_ERROR_INVALID_PARAM;
	if (dacs != 0) {
		/* DAC PAGE */
		if (dacs & AD917X_DAC0)
			tmp_reg |= AD917X_MAINDAC_PAGE_0;
		if (dacs & AD917X_DAC1)
			tmp_reg |= AD917X_MAINDAC_PAGE_1;
	}
	if (channels != 0) {
		/* Channel PAGE */
		tmp_reg |= (channels & 0x3F);
	}
	err = DACApi::dac_register_write(AD917X_SPI_PAGEINDX_REG, tmp_reg);
	if (err != API_ERROR_OK)
		return err;
	if (dacs != 0) {
		/* DAC PAGE */
		err = DACApi::dac_register_read(AD917X_DDSM_PHASE_OFFSET1_REG, &tmp_reg);
		if (err != API_ERROR_OK)
			return err;
		*dacs_po = tmp_reg;
		*dacs_po <<= 8;
		err = DACApi::dac_register_read(AD917X_DDSM_PHASE_OFFSET0_REG, &tmp_reg);
		if (err != API_ERROR_OK)
			return err;
		*dacs_po |= tmp_reg;
	}
	if (channels != 0) {
		/* Channel PAGE */
		err = DACApi::dac_register_read(AD917X_DDSC_PHASE_OFFSET1_REG, &tmp_reg);
		if (err != API_ERROR_OK)
			return err;
		*ch_po = tmp_reg;
		*ch_po <<= 8;
		err = DACApi::dac_register_read(AD917X_DDSC_PHASE_OFFSET0_REG, &tmp_reg);
		if (err != API_ERROR_OK)
			return err;
		*ch_po |= tmp_reg;
	}

	return API_ERROR_OK;
}

int32_t DACApi::dac_nco_set_ftw(
	const dac_dds_select_t dds,
	const uint64_t ftw, const uint64_t acc_modulus,
	const uint64_t acc_delta)
{
	int32_t err;
	uint32_t tmp_reg;
	if (DACApi::h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;
	if ((dds != AD917X_DDSM) && (dds != AD917X_DDSC))
		return API_ERROR_INVALID_PARAM;

	/* Set FTW and MOD for the main NCO */
	err = DACApi::dac_register_read(AD917X_X_FTW_UPDATE_REG(dds), &tmp_reg);
	if (err != API_ERROR_OK)
		return err;
	tmp_reg &= ~AD917X_DDSM_FTW_LOAD_REQ;
	err = DACApi::dac_register_write(AD917X_X_FTW_UPDATE_REG(dds), tmp_reg);
	if (err != API_ERROR_OK)
		return err;

	err = DACApi::dac_register_write(AD917X_X_FTW0_REG(dds), ADI_GET_BYTE(ftw, 0));
	if (err != API_ERROR_OK)
		return err;
	err = DACApi::dac_register_write(AD917X_X_FTW1_REG(dds), ADI_GET_BYTE(ftw, 8));
	if (err != API_ERROR_OK)
		return err;
	err = DACApi::dac_register_write(AD917X_X_FTW2_REG(dds), ADI_GET_BYTE(ftw, 16));
	if (err != API_ERROR_OK)
		return err;
	err = DACApi::dac_register_write(AD917X_X_FTW3_REG(dds), ADI_GET_BYTE(ftw, 24));
	if (err != API_ERROR_OK)
		return err;
	err = DACApi::dac_register_write(AD917X_X_FTW4_REG(dds), ADI_GET_BYTE(ftw, 32));
	if (err != API_ERROR_OK)
		return err;
	err = DACApi::dac_register_write(AD917X_X_FTW5_REG(dds), ADI_GET_BYTE(ftw, 40));
	if (err != API_ERROR_OK)
		return err;
	if ((acc_modulus != 0) && (acc_delta != 0)) {
		err = DACApi::dac_register_write(AD917X_X_ACC_DELTA0_REG(dds),
			ADI_GET_BYTE(acc_delta, 0));
		if (err != API_ERROR_OK)
			return err;
		err = DACApi::dac_register_write(AD917X_X_ACC_DELTA1_REG(dds),
			ADI_GET_BYTE(acc_delta, 8));
		if (err != API_ERROR_OK)
			return err;
		err = DACApi::dac_register_write(AD917X_X_ACC_DELTA2_REG(dds),
			ADI_GET_BYTE(acc_delta, 16));
		if (err != API_ERROR_OK)
			return err;
		err = DACApi::dac_register_write(AD917X_X_ACC_DELTA3_REG(dds),
			ADI_GET_BYTE(acc_delta, 24));
		if (err != API_ERROR_OK)
			return err;
		err = DACApi::dac_register_write(AD917X_X_ACC_DELTA4_REG(dds),
			ADI_GET_BYTE(acc_delta, 32));
		if (err != API_ERROR_OK)
			return err;
		err = DACApi::dac_register_write(AD917X_X_ACC_DELTA5_REG(dds),
			ADI_GET_BYTE(acc_delta, 40));
		if (err != API_ERROR_OK)
			return err;
		err = DACApi::dac_register_write(AD917X_X_ACC_MODULUS0_REG(dds),
			ADI_GET_BYTE(acc_modulus, 0));
		if (err != API_ERROR_OK)
			return err;
		err = DACApi::dac_register_write(AD917X_X_ACC_MODULUS1_REG(dds),
			ADI_GET_BYTE(acc_modulus, 8));
		if (err != API_ERROR_OK)
			return err;
		err = DACApi::dac_register_write(AD917X_X_ACC_MODULUS2_REG(dds),
			ADI_GET_BYTE(acc_modulus, 16));
		if (err != API_ERROR_OK)
			return err;
		err = DACApi::dac_register_write(AD917X_X_ACC_MODULUS3_REG(dds),
			ADI_GET_BYTE(acc_modulus, 24));
		if (err != API_ERROR_OK)
			return err;
		err = DACApi::dac_register_write(AD917X_X_ACC_MODULUS4_REG(dds),
			ADI_GET_BYTE(acc_modulus, 32));
		if (err != API_ERROR_OK)
			return err;
		err = DACApi::dac_register_write(AD917X_X_ACC_MODULUS5_REG(dds),
			ADI_GET_BYTE(acc_modulus, 40));
		if (err != API_ERROR_OK)
			return err;
	}
	/* FTW_LOAD_REQ (rising edge )*/
	tmp_reg |= AD917X_DDSM_FTW_LOAD_REQ;
	err = DACApi::dac_register_write(AD917X_X_FTW_UPDATE_REG(dds), tmp_reg);
	if (err != API_ERROR_OK)
		return err;
	/* FTW_LOAD_ACK check */
	err = DACApi::dac_register_read(AD917X_X_FTW_UPDATE_REG(dds), &tmp_reg);
	if (err != API_ERROR_OK)
		return err;
	if ((tmp_reg & AD917X_DDSM_FTW_LOAD_ACK) != 0)
		return API_ERROR_OK;
	return API_ERROR_FTW_LOAD_ACK;
}

int32_t DACApi::dac_nco_get_ftw(const dac_dds_select_t dds,
	uint64_t* ftw, uint64_t* acc_modulus,
	uint64_t* acc_delta)
{
	uint32_t tmp_reg;
	int32_t err;
	if (DACApi::h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;
	if (ftw == NULL)
		return API_ERROR_INVALID_PARAM;
	if ((dds != AD917X_DDSM) && (dds != AD917X_DDSC))
		return API_ERROR_INVALID_PARAM;

	/* Get FTW for the main NCO */
	err = DACApi::DACApi::dac_register_read(AD917X_X_FTW5_REG(dds), &tmp_reg);
	if (err != API_ERROR_OK)
		return err;
	*ftw = tmp_reg;
	(*ftw) <<= 8;
	err = DACApi::dac_register_read(AD917X_X_FTW4_REG(dds), &tmp_reg);
	if (err != API_ERROR_OK)
		return err;
	*ftw |= tmp_reg;
	(*ftw) <<= 8;
	err = DACApi::dac_register_read(AD917X_X_FTW3_REG(dds), &tmp_reg);
	if (err != API_ERROR_OK)
		return err;
	*ftw |= tmp_reg;
	(*ftw) <<= 8;
	err = DACApi::dac_register_read(AD917X_X_FTW2_REG(dds), &tmp_reg);
	if (err != API_ERROR_OK)
		return err;
	*ftw |= tmp_reg;
	(*ftw) <<= 8;
	err = DACApi::dac_register_read(AD917X_X_FTW1_REG(dds), &tmp_reg);
	if (err != API_ERROR_OK)
		return err;
	*ftw |= tmp_reg;
	(*ftw) <<= 8;
	err = DACApi::dac_register_read(AD917X_X_FTW0_REG(dds), &tmp_reg);
	if (err != API_ERROR_OK)
		return err;
	*ftw |= tmp_reg;

	/* Get the MODULUS if needed */
	if ((acc_modulus != NULL) && (acc_delta != NULL)) {
		err = DACApi::dac_register_read(AD917X_X_ACC_DELTA5_REG(dds), &tmp_reg);
		if (err != API_ERROR_OK)
			return err;
		*acc_delta = tmp_reg;
		(*acc_delta) <<= 8;
		err = DACApi::dac_register_read(AD917X_X_ACC_DELTA4_REG(dds), &tmp_reg);
		if (err != API_ERROR_OK)
			return err;
		*acc_delta |= tmp_reg;
		(*acc_delta) <<= 8;
		err = DACApi::dac_register_read(AD917X_X_ACC_DELTA3_REG(dds), &tmp_reg);
		if (err != API_ERROR_OK)
			return err;
		*acc_delta |= tmp_reg;
		(*acc_delta) <<= 8;
		err = DACApi::dac_register_read(AD917X_X_ACC_DELTA2_REG(dds), &tmp_reg);
		if (err != API_ERROR_OK)
			return err;
		*acc_delta |= tmp_reg;
		(*acc_delta) <<= 8;
		err = DACApi::dac_register_read(AD917X_X_ACC_DELTA1_REG(dds), &tmp_reg);
		if (err != API_ERROR_OK)
			return err;
		*acc_delta |= tmp_reg;
		(*acc_delta) <<= 8;
		err = DACApi::dac_register_read(AD917X_X_ACC_DELTA0_REG(dds), &tmp_reg);
		if (err != API_ERROR_OK)
			return err;
		*acc_delta |= tmp_reg;

		err = DACApi::dac_register_read(AD917X_X_ACC_MODULUS5_REG(dds), &tmp_reg);
		if (err != API_ERROR_OK)
			return err;
		*acc_modulus = tmp_reg;
		(*acc_modulus) <<= 8;
		err = DACApi::dac_register_read(AD917X_X_ACC_MODULUS4_REG(dds), &tmp_reg);
		if (err != API_ERROR_OK)
			return err;
		*acc_modulus |= tmp_reg;
		(*acc_modulus) <<= 8;
		err = DACApi::dac_register_read(AD917X_X_ACC_MODULUS3_REG(dds), &tmp_reg);
		if (err != API_ERROR_OK)
			return err;
		*acc_modulus |= tmp_reg;
		(*acc_modulus) <<= 8;
		err = DACApi::dac_register_read(AD917X_X_ACC_MODULUS2_REG(dds), &tmp_reg);
		if (err != API_ERROR_OK)
			return err;
		*acc_modulus |= tmp_reg;
		(*acc_modulus) <<= 8;
		err = DACApi::dac_register_read(AD917X_X_ACC_MODULUS1_REG(dds), &tmp_reg);
		if (err != API_ERROR_OK)
			return err;
		*acc_modulus |= tmp_reg;
		(*acc_modulus) <<= 8;
		err = DACApi::dac_register_read(AD917X_X_ACC_MODULUS0_REG(dds), &tmp_reg);
		if (err != API_ERROR_OK)
			return err;
		*acc_modulus |= tmp_reg;
	}
	return API_ERROR_OK;
}

int32_t DACApi::dac_set_channel_gain(const uint16_t gain)
{
	int32_t err;
	if (DACApi::h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;

	err = DACApi::dac_register_write(AD917X_CHNL_GAIN0_REG, CHANNEL_GAIN0(gain));
	if (err != API_ERROR_OK)
		return err;
	err = DACApi::dac_register_write(AD917X_CHNL_GAIN1_REG, CHANNEL_GAIN1(gain));
	if (err != API_ERROR_OK)
		return err;
	return API_ERROR_OK;
}

int32_t DACApi::dac_set_dc_cal_tone_amp(const uint16_t amp)
{
	int32_t err;
	if (DACApi::h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;

	err = DACApi::dac_register_write(AD917X_DC_CAL_TONE0_REG, (amp & 0xFF));
	if (err != API_ERROR_OK)
		return err;
	err = DACApi::dac_register_write(AD917X_DC_CAL_TONE1_REG, ((amp >> 8) & 0xFF));
	if (err != API_ERROR_OK)
		return err;
	return API_ERROR_OK;
}

int32_t DACApi::dac_get_channel_gain(uint16_t* gain)
{
	int32_t err;
	uint32_t gain0, gain1;
	if (DACApi::h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;
	if (gain == NULL)
		return API_ERROR_INVALID_PARAM;
	err = DACApi::dac_register_read(AD917X_CHNL_GAIN0_REG, &gain0);
	if (err != API_ERROR_OK)
		return err;
	err = DACApi::dac_register_read(AD917X_CHNL_GAIN1_REG, &gain1);
	if (err != API_ERROR_OK)
		return err;
	*gain = CHANNEL_GAIN(gain0, gain1);
	return API_ERROR_OK;
}

int32_t DACApi::dac_nco_enable(const dac_dac_select_t dacs, const dac_channel_select_t channels)
{
	int32_t err;
	uint32_t tmp_reg;
	uint32_t ddsc_datapath_cfg, ddsm_datapath_cfg;

	err = DACApi::dac_register_read(AD917X_DDSC_DATAPATH_CFG_REG, &ddsc_datapath_cfg);
	if (err != API_ERROR_OK)
		return err;

	err = DACApi::dac_register_read(AD917X_DDSM_DATAPATH_CFG_REG, &ddsm_datapath_cfg);
	if (err != API_ERROR_OK)
		return err;

	tmp_reg = AD917X_CHANNEL_PAGE_1;
	//Disable all DACs
	err = DACApi::dac_register_write(AD917X_SPI_PAGEINDX_REG, tmp_reg | AD917X_MAINDAC_PAGE_0 | AD917X_MAINDAC_PAGE_1);
	if (err != API_ERROR_OK)
		return err;
	ddsm_datapath_cfg &= ~AD917X_DDSM_NCO_EN;
	err = DACApi::dac_register_write(AD917X_DDSM_DATAPATH_CFG_REG, ddsm_datapath_cfg);
	if (err != API_ERROR_OK)
		return err;

	if (dacs != 0) {
		/* DAC PAGE */
		if (dacs & AD917X_DAC0)
			tmp_reg |= AD917X_MAINDAC_PAGE_0;
		if (dacs & AD917X_DAC1)
			tmp_reg |= AD917X_MAINDAC_PAGE_1;
		//tmp_reg = 0xff;
		err = DACApi::dac_register_write(AD917X_SPI_PAGEINDX_REG, tmp_reg);
		if (err != API_ERROR_OK)
			return err;

		ddsm_datapath_cfg |=  AD917X_DDSM_MODE(2); //bit 4,5 I Path Q Path
		err = DACApi::dac_register_write(AD917X_DDSM_DATAPATH_CFG_REG, ddsm_datapath_cfg);
		if (err != API_ERROR_OK)
			return err;
	}
	
	//Disable all channel NCOs
	err = DACApi::dac_register_write(AD917X_SPI_PAGEINDX_REG, 0x3F);
	if (err != API_ERROR_OK)
		return err;
	ddsc_datapath_cfg &= ~AD917X_DDSC_NCO_EN;
	err = DACApi::dac_register_write(AD917X_DDSC_DATAPATH_CFG_REG, ddsc_datapath_cfg);
	if (err != API_ERROR_OK)
		return err;

	if (channels != 0) {
		//Enable channel NCOs desired
		/* Channel PAGE */
		err = DACApi::dac_register_write(AD917X_SPI_PAGEINDX_REG, (channels & 0x3F));
		if (err != API_ERROR_OK)
			return err;
		ddsc_datapath_cfg |= AD917X_DDSC_NCO_EN;
		err = DACApi::dac_register_write(AD917X_DDSC_DATAPATH_CFG_REG, ddsc_datapath_cfg);
		if (err != API_ERROR_OK)
			return err;
	}
	return API_ERROR_OK;
}

int32_t DACApi::dac_dc_test_tone_set(int32_t dc_test_tone_en)
{
	int32_t err;
	uint32_t ddsc_datapath_cfg;
	if (DACApi::h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;
	/* DC TEST TONE EN */
	err = DACApi::dac_register_read(AD917X_DDSC_DATAPATH_CFG_REG, &ddsc_datapath_cfg);
	if (err != API_ERROR_OK)
		return err;
	if (dc_test_tone_en != 0)
		ddsc_datapath_cfg |= AD917X_DDSC_TEST_TONE_EN;
	else
		ddsc_datapath_cfg &= ~AD917X_DDSC_TEST_TONE_EN;
	err = DACApi::dac_register_write(AD917X_DDSC_DATAPATH_CFG_REG, ddsc_datapath_cfg);
	if (err != API_ERROR_OK)
		return err;
	return API_ERROR_OK;
}

int32_t DACApi::dac_dc_test_tone_get(int32_t* dc_test_tone_en)
{
	int32_t err;
	uint32_t ddsc_datapath_cfg;
	if (DACApi::h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;
	if (dc_test_tone_en == NULL)
		return API_ERROR_INVALID_PARAM;
	/* DC TEST TONE EN */
	err = DACApi::dac_register_read(AD917X_DDSC_DATAPATH_CFG_REG, &ddsc_datapath_cfg);
	if (err != API_ERROR_OK)
		return err;

	*dc_test_tone_en = 0;
	if (ddsc_datapath_cfg & AD917X_DDSC_TEST_TONE_EN)
		*dc_test_tone_en = 1;
	return API_ERROR_OK;
}

int32_t DACApi::dac_ddsm_cal_dc_input_set(int32_t ddsm_cal_dc_input_en)
{
	int32_t err;
	uint32_t tmp_reg;
	if (DACApi::h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;
	/*DDSM CAL EN */
	err = DACApi::dac_register_read(AD917X_DDSM_CAL_MODE_DEF_REG, &tmp_reg);
	if (err != API_ERROR_OK)
		return err;
	if (ddsm_cal_dc_input_en != 0)
		tmp_reg |= AD917X_DDSM_EN_CAL_DC_INPUT;
	else
		tmp_reg &= ~AD917X_DDSM_EN_CAL_DC_INPUT;
	err = DACApi::dac_register_write(AD917X_DDSM_CAL_MODE_DEF_REG, tmp_reg);
	if (err != API_ERROR_OK)
		return err;
	return API_ERROR_OK;
}

int32_t DACApi::dac_ddsm_cal_dc_input_get(int32_t* ddsm_cal_dc_input_en)
{
	int32_t err;
	uint32_t tmp_reg;
	if (DACApi::h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;
	if (ddsm_cal_dc_input_en == NULL)
		return API_ERROR_INVALID_PARAM;
	/*DDSM CAL EN */
	err = DACApi::dac_register_read(AD917X_DDSM_CAL_MODE_DEF_REG, &tmp_reg);
	if (err != API_ERROR_OK)
		return err;

	*ddsm_cal_dc_input_en = 0;
	if (tmp_reg & AD917X_DDSM_EN_CAL_DC_INPUT)
		*ddsm_cal_dc_input_en = 1;
	return API_ERROR_OK;
}

/**
	amplitude - amplitude full scale. 0xFFFF = full scale.
*/
int32_t DACApi::dac_nco_set(
	const dac_dac_select_t dacs,
	const dac_channel_select_t channels,
	const int64_t carrier_freq_hz,
	const uint16_t amplitude,
	int32_t dc_test_tone_en, int32_t ddsm_cal_dc_input_en)
{
	uint64_t tmp_freq;
	uint32_t ddsc_datapath_cfg;
	uint8_t is_pow2 = 0;
	uint32_t tmp_reg;
	int32_t err;

	/*todo: phase offset - as param */
	if (DACApi::h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;

	if (!((carrier_freq_hz >= (int64_t)(0ll - DACApi::h->dac_freq_hz / 2)) &&
		(carrier_freq_hz < (int64_t)(DACApi::h->dac_freq_hz / 2))))
		return API_ERROR_INVALID_PARAM;

	if (carrier_freq_hz == 0)
		return API_ERROR_INVALID_PARAM;

	tmp_freq = carrier_freq_hz;
	while (tmp_freq <= DACApi::h->dac_freq_hz) {
		if ((tmp_freq) == DACApi::h->dac_freq_hz) {
			/* It is power of 2 */
			is_pow2 = 1;
			break;
		}
		tmp_freq *= 2;
	}

	/* DC TEST TONE EN */
	err = dac_dc_test_tone_set(dc_test_tone_en);
	if (err != API_ERROR_OK)
		return err;

	if (is_pow2 == 1) {
		/* Integer NCO mode */
		/* As we are in Integer NCO mode it guranteed the
		   value is integer power of 2 */
		tmp_freq = NO_OS_DIV_U64(DACApi::h->dac_freq_hz, carrier_freq_hz);
		tmp_freq = NO_OS_DIV_U64(ADI_POW2_48, tmp_freq);

		/* Disable modulus */
		if (dacs != 0) {
			/* DAC PAGE */
			tmp_reg = AD917X_CHANNEL_PAGE_0;
			if (dacs & AD917X_DAC0)
				tmp_reg |= AD917X_MAINDAC_PAGE_0;
			if (dacs & AD917X_DAC1)
				tmp_reg |= AD917X_MAINDAC_PAGE_1;
			err = DACApi::dac_register_write(AD917X_SPI_PAGEINDX_REG, tmp_reg);
			if (err != API_ERROR_OK)
				return err;
			err = DACApi::dac_register_write(AD917X_DDSM_DATAPATH_CFG_REG,
				AD917X_DDSM_MODE(0) | AD917X_DDSM_NCO_EN);
			if (err != API_ERROR_OK)
				return err;
			/*DDSM CAL EN */
			err = dac_ddsm_cal_dc_input_set(ddsm_cal_dc_input_en);
			if (err != API_ERROR_OK)
				return err;
			/* Write FTW */
			err = dac_nco_set_ftw(AD917X_DDSM, tmp_freq, 0, 0);
			if (err != API_ERROR_OK)
				return err;
		}
		if (channels != 0) {
			/* Channel PAGE */
			err = DACApi::dac_register_write(AD917X_SPI_PAGEINDX_REG, (channels & 0x3F));
			if (err != API_ERROR_OK)
				return err;
			err = DACApi::dac_register_read(AD917X_DDSC_DATAPATH_CFG_REG, &ddsc_datapath_cfg);
			if (err != API_ERROR_OK)
				return err;
			ddsc_datapath_cfg |= AD917X_DDSC_NCO_EN;
			err = DACApi::dac_register_write(AD917X_DDSC_DATAPATH_CFG_REG, ddsc_datapath_cfg);
			if (err != API_ERROR_OK)
				return err;
			/* Write FTW */
			err = dac_nco_set_ftw(AD917X_DDSC, tmp_freq, 0, 0);
			if (err != API_ERROR_OK)
				return err;
		}
	}
	else {
		int32_t gcd;
		uint64_t int_part;
		uint64_t frac_part_a;
		uint64_t frac_part_b;
		uint64_t M, N;
		uint64_t tmp_ah, tmp_al, tmp_bh, tmp_bl, tmp_fh, tmp_fl;
		/* Modulus NCO mode */

		gcd = adi_api_utils_gcd(carrier_freq_hz, DACApi::h->dac_freq_hz);
		M = NO_OS_DIV_U64(carrier_freq_hz, gcd);
		N = NO_OS_DIV_U64(DACApi::h->dac_freq_hz, gcd);

		if (M > NO_OS_S16_MAX) {
			uint64_t mask = U64MSB;
			int32_t i = 0;
			while (((mask & M) == 0) && (mask != 1)) {
				mask >>= 1;
				i++;
			}
			int_part = NO_OS_DIV_U64(M * ((uint64_t)1u << i), N);
			int_part *= ((uint64_t)1u << (48 - i));
		}
		else
			int_part = NO_OS_DIV_U64(M * (ADI_POW2_48), N);

		adi_api_utils_mult_128(M, ADI_POW2_48, &tmp_ah, &tmp_al);
		adi_api_utils_mult_128(N, int_part, &tmp_bh, &tmp_bl);
		adi_api_utils_subt_128(tmp_ah, tmp_al, tmp_bh, tmp_bl, &tmp_fh, &tmp_fl);
		frac_part_a = tmp_fl;
		frac_part_b = N;

		gcd = adi_api_utils_gcd(frac_part_a, frac_part_b);
		frac_part_a = NO_OS_DIV_U64(frac_part_a, gcd);
		frac_part_b = NO_OS_DIV_U64(frac_part_b, gcd);

		if ((frac_part_a > ADI_MAXUINT48) || (frac_part_b > ADI_MAXUINT48)) {
			/* TODO: a better error */
			return API_ERROR_INVALID_PARAM;
		}

		/* Enable modulus */
		if (dacs != 0) {
			/* DAC PAGE */
			tmp_reg = AD917X_CHANNEL_PAGE_0;
			if (dacs & AD917X_DAC0)
				tmp_reg |= AD917X_MAINDAC_PAGE_0;
			if (dacs & AD917X_DAC1)
				tmp_reg |= AD917X_MAINDAC_PAGE_1;
			err = DACApi::dac_register_write(AD917X_SPI_PAGEINDX_REG, tmp_reg);
			if (err != API_ERROR_OK)
				return err;

			err = DACApi::dac_register_write(AD917X_DDSM_DATAPATH_CFG_REG,
				AD917X_DDSM_MODE(0) | AD917X_DDSM_NCO_EN | AD917X_DDSM_MODULUS_EN);
			if (err != API_ERROR_OK)
				return err;
			/*DDSM CAL EN */
			err = dac_ddsm_cal_dc_input_set(ddsm_cal_dc_input_en);
			if (err != API_ERROR_OK)
				return err;
			/* Write FTW, A and B */
			err = dac_nco_set_ftw(AD917X_DDSM, int_part, frac_part_a, frac_part_b);
			if (err != API_ERROR_OK)
				return err;
		}
		if (channels != 0) {
			/* Channel PAGE */
			err = DACApi::DACApi::dac_register_write(AD917X_SPI_PAGEINDX_REG, (channels & 0x3F));
			if (err != API_ERROR_OK)
				return err;
			err = DACApi::dac_register_read(AD917X_DDSC_DATAPATH_CFG_REG, &ddsc_datapath_cfg);
			if (err != API_ERROR_OK)
				return err;
			ddsc_datapath_cfg |= AD917X_DDSC_NCO_EN | AD917X_DDSC_MODULUS_EN;
			err = DACApi::dac_register_write(AD917X_DDSC_DATAPATH_CFG_REG, ddsc_datapath_cfg);
			if (err != API_ERROR_OK)
				return err;
			/* Write FTW, A and B */
			err = dac_nco_set_ftw(AD917X_DDSC, int_part, frac_part_a, frac_part_b);
			if (err != API_ERROR_OK)
				return err;
		}
	}
	/* Amplitude */
	err = dac_set_dc_cal_tone_amp(amplitude);
	if (err != API_ERROR_OK)
		return err;
	return API_ERROR_OK;
}

int32_t DACApi::dac_nco_main_freq_get(dac_dac_select_t dac, int64_t* carrier_freq_hz)
{
	if (h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;

	if ((dac != AD917X_DAC0) &&
		(dac != AD917X_DAC1))
		return API_ERROR_INVALID_PARAM;

	if (carrier_freq_hz == NULL)
		return API_ERROR_INVALID_PARAM;

	return dac_nco_get_freq(AD917X_DDSM, dac, (dac_channel_select_t)0, carrier_freq_hz);
}

int32_t DACApi::dac_nco_channel_freq_get(dac_channel_select_t channel, int64_t* carrier_freq_hz)
{
	if (DACApi::h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;

	if ((channel != AD917X_CH_0) &&
		(channel != AD917X_CH_1) &&
		(channel != AD917X_CH_2) &&
		(channel != AD917X_CH_3) &&
		(channel != AD917X_CH_4) &&
		(channel != AD917X_CH_5))
		return API_ERROR_INVALID_PARAM;

	if (carrier_freq_hz == NULL)
		return API_ERROR_INVALID_PARAM;
	
	return dac_nco_get_freq(AD917X_DDSC, (dac_dac_select_t)0, channel, carrier_freq_hz);
}

