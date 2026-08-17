// SPDX-License-Identifier: GPL-2.0
/**
 * \file AD917x.h
 *
 * \brief AD917X API interface header file
 *
 * This file contains all the publicly exposed methods and data structures to
 * interface with the AD917X API.
 *
 * Release 1.1.X
 *
 * Copyright(c) 2017 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
 */
#pragma once
#ifndef __AD917XAPI_H__
#define __AD917XAPI_H__

#include <stdint.h>
#include "api_def.h"
#include "no_os.h"



 /*Device DEFINITION */
#define AD917X_JESD_NOF_LANES         8
#define AD917X_JESD_NOF_LINKS         2
#define AD917X_JESD_NOF_SYNCOUTB      2
#define AD9171_ID		      9171
#define AD9172_ID		      9172
#define AD9173_ID		      9173

/*REGISTER SUMMARY : (AD917X_REGMAP_V4)*/
#define AD917X_IF_CFG_A_REG           0x000
#define AD917X_IF_CFG_B_REG           0x001
#define AD917X_DEV_CFG_REG            0x002
#define AD917X_CHIP_TYPE_REG          0x003
#define AD917X_PROD_ID_LSB_REG        0x004
#define AD917X_PROD_ID_MSB_REG        0x005
#define AD917X_CHIP_GRADE_REG         0x006
#define AD917X_SPI_PAGEINDX_REG       0x008
#define AD917X_CHANNEL_PAGE_0         NO_OS_BIT(0)
#define AD917X_CHANNEL_PAGE_1         NO_OS_BIT(1)
#define AD917X_CHANNEL_PAGE_2         NO_OS_BIT(2)
#define AD917X_CHANNEL_PAGE_3         NO_OS_BIT(3)
#define AD917X_CHANNEL_PAGE_4         NO_OS_BIT(4)
#define AD917X_CHANNEL_PAGE_5         NO_OS_BIT(5)
#define AD917X_MAINDAC_PAGE_0         NO_OS_BIT(6)
#define AD917X_MAINDAC_PAGE_1         NO_OS_BIT(7)

#define AD917X_SYSREF_ROTATION_REG    0x03B
#define AD917X_SYNC_LOGIC_EN          NO_OS_BIT(7)
#define AD917X_SYNC_RSV_EN            NO_OS_BIT(6)
#define AD917X_PERIODIC_RST_EN        NO_OS_BIT(5)
#define AD917X_NCORST_AFTER_ROTATION  NO_OS_BIT(4)
#define AD917X_ROTATION_MODE(x)       (((x) & 0x3) << 0)



#define AD917X_SYSREF_CTRL_REG         0x084
#define AD917X_SYSREF_PD               NO_OS_BIT(0)
#define AD917X_SYSREF_DC_COUPLED       NO_OS_BIT(6)

#define AD917X_PLL_VCO_CTRL_REG        0x094
#define AD917X_PLL_VCO_DIV_EN(x)       (((x) & 0x3) << 0)

#define AD917X_PLL_BYPASS_REG         0x095
#define AD917X_PLL_BYPASS(x)          ((x) ? NO_OS_BIT(0) : 0)

#define AD917X_DLL_CTRL0_REG          0x0C1
#define AD917X_DLL_CFG                NO_OS_BIT(6) | NO_OS_BIT(3)
#define AD917X_DLL_HF                 NO_OS_BIT(5)
#define AD917X_DLL_RST                NO_OS_BIT(0)
#define AD917X_DLL_STATUS_REG         0x0C3
#define AD917X_DLL_LOCK               NO_OS_BIT(0)

#define AD917X_DIG_RESET_REG          0x100
#define AD917X_DIG_PATH_PDN(x)        ((x) ? NO_OS_BIT(0) : 0)

#define AD917X_JESD_MODE_REG          0x110
#define AD917X_JESD_MODE_INVALID      NO_OS_BIT(7)
#define AD917X_LINK_MODE              NO_OS_BIT(5)
#define AD917X_JESD_MODE(x)           (((x) & 0x1F) << 0)

#define AD917X_INTERP_MODE_REG        0x111
#define AD917X_CH_INTERP_MODE(x)      (((x) & 0xF) << 0)
#define AD917X_DP_INTERP_MODE(x)      (((x) & 0xF) << 4)

#define AD917X_DDSM_DATAPATH_CFG_REG  0x112
#define AD917X_DDSM_MODE(x)           (((x) & 0x3) << 4)
#define AD917X_DDSM_NCO_EN            NO_OS_BIT(3)
#define AD917X_DDSM_MODULUS_EN        NO_OS_BIT(2)
#define AD917X_DDSM_SEL_SIDEBAND      NO_OS_BIT(1)
#define AD917X_DDSM_EN_SYNC_ALL_CHNL_NCO_RESETS NO_OS_BIT(0)

#define AD917X_DDSM_FTW_UPDATE_REG    0x113
#define AD917X_DDSM_FTW_LOAD_SYSREF   NO_OS_BIT(2)
#define AD917X_DDSM_FTW_LOAD_ACK      NO_OS_BIT(1)
#define AD917X_DDSM_FTW_LOAD_REQ      NO_OS_BIT(0)

#define AD917X_DDSM_FTW0_REG          0x114
#define AD917X_DDSM_FTW1_REG          0x115
#define AD917X_DDSM_FTW2_REG          0x116
#define AD917X_DDSM_FTW3_REG          0x117
#define AD917X_DDSM_FTW4_REG          0x118
#define AD917X_DDSM_FTW5_REG          0x119

#define AD917X_DDSM_PHASE_OFFSET0_REG 0x11C
#define AD917X_DDSM_PHASE_OFFSET1_REG 0x11D

#define AD917X_DDSM_ACC_MODULUS0_REG  0x124
#define AD917X_DDSM_ACC_MODULUS1_REG  0x125
#define AD917X_DDSM_ACC_MODULUS2_REG  0x126
#define AD917X_DDSM_ACC_MODULUS3_REG  0x127
#define AD917X_DDSM_ACC_MODULUS4_REG  0x128
#define AD917X_DDSM_ACC_MODULUS5_REG  0x129

#define AD917X_DDSM_ACC_DELTA0_REG    0x12A
#define AD917X_DDSM_ACC_DELTA1_REG    0x12B
#define AD917X_DDSM_ACC_DELTA2_REG    0x12C
#define AD917X_DDSM_ACC_DELTA3_REG    0x12D
#define AD917X_DDSM_ACC_DELTA4_REG    0x12E
#define AD917X_DDSM_ACC_DELTA5_REG    0x12F

#define AD917X_DDSC_DATAPATH_CFG_REG  0x130
#define AD917X_DDSC_NCO_EN            NO_OS_BIT(6)
#define AD917X_DDSC_MODULUS_EN        NO_OS_BIT(2)
#define AD917X_DDSC_SEL_SIDEBAND      NO_OS_BIT(1)
#define AD917X_DDSC_TEST_TONE_EN      NO_OS_BIT(0)

#define AD917X_DDSC_FTW_UPDATE_REG    0x131
#define AD917X_DDSC_FTW_LOAD_SYSREF   NO_OS_BIT(2)
#define AD917X_DDSC_FTW_LOAD_ACK      NO_OS_BIT(1)
#define AD917X_DDSC_FTW_LOAD_REQ      NO_OS_BIT(0)

#define AD917X_DDSC_FTW0_REG          0x132
#define AD917X_DDSC_FTW1_REG          0x133
#define AD917X_DDSC_FTW2_REG          0x134
#define AD917X_DDSC_FTW3_REG          0x135
#define AD917X_DDSC_FTW4_REG          0x136
#define AD917X_DDSC_FTW5_REG          0x137

#define AD917X_DDSC_PHASE_OFFSET0_REG 0x138
#define AD917X_DDSC_PHASE_OFFSET1_REG 0x139

#define AD917X_DDSC_ACC_MODULUS0_REG  0x13A
#define AD917X_DDSC_ACC_MODULUS1_REG  0x13B
#define AD917X_DDSC_ACC_MODULUS2_REG  0x13C
#define AD917X_DDSC_ACC_MODULUS3_REG  0x13D
#define AD917X_DDSC_ACC_MODULUS4_REG  0x13E
#define AD917X_DDSC_ACC_MODULUS5_REG  0x13F

#define AD917X_DDSC_ACC_DELTA0_REG    0x140
#define AD917X_DDSC_ACC_DELTA1_REG    0x141
#define AD917X_DDSC_ACC_DELTA2_REG    0x142
#define AD917X_DDSC_ACC_DELTA3_REG    0x143
#define AD917X_DDSC_ACC_DELTA4_REG    0x144
#define AD917X_DDSC_ACC_DELTA5_REG    0x145

#define AD917X_X_FTW_UPDATE_REG(x)    ((x)==AD917X_DDSM?0x113:0x131)

#define AD917X_X_FTW0_REG(x)          ((x)==AD917X_DDSM?0x114:0x132)
#define AD917X_X_FTW1_REG(x)          ((x)==AD917X_DDSM?0x115:0x133)
#define AD917X_X_FTW2_REG(x)          ((x)==AD917X_DDSM?0x116:0x134)
#define AD917X_X_FTW3_REG(x)          ((x)==AD917X_DDSM?0x117:0x135)
#define AD917X_X_FTW4_REG(x)          ((x)==AD917X_DDSM?0x118:0x136)
#define AD917X_X_FTW5_REG(x)          ((x)==AD917X_DDSM?0x119:0x137)

#define AD917X_X_PHASE_OFFSET0_REG(x) ((x)==AD917X_DDSM?0x11C:0x138)
#define AD917X_X_PHASE_OFFSET1_REG(x) ((x)==AD917X_DDSM?0x11D:0x139)

#define AD917X_X_ACC_MODULUS0_REG(x)  ((x)==AD917X_DDSM?0x124:0x13A)
#define AD917X_X_ACC_MODULUS1_REG(x)  ((x)==AD917X_DDSM?0x125:0x13B)
#define AD917X_X_ACC_MODULUS2_REG(x)  ((x)==AD917X_DDSM?0x126:0x13C)
#define AD917X_X_ACC_MODULUS3_REG(x)  ((x)==AD917X_DDSM?0x127:0x13D)
#define AD917X_X_ACC_MODULUS4_REG(x)  ((x)==AD917X_DDSM?0x128:0x13E)
#define AD917X_X_ACC_MODULUS5_REG(x)  ((x)==AD917X_DDSM?0x129:0x13F)

#define AD917X_X_ACC_DELTA0_REG(x)    ((x)==AD917X_DDSM?0x12A:0x140)
#define AD917X_X_ACC_DELTA1_REG(x)    ((x)==AD917X_DDSM?0x12B:0x141)
#define AD917X_X_ACC_DELTA2_REG(x)    ((x)==AD917X_DDSM?0x12C:0x142)
#define AD917X_X_ACC_DELTA3_REG(x)    ((x)==AD917X_DDSM?0x12D:0x143)
#define AD917X_X_ACC_DELTA4_REG(x)    ((x)==AD917X_DDSM?0x12E:0x144)
#define AD917X_X_ACC_DELTA5_REG(x)    ((x)==AD917X_DDSM?0x12F:0x145)

#define AD917X_CHNL_GAIN0_REG          0x146
#define AD917X_CHNL_GAIN1_REG          0x147
#define CHANNEL_GAIN0(x)               (uint8_t)((x) & 0xFF)
#define CHANNEL_GAIN1(x)               (uint8_t)(((x) >> 8) & 0x0F)
#define CHANNEL_GAIN(g0, g1)           (uint16_t)(((uint16_t)((g1) << 8) | (g0)) & 0xFFF)

#define AD917X_DC_CAL_TONE0_REG          0x148
#define AD917X_DC_CAL_TONE1_REG          0x149

#define AD917X_DDSM_CAL_MODE_DEF_REG     0x1E6
#define AD917X_DDSM_EN_CAL_ACC           NO_OS_BIT(2)
#define AD917X_DDSM_EN_CAL_DC_INPUT      NO_OS_BIT(1)
#define AD917X_DDSM_EN_CAL_FREQ_TUNE     NO_OS_BIT(0)


#define AD917X_MASTER_PD_REG           0x200
#define AD917X_SERDES_PDN(x)           ((x) ? NO_OS_BIT(0) : 0)
#define AD917X_PHY_PD_REG              0x201
#define AD917X_PLL_EN_CTRL_REG         0x280
#define AD917X_SERDES_PLL_STARTUP      NO_OS_BIT(0)

#define AD917X_GEN_PD_REG              0x203
#define AD917X_SYNCOUTB_0_PD           NO_OS_BIT(1)
#define AD917X_SYNCOUTB_1_PD           NO_OS_BIT(0)
#define AD917X_SYNCOUTB_PD(x)          (((x) & 0x3) << 0)

#define AD917X_SYNCOUTB_CTRL_0_REG     0x253
#define AD917X_SYNCOUTB_CTRL_1_REG     0x254
#define AD917X_SYNCOUTB_MODE(x)        ((x) ? NO_OS_BIT(0) : 0)

#define AD917X_PLL_STATUS_REG          0x281

#define AD917X_JESD_RX_CTL_REG         0x300
#define AD917X_DUAL_MODE               NO_OS_BIT(3)
#define AD917X_LINK_PAGE(x)            ((x) ? NO_OS_BIT(2) : 0)
#define AD917X_LINK_EN(x)              (((x) & 0x3) << 0)
#define AD917X_LINK_0_EN               NO_OS_BIT(0)
#define AD917X_LINK_1_EN               NO_OS_BIT(1)

#define AD917X_JESD_LMFC_DELAY0_REG    0x304
#define AD917X_JESD_LMFC_DELAY1_REG    0x305
#define AD917X_JESD_LMFC_DELAY(x)      (((x) & 0x3F) << 0)
#define AD917X_JESD_LMFC_VAR0_REG      0x306
#define AD917X_JESD_LMFC_VAR1_REG      0x307
#define AD917X_JESD_LMFC_VAR(x)        (((x) & 0x3F) << 0)

#define AD917X_JESD_XBAR_LANE_REG      0x308
#define AD917X_JESD_XBAR_LANE_REG      0x308
#define AD917X_JESD_XBAR_LANE_REG      0x308

#define AD917X_JESD_XBAR_LANE_REG      0x308
#define AD917X_XBAR_LANE_EVEN(x)       (((x) & 0x7) << 0)
#define AD917X_XBAR_LANE_ODD(x)        (((x) & 0x7) << 3)

#define AD917X_JESD_INVERT_LANE_REG    0x334
#define AD917X_JESD_INVERT_LANE(x)     NO_OS_BIT(x)

#define AD917X_JESD_PARAM_REG_BASE     0x450
#define AD917X_JESD_PARAM_REG_LEN      0xB
#define AD917X_JESD_L_GET(x)           ((x[3] & 0xF) + 1)
#define AD917X_JESD_F_GET(x)           ((x[4]) + 1)
#define AD917X_JESD_K_GET(x)           ((x[5] & 0x1F) + 1)
#define AD917X_JESD_M_GET(x)           ((x[6]) + 1)
#define AD917X_JESD_N_GET(x)           ((x[7] & 0xF) + 1)
#define AD917X_JESD_NP_GET(x)          ((x[8] & 0xF) + 1)
#define AD917X_JESD_S_GET(x)           ((x[9] & 0xF) + 1)
#define AD917X_JESD_HD_GET(x)          ((x[10] & 0x80) >> 7)
#define AD917X_JESD_DID_GET(x)         (x[0])
#define AD917X_JESD_BID_GET(x)         (x[1])
#define AD917X_JESD_LID0_GET(x)        (x[2] & 0xF)
#define AD917X_JESD_V_GET(x)           ((x[9] & 0xF8) >> 5)
#define AD917X_JESD_L(x)               (((x) & 0xF) << 0)
#define AD917X_JESD_NP(x)              (((x) & 0xF) << 0)

#define AD917X_JESD_ILS_SCR_L_REG      0x453
#define AD917X_JESD_SCR                NO_OS_BIT(7)
#define AD917X_JESD_ILS_NP_REG         0x458
#define AD917X_JESD_JESDV              NO_OS_BIT(5)

#define AD917X_JESD_CODE_GRP_SYNC_REG   0x470
#define AD917X_JESD_FRAME_SYNC_REG      0x471
#define AD917X_JESD_GOOD_CHECKSUM_REG   0x472
#define AD917X_JESD_INIT_LANE_SYNC_REG  0x473

#define AD917X_JESD_CTRL0_REG           0x475
#define AD917X_JESD_QBD_SOFT_RST        NO_OS_BIT(3)



#define AD917X_NVM_LOADER_REG          0x705
#define AD917X_NVM_BLR_DONE            NO_OS_BIT(1)


#define AD917X_DACPLL_CTRLX_REG        0x790
#define AD917X_DACPLL_CTRLY_REG        0x791
#define AD917X_DACPLL_CTRL0_REG        0x792
#define AD917X_RESET_VCO_DIV           NO_OS_BIT(1)

#define AD917X_DACPLL_CTRL1_REG        0x793
#define AD917X_M_DIV(x)                (((x) & 0x3) << 0)

#define AD917X_DACPLL_CTRL7_REG        0x799
#define AD917X_L_DIV(x)                (((x) & 0x3) << 6)
#define AD917X_N_DIV(x)                (((x) & 0x3F) << 0)


#define AD917X_DACPLL_STATUS_REG       0x7B5
#define AD917X_DACPLL_LOCK             NO_OS_BIT(0)

#define SERDES_PWRUP_DELAY 100000
#define LANE_MIN 1
#define LANE_INDEX_MAX (AD917X_JESD_NOF_LANES -1)
#define LINK_INDEX_MAX (AD917X_JESD_NOF_LINKS -1)
#define LINK_INDEX(x)  ( ((x) == 0xFF) ? (x) : NO_OS_BIT(x))
#define SYNCOUTB_INDEX_MAX (AD917X_JESD_NOF_SYNCOUTB-1)
#define SYNCOUTB_INDEX(x)  (((x)==0xFF) ? (x) : NO_OS_BIT(x))
#define K_MAX 32
#define M_DEFAULT 2
#define N_DEFAULT 16
#define NP_DEFAULT 16
#define S_MIN 1
#define S_MAX 4
#define CF_DEFAULT 0
#define CS_DEFAULT 0
#define HD_DEFAULT 0
#define LANE_RATE_MIN 750
#define LANE_RATE_MAX 12500
#define INTERPOLATION_MIN 1
#define DP_INTERPOLATION_MAX 12
#define CH_INTERPOLATION_MAX 8
#define DAC_CLK_FREQ_MIN 850
#define DAC_CLK_FREQ_MAX 6000
#define SYSREF_JITTER_WIN_MAX 28
#define INTPL_MODE_INVALID    25
#define JESD_MODE_INVALID     22
#define AD917X_LMFC_VAR_MAX   0xC



#define LOWER_16(A) ((A) & 0xFFFF)
#define UPPER_16(A) (((A) >> 16) & 0xFFFF)
#define LOWER_32(A) ((A) & (uint32_t) 0xFFFFFFFF)

#define IN_OUT_BUFF_SZ 3

#define HW_RESET_PERIOD_US 10
#define SPI_RESET_PERIOD_US 50
#define NVRAM_RESET_PERIOD_US 1000
#define DAC_9171_CLK_FREQ_MHZ_MAX 6000
#define DAC_CLK_FREQ_MHZ_MAX 12000
#define DAC_CLK_FREQ_MHZ_MIN 2900
#define REF_CLK_FREQ_MHZ_MIN 30
#define REF_CLK_FREQ_MHZ_MAX 2000
#define PFD_CLK_FREQ_MHZ_MIN 25
#define PFD_CLK_FREQ_MHZ_MAX 770
#define DLL_CLK_FREQ_THRES_HZ 4500000000ull
#define DAC_9171_CLK_FREQ_MAX_HZ 6000000000ull
#define DAC_CLK_FREQ_MAX_HZ 12000000000ull
#define DAC_CLK_FREQ_MIN_HZ 2900000000ull


using namespace API_DEF;

namespace DAC 
{
	union uint32_chararray
	{
		char c[4];
		int i;
	};

	typedef struct {
		void* user_data;					/**< Void pointer to user defined data for HAL initialization */
		spi_sdo_config_t sdo;			    /**< DAC SPI interface configuration*/
		signal_type_t syncoutb;			    /**< Desired Signal type for SYNCOUTB signal*/
		signal_coupling_t sysref;			/**< Desired Input coupling for sysref signal*/
		uint64_t dac_freq_hz;				/**< DAC Clock Frequency in Hz. Valid range 2.9GHz to 12GHz*/
		spi_xfer_t dev_xfer;				/**< Function Pointer to HAL SPI access function*/
		delay_us_t delay_us;				/**< Function Pointer to HAL delay function*/
		tx_en_pin_ctrl_t tx_en_pin_ctrl;    /**< Function Pointer to HAL TX_ENABLE Pin Ctrl function*/
		reset_pin_ctrl_t reset_pin_ctrl;	/**< Function Point to HAL RESETB Pin Ctrl Function*/
		hw_open_t hw_open;					/**< Function Pointer to HAL initialization function*/
		hw_close_t hw_close;				/**< Function Pointer to HAL de-initialization function*/
		int32_t fd;
	} dac_handle_t;

	typedef struct dac_dev {
		/* SPI */
		struct no_os_spi_desc* spi_desc;
		/* GPIO */
		struct no_os_gpio_desc* gpio_reset;
		struct no_os_gpio_desc* gpio_txen0;
		struct no_os_gpio_desc* gpio_txen1;
		struct dac_state* st;
	} dac_dev;

	enum chip_id {
		CHIPID_AD9171 = 0x71,
		CHIPID_AD9172 = 0x72,
		CHIPID_AD9173 = 0x73,
		CHIPID_AD9174 = 0x74,
		CHIPID_AD9175 = 0x75,
		CHIPID_AD9176 = 0x76,
	};

	struct dac_state {
		enum chip_id id;
		dac_handle_t dac_h;
		jesd_param_t appJesdConfig;
		uint32_t dac_rate_khz;
		uint64_t dac_clkin_Hz;
		uint32_t dac_interpolation;
		uint32_t link_mode;
		uint32_t channel_interpolation;
		uint32_t interpolation;
		uint32_t jesd_mode;
		uint32_t jesd_subclass;
		uint32_t clock_output_config;
		signal_type_t syncoutb_type;
		signal_coupling_t sysref_coupling;
		uint8_t nco_main_enable;
		uint8_t nco_channel_enable;
		uint32_t logic_lane;
		uint32_t physical_lane;
		enum dac_dac_select_t dac_mask;
	};

	typedef struct dac_init_param {
		/* SPI */
		struct no_os_spi_init_param* spi_init;
		/* GPIO */
		struct no_os_gpio_init_param gpio_txen0;
		struct no_os_gpio_init_param gpio_txen1;
		struct no_os_gpio_init_param gpio_reset;
		uint32_t dac_rate_khz;
		uint32_t dac_clkin_Hz;
		uint32_t link_mode;
		uint32_t jesd_mode;
		uint32_t jesd_subclass;
		uint32_t dac_interpolation;
		uint32_t channel_interpolation;
		uint32_t clock_output_config;
		signal_type_t syncoutb_type;
		signal_coupling_t sysref_coupling;
		uint32_t logic_lane;
		uint32_t physical_lane;
		enum dac_dac_select_t dac_mask;
	} dac_init_param;

	/** DDS Select */
	typedef enum {
		/** Main DDS */
		AD917X_DDSM = 0,
		/** Channel DDS */
		AD917X_DDSC = 1
	} dac_dds_select_t;

	/** DAC Select */
	enum dac_dac_select_t {
		/** No DAC */
		AD917X_DAC_NONE = 0,
		/** DAC0 */
		AD917X_DAC0 = 1,
		/** DAC1 */
		AD917X_DAC1 = 2,
		/** DACALL */
		AD917X_DACALL = 3
	};

	/** Channel select */
	typedef enum {
		/** No Channel */
		AD917X_CH_NONE = 0,
		/** Channel 0 */
		AD917X_CH_0 = NO_OS_BIT(0),
		/** Channel 1 */
		AD917X_CH_1 = NO_OS_BIT(1),
		/** Channel 2 */
		AD917X_CH_2 = NO_OS_BIT(2),
		/** Channel 3 */
		AD917X_CH_3 = NO_OS_BIT(3),
		/** Channel 4 */
		AD917X_CH_4 = NO_OS_BIT(4),
		/** Channel 5 */
		AD917X_CH_5 = NO_OS_BIT(5)
	} dac_channel_select_t;

	/** JESD Interface Link Status */
	typedef struct {
		/** Bit wise Code Group Sync Status for all JESD Lanes*/
		uint8_t code_grp_sync_stat;
		/** Bit wise Frame Sync Status for all JESD Lanes*/
		uint8_t frame_sync_stat;
		/** Bit wise Good Checksum Status for all JESD Lanes*/
		uint8_t good_checksum_stat;
		/** Bit wise Initial Lane Sync Status for all JESD Lanes*/
		uint8_t init_lane_sync_stat;
	} dac_jesd_link_stat_t;

	/** Enumerates SERDES PLL Status Flags*/
	typedef enum {
		AD917X_PLL_LOCK_STAT = 0x1, /**< Serdes PLL lock Status Flag*/
		AD917X_PLL_REG_RDY = 0x2,   /**< Serdes PLL Regulator RDY Status Flag*/
		AD917X_PLL_CAL_STAT = 0x4,  /**< Serdes PLL VCO Calibration Status Flag*/
		AD917X_PLL_LOSSLOCK = 0x8   /**< Serdes PLL Upper Calibration Threshold flag*/
	} dac_jesd_serdes_pll_flg_t;

	/** AD917X API handle */

	static struct adi_reg_data ADI_REC_EQ_INIT_TBL[] = {
			{0x240, 0xAA}, /*ADI INTERNAL Equaliser Settings*/
			{0x241, 0xAA}, /*ADI INTERNAL Equaliser Settings*/
			{0x242, 0x55}, /*ADI INTERNAL Equaliser Settings*/
			{0x243, 0x55}, /*ADI INTERNAL Equaliser Settings*/
			{0x244, 0x1F}, /*ADI INTERNAL Equaliser Settings*/
			{0x245, 0x1F}, /*ADI INTERNAL Equaliser Settings*/
			{0x246, 0x1F}, /*ADI INTERNAL Equaliser Settings*/
			{0x247, 0x1F}, /*ADI INTERNAL Equaliser Settings*/
			{0x248, 0x1F}, /*ADI INTERNAL Equaliser Settings*/
			{0x249, 0x1F}, /*ADI INTERNAL Equaliser Settings*/
			{0x24A, 0x1F}, /*ADI INTERNAL Equaliser Settings*/
			{0x24B, 0x1F} /*ADI INTERNAL Equaliser Settings*/
	};

	/*Engineering Sample DataSheet Table 48 */
	static struct adi_reg_data ADI_REC_ES_SERDES_INIT_TBL_1[] = {
		{0x200, 0x01},  /*Power Down Serdes Blocks*/
		{0x210, 0x16}, /*ADI INTERNAL Init Serdes PLL Settings*/
		{0x216, 0x05}, /*ADI INTERNAL Init Serdes PLL Settings*/
		{0x212, 0xFF}, /*ADI INTERNAL Init Serdes PLL Settings*/
		{0x212, 0x00}, /*ADI INTERNAL Init Serdes PLL Settings*/
		{0x210, 0x87}, /*ADI INTERNAL Init Serdes PLL Settings*/
		{0x210, 0x87}, /*ADI INTERNAL Init Serdes PLL Settings*/
		{0x216, 0x11}, /*ADI INTERNAL Init Serdes PLL Settings*/
		{0x213, 0x01}, /*ADI INTERNAL Init Serdes PLL Settings*/
		{0x213, 0x00}, /*ADI INTERNAL Init Serdes PLL Settings*/
		{0x200, 0x00}  /*Power up Serdes Blocks*/
	};

	static struct adi_reg_data ADI_REC_ES_SERDES_INIT_TBL_2[] = {
		{0x210, 0x86}, /*ADI INTERNAL Init Serdes PLL Settings*/
		{0x216, 0x40}, /*ADI INTERNAL Init Serdes PLL Settings*/
		{0x213, 0x01}, /*ADI INTERNAL Init Serdes PLL Settings*/
		{0x213, 0x00}, /*ADI INTERNAL Init Serdes PLL Settings*/
		{0x210, 0x86}, /*ADI INTERNAL Init Serdes PLL Settings*/
		{0x210, 0x00}, /*ADI INTERNAL Init Serdes PLL Settings*/
		{0x213, 0x01}, /*ADI INTERNAL Init Serdes PLL Settings*/
		{0x213, 0x00}, /*ADI INTERNAL Init Serdes PLL Settings*/
		{0x210, 0x87}, /*ADI INTERNAL Init Serdes PLL Settings*/
		{0x216, 0x01}, /*ADI INTERNAL Init Serdes PLL Settings*/
		{0x213, 0x01}, /*ADI INTERNAL Init Serdes PLL Settings*/
		{0x213, 0x00}, /*ADI INTERNAL Init Serdes PLL Settings*/
		{0x280, 0x05}, /*ADI INTERNAL Init Serdes PLL Settings*/
		{0x280, 0x01} /*Start SERDES PLL and Init SERDES PLL CAL*/
	};

	static struct adi_reg_data ADI_RECOMMENDED_BOOT_TBL[] = {
	{0x091, 0x00}, /*Power Up Clock Receiver*/
	{0x206, 0x01}, /*Power Up Phys*/
	{0x705, 0x01}, /*ADI INTERNAL:LOAD NVRAM FACTORY SETTINGS*/
	{0x090, 0x00}, /*TODO:Review Power on DACs and Bias CCT*/
	};

	static struct adi_reg_data ADI_RECOMMENDED_PLL_TBL_1[] = {
		{0x796, 0xE5}, /*DAC PLL Recommmended write*/
		{0x7A0, 0xBC}, /*DAC PLL Recommmended write*/
		{0x794, 0x08}, /*DAC PLL Charge Pump Current Recommmended write*/
		{0x797, 0x20}, /*DAC PLL Recommmended write*/
		{0x798, 0x10}, /*DAC PLL Recommmended write*/
		{0x7A2, 0x7F}  /*DAC PLL Recommmended write*/
	};

	static struct adi_reg_data ADI_RECOMMENDED_DLL_TBL[] = {
		{0x0C0, 0x00}, /*Power Up delay Line*/
		{0x0DB, 0x00}, /*Implement Update*/
		{0x0DB, 0x01}, /*Implement Update*/
		{0x0DB, 0x00}, /*Implement Update*/
		{0x0C7, 0x01} /*Enable Status Read*/
	};

	static struct adi_reg_data ADI_RECOMMENDED_DAC_CAL_TBL[] = {
		{0x050, 0x2A}, /*ADI RECOMMENDED OPTIMIZED CALIBRATION*/
		{0x061, 0x68}, /*ADI RECOMMENDED OPTIMIZED CALIBRATION*/
		{0x051, 0x82}, /*ADI RECOMMENDED OPTIMIZED CALIBRATION*/
		{0x051, 0x83}, /*ADI RECOMMENDED OPTIMIZED CALIBRATION*/
		{0x081, 0x03} /*ADI RECOMMENDED OPTIMIZED CALIBRATION*/
	};


	class DACApi
	{
	private:


	public:
		
		static dac_handle_t* h;

		static int32_t dac_init(dac_dev* dev, dac_init_param* init_param);

		int32_t dac_remove(dac_dev* device);

		static int32_t dac_setup(struct dac_state* st);

		/**
		 * \brief Initialize AD917X Device
		 * This API must be called first before any other API calls.
		 * It performs internal API initialization of the memory and API states.
		 * If AD917X handle member hw_open is not NULL it shall call the function
		 * to which it points. This feature may be used to get and initialize the
		 * hardware resources required by the API and the AD917X devices.
		 * For example GPIO, SPI etc.
		 *
		 * Its is recommended to call the Reset API after this API to ensure all
		 * spi registers are reset to ADI recommended defaults.
		 *
		 * \param h Pointer to the AD917X device reference handle.
		 *
		 * \retval API_ERROR_OK API Completed Successfully
		 * \retval API_ERROR_INVALID_XFER_PTR SPI Access Failed
		 * \retval API_ERROR_INVALID_HANDLE_PTR  Invalid reference handle
		 * \retval API_ERROR_INIT_SEQ_FAIL Init sequence failed
		 * \retval API_ERROR_SPI_SDO    Invalid SPI configuration
		 */
		static int32_t dac_init();
		
		/**
		 * \brief De-initialize the AD917X Device.
		 *
		 * This API must be called last. No other API should be called after this call.
		 * It performs internal API de-initialization of the memory and API states.
		 * If AD917X handle member hw_close, is not NULL it shall call the function
		 * to which it points. This feature may be used to de-initialize and release
		 * any hardware resources required by the API and the AD917X Device.
		 * For example GPIO, SPI etc
		 *
		 * \param h Pointer to the AD917X device reference handle.
		 *
		 * \retval API_ERROR_OK API Completed Successfully
		 * \retval API_ERROR_INVALID_HANDLE_PTR   Invalid reference handle
		 */
		int32_t dac_deinit();

		/**
		 * \brief Reset the AD917X
		 *
		 * Issues a hard reset or soft reset of the device.
		 * Performs a full reset of AD917X via the hardware pin (hard) or
		 * via the spi register (soft).
		 * Resetting all SPI registers to default and triggering the required
		 * initialization sequence.
		 *
		 * \param h         Pointer to the AD917X device reference handle.
		 * \param hw_reset  A parameter to indicate if the reset issues is to be via the
		 *                  hardware pin or SPI register.
		 *                  A value of 1 indicates a hardware reset is required.
		 *                  A value of 0 indicates a software reset is required.
		 *
		 * \retval API_ERROR_OK API Completed Successfully
		 * \retval API_ERROR_INVALID_HANDLE_PTR   Invalid reference handle
		 * \retval API_ERROR_INVALID_XFER_PTR SPI Access Failed
		 * \retval API_ERROR_INVALID_PARAM    Invalid Parameter
		 * \retval API_ERROR_INIT_SEQ_FAIL Init sequence failed
		 * \retval API_ERROR_SPI_SDO    Invalid SPI configuration
		 */
		static int32_t dac_reset(uint8_t hw_reset);

		/**
		 * \brief Get Chip Identification Data
		 *
		 * read-back Product type, identification and revision data
		 *
		 * \param h          Pointer to the AD917X device reference handle.
		 * \param chip_id    Pointer to a variable of type  dac_chip_id_t to
		 *                   which the product data read-back from chip shall be stored.
		 *
		 * \retval API_ERROR_OK API Completed Successfully
		 * \retval API_ERROR_INVALID_HANDLE_PTR Invalid reference handle.
		 * \retval API_ERROR_INVALID_XFER_PTR SPI Access Failed
		 * \retval API_ERROR_INVALID_PARAM    Invalid Parameter
		 */
		static int32_t dac_get_chip_id(adi_chip_id_t* chip_id);

		/**
		 * \brief Perform SPI register write access to AD917X Device
		 *
		 * \param h        Pointer to the AD917X device reference handle.
		 * \param address  AD917X Device SPI address to which the value of data
		 *                 parameter shall be written
		 * \param data     8-bit value to be written to SPI register defined
		 *                 by the address parameter.
		 *
		 * \retval API_ERROR_OK API Completed Successfully
		 * \retval API_ERROR_INVALID_XFER_PTR SPI Access Failed
		 * \retval API_ERROR_INVALID_PARAM    Invalid Parameter
		 */
		//int32_t dac_register_write(const uint16_t address, const uint32_t data);
		/**
		 * \brief Perform SPI register read access to AD917X Device.
		 *
		 *
		 * \param h        Pointer to the AD917X device reference handle.
		 * \param address  AD917X Device SPI address from which the value of data
		 *                 parameter shall be read,
		 * \param data     Pointer to an 8-bit variable to which the value of the
		 *                 SPI register at the address defined by address parameter
		 *                 shall be stored.
		 *
		 * \retval API_ERROR_OK API Completed Successfully
		 * \retval API_ERROR_INVALID_XFER_PTR SPI Access Failed
		 * \retval API_ERROR_INVALID_PARAM    Invalid Parameter
		 */
		//int32_t dac_register_read(const uint16_t address, uint32_t* data);
		/**
		 * \brief Get API Revision Data
		 *
		 * \param h             Pointer to the AD917X device reference handle.
		 * \param rev_major     Pointer to variable to store the Major Revision Number
		 * \param rev_minor     Pointer to variable to store the Minor Revision Number
		 * \param rev_rc        Pointer to variable to store the RC Revision Number
		 *
		 * \retval API_ERROR_OK API Completed Successfully
		 * \retval API_ERROR_INVALID_PARAM    Invalid Parameter
		 */
		static int32_t dac_get_revision(uint8_t* rev_major, uint8_t* rev_minor, uint8_t* rev_rc);


		/**
		 * \brief Configure the On Chip DAC PLL
		 *
		 * The AD917X may be configured to use a clock directly applied to the device
		 * as the DAC clock or may generate a DAC Clock using the clock applied
		 * by the system as a reference.
		 * This API allows direct configuration of the On Chip PLL parameters.
		 *
		 * \param h            Pointer to the AD917X device reference handle.
		 * \param dac_pll_en   Enable for internal DAC Clock generation.
		 *                     If set, ref_clk_freq_khz must be set with
		 *                     value of reference clock applied by the system.
		 *                     0 - Do not generate DAC CLK internally.
		 *                     1 - Generate DAC CLK internally
		 * \param m_div        Reference Clock Pre-divider. Where
		 *                     M_DIVIDER = Ceiling (Fref_clk_mhz/500 MHz)
		 *                     Valid Range 1 to 4
		 * \param n_div        VCO Feedback Divider Ratio. Where
		 *                     N_DIVIDER = Fvco * M_DIVIDER/(8 * Fref_clk)
		 *                     Valid Range 2 -50
		 * \param vco_div      Required VCO Divider for the Desired DAC CLK, where
		 *                     Fdac = Fvco/VCO_DIVIDER
		 *                     Valid range 1-3
		 *
		 *
		 * \retval API_ERROR_OK API Completed Successfully
		 * \retval API_ERROR_INVALID_PARAM    Invalid Parameter
		 */
		static int32_t dac_set_dac_pll_config(uint8_t dac_pll_en, uint8_t m_div, uint8_t n_div, uint8_t vco_div);
		/**
		 * \brief Set the DAC CLK Frequency
		 *
		 * The AD917X may be configured to use a clock directly applied to the device
		 * as the DAC clock or may generate a DAC Clock using the clock applied
		 * by the system as a reference.
		 * This API set the desired DAC Clock Frequency, irrespective of how the
		 * DAC CLK is generated.
		 * DAC Clock Frequency range is 2.9 GHz to 12GHz
		 * for AD9172 and AD9173. DAC CLK Frequency range is 2.9 GHz to 6GHz for AD9171.
		 *
		 * This function shall be used in conjunction with the following API
		 * dac_set_dac_pll_cfg
		 *
		 * \param h                Pointer to the AD917X device reference handle.
		 * \param dac_clk_freq_hz  Desired DAC CLK Frequency value in Hz
		 *
		 *
		 * \retval API_ERROR_OK API Completed Successfully
		 * \retval API_ERROR_INVALID_PARAM    Invalid Parameter
		 */
		static int32_t dac_set_dac_clk_freq(uint64_t dac_clk_freq_hz);

		/**
		 * \brief Get the DAC CLK Frequency
		 *
		 * The AD917X may be configured to use a clock directly applied to the device
		 * as the DAC clock or may generate a DAC Clock using the clock applied
		 * by the system as a reference.
		 * This API get the desired DAC Clock Frequency, irrespective of how the
		 * DAC CLK is generated.
		 * DAC Clock Frequency range is 2.9 GHz to 12GHz
		 * for AD9172 and AD9173. DAC CLK Frequency range is 2.9 GHz to 6GHz for AD9171.
		 *
		 * \param h                Pointer to the AD917X device reference handle.
		 * \param dac_clk_freq_hz  Pointer to the uint64_t variable in which the dac
		 *                         clk frequency value in Hz shall be stored.
		 *                         See description for valid range
		 *
		 *
		 * \retval API_ERROR_OK API Completed Successfully
		 * \retval API_ERROR_INVALID_PARAM    Invalid Parameter
		 */
		static int32_t dac_get_dac_clk_freq(uint64_t* dac_clk_freq_hz);

		/**
		 * \brief Get DAC CLK Status
		 *
		 *
		 * \param h              Pointer to the AD917X device reference handle.
		 * \param pll_lock_stat  Pointer to which DAC PLL Lock Status shall be stored.
		 * 			 Set to NULL if status data not required.
		 *                       0 - DAC PLL Not Locked.
		 *                       1-  DAC PLL Locked.
		 * \param dll_lock_stat  Pointer to which DAC DLL Lock Status Shall be stored.
		 * 			 Set to NULL if status data not required.
		 *                       0 - DAC PLL Not Locked.
		 *                       1-  DAC PLL Locked.
		 *
		 * \retval API_ERROR_OK API Completed Successfully
		 * \retval API_ERROR_INVALID_PARAM    Invalid Parameter
		 */
		static int32_t dac_get_dac_clk_status(uint32_t* pll_lock_stat, uint32_t* dll_lock_stat);

		/**
		 * \brief Set CLKOUT configuration
		 *
		 *
		 * \param h      Pointer to the AD917X device reference handle.
		 * \param l_div  Output CLK divider setting. Valid range 1 to 4
		 *
		 * \retval API_ERROR_OK API Completed Successfully
		 * \retval API_ERROR_INVALID_PARAM    Invalid Parameter
		 */
		static int32_t dac_set_clkout_config(uint8_t l_div);

		/**
		 * \brief Configure the DAC Clock Input path based on a the desired dac clock
		 * frequency, the applied reference clock and the onchip PLL
		 *
		 * The AD917X may be configured to use a clock directly applied to the device
		 * as the DAC clock or may generate a DAC Clock using the clock applied
		 * by the system as a reference.
		 * This function shall calculate and apply the required onchip PLL configuration
		 * based on the desired dac clock frequency and the applied referenc clock
		 * frequency.
		 *
		 * This function may be used instead of the following two APIs
		 * dac_set_dac_pll_cfg
		 * DAC Clock Frequency range is 2.9 GHz to 12GHz
		 * for AD9172 and AD9173. DAC CLK Frequency range is 2.9 GHz to 6GHz for AD9171.
		 *
		 * \param h              Pointer to the AD917X device reference handle.
		 * \param dac_clk_freq_hz   Desired DAC Clk Frequency in Hz. See description for range.
		 * \param dac_pll_en    Enable for internal DAC Clock generation.
		 *                       If set, ref_clk_freq_khz must be set with
		 *                       value of reference clock applied by the system.
		 *                       0 - Do not generate DAC CLK internally.
		 *                       Clock applied to DAC is the dac clock.
		 *                       1 - Generate DAC CLK internally from a reference clock
		 *                       applied to the DAC
		 * \param ref_clk_freq_hz   Value of reference clk frequency applied to AD917X
		 *                          Set to 0 if DAC CLK is applied to the pin.
		 *
		 * \retval API_ERROR_OK API Completed Successfully
		 * \retval API_ERROR_INVALID_PARAM    Invalid Parameter
		 */
		static int32_t dac_set_dac_clk(
			uint64_t dac_clk_freq_hz, uint8_t dac_pll_en, uint64_t ref_clk_freq_hz);

		/**
		 * \brief Configure the JESD Datapath for AD917X
		 *
		 * Configure JESD interface parameters and digital datapath interpolation mode.
		 * An error shall be returned if the input parameters define an
		 * unsupported configuration. Refer to AD917x Device Datasheet for full details.
		 *
		 *
		 * \param h           Pointer to the AD917X device reference handle.
		 * \param dual_en     Dual Link enable setting
		 *                    0 - Single Link Mode
		 *                    1 - Dual Link Mode
		 * \param jesd_mode   The desired value of the pre-definded JESD link modes
		 *                    supported by the AD917X. Valid range 0 to 21.
		 *                    Based on this value the AD917X JESD interface
		 *                    shall be configured as per one of the supported JESD
		 *                    parameter configurations.
		 *                    Refer to the user guide for full details on the modes
		 *                    and the corresponding JESD settings.
		 * \param ch_intpl    The desired channel interpolation. Valid range 1 to 12
		 * \param dp_intpl  The desired main dac datapath interpolation. Valid range 1 to 10
		 *
		 * \retval API_ERROR_OK API Completed Successfully
		 * \retval API_ERROR_INVALID_HANDLE_PTR Invalid reference handle.
		 * \retval API_ERROR_INVALID_XFER_PTR SPI Access Failed
		 * \retval API_ERROR_INVALID_PARAM    Invalid Parameter
		 */
		static int32_t dac_jesd_config_datapath(uint8_t dual_en, uint8_t jesd_mode, uint8_t ch_intpl, uint8_t dp_intpl);

		/**
		 * \brief Get JESD Configuration Status
		 *
		 * Returns JESD Configuration Valid Mode Status
		 *
		 * \param h              Pointer to the AD917X device reference handle.
		 * \param cfg_valid      Pointer to a variable in which the Valid JESD
		 *                       Configuration status shall be stored
		 *                       0 - Invalid JESD and Interpolation Mode Configured
		 *                       1 - Valid JESD and Interpolation Mode Configured.
		 *
		 *
		 *
		 * \retval API_ERROR_OK API Completed Successfully
		 * \retval API_ERROR_INVALID_HANDLE_PTR Invalid reference handle.
		 * \retval API_ERROR_INVALID_XFER_PTR SPI Access Failed
		 * \retval API_ERROR_INVALID_PARAM    Invalid Parameter
		 */
		static int32_t dac_jesd_get_cfg_status(uint8_t* cfg_valid);

		/**
		 * \brief Read back all current JESD parameter settings.
		 *
		 * Read-back all the currently configured JESD Interface parameters.
		 *
		 * \param h           Pointer to the AD917X device reference handle.
		 * \param jesd_param  Pointer to a variable that will be set will the
		 *                    current values of the JESD interface parameters.
		 *
		 *
		 *
		 * \retval API_ERROR_OK API Completed Successfully
		 * \retval API_ERROR_INVALID_HANDLE_PTR Invalid reference handle.
		 * \retval API_ERROR_INVALID_XFER_PTR SPI Access Failed
		 * \retval API_ERROR_INVALID_PARAM    Invalid Parameter
		 */
		static int32_t dac_jesd_get_cfg_param(jesd_param_t* jesd_param);

		/**
		 * \brief Enable SysRef Input
		 *
		 * Enable AD917X SYSREF +- Pin Input Interface for the target system SYSREF signal
		 *
		 * \param h            Pointer to the AD917X device reference handle.
		 * \param en           Enable SYSREF Input Interface
		 *                     1 - Power Up SYSREF Input
		 *                     0 - Power Down SYSREF Input
		 *
		 * \retval API_ERROR_OK API Completed Successfully
		 * \retval API_ERROR_INVALID_HANDLE_PTR Invalid reference handle.
		 * \retval API_ERROR_INVALID_XFER_PTR SPI Access Failed
		 * \retval API_ERROR_INVALID_PARAM    Invalid Parameter
		 */
		static int32_t dac_jesd_set_sysref_enable(uint8_t en);

		/**
		 * \brief Get the current SYSREF Input
		 *
		 * Configure AD917X SYSREF +- Pin Input Interface for the target
		 * system SYSREF signal
		 *
		 * \param h            Pointer to the AD917X device reference handle.
		 * \param *en          Pointer to variable to which SYSREF Input Interface
		 *                     Enable status shall be stored
		 *                     1 - Power Up SYSREF Input
		 *                     0 - Power Down SYSREF Input
		 *
		 * \retval API_ERROR_OK API Completed Successfully
		 * \retval API_ERROR_INVALID_HANDLE_PTR Invalid reference handle.
		 * \retval API_ERROR_INVALID_XFER_PTR SPI Access Failed
		 * \retval API_ERROR_INVALID_PARAM    Invalid Parameter
		 */
		static int32_t dac_jesd_get_sysref_enable(uint8_t* en);


		/**
		 * \brief Set the LMFC Delay and Variance for the JESD Links
		 *
		 * API to configure the LMFC Delay and Variance for JESD Link Setup.
		 *
		 * \param h     Pointer to the AD917X device reference handle.
		 * \param link  Target JESD Link on which to configure LMFC Parameters
		 * \param delay Dynamic Link Latency for LMFC Rx in PCLK cycles. Range 0 to 63
		 * \param var   Variable Delay Buffer in PCLK cycles. Range 0 to 12
		 *
		 * \retval API_ERROR_OK API Completed Successfully
		 * \retval API_ERROR_INVALID_HANDLE_PTR Invalid reference handle.
		 * \retval API_ERROR_INVALID_XFER_PTR SPI Access Failed
		 * \retval API_ERROR_INVALID_PARAM    Invalid Parameter
		 */
		static int32_t dac_jesd_set_lmfc_delay(jesd_link_t link, uint8_t delay, uint8_t var);

		/**
		 * \brief Enable the SYNCOUTB Output Signal
		 *
		 * Configure and enable/disable the SYNCOUT_B Output Signal
		 *
		 * \param h            Pointer to the AD917X device reference handle.
		 * \param syncoutb     Target SYNCOUTB Signal.
		 *                     Valid values defined by dac_syncoutb_t
		 *                     SYNCOUTB_0
		 *                     SYNCOUTB_1
		 *                     SYCNOUTB_ALL
		 * \param en           Enable/Disable SYNCOUTB
		 *                     for target SYNCOUTB signal. Range 0 to 1
		 *                     0 - Disable
		 *                     1 - Enable
		 *
		 * \retval API_ERROR_OK API Completed Successfully
		 * \retval API_ERROR_INVALID_HANDLE_PTR Invalid reference handle.
		 * \retval API_ERROR_INVALID_XFER_PTR SPI Access Failed
		 * \retval API_ERROR_INVALID_PARAM    Invalid Parameter
		 */
		static int32_t dac_jesd_set_syncoutb_enable(jesd_syncoutb_t syncoutb, uint8_t en);

		/**
		 * \brief Enable the de-scrambler for the JESD Interface
		 *
		 * Enable or Disable the descrambler for the JESD Interface
		 *
		 * \param h   Pointer to the dac device reference handle.
		 * \param en  Enable control for JESD Scrambler.
		 *
		 * \retval API_ERROR_OK API Completed Successfully
		 * \retval API_ERROR_INVALID_HANDLE_PTR Invalid reference handle.
		 * \retval API_ERROR_INVALID_XFER_PTR SPI Access Failed
		 * \retval API_ERROR_INVALID_PARAM    Invalid Parameter
		 */
		static int32_t dac_jesd_set_scrambler_enable(uint8_t en);

		/**
		 * \brief Configure the Lane Cross Bar in the JESD datalink layer
		 *
		 * Configure AD917X Lane Cross Bar to route the physical JESD lanes
		 * to the desired logical lanes.
		 *
		 * \param h              Pointer to the AD917X device reference handle.
		 * \param logical_lane   uint8_t value indicating the corresponding logical
		 *                       lane for the physical lane listed
		 *                       in parameter physical_lane.
		 * \param physical_lane  uint8_t value indicating the Physical Lane
		 *                       to be routed to the serdes logical indicated
		 *                       by the logical_lane parameter
		 *
		 * \retval API_ERROR_OK API Completed Successfully
		 * \retval API_ERROR_INVALID_HANDLE_PTR Invalid reference handle.
		 * \retval API_ERROR_INVALID_XFER_PTR SPI Access Failed
		 * \retval API_ERROR_INVALID_PARAM    Invalid Parameter
		 */
		static int32_t dac_jesd_set_lane_xbar(uint8_t logical_lane, uint8_t physical_lane);

		/**
		 * \brief Get current Lane Cross Bar configuration for the JESD datalink layer
		 *
		 * Return the physical to logical lane mapping set by the configured by the
		 * current Lane Cross Bar configuration.
		 *
		 * \param h             Pointer to the AD917X device reference handle.
		 * \param phy_log_map   Pointer a 8 deep uint8_t array.Each element of the array
		 *                      represents the physical lane 0 - 7 and
		 *                      the value represents the logical lane assigned to
		 *                      that physical lane.
		 *
		 * \retval API_ERROR_OK API Completed Successfully
		 * \retval API_ERROR_INVALID_HANDLE_PTR Invalid reference handle.
		 * \retval API_ERROR_INVALID_XFER_PTR SPI Access Failed
		 * \retval API_ERROR_INVALID_PARAM    Invalid Parameter
		 */
		static int32_t dac_jesd_get_lane_xbar(uint8_t* phy_log_map);

		/**
		 * \brief Invert or un-invert logical lanes
		 *
		 * Each logical lane can be inverted which can be used to ease
		 * routing of SERDIN signals.
		 *
		 * \param h             Pointer to the AD917X device reference handle.
		 * \param logical_lane  Logical lane ID to be inverted. 0 to 7
		 * \param invert        Desired invert status for the logical lane
		 *                      represented in logical_lane parameter.
		 *                      Set to 1 to invert.
		 *                      Set to 0 to de-invert.
		 *
		 * \retval API_ERROR_OK API Completed Successfully
		 * \retval API_ERROR_INVALID_HANDLE_PTR Invalid reference handle.
		 * \retval API_ERROR_INVALID_XFER_PTR SPI Access Failed
		 * \retval API_ERROR_INVALID_PARAM    Invalid Parameter
		 */
		static int32_t dac_jesd_invert_lane(uint8_t logical_lane, uint8_t invert);

		/**
		 * \brief Enable the JESD Interface
		 *
		 * Configure power up and enable the dac the JESD Interface
		 *
		 * \param h         Pointer to the AD917X device reference handle.
		 * \param lanes_msk Lanes to be enabled on JESD Interface.
		 *                  8-bit mask where bit 0 represents lane 0,
		 *                  bit 1 represents lane 1 etc.
		 *                  Set to 1 to enable JESD lane, set to 0 to disable JESD Lane.
		 * \param run_cal   Run JESD interface equalisation routine
		 * \param en        Enable control for the JESD Interface
		 *
		 * \retval API_ERROR_OK API Completed Successfully
		 * \retval API_ERROR_INVALID_HANDLE_PTR Invalid reference handle.
		 * \retval API_ERROR_INVALID_XFER_PTR SPI Access Failed
		 * \retval API_ERROR_INVALID_PARAM    Invalid Parameter
		 */
		static int32_t dac_jesd_enable_datapath(uint8_t lanes_msk, uint8_t run_cal, uint8_t en);

		/**
		 * \brief  Get SERDES PLL Status
		 *
		 * Read Serdes PLL status and return the status via the
		 * pll_status parameter.
		 *
		 *
		 * \param h            Pointer to the AD917X device reference handle.
		 * \param *pll_status  Pointer to the variable that will be set with
		 *                     the PLL status.
		 *                     bit[0] => SERDES PLL Lock Status
		 *                     bit[1] => Regulator Status
		 *                     bit[2] => Calibration Status
		 *                     bit[3] => LOSS_LOCK Status
		 *
		 *
		 * \retval API_ERROR_OK API Completed Successfully
		 * \retval API_ERROR_INVALID_HANDLE_PTR Invalid reference handle.
		 * \retval API_ERROR_INVALID_XFER_PTR SPI Access Failed
		 * \retval API_ERROR_INVALID_PARAM    Invalid Parameter
		 */
		static int32_t dac_jesd_get_pll_status(uint32_t* pll_status);

		/**
		 * \brief  Enable JESD Link
		 *
		 * Enable SERDES Link to start the bring up JESD Link procedure
		 *
		 * JESD Transmitter Link shall be enabled and ready to begin link bring
		 * prior to calling this function. SERDES PLL shall be locked.
		 *
		 * \param h     Pointer to the AD917X device reference handle.
		 * \param link  Target Link on which to start JESD Link Bring up Procedure
		 * \param en    Enable control for the JESD Link
		 *              0 - Enable Link
		 *              1 - Disable Link
		 *
		 * \retval API_ERROR_OK API Completed Successfully
		 * \retval API_ERROR_INVALID_HANDLE_PTR Invalid reference handle.
		 * \retval API_ERROR_INVALID_XFER_PTR SPI Access Failed
		 * \retval API_ERROR_INVALID_PARAM    Invalid Parameter
		 */
		static int32_t dac_jesd_enable_link(jesd_link_t link, uint8_t en);

		/**
		 * \brief  Get JESD Link Status
		 *
		 * Read-back JESD Status for all lanes. JESD status include
		 * Code Group Sync Status, Frame Sync Status, Checksum Status
		 * Initial Lane Sync Status for the active JESD link.
		 *
		 *
		 * \param h             Pointer to the AD917x device reference handle.
		 * \param link          Desired link of which to retrieve status.
		 * \param *link_status  Pointer to the variable of type jesd_link_status
		 *                      that will be set with current jesd link read-back data.
		 *
		 * \retval API_ERROR_OK API Completed Successfully
		 * \retval API_ERROR_INVALID_HANDLE_PTR Invalid reference handle.
		 * \retval API_ERROR_INVALID_XFER_PTR SPI Access Failed
		 * \retval API_ERROR_INVALID_PARAM    Invalid Parameter
		 */
		static int32_t dac_jesd_get_link_status(jesd_link_t link, dac_jesd_link_stat_t* link_status);

		/**
		 * \brief  Select Page
		 *
		 * \param h             Pointer to the AD916x device reference handle.
		 * \param dac           DAC number. Valid values are:
		 *						AD9172_DAC_NONE - No DAC selected
		 *						AD9172_DAC0 - DAC0 selected
		 *						AD9172_DAC1 - DAC1 selected
		 *						AD9172_DAC0 | AD9172_DAC1 - Both DACs selected
		 * \param channel       Channel number.Valid values are:
		 *						AD9172_CH_NONE - No channel selected
		 *						AD9172_CH_0 - Channel 0 selected
		 *						...
		 *						AD9172_CH_5 - Channel 5 selected
		 *						More than one channel can be selected at once by ORing them.
		 *
		 * \retval API_ERROR_OK API Completed Successfully
		 * \retval API_ERROR_INVALID_HANDLE_PTR Invalid reference handle.
		 * \retval API_ERROR_INVALID_XFER_PTR SPI Access Failed
		 * \retval API_ERROR_INVALID_PARAM    Invalid Parameter
		 */
		static int32_t dac_set_page_idx(const uint32_t dac, const uint32_t channel);

		/**
		 * \brief  Get select page index
		 *
		 * \param h             Pointer to the AD917x device reference handle.
		 * \param *dac          Pointer to the selected DAC number.
		 * \param *channel      Pointer to the selected Channel number.
		 *
		 * \retval API_ERROR_OK API Completed Successfully
		 * \retval API_ERROR_INVALID_HANDLE_PTR Invalid reference handle.
		 * \retval API_ERROR_INVALID_XFER_PTR SPI Access Failed
		 * \retval API_ERROR_INVALID_PARAM    Invalid Parameter
		 */
		static int32_t dac_get_page_idx(int32_t* dac, int32_t* channel);

		/**
		 * \brief  Set Channel gain
		 *
		 * Sets the scalar channel gain value. It is paged by CHANNEL_PAGE in Reg08
		 *
		 * \param h     Pointer to the AD917x device reference handle.
		 * \param gain  Gain value
		 *
		 * \retval API_ERROR_OK API Completed Successfully
		 * \retval API_ERROR_INVALID_HANDLE_PTR Invalid reference handle.
		 * \retval API_ERROR_INVALID_XFER_PTR SPI Access Failed
		 * \retval API_ERROR_INVALID_PARAM    Invalid Parameter
		 */
		static int32_t dac_set_channel_gain(const uint16_t gain);

		/**
		 * \brief  Get Channel gain
		 *
		 * Gets the scalar channel gain value. It is paged by CHANNEL_PAGE in Reg08
		 *
		 * \param h     Pointer to the AD917x device reference handle.
		* \param gain  Pointer to the gain value
		 *
		 * \retval API_ERROR_OK API Completed Successfully
		 * \retval API_ERROR_INVALID_HANDLE_PTR Invalid reference handle.
		 * \retval API_ERROR_INVALID_XFER_PTR SPI Access Failed
		 * \retval API_ERROR_INVALID_PARAM    Invalid Parameter
		 */
		static int32_t dac_get_channel_gain(uint16_t* gain);


		/**
		 * \brief  Set DC Calibration tone
		 *
		 * Sets the DC tone amplitude. This amplitude goes to both I and Q paths.
		 * It is paged by CHANNEL_PAGE in Reg08
		 *
		 * \param h     Pointer to the AD917x device reference handle.
		* \param amp    Calibration tone amplitude
		 *
		 * \retval API_ERROR_OK API Completed Successfully
		 * \retval API_ERROR_INVALID_HANDLE_PTR Invalid reference handle.
		 * \retval API_ERROR_INVALID_XFER_PTR SPI Access Failed
		 * \retval API_ERROR_INVALID_PARAM    Invalid Parameter
		 */
		static int32_t dac_set_dc_cal_tone_amp(const uint16_t amp);

		/**
		 * \brief  Get NCO phase offset
		 *
		 * Gets main datapath and/or channel datapath NCO phase offset.
		 *
		 * \param h     Pointer to the AD917x device reference handle.
		 * \param dacs  DAC number
		 *              AD917X_DAC0 - DAC0 NCO
		 *              AD917X_DAC1 - DAC1 NCO
		 *              AD917X_DAC0 | AD917X_DAC1 - DAC0 NCO and DAC1 NCO
		 *
		 * \param channels  Channel number
		 *              AD917X_CH_0 - Channel 0 NCO
		 *              AD917X_CH_1 - Channel 1 NCO
		 *              AD917X_CH_2 - Channel 2 NCO
		 *              AD917X_CH_3 - Channel 3 NCO
		 *              AD917X_CH_4 - Channel 4 NCO
		 *              AD917X_CH_5 - Channel 5 NCO
		 * \param dacs_po  Phase offset for the selected DAC NCO(s)
		 * \param ch_po  Phase offset for the selected channel NCO(s)
		 *
		* \retval API_ERROR_OK API Completed Successfully
		 * \retval API_ERROR_INVALID_HANDLE_PTR Invalid reference handle.
		 * \retval API_ERROR_INVALID_XFER_PTR SPI Access Failed
		 * \retval API_ERROR_INVALID_PARAM    Invalid Parameter
		 */
		static int32_t dac_nco_get_phase_offset(const dac_dac_select_t dacs, uint16_t* dacs_po,
			const dac_channel_select_t channels, uint16_t* ch_po);

		/**
		 * \brief  Set NCO phase offset
		 *
		 * Sets main datapath and/or channel datapath NCO phase offset.
		 *
		 * \param h     Pointer to the AD917x device reference handle.
		 * \param dacs  DAC number
		 *              AD917X_DAC0 - DAC0 NCO
		 *              AD917X_DAC1 - DAC1 NCO
		 *
		 * \param channels  Channel number
		 *              AD917X_CH_0 - Channel 0 NCO
		 *              AD917X_CH_1 - Channel 1 NCO
		 *              AD917X_CH_2 - Channel 2 NCO
		 *              AD917X_CH_3 - Channel 3 NCO
		 *              AD917X_CH_4 - Channel 4 NCO
		 *              AD917X_CH_5 - Channel 5 NCO
		 * \param dacs_po  Pointer to the phase offset for the selected DAC NCO(s)
		 * \param ch_po  Pointer to the phase offset for the selected channel NCO(s)
		 *
		* \retval API_ERROR_OK API Completed Successfully
		 * \retval API_ERROR_INVALID_HANDLE_PTR Invalid reference handle.
		 * \retval API_ERROR_INVALID_XFER_PTR SPI Access Failed
		 * \retval API_ERROR_INVALID_PARAM    Invalid Parameter
		 */
		static int32_t dac_nco_set_phase_offset(const dac_dac_select_t dacs, const uint16_t dacs_po,
			const dac_channel_select_t channels, const uint16_t ch_po);

		/**
		 * \brief  Set FTW, ACC and MOD values
		 *
		 * Set FTW, ACC and MOD values for the paged NCO. The page has to be selected in advance.
		 *
		 * \param h     Pointer to the AD917x device reference handle.
		 * \param nco   Channel or Main data path select
		 *              AD917X_DDSM - Main data path select
		 *              AD917X_DDSC - Channel data path select
		 * \param ftw  FTW value
		 * \param acc_modulus  Modulus value
		 * \param acc_delta  Delta value
		 *
		* \retval API_ERROR_OK API Completed Successfully
		 * \retval API_ERROR_INVALID_HANDLE_PTR Invalid reference handle.
		 * \retval API_ERROR_INVALID_XFER_PTR SPI Access Failed
		 * \retval API_ERROR_INVALID_PARAM    Invalid Parameter
		 */
		static int32_t dac_nco_set_ftw(
			const dac_dds_select_t nco,
			const uint64_t ftw,
			const uint64_t acc_modulus,
			const uint64_t acc_delta);

		/**
		 * \brief  Get FTW, ACC and MOD values
		 *
		 * Get FTW, ACC and MOD values for the paged NCO. The page has to be selected in advance.
		 *
		 * \param h     Pointer to the AD917x device reference handle.
		 * \param nco   Channel or Main data path select
		 *              AD917X_DDSM - Main data path select
		 *              AD917X_DDSC - Channel data path select
		 * \param ftw  Pointer to the FTW value, Set to NULL if data not required
		 * \param acc_modulus  Pointer to the Modulus value, Set to NULL if data not required
		 * \param acc_delta  Pointer to the Delta value, Set to NULL if data not required
		 *
		* \retval API_ERROR_OK API Completed Successfully
		 * \retval API_ERROR_INVALID_HANDLE_PTR Invalid reference handle.
		 * \retval API_ERROR_INVALID_XFER_PTR SPI Access Failed
		 * \retval API_ERROR_INVALID_PARAM    Invalid Parameter
		 */
		static int32_t dac_nco_get_ftw(const dac_dds_select_t nco, uint64_t* ftw,
			uint64_t* acc_modulus, uint64_t* acc_delta);

		/**
		 * \brief  Set NCO
		 *
		 * Set NCO to produce a desired frequency with a desired amplitude
		 *
		 * \param h     Pointer to the AD917x device reference handle.
		 * \param dacs   Main data path DAC NCO select
		 *              AD917X_DAC0 - DAC0 NCO select
		 *              AD917X_DAC1 - DAC1 NCO select
		 * \param channels  Channel number
		 *              AD917X_CH_0 - Channel 0 NCO
		 *              AD917X_CH_1 - Channel 1 NCO
		 *              AD917X_CH_2 - Channel 2 NCO
		 *              AD917X_CH_3 - Channel 3 NCO
		 *              AD917X_CH_4 - Channel 4 NCO
		 *              AD917X_CH_5 - Channel 5 NCO
		 * \param carrier_freq_hz  Desired carrier frequency in Hz
		 * \param amplitude  Desired amplitude value
		 * \param dc_test_tone_en - enable test tone
		 * \param ddsm_cal_dc_input_en - enable main datapath test tone
		 *
		* \retval API_ERROR_OK API Completed Successfully
		 * \retval API_ERROR_INVALID_HANDLE_PTR Invalid reference handle.
		 * \retval API_ERROR_INVALID_XFER_PTR SPI Access Failed
		 * \retval API_ERROR_INVALID_PARAM    Invalid Parameter
		 */
		int32_t dac_nco_set(
			const dac_dac_select_t dacs,
			const dac_channel_select_t channels,
			const int64_t carrier_freq_hz,
			const uint16_t amplitude,
			int32_t dc_test_tone_en,
			int32_t ddsm_cal_dc_input_en);

		/**
		 * \brief  NCO Enable
		 *
		 * Enable/Disable NCOs. Enables only the DACs and Channel NCOs provided as
		 * parameters. All other DACs and Channel NCOs are disabled.
		 *
		 * \param h     Pointer to the AD917x device reference handle.
		 * \param dacs   Main data path DAC NCO select
		 *              AD917X_DAC0 - DAC0 NCO select
		 *              AD917X_DAC1 - DAC1 NCO select
		 * \param channels  Channel number
		 *              AD917X_CH_0 - Channel 0 NCO
		 *              AD917X_CH_1 - Channel 1 NCO
		 *              AD917X_CH_2 - Channel 2 NCO
		 *              AD917X_CH_3 - Channel 3 NCO
		 *              AD917X_CH_4 - Channel 4 NCO
		 *              AD917X_CH_5 - Channel 5 NCO
		 *
		 * \retval API_ERROR_OK API Completed Successfully
		 * \retval API_ERROR_INVALID_HANDLE_PTR Invalid reference handle.
		 * \retval API_ERROR_INVALID_XFER_PTR SPI Access Failed
		 * \retval API_ERROR_INVALID_PARAM    Invalid Parameter
		 */
		static int32_t dac_nco_enable(const dac_dac_select_t dacs, const dac_channel_select_t channels);

		/**
		 * \brief  Set Main DAC Cal DC Input
		 *
		 * Set Main DAC Cal DC Input
		 *
		 * \param h     Pointer to the AD917x device reference handle.
		 * \param ddsm_cal_dc_input_en  Enable flag:
		 *				0 - Disabled
		 *				1 - Enabled

		 *
		 * \retval API_ERROR_OK API Completed Successfully
		 * \retval API_ERROR_INVALID_HANDLE_PTR Invalid reference handle.
		 * \retval API_ERROR_INVALID_XFER_PTR SPI Access Failed
		 * \retval API_ERROR_INVALID_PARAM    Invalid Parameter
		 */
		int32_t dac_ddsm_cal_dc_input_set(int32_t ddsm_cal_dc_input_en);

		/**
		 * \brief  Get Main DAC Cal DC Input
		 *
		 * Get Main DAC Cal DC Input
		 *
		 * \param h     Pointer to the AD917x device reference handle.
		 * \param ddsm_cal_dc_input_en  Pointer to integer, where the result will be storred
		 *              0 - Disabled
		 *              1 - Enabled

		 *
		 * \retval API_ERROR_OK API Completed Successfully
		 * \retval API_ERROR_INVALID_HANDLE_PTR Invalid reference handle.
		 * \retval API_ERROR_INVALID_XFER_PTR SPI Access Failed
		 * \retval API_ERROR_INVALID_PARAM    Invalid Parameter
		 */
		int32_t dac_ddsm_cal_dc_input_get(int32_t* ddsm_cal_dc_input_en);

		/**
		 * \brief  Set DC Test Tone enable status
		 *
		 * Set DC Test Tone enable status.
		 *
		 * \param h     Pointer to the AD917x device reference handle.
		 * \param dc_test_tone_en  Enable flag:
		 *				0 - Disabled
		 *				1 - Enabled
		 *
		 * \retval API_ERROR_OK API Completed Successfully
		 * \retval API_ERROR_INVALID_HANDLE_PTR Invalid reference handle.
		 * \retval API_ERROR_INVALID_XFER_PTR SPI Access Failed
		 * \retval API_ERROR_INVALID_PARAM    Invalid Parameter
		 */
		int32_t dac_dc_test_tone_set(int32_t dc_test_tone_en);

		/**
		 * \brief  Get DC Test Tone enable status
		 *
		 * Get DC Test Tone enable status.
		 *
		 * \param h     Pointer to the AD917x device reference handle.
		 * \param dc_test_tone_en  Pointer to integer, where the result will be storred
		 *              0 - Disabled
		 *              1 - Enabled
		 *
		 * \retval API_ERROR_OK API Completed Successfully
		 * \retval API_ERROR_INVALID_HANDLE_PTR Invalid reference handle.
		 * \retval API_ERROR_INVALID_XFER_PTR SPI Access Failed
		 * \retval API_ERROR_INVALID_PARAM    Invalid Parameter
		 */
		int32_t dac_dc_test_tone_get(int32_t* dc_test_tone_en);

		/**
		 * \brief  Get Channel NCO frequency.
		 *
		 * Get a Channel NCO frequency in Hz.
		 *
		 * \param h     Pointer to the AD917x device reference handle.
		 * \param channel  Channel number. Can be only one of:
		 *              AD917X_CH_0 - Channel 0 NCO
		 *              AD917X_CH_1 - Channel 1 NCO
		 *              AD917X_CH_2 - Channel 2 NCO
		 *              AD917X_CH_3 - Channel 3 NCO
		 *              AD917X_CH_4 - Channel 4 NCO
		 *              AD917X_CH_5 - Channel 5 NCO
		 *
		 * \param carrier_freq_hz - pointer to 64 bit integer, where the
		 * result frequency will be storred
		 * \retval API_ERROR_OK API Completed Successfully
		 * \retval API_ERROR_INVALID_HANDLE_PTR Invalid reference handle.
		 * \retval API_ERROR_INVALID_XFER_PTR SPI Access Failed
		 * \retval API_ERROR_INVALID_PARAM    Invalid Parameter
		 */
		int32_t dac_nco_channel_freq_get(dac_channel_select_t channel, int64_t* carrier_freq_hz);

		/**
		 * \brief  Get Main DAC NCO frequency.
		 *
		 * Get a Main DAC NCO frequency in Hz.
		 *
		 * \param h     Pointer to the AD917x device reference handle.
		 * \param dac   Main data path DAC NCO select. Can be only one of:
		 *              AD917X_DAC0 - DAC0 NCO select
		 *              AD917X_DAC1 - DAC1 NCO select
		 * \param carrier_freq_hz  pointer to 64 bit integer, where the
		 * result frequency will be storred
		 *
		 * \retval API_ERROR_OK API Completed Successfully
		 * \retval API_ERROR_INVALID_HANDLE_PTR Invalid reference handle.
		 * \retval API_ERROR_INVALID_XFER_PTR SPI Access Failed
		 * \retval API_ERROR_INVALID_PARAM    Invalid Parameter
		 */
		int32_t dac_nco_main_freq_get(dac_dac_select_t dac, int64_t* carrier_freq_hz);

		static int32_t dac_register_write(const uint16_t address, uint32_t data);

		static int32_t dac_register_read(const uint16_t address, uint32_t* data);

		static int32_t dac_register_write_tbl(struct adi_reg_data* tbl, uint32_t count);

		static int32_t dac_register_read_block(const uint16_t address, uint32_t* data, uint32_t count);

		static int32_t jesd_get_link_count(uint8_t* link_count);

		static int32_t jesd_set_link(int32_t link);

	};

	

}


#endif /* !__AD917XAPI_H__ */

