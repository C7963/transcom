#pragma once
#include <stdint.h>
#include "api_def.h"

#ifndef XILINX_JESD204B_H_
#define XILINX_JESD204B_H_

#define XLNX_JESD204_REG_VERSION		0x000
#define XLNX_JESD204_VERSION_MAJOR(x)		(((x) >> 24) & 0xFF)
#define XLNX_JESD204_VERSION_MINOR(x)		(((x) >> 16) & 0xFF)
#define XLNX_JESD204_VERSION_REV(x)		(((x) >> 8) & 0xFF)

#define XLNX_JESD204_REG_CONFIG			0x004
//#define XLNX_JESD204_CONFIG				(1 << 0)

#define XLNX_JESD204_REG_RESET			0x020
#define XLNX_JESD204_RESET			(1 << 0)

#define XLNX_JESD204_REG_CTRL_ENABLE			0x024
//#define XLNX_JESD204_CTRL_ENABLE			(1 << 0)

#define XLNX_JESD204_REG_CTRL_TX_SYNC			0x028
//#define XLNX_JESD204_CTRL_TX_SYNC			(1 << 0)

#define XLNX_JESD204_REG_CTRL_MB_IN_EMB			0x030
//#define XLNX_JESD204_CTRL_MB_IN_EMB			(1 << 0)

#define XLNX_JESD204_REG_CTRL_SUB_CLASS			0x034
#define XLNX_JESD204_LANE_SUBCLASS(x)		(((x) >> 0) & 0x7)

#define XLNX_JESD204_REG_CTRL_META_MODE			0x038
//#define XLNX_JESD204_CTRL_MB_IN_EMB			(1 << 0)

#define XLNX_JESD204_REG_CTRL_8B10B_CFG			0x03C
//#define XLNX_JESD204_CTRL_MB_IN_EMB			(1 << 0)

#define XLNX_JESD204_REG_CTRL_LANE_ENA			0x040
//#define XLNX_JESD204_CTRL_MB_IN_EMB			(1 << 0)

#define XLNX_JESD204_CTRL_TX_ILA_CFG0		    0x070
//#define XLNX_JESD204_CTRL_TX_ILA_CFG0			(1 << 0)

#define XLNX_JESD204_CTRL_TX_ILA_CFG1		    0x074
//#define XLNX_JESD204_CTRL_TX_ILA_CFG0			(1 << 0)


#define XLNX_JESD204_CTRL_TX_ILA_CFG2		    0x078
//#define XLNX_JESD204_CTRL_TX_ILA_CFG0			(1 << 0)


#define XLNX_JESD204_CTRL_SYSREF		        0x050
//#define XLNX_JESD204_CTRL_SYSREF		       (1 << 0)

#define XLNX_DAC_JESD204_RESET		        0x1000

#define XLNX_JESD204_LN_EVEN(x)    (((x)&0xff) << 0)
#define XLNX_JESD204_LN_EVEN1(x)    (((x)&0x1f) << 8)
#define XLNX_JESD204_LN_EVEN2(x)    (((x)&0x1f) << 16)
#define XLNX_JESD204_LN_EVEN3(x)    (((x)&0x3) << 24)

#define XLNX_JESD204_LN_EVEN4(x)    (((x)&0x1F) << 8)
#define XLNX_JESD204_LN_EVEN5(x)    (((x)&0x1) << 16)
#define XLNX_JESD204_LN_EVEN6(x)    (((x)&0x1F) << 24)

using namespace API_DEF;

namespace JESD
{
	union uint32_chararray
	{
		char c[4];
		int i;
	};

	struct jesd_init_param {
		struct device* dev;
		void* regs;
		void* phy;
		struct clk* clk;
		uint32_t lanes;
		uint32_t vers_id;
		uint32_t addr;
		uint32_t band;
		uint32_t transmit;
		uint32_t pll;
		uint64_t rate;
		jesd_param_t* jesd_param;
		uint32_t jesd_subclass;
	};

	class Jesd204Api
	{
	public:
		static int32_t adc_jesd_init(jesd_init_param* init_param);
		static int32_t dac_jesd_init(jesd_init_param* init_param);

		static int adc_jesd_reset(uint8_t hw_reset);
		static int adc_jesd_subclass_set(uint8_t subclass);
		static int adc_jesd_8B10B_set();
		static int adc_jesd_register_write(const uint16_t address, const uint32_t data);
		static int adc_jesd_register_read(const uint16_t address, uint32_t* data);

		static int dac_jesd_reset();
		static int dac_jesd_reset1(uint8_t hw_reset);
		static int dac_jesd_8B10B_set();
		static int dac_jesd_subclass_set(uint8_t subclass);
		static int dac_jesd_tx_ila_cfg0_set();
		static int dac_jesd_tx_ila_cfg1_set(jesd_param_t* jesd_param);
		static int dac_jesd_tx_ila_cfg2_set(jesd_param_t* jesd_param);
		static int dac_jesd_ctrl_sysref_set();
		static int dac_jesd_register_write(const uint16_t address, const uint32_t data);
		static int dac_jesd_register_read(const uint16_t address, uint32_t* data);

	private:

	};
}



#endif /* ADI_JESD204B_V51_H_ */
