#include "PlayBack.h"
#include "Device_MEM32.h"

using namespace PLAYBACK;

void AUX_CTRL::set_clk_request(bool flag)
{
	if (flag)
	{
		pcie_mem->SendData(CLK_DIV_CDDCREQ, (unsigned int)1);
	}
	else
	{
		pcie_mem->SendData(CLK_DIV_CDDCREQ, (unsigned int)0);
	}
}

void AUX_CTRL::interpolation_fir_dout_shift(uint32_t value)
{
	pcie_mem->SendData(DATA_SHIFT, (unsigned int)value);
}

void AUX_CTRL::interpolation_data_bypass(bool bypass)
{
	if (bypass)
	{
		pcie_mem->SendData(DATA_BYPASS, (unsigned int)1);
		//pcie_mem->SendData(DATA_BYPASS, (unsigned int)0x101);
		//pcie_mem->SendData(DATA_BYPASS, (unsigned int)1);
	}
	else
	{
		pcie_mem->SendData(DATA_BYPASS, (unsigned int)0);
		pcie_mem->SendData(DATA_BYPASS, (unsigned int)0x100);
		pcie_mem->SendData(DATA_BYPASS, (unsigned int)0);
	}
}

void CLK_DIV::set_clkout0(freq_list_1228d8MHz freq)
{
	switch (freq)
	{
	case freq_list_1228d8MHz::freq307d2MHz:
		pcie_mem->SendData(CLKOUT0_REG1, (unsigned int)0x1041);
		pcie_mem->SendData(CLKOUT0_REG2, (unsigned int)0x2800);
		pcie_mem->SendData(CLKOUT5_REG2, (unsigned int)0xa0c0);
		pcie_mem->SendData(CLKOUT6_REG2, (unsigned int)0xa0c0);
		pcie_mem->SendData(DIV_CLK_REG, (unsigned int)0x1041);
		pcie_mem->SendData(CLKFBOUT_REG1, (unsigned int)0x1041);
		pcie_mem->SendData(CLKFBOUT_REG2, (unsigned int)0x2800);
		pcie_mem->SendData(LOCK_REG1, (unsigned int)0x03e8);
		pcie_mem->SendData(LOCK_REG2, (unsigned int)0x2001);
		pcie_mem->SendData(LOCK_REG3, (unsigned int)0x23e9);
		pcie_mem->SendData(FILTER_REG1, (unsigned int)0x0800);
		pcie_mem->SendData(FILTER_REG2, (unsigned int)0x9990);
		break;
	case freq_list_1228d8MHz::freq204d8MHz:
		pcie_mem->SendData(CLKOUT0_REG1, (unsigned int)0x1041);
		pcie_mem->SendData(CLKOUT0_REG2, (unsigned int)0x7c00);
		pcie_mem->SendData(CLKOUT5_REG2, (unsigned int)0x70c0);
		pcie_mem->SendData(CLKOUT6_REG2, (unsigned int)0xa0c0);
		pcie_mem->SendData(DIV_CLK_REG, (unsigned int)0x1041);
		pcie_mem->SendData(CLKFBOUT_REG1, (unsigned int)0x1041);
		pcie_mem->SendData(CLKFBOUT_REG2, (unsigned int)0x2800);
		pcie_mem->SendData(LOCK_REG1, (unsigned int)0x03e8);
		pcie_mem->SendData(LOCK_REG2, (unsigned int)0x2001);
		pcie_mem->SendData(LOCK_REG3, (unsigned int)0x23e9);
		pcie_mem->SendData(FILTER_REG1, (unsigned int)0x0800);
		pcie_mem->SendData(FILTER_REG2, (unsigned int)0x9990);
		break;
	case freq_list_1228d8MHz::freq153d6MHz:
		pcie_mem->SendData(CLKOUT0_REG1, (unsigned int)0x1082);
		pcie_mem->SendData(CLKOUT0_REG2, (unsigned int)0x4c00);
		pcie_mem->SendData(CLKOUT5_REG2, (unsigned int)0x50c0);
		pcie_mem->SendData(CLKOUT6_REG2, (unsigned int)0xa0c0);
		pcie_mem->SendData(DIV_CLK_REG, (unsigned int)0x1041);
		pcie_mem->SendData(CLKFBOUT_REG1, (unsigned int)0x1041);
		pcie_mem->SendData(CLKFBOUT_REG2, (unsigned int)0x2800);
		pcie_mem->SendData(LOCK_REG1, (unsigned int)0x03e8);
		pcie_mem->SendData(LOCK_REG2, (unsigned int)0x2001);
		pcie_mem->SendData(LOCK_REG3, (unsigned int)0x23e9);
		pcie_mem->SendData(FILTER_REG1, (unsigned int)0x0800);
		pcie_mem->SendData(FILTER_REG2, (unsigned int)0x9990);
		break;
	case freq_list_1228d8MHz::freq122d88MHz:
		pcie_mem->SendData(CLKOUT0_REG1, (unsigned int)0x10c3);
		pcie_mem->SendData(CLKOUT0_REG2, (unsigned int)0x1c00);
		pcie_mem->SendData(CLKOUT5_REG2, (unsigned int)0x00c0);
		pcie_mem->SendData(CLKOUT6_REG2, (unsigned int)0xa0c0);
		pcie_mem->SendData(DIV_CLK_REG, (unsigned int)0x1041);
		pcie_mem->SendData(CLKFBOUT_REG1, (unsigned int)0x1041);
		pcie_mem->SendData(CLKFBOUT_REG2, (unsigned int)0x2800);
		pcie_mem->SendData(LOCK_REG1, (unsigned int)0x03e8);
		pcie_mem->SendData(LOCK_REG2, (unsigned int)0x2001);
		pcie_mem->SendData(LOCK_REG3, (unsigned int)0x23e9);
		pcie_mem->SendData(FILTER_REG1, (unsigned int)0x0800);
		pcie_mem->SendData(FILTER_REG2, (unsigned int)0x9990);
		break;
	case freq_list_1228d8MHz::freq102d4MHz:
		pcie_mem->SendData(CLKOUT0_REG1, (unsigned int)0x1104);
		pcie_mem->SendData(CLKOUT0_REG2, (unsigned int)0x6800);
		pcie_mem->SendData(CLKOUT5_REG2, (unsigned int)0xe0c0);
		pcie_mem->SendData(CLKOUT6_REG2, (unsigned int)0xa0c0);
		pcie_mem->SendData(DIV_CLK_REG, (unsigned int)0x1041);
		pcie_mem->SendData(CLKFBOUT_REG1, (unsigned int)0x1041);
		pcie_mem->SendData(CLKFBOUT_REG2, (unsigned int)0x2800);
		pcie_mem->SendData(LOCK_REG1, (unsigned int)0x03e8);
		pcie_mem->SendData(LOCK_REG2, (unsigned int)0x2001);
		pcie_mem->SendData(LOCK_REG3, (unsigned int)0x23e9);
		pcie_mem->SendData(FILTER_REG1, (unsigned int)0x0800);
		pcie_mem->SendData(FILTER_REG2, (unsigned int)0x9990);
		break;
	case freq_list_1228d8MHz::freq81d92MHz:
		pcie_mem->SendData(CLKOUT0_REG1, (unsigned int)0x1145);
		pcie_mem->SendData(CLKOUT0_REG2, (unsigned int)0x2c00);
		pcie_mem->SendData(CLKOUT5_REG2, (unsigned int)0x30c0);
		pcie_mem->SendData(CLKOUT6_REG2, (unsigned int)0x00c0);
		pcie_mem->SendData(DIV_CLK_REG, (unsigned int)0x01c8);
		pcie_mem->SendData(CLKFBOUT_REG1, (unsigned int)0x1619);
		pcie_mem->SendData(CLKFBOUT_REG2, (unsigned int)0x0080);
		pcie_mem->SendData(LOCK_REG1, (unsigned int)0x00fa);
		pcie_mem->SendData(LOCK_REG2, (unsigned int)0x7c01);
		pcie_mem->SendData(LOCK_REG3, (unsigned int)0x7fe9);
		pcie_mem->SendData(FILTER_REG1, (unsigned int)0x0800);
		pcie_mem->SendData(FILTER_REG2, (unsigned int)0x9090);
		break;
	case freq_list_1228d8MHz::freq76d8MHz:
		pcie_mem->SendData(CLKOUT0_REG1, (unsigned int)0x1187);
		pcie_mem->SendData(CLKOUT0_REG2, (unsigned int)0x0080);
		pcie_mem->SendData(CLKOUT5_REG2, (unsigned int)0x00c0);
		pcie_mem->SendData(CLKOUT6_REG2, (unsigned int)0xa0c0);
		pcie_mem->SendData(DIV_CLK_REG, (unsigned int)0x1041);
		pcie_mem->SendData(CLKFBOUT_REG1, (unsigned int)0x1041);
		pcie_mem->SendData(CLKFBOUT_REG2, (unsigned int)0x2800);
		pcie_mem->SendData(LOCK_REG1, (unsigned int)0x03e8);
		pcie_mem->SendData(LOCK_REG2, (unsigned int)0x2001);
		pcie_mem->SendData(LOCK_REG3, (unsigned int)0x23e9);
		pcie_mem->SendData(FILTER_REG1, (unsigned int)0x0800);
		pcie_mem->SendData(FILTER_REG2, (unsigned int)0x9990);
		break;
	case freq_list_1228d8MHz::freq61d44MHz:
		pcie_mem->SendData(CLKOUT0_REG1, (unsigned int)0x11c7);
		pcie_mem->SendData(CLKOUT0_REG2, (unsigned int)0x2c00);
		pcie_mem->SendData(CLKOUT5_REG2, (unsigned int)0x30c0);
		pcie_mem->SendData(CLKOUT6_REG2, (unsigned int)0xa0c0);
		pcie_mem->SendData(DIV_CLK_REG, (unsigned int)0x1041);
		pcie_mem->SendData(CLKFBOUT_REG1, (unsigned int)0x1041);
		pcie_mem->SendData(CLKFBOUT_REG2, (unsigned int)0x2800);
		pcie_mem->SendData(LOCK_REG1, (unsigned int)0x03e8);
		pcie_mem->SendData(LOCK_REG2, (unsigned int)0x2001);
		pcie_mem->SendData(LOCK_REG3, (unsigned int)0x23e9);
		pcie_mem->SendData(FILTER_REG1, (unsigned int)0x0800);
		pcie_mem->SendData(FILTER_REG2, (unsigned int)0x9990);
		break;
	case freq_list_1228d8MHz::freq51d2MHz:
		pcie_mem->SendData(CLKOUT0_REG1, (unsigned int)0x1249);
		pcie_mem->SendData(CLKOUT0_REG2, (unsigned int)0x4800);
		pcie_mem->SendData(CLKOUT5_REG2, (unsigned int)0xc0c0);
		pcie_mem->SendData(CLKOUT6_REG2, (unsigned int)0xa0c0);
		pcie_mem->SendData(DIV_CLK_REG, (unsigned int)0x1041);
		pcie_mem->SendData(CLKFBOUT_REG1, (unsigned int)0x1041);
		pcie_mem->SendData(CLKFBOUT_REG2, (unsigned int)0x2800);
		pcie_mem->SendData(LOCK_REG1, (unsigned int)0x03e8);
		pcie_mem->SendData(LOCK_REG2, (unsigned int)0x2001);
		pcie_mem->SendData(LOCK_REG3, (unsigned int)0x23e9);
		pcie_mem->SendData(FILTER_REG1, (unsigned int)0x0800);
		pcie_mem->SendData(FILTER_REG2, (unsigned int)0x9990);
		break;
	case freq_list_1228d8MHz::freq40d96MHz:
		pcie_mem->SendData(CLKOUT0_REG1, (unsigned int)0x12cb);
		pcie_mem->SendData(CLKOUT0_REG2, (unsigned int)0x3c00);
		pcie_mem->SendData(CLKOUT5_REG2, (unsigned int)0x30c0);
		pcie_mem->SendData(CLKOUT6_REG2, (unsigned int)0xa0c0);
		pcie_mem->SendData(DIV_CLK_REG, (unsigned int)0x1041);
		pcie_mem->SendData(CLKFBOUT_REG1, (unsigned int)0x1041);
		pcie_mem->SendData(CLKFBOUT_REG2, (unsigned int)0x2800);
		pcie_mem->SendData(LOCK_REG1, (unsigned int)0x03e8);
		pcie_mem->SendData(LOCK_REG2, (unsigned int)0x2001);
		pcie_mem->SendData(LOCK_REG3, (unsigned int)0x23e9);
		pcie_mem->SendData(FILTER_REG1, (unsigned int)0x0800);
		pcie_mem->SendData(FILTER_REG2, (unsigned int)0x9990);
		break;
	case freq_list_1228d8MHz::freq38d4MHz:
		pcie_mem->SendData(CLKOUT0_REG1, (unsigned int)0x134d);
		pcie_mem->SendData(CLKOUT0_REG2, (unsigned int)0x0);
		pcie_mem->SendData(CLKOUT5_REG2, (unsigned int)0x00c0);
		pcie_mem->SendData(CLKOUT6_REG2, (unsigned int)0xa0c0);
		pcie_mem->SendData(DIV_CLK_REG, (unsigned int)0x1041);
		pcie_mem->SendData(CLKFBOUT_REG1, (unsigned int)0x1041);
		pcie_mem->SendData(CLKFBOUT_REG2, (unsigned int)0x2800);
		pcie_mem->SendData(LOCK_REG1, (unsigned int)0x03e8);
		pcie_mem->SendData(LOCK_REG2, (unsigned int)0x2001);
		pcie_mem->SendData(LOCK_REG3, (unsigned int)0x23e9);
		pcie_mem->SendData(FILTER_REG1, (unsigned int)0x0800);
		pcie_mem->SendData(FILTER_REG2, (unsigned int)0x9990);
		break;
	case freq_list_1228d8MHz::freq30d72MHz:
		pcie_mem->SendData(CLKOUT0_REG1, (unsigned int)0x1410);
		pcie_mem->SendData(CLKOUT0_REG2, (unsigned int)0x0);
		pcie_mem->SendData(CLKOUT5_REG2, (unsigned int)0x00c0);
		pcie_mem->SendData(CLKOUT6_REG2, (unsigned int)0x00c0);
		pcie_mem->SendData(DIV_CLK_REG, (unsigned int)0x0083);
		pcie_mem->SendData(CLKFBOUT_REG1, (unsigned int)0x1208);
		pcie_mem->SendData(CLKFBOUT_REG2, (unsigned int)0x0);
		pcie_mem->SendData(LOCK_REG1, (unsigned int)0x0271);
		pcie_mem->SendData(LOCK_REG2, (unsigned int)0x7c01);
		pcie_mem->SendData(LOCK_REG3, (unsigned int)0x7fe9);
		pcie_mem->SendData(FILTER_REG1, (unsigned int)0x0800);
		pcie_mem->SendData(FILTER_REG2, (unsigned int)0x1190);
		break;
	case freq_list_1228d8MHz::freq25d6MHz:
		pcie_mem->SendData(CLKOUT0_REG1, (unsigned int)0x1451);
		pcie_mem->SendData(CLKOUT0_REG2, (unsigned int)0x7800);
		pcie_mem->SendData(CLKOUT5_REG2, (unsigned int)0xe0c0);
		pcie_mem->SendData(CLKOUT6_REG2, (unsigned int)0xe0c0);
		pcie_mem->SendData(DIV_CLK_REG, (unsigned int)0x0186);
		pcie_mem->SendData(CLKFBOUT_REG1, (unsigned int)0x1451);
		pcie_mem->SendData(CLKFBOUT_REG2, (unsigned int)0x7800);
		pcie_mem->SendData(LOCK_REG1, (unsigned int)0x0113);
		pcie_mem->SendData(LOCK_REG2, (unsigned int)0x7c01);
		pcie_mem->SendData(LOCK_REG3, (unsigned int)0x7fe9);
		pcie_mem->SendData(FILTER_REG1, (unsigned int)0x0800);
		pcie_mem->SendData(FILTER_REG2, (unsigned int)0x1890);
		break;
	case freq_list_1228d8MHz::freq20d48MHz:
		pcie_mem->SendData(CLKOUT0_REG1, (unsigned int)0x1514);
		pcie_mem->SendData(CLKOUT0_REG2, (unsigned int)0x6800);
		pcie_mem->SendData(CLKOUT5_REG2, (unsigned int)0xe0c0);
		pcie_mem->SendData(CLKOUT6_REG2, (unsigned int)0xe0c0);
		pcie_mem->SendData(DIV_CLK_REG, (unsigned int)0x01c8);
		pcie_mem->SendData(CLKFBOUT_REG1, (unsigned int)0x1514);
		pcie_mem->SendData(CLKFBOUT_REG2, (unsigned int)0x6800);
		pcie_mem->SendData(LOCK_REG1, (unsigned int)0x00fa);
		pcie_mem->SendData(LOCK_REG2, (unsigned int)0x7c01);
		pcie_mem->SendData(LOCK_REG3, (unsigned int)0x7fe9);
		pcie_mem->SendData(FILTER_REG1, (unsigned int)0x0800);
		pcie_mem->SendData(FILTER_REG2, (unsigned int)0x8890);
		break;
	case freq_list_1228d8MHz::freq15d36MHz:
		pcie_mem->SendData(CLKOUT0_REG1, (unsigned int)0x1659);
		pcie_mem->SendData(CLKOUT0_REG2, (unsigned int)0x4800);
		pcie_mem->SendData(CLKOUT5_REG2, (unsigned int)0xc0c0);
		pcie_mem->SendData(CLKOUT6_REG2, (unsigned int)0x70c0);
		pcie_mem->SendData(DIV_CLK_REG, (unsigned int)0x0083);
		pcie_mem->SendData(CLKFBOUT_REG1, (unsigned int)0x1145);
		pcie_mem->SendData(CLKFBOUT_REG2, (unsigned int)0x7c00);
		pcie_mem->SendData(LOCK_REG1, (unsigned int)0x0339);
		pcie_mem->SendData(LOCK_REG2, (unsigned int)0x7c01);
		pcie_mem->SendData(LOCK_REG3, (unsigned int)0x7fe9);
		pcie_mem->SendData(FILTER_REG1, (unsigned int)0x0800);
		pcie_mem->SendData(FILTER_REG2, (unsigned int)0x9190);
		break;
	case freq_list_1228d8MHz::freq12d8MHz:
		pcie_mem->SendData(CLKOUT0_REG1, (unsigned int)0x175d);
		pcie_mem->SendData(CLKOUT0_REG2, (unsigned int)0x2800);
		pcie_mem->SendData(CLKOUT5_REG2, (unsigned int)0xa0c0);
		pcie_mem->SendData(CLKOUT6_REG2, (unsigned int)0xe0c0);
		pcie_mem->SendData(DIV_CLK_REG, (unsigned int)0x0082);
		pcie_mem->SendData(CLKFBOUT_REG1, (unsigned int)0x1104);
		pcie_mem->SendData(CLKFBOUT_REG2, (unsigned int)0x7800);
		pcie_mem->SendData(LOCK_REG1, (unsigned int)0x03e8);
		pcie_mem->SendData(LOCK_REG2, (unsigned int)0x6401);
		pcie_mem->SendData(LOCK_REG3, (unsigned int)0x67e9);
		pcie_mem->SendData(FILTER_REG1, (unsigned int)0x0800);
		pcie_mem->SendData(FILTER_REG2, (unsigned int)0x1990);
		break;
	default:

		break;
	}
	pcie_mem->SendData(CCR, (unsigned int)0x03);
}

void CLK_DIV::set_clkout1(freq_list_819d2MHz freq)
{
	switch (freq)
	{
	case freq_list_819d2MHz::freq204d8MHz:
		pcie_mem->SendData(CLKOUT0_REG1, (unsigned int)0x1041);
		pcie_mem->SendData(CLKOUT0_REG2, (unsigned int)0x7c00);
		pcie_mem->SendData(CLKOUT5_REG2, (unsigned int)0x70c0);
		pcie_mem->SendData(CLKOUT6_REG2, (unsigned int)0x70c0);
		pcie_mem->SendData(DIV_CLK_REG, (unsigned int)0x1041);
		pcie_mem->SendData(CLKFBOUT_REG1, (unsigned int)0x1041);
		pcie_mem->SendData(CLKFBOUT_REG2, (unsigned int)0x7c00);
		pcie_mem->SendData(LOCK_REG1, (unsigned int)0x03e8);
		pcie_mem->SendData(LOCK_REG2, (unsigned int)0x2c01);
		pcie_mem->SendData(LOCK_REG3, (unsigned int)0x2fe9);
		pcie_mem->SendData(FILTER_REG1, (unsigned int)0x0800);
		pcie_mem->SendData(FILTER_REG2, (unsigned int)0x9990);
		break;

	case freq_list_819d2MHz::freq102d4MHz:
		pcie_mem->SendData(CLKOUT0_REG1, (unsigned int)0x1104);
		pcie_mem->SendData(CLKOUT0_REG2, (unsigned int)0x6800);
		pcie_mem->SendData(CLKOUT5_REG2, (unsigned int)0xe0c0);
		pcie_mem->SendData(CLKOUT6_REG2, (unsigned int)0x70c0);
		pcie_mem->SendData(DIV_CLK_REG, (unsigned int)0x1041);
		pcie_mem->SendData(CLKFBOUT_REG1, (unsigned int)0x1041);
		pcie_mem->SendData(CLKFBOUT_REG2, (unsigned int)0x7c00);
		pcie_mem->SendData(LOCK_REG1, (unsigned int)0x03e8);
		pcie_mem->SendData(LOCK_REG2, (unsigned int)0x2c01);
		pcie_mem->SendData(LOCK_REG3, (unsigned int)0x2fe9);
		pcie_mem->SendData(FILTER_REG1, (unsigned int)0x0800);
		pcie_mem->SendData(FILTER_REG2, (unsigned int)0x9990);
		break;
	case freq_list_819d2MHz::freq68d265MHz:
		pcie_mem->SendData(CLKOUT0_REG1, (unsigned int)0x1186);
		pcie_mem->SendData(CLKOUT0_REG2, (unsigned int)0x5c00);
		pcie_mem->SendData(CLKOUT5_REG2, (unsigned int)0x50c0);
		pcie_mem->SendData(CLKOUT6_REG2, (unsigned int)0x70c0);
		pcie_mem->SendData(DIV_CLK_REG, (unsigned int)0x1041);
		pcie_mem->SendData(CLKFBOUT_REG1, (unsigned int)0x1041);
		pcie_mem->SendData(CLKFBOUT_REG2, (unsigned int)0x7c00);
		pcie_mem->SendData(LOCK_REG1, (unsigned int)0x03e8);
		pcie_mem->SendData(LOCK_REG2, (unsigned int)0x2c01);
		pcie_mem->SendData(LOCK_REG3, (unsigned int)0x2fe9);
		pcie_mem->SendData(FILTER_REG1, (unsigned int)0x0800);
		pcie_mem->SendData(FILTER_REG2, (unsigned int)0x9990);
		break;
	case  freq_list_819d2MHz::freq51d2MHz:
		pcie_mem->SendData(CLKOUT0_REG1, (unsigned int)0x1249);
		pcie_mem->SendData(CLKOUT0_REG2, (unsigned int)0x4800);
		pcie_mem->SendData(CLKOUT5_REG2, (unsigned int)0xc0c0);
		pcie_mem->SendData(CLKOUT6_REG2, (unsigned int)0x70c0);
		pcie_mem->SendData(DIV_CLK_REG, (unsigned int)0x1041);
		pcie_mem->SendData(CLKFBOUT_REG1, (unsigned int)0x1041);
		pcie_mem->SendData(CLKFBOUT_REG2, (unsigned int)0x7c00);
		pcie_mem->SendData(LOCK_REG1, (unsigned int)0x03e8);
		pcie_mem->SendData(LOCK_REG2, (unsigned int)0x2c01);
		pcie_mem->SendData(LOCK_REG3, (unsigned int)0x2fe9);
		pcie_mem->SendData(FILTER_REG1, (unsigned int)0x0800);
		pcie_mem->SendData(FILTER_REG2, (unsigned int)0x9990);
		break;
	case  freq_list_819d2MHz::freq40d96MHz:
		pcie_mem->SendData(CLKOUT0_REG1, (unsigned int)0x12cb);
		pcie_mem->SendData(CLKOUT0_REG2, (unsigned int)0x3c00);
		pcie_mem->SendData(CLKOUT5_REG2, (unsigned int)0x30c0);
		pcie_mem->SendData(CLKOUT6_REG2, (unsigned int)0x70c0);
		pcie_mem->SendData(DIV_CLK_REG, (unsigned int)0x1041);
		pcie_mem->SendData(CLKFBOUT_REG1, (unsigned int)0x1041);
		pcie_mem->SendData(CLKFBOUT_REG2, (unsigned int)0x7c00);
		pcie_mem->SendData(LOCK_REG1, (unsigned int)0x03e8);
		pcie_mem->SendData(LOCK_REG2, (unsigned int)0x2c01);
		pcie_mem->SendData(LOCK_REG3, (unsigned int)0x2fe9);
		pcie_mem->SendData(FILTER_REG1, (unsigned int)0x0800);
		pcie_mem->SendData(FILTER_REG2, (unsigned int)0x9990);
		break;
	case  freq_list_819d2MHz::freq34d13MHz:
		pcie_mem->SendData(CLKOUT0_REG1, (unsigned int)0x138e);
		pcie_mem->SendData(CLKOUT0_REG2, (unsigned int)0x2800);
		pcie_mem->SendData(CLKOUT5_REG2, (unsigned int)0xa0c0);
		pcie_mem->SendData(CLKOUT6_REG2, (unsigned int)0x70c0);
		pcie_mem->SendData(DIV_CLK_REG, (unsigned int)0x1041);
		pcie_mem->SendData(CLKFBOUT_REG1, (unsigned int)0x1041);
		pcie_mem->SendData(CLKFBOUT_REG2, (unsigned int)0x7c00);
		pcie_mem->SendData(LOCK_REG1, (unsigned int)0x03e8);
		pcie_mem->SendData(LOCK_REG2, (unsigned int)0x2c01);
		pcie_mem->SendData(LOCK_REG3, (unsigned int)0x2fe9);
		pcie_mem->SendData(FILTER_REG1, (unsigned int)0x0800);
		pcie_mem->SendData(FILTER_REG2, (unsigned int)0x9990);
		break;
	case  freq_list_819d2MHz::freq27d305MHz:
		pcie_mem->SendData(CLKOUT0_REG1, (unsigned int)0x1451);
		pcie_mem->SendData(CLKOUT0_REG2, (unsigned int)0x6c00);
		pcie_mem->SendData(CLKOUT5_REG2, (unsigned int)0x70c0);
		pcie_mem->SendData(CLKOUT6_REG2, (unsigned int)0x50c0);
		pcie_mem->SendData(DIV_CLK_REG, (unsigned int)0x0083);
		pcie_mem->SendData(CLKFBOUT_REG1, (unsigned int)0x12cb);
		pcie_mem->SendData(CLKFBOUT_REG2, (unsigned int)0x4c00);
		pcie_mem->SendData(LOCK_REG1, (unsigned int)0x0190);
		pcie_mem->SendData(LOCK_REG2, (unsigned int)0x7c01);
		pcie_mem->SendData(LOCK_REG3, (unsigned int)0x7fe9);
		pcie_mem->SendData(FILTER_REG1, (unsigned int)0x0800);
		pcie_mem->SendData(FILTER_REG2, (unsigned int)0x9890);
		break;
	case  freq_list_819d2MHz::freq25d6MHz:
		pcie_mem->SendData(CLKOUT0_REG1, (unsigned int)0x1451);
		pcie_mem->SendData(CLKOUT0_REG2, (unsigned int)0x7800);
		pcie_mem->SendData(CLKOUT5_REG2, (unsigned int)0xe0c0);
		pcie_mem->SendData(CLKOUT6_REG2, (unsigned int)0xe0c0);
		pcie_mem->SendData(DIV_CLK_REG, (unsigned int)0x0104);
		pcie_mem->SendData(CLKFBOUT_REG1, (unsigned int)0x1451);
		pcie_mem->SendData(CLKFBOUT_REG2, (unsigned int)0x7800);
		pcie_mem->SendData(LOCK_REG1, (unsigned int)0x0113);
		pcie_mem->SendData(LOCK_REG2, (unsigned int)0x7c01);
		pcie_mem->SendData(LOCK_REG3, (unsigned int)0x7fe9);
		pcie_mem->SendData(FILTER_REG1, (unsigned int)0x0800);
		pcie_mem->SendData(FILTER_REG2, (unsigned int)0x1890);
		break;
	case  freq_list_819d2MHz::freq20d48MHz:
		pcie_mem->SendData(CLKOUT0_REG1, (unsigned int)0x1514);
		pcie_mem->SendData(CLKOUT0_REG2, (unsigned int)0x6800);
		pcie_mem->SendData(CLKOUT5_REG2, (unsigned int)0xe0c0);
		pcie_mem->SendData(CLKOUT6_REG2, (unsigned int)0x70c0);
		pcie_mem->SendData(DIV_CLK_REG, (unsigned int)0x0083);
		pcie_mem->SendData(CLKFBOUT_REG1, (unsigned int)0x1249);
		pcie_mem->SendData(CLKFBOUT_REG2, (unsigned int)0x7c00);
		pcie_mem->SendData(LOCK_REG1, (unsigned int)0x01f4);
		pcie_mem->SendData(LOCK_REG2, (unsigned int)0x7c01);
		pcie_mem->SendData(LOCK_REG3, (unsigned int)0x7fe9);
		pcie_mem->SendData(FILTER_REG1, (unsigned int)0x0800);
		pcie_mem->SendData(FILTER_REG2, (unsigned int)0x9890);
		break;
	case  freq_list_819d2MHz::freq17d065MHz:
		pcie_mem->SendData(CLKOUT0_REG1, (unsigned int)0x15d7);
		pcie_mem->SendData(CLKOUT0_REG2, (unsigned int)0x5800);
		pcie_mem->SendData(CLKOUT5_REG2, (unsigned int)0xc0c0);
		pcie_mem->SendData(CLKOUT6_REG2, (unsigned int)0xe0c0);
		pcie_mem->SendData(DIV_CLK_REG, (unsigned int)0x0082);
		pcie_mem->SendData(CLKFBOUT_REG1, (unsigned int)0x11c7);
		pcie_mem->SendData(CLKFBOUT_REG2, (unsigned int)0x7800);
		pcie_mem->SendData(LOCK_REG1, (unsigned int)0x028a);
		pcie_mem->SendData(LOCK_REG2, (unsigned int)0x7c01);
		pcie_mem->SendData(LOCK_REG3, (unsigned int)0x7fe9);
		pcie_mem->SendData(FILTER_REG1, (unsigned int)0x0800);
		pcie_mem->SendData(FILTER_REG2, (unsigned int)0x1190);
		break;
	case  freq_list_819d2MHz::freq13d65MHz://
		pcie_mem->SendData(CLKOUT0_REG1, (unsigned int)0x16db);
		pcie_mem->SendData(CLKOUT0_REG2, (unsigned int)0x3c00);
		pcie_mem->SendData(CLKOUT5_REG2, (unsigned int)0x30c0);
		pcie_mem->SendData(CLKOUT6_REG2, (unsigned int)0x30c0);
		pcie_mem->SendData(DIV_CLK_REG, (unsigned int)0x01c8);
		pcie_mem->SendData(CLKFBOUT_REG1, (unsigned int)0x16db);
		pcie_mem->SendData(CLKFBOUT_REG2, (unsigned int)0x3c00);
		pcie_mem->SendData(LOCK_REG1, (unsigned int)0x00fa);
		pcie_mem->SendData(LOCK_REG2, (unsigned int)0x7c01);
		pcie_mem->SendData(LOCK_REG3, (unsigned int)0x7fe9);
		pcie_mem->SendData(FILTER_REG1, (unsigned int)0x0800);
		pcie_mem->SendData(FILTER_REG2, (unsigned int)0x9090);
		break;
	case  freq_list_819d2MHz::freq10d24MHz://
		pcie_mem->SendData(CLKOUT0_REG1, (unsigned int)0x1965);
		pcie_mem->SendData(CLKOUT0_REG2, (unsigned int)0x5800);
		pcie_mem->SendData(CLKOUT5_REG2, (unsigned int)0xc0c0);
		pcie_mem->SendData(CLKOUT6_REG2, (unsigned int)0x90c0);
		pcie_mem->SendData(DIV_CLK_REG, (unsigned int)0x0082);
		pcie_mem->SendData(CLKFBOUT_REG1, (unsigned int)0x11c6);
		pcie_mem->SendData(CLKFBOUT_REG2, (unsigned int)0x1800);
		pcie_mem->SendData(LOCK_REG1, (unsigned int)0x028a);
		pcie_mem->SendData(LOCK_REG2, (unsigned int)0x7c01);
		pcie_mem->SendData(LOCK_REG3, (unsigned int)0x7fe9);
		pcie_mem->SendData(FILTER_REG1, (unsigned int)0x0800);
		pcie_mem->SendData(FILTER_REG2, (unsigned int)0x1190);
		break;
	case  freq_list_819d2MHz::freq8d53MHz:
		pcie_mem->SendData(CLKOUT0_REG1, (unsigned int)0x1b2d);
		pcie_mem->SendData(CLKOUT0_REG2, (unsigned int)0x0080);
		pcie_mem->SendData(CLKOUT5_REG2, (unsigned int)0x00c0);
		pcie_mem->SendData(CLKOUT6_REG2, (unsigned int)0x90c0);
		pcie_mem->SendData(DIV_CLK_REG, (unsigned int)0x0042);
		pcie_mem->SendData(CLKFBOUT_REG1, (unsigned int)0x1144);
		pcie_mem->SendData(CLKFBOUT_REG2, (unsigned int)0x1800);
		pcie_mem->SendData(LOCK_REG1, (unsigned int)0x0384);
		pcie_mem->SendData(LOCK_REG2, (unsigned int)0x7c01);
		pcie_mem->SendData(LOCK_REG3, (unsigned int)0x7fe9);
		pcie_mem->SendData(FILTER_REG1, (unsigned int)0x0800);
		pcie_mem->SendData(FILTER_REG2, (unsigned int)0x9190);
		break;
	default:

		break;
	}
	pcie_mem->SendData(CCR, (unsigned int)0x03);
}

void CLK_DIV::reset_clk()
{
	pcie_mem->SendData(SRR, (unsigned int)0x0A);
}

int CLK_DIV::get_clk_status()
{
	uint32_chararray value;
	pcie_mem->ReadBackData(SR, 1, value.c);
	return value.i;
}



void FIR_CONFIG::fir_config_txdata_fifo_reset()
{
	pcie_mem->SendData(TDFR, (unsigned int)0xa5);
}

//lengthµ¥Î»ÊÇword(4bytes)

void FIR_CONFIG::config_din(uint32_t* value, uint32_t length)
{
	for (int i = 0; i < length; i++)
	{
		pcie_mem->SendData(TDFD, (unsigned int)value[i]);

	}
	// This register is used to store packet length values (the number of bytes in the packet)
	// corresponding to valid packets ready for transmit.
	pcie_mem->SendData(TLR, (unsigned int)length * 4);
}

void FIR_CONFIG::fir_config_axis_reset()
{
	pcie_mem->SendData(SRR, (unsigned int)0xa5);
}