#include <windows.h>
#include "ADC_Api.h"
#include "Device_MEM32.h"


using namespace NO_OS;
using namespace API_DEF;
using namespace ADC;
 /*
  * ======================================
  * Revision Data
  *=====================================
  */
adc_handle_t* ADCApi::h = nullptr;
static uint8_t api_revision[3] = { 1, 0, 1 };

static int spi_configure()
{
	int err;
	err = ADCApi::adc_register_write(AD9208_IF_CFG_A_REG, 0x00);
	if (err != API_ERROR_OK)
		return err;
	err = ADCApi::adc_register_write(AD9208_IF_CFG_B_REG, 0x00);
	if (err != API_ERROR_OK)
		return err;
	return API_ERROR_OK;
}

int ADCApi::adc_init()
{
	int err;

	if (h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;

	if (h->dev_xfer == NULL)
		return API_ERROR_INVALID_XFER_PTR;

	if (h->delay_us == NULL)
		return API_ERROR_INVALID_DELAYUS_PTR;

	if (h->hw_open != NULL) {
		//err = h->hw_open((const char*)(h->user_data));
		err = h->fd;
		if (err < 0)
			return API_ERROR_HW_OPEN;
	}

	err = spi_configure();
	if (err != API_ERROR_OK)
		return err;
	err = adc_register_write(0xA, 0xFB);
	if (err != API_ERROR_OK)
		return err;
	return API_ERROR_OK;
}

int ADCApi::adc_deinit()
{
	int err;

	if (h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;
	if (h->hw_close != NULL) {
		err = h->hw_close((int)(h->user_data));
		if (err != 0)
			return API_ERROR_HW_CLOSE;
	}
	return API_ERROR_OK;
}

int ADCApi::adc_get_chip_id(adi_chip_id_t* chip_id)
{
	int err;
	uint32_t tmp_reg;

	if (h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;
	if (chip_id == NULL)
		return API_ERROR_INVALID_PARAM;
	err = adc_register_read(AD9208_CHIP_TYPE_REG, &tmp_reg);
	if (err != API_ERROR_OK)
		return err;
	chip_id->chip_type = tmp_reg;

	err = adc_register_read(AD9208_PROD_ID_MSB_REG, &tmp_reg);
	if (err != API_ERROR_OK)
		return err;
	chip_id->prod_id = tmp_reg;
	chip_id->prod_id <<= 8;

	err = adc_register_read(AD9208_PROD_ID_LSB_REG, &tmp_reg);
	if (err != API_ERROR_OK)
		return err;
	chip_id->prod_id |= tmp_reg;

	err = adc_register_read(AD9208_CHIP_GRADE_REG, &tmp_reg);
	if (err != API_ERROR_OK)
		return err;
	chip_id->prod_grade = (tmp_reg >> 4);
	chip_id->dev_revision = (tmp_reg & 0x0F);

	return API_ERROR_OK;
}

int ADCApi::adc_reset(uint8_t hw_reset)
{
	int err;

	if (h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;

	if (hw_reset) {
		if (h->reset_pin_ctrl == NULL)
			return API_ERROR_INVALID_PARAM;
		err = h->reset_pin_ctrl(h->user_data, 0x1);
		if (err != 0)
			return API_ERROR_RESET_PIN_CTRL;
		if (h->delay_us != NULL) {
			err = h->delay_us(h->user_data, HW_RESET_PERIOD_US);
			if (err != 0)
				return API_ERROR_US_DELAY;
		}
		err = h->reset_pin_ctrl(h->user_data, 0x0);
		if (err != 0)
			return API_ERROR_RESET_PIN_CTRL;
	}

	err = adc_register_write(AD9208_IF_CFG_A_REG, 0x81);
	//After issuing a soft reset by programming 0x8l to Register 0x0000the AD9695 requires 5 ms to recover.When programming 
	//theAD9695 for application setup, ensure that an adequate delay is programmed into the firmware after asserting the soft 
	// reset andbefore starting the device setup
	Sleep(10);
	if (err != API_ERROR_OK)
		return err;
	err = adc_register_write(AD9208_IF_CFG_B_REG, 0x02);
	if (err != API_ERROR_OK)
		return err;

	if (h->delay_us != NULL) {
		err = h->delay_us(h->user_data, HW_RESET_PERIOD_US);
		if (err != 0)
			return API_ERROR_US_DELAY;
	}

	return API_ERROR_OK;

}

int ADCApi::adc_adc_set_channel_select(uint8_t ch)
{
	int err;
	
	if (h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;

	if (ch > AD9208_ADC_CH_ALL)
		return API_ERROR_INVALID_PARAM;
	err = adc_register_write(AD9208_CH_INDEX_REG, ch);
	if (err != API_ERROR_OK)
		return err;

	return API_ERROR_OK;
}

int ADCApi::adc_adc_get_channel_select(uint8_t* ch)
{
	int err;
	uint32_t tmp_reg;

	if (h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;

	if (ch == NULL)
		return API_ERROR_INVALID_PARAM;
	
	err = adc_register_read(AD9208_CH_INDEX_REG, &tmp_reg);
	if (err != API_ERROR_OK)
		return err;
	*ch = tmp_reg;

	return API_ERROR_OK;
}

int ADCApi::adc_set_pdn_pin_mode(uint8_t pin_enable, adc_pdn_mode_t pin_mode)
{
	int err;
	uint32_t tmp_reg;

	if (h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;

	if ((pin_mode != AD9208_POWERDOWN) && (pin_mode != AD9208_STANDBY))
		return API_ERROR_INVALID_PARAM;
	if (pin_enable > 1)
		return API_ERROR_INVALID_PARAM;

	if (!pin_enable) {
		err = adc_register_write(
			AD9208_CHIP_PIN_CTRL0_REG, (uint8_t)
			AD9208_CHIP_PDN_PIN_DISABLE);
		if (err != API_ERROR_OK)
			return err;
	}
	else {
		err = adc_register_write(
			AD9208_CHIP_PIN_CTRL0_REG,
			(uint8_t)~
			AD9208_CHIP_PDN_PIN_DISABLE);
		if (err != API_ERROR_OK)
			return err;
		err = adc_register_read(AD9208_CHIP_PIN_CTRL1_REG,
			&tmp_reg);
		if (err != API_ERROR_OK)
			return err;
		tmp_reg &= (~AD9208_CHIP_PDN_MODE(ALL));
		tmp_reg |= ((pin_mode == AD9208_STANDBY) ? 0x1 : 0x0);
		err = adc_register_write(AD9208_CHIP_PIN_CTRL1_REG,
			tmp_reg);
		if (err != API_ERROR_OK)
			return err;
	}

	return API_ERROR_OK;
}

int ADCApi::adc_adc_set_ch_pdn_mode(adc_pdn_mode_t mode)
{
	int err;

	if (h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;

	if (mode > AD9208_POWERDOWN)
		return API_ERROR_INVALID_PARAM;
#if 0
	err = adc_register_read(AD9208_DEV_CFG_REG, &tmp_reg);
	if (err != API_ERROR_OK)
		return err;
	tmp_reg &= ~AD9208_PDN_MODE(ALL);
	tmp_reg |= AD9208_PDN_MODE(mode);
#endif
	err =
		adc_register_write(AD9208_DEV_CFG_REG, AD9208_PDN_MODE(mode));
	if (err != API_ERROR_OK)
		return err;

	return API_ERROR_OK;
}

int ADCApi::adc_set_input_clk_cfg(uint64_t clk_freq_hz, uint8_t div)
{
	int err;
	uint64_t fs_hz = 0x0;
	uint32_t tmp_reg;

	if (h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;
	if ((clk_freq_hz > AD9208_IP_CLK_MAX_HZ) ||
		(clk_freq_hz < AD9208_IP_CLK_MIN_HZ))
		return API_ERROR_INVALID_PARAM;
	if ((div != 1) && (div != 2) && (div != 4))
		return API_ERROR_INVALID_PARAM;

	fs_hz = NO_OS_DIV_U64(clk_freq_hz, div);

	if ((fs_hz > AD9208_ADC_CLK_MAX_HZ) || (fs_hz < AD9208_ADC_CLK_MIN_HZ))
		return API_ERROR_INVALID_PARAM;

	err = adc_register_read(AD9208_IP_CLK_CFG_REG, &tmp_reg);
	if (err != API_ERROR_OK)
		return err;

	tmp_reg &= ~AD9208_IP_CLK_DIV(ALL);
	tmp_reg |= AD9208_IP_CLK_DIV(div - 1);
	err = adc_register_write(AD9208_IP_CLK_CFG_REG, tmp_reg);
	if (err != API_ERROR_OK)
		return err;

	h->adc_clk_freq_hz = fs_hz;
	return API_ERROR_OK;
}

int ADCApi::adc_get_adc_clk_freq(uint64_t* adc_clk_freq_hz)
{
	if (h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;
	if (adc_clk_freq_hz == NULL)
		return API_ERROR_INVALID_PARAM;
	*adc_clk_freq_hz = h->adc_clk_freq_hz;

	return API_ERROR_OK;
}

int ADCApi::adc_set_input_clk_duty_cycle_stabilizer(uint8_t en)
{
	int err;
	uint16_t tmp_reg_addr;

	if (h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;
	if (en > 1)
		return API_ERROR_INVALID_PARAM;

	tmp_reg_addr = AD9208_IP_CLK_DCS1_REG;
	err = adc_register_write(tmp_reg_addr, en);
	if (err != API_ERROR_OK)
		return err;
	tmp_reg_addr = AD9208_IP_CLK_DCS2_REG;
	err = adc_register_write(tmp_reg_addr, en);
	if (err != API_ERROR_OK)
		return err;

	return API_ERROR_OK;
}

#if 0
int ADCApi::ad9208_clk_set_phase_adjust(uint8_t ch, int8_t phase_adjust)
{
	int err;
	uint32_t tmp_reg;

	if (h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;
	if (ch > AD9208_ADC_CH_ALL)
		return API_ERROR_INVALID_PARAM;
	err = ad9208_adc_select_ch(ch);

	err = ad9208_register_read(AD9208_IP_CLK_PHASE_ADJ_REG, &tmp_reg);
	if (err != API_ERROR_OK)
		return err;
	tmp_reg &= ~(AD9208_IP_CLK_DIV(ALL));
	tmp_reg |= AD9208_IP_CLK_PHASE_ADJ(phase_adjust);
	err = ad9208_register_write(AD9208_IP_CLK_PHASE_ADJ_REG, tmp_reg);
	if (err != API_ERROR_OK)
		return err;

	return API_ERROR_OK;
}
#endif

int ADCApi::adc_get_revision(uint8_t* rev_major, uint8_t* rev_minor, uint8_t* rev_rc)
{
	int err = API_ERROR_OK;

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

int ADCApi::adc_adc_get_overange_status(uint32_t* status)
{
	int err;

	if (h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;
	if (status == NULL)
		return API_ERROR_INVALID_PARAM;

	err = adc_register_read(AD9208_OP_OVERANGE_STAT_REG, status);
	if (err != API_ERROR_OK)
		return err;

	err = adc_register_write(AD9208_OP_OVERANGE_CLR_REG, *status);
	if (err != API_ERROR_OK)
		return err;
	return API_ERROR_OK;
}

int ADCApi::adc_adc_set_fd_thresholds(uint16_t upper_dbfs, uint16_t lower_dbfs, uint16_t dwell_time)
{
	int err;

	if (h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;

	if ((upper_dbfs > FD_THRESHOLD_MAG_DBFS_MAX) ||
		(lower_dbfs > FD_THRESHOLD_MAG_DBFS_MAX) ||
		(dwell_time > FD_DWELL_CLK_CYCLES_MAX))
		return API_ERROR_INVALID_PARAM;

	err = adc_register_write(
		AD9208_FD_DWELL_LSB_REG,
		AD9208_FD_DWELL_LSB(dwell_time));
	if (err != API_ERROR_OK)
		return err;
	err = adc_register_write(
		AD9208_FD_DWELL_MSB_REG,
		AD9208_FD_DWELL_MSB(dwell_time));
	if (err != API_ERROR_OK)
		return err;
	err = adc_register_write(
		AD9208_FD_UT_LSB_REG,
		AD9208_FD_UT_LSB(upper_dbfs));
	if (err != API_ERROR_OK)
		return err;
	err = adc_register_write(
		AD9208_FD_UT_MSB_REG,
		AD9208_FD_UT_MSB(upper_dbfs));
	if (err != API_ERROR_OK)
		return err;
	err = adc_register_write(
		AD9208_FD_UT_LSB_REG,
		AD9208_FD_LT_LSB(lower_dbfs));
	if (err != API_ERROR_OK)
		return err;
	err = adc_register_write(
		AD9208_FD_LT_MSB_REG,
		AD9208_FD_LT_MSB(lower_dbfs));
	if (err != API_ERROR_OK)
		return err;

	return API_ERROR_OK;
}

int ADCApi::filter_ctrl(uint16_t pfilt_mode)
{
	int err;

	err = adc_filter_write(ADC_PRO_FILTER_CTRL, pfilt_mode);
	if (err != API_ERROR_OK)
		return err;
	
	return API_ERROR_OK;
}

int ADCApi::filter_x_coef(uint32_t* filter)
{
	int err;

	for (int i = 0; i < 24; i++)
	{
		err = adc_filter_write(ADC_PRO_FILTER_X_COEF + i * 2, filter[i] & 0xFF);
		err = adc_filter_write(ADC_PRO_FILTER_X_COEF + i * 2 + 1, filter[i] >> 8);
		if (err != API_ERROR_OK)
			return err;
	}

}

int ADCApi::filter_y_coef(uint32_t* filter)
{
	int err;

	for (int i = 0; i < 24; i++)
	{
		err = adc_filter_write(ADC_PRO_FILTER_Y_COEF + i * 2, filter[i + 24] & 0xFF);
		err = adc_filter_write(ADC_PRO_FILTER_Y_COEF + i * 2 + 1, filter[i + 24] >> 8);
		if (err != API_ERROR_OK)
			return err;
	}

}

static int adc_get_decimation_cfg(uint8_t dcm, uint8_t* cfg)
{
	int i;

	for (i = 0; i < NO_OS_ARRAY_SIZE(ad9208_dcm_table); i++) {
		if (dcm == ad9208_dcm_table[i].dcm) {
			*cfg = ad9208_dcm_table[i].dcm_cfg;
			return API_ERROR_OK;
		}
	}

	return API_ERROR_INVALID_PARAM;
}

int ADCApi::adc_get_decimation(uint8_t* dcm)
{
	int i, err;
	uint32_t tmp_reg, cfg;

	if (h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;

	err = adc_register_read(AD9208_ADC_DCM_REG, &tmp_reg);
	if (err != API_ERROR_OK)
		return err;

	cfg = AD9208_ADC_DCM_RATE(tmp_reg);

	for (i = 0; i < NO_OS_ARRAY_SIZE(ad9208_dcm_table); i++) {
		if (cfg == ad9208_dcm_table[i].dcm_cfg) {
			*dcm = ad9208_dcm_table[i].dcm;
			return API_ERROR_OK;
		}
	}

	return API_ERROR_INVALID_PARAM;
}

static int adi_api_utils_gcd(int u, int v)
{
	int t;

	while (v != 0) {
		t = u;
		u = v;
		v = t % v;
	}
	return u < 0 ? -u : u;	/* abs(u) */
}

static void adi_api_utils_mult_128(uint64_t a, uint64_t b, uint64_t* hi,
	uint64_t* lo)
{
	uint64_t ah = (a >> 32), al = a & 0xffffffff,
		bh = (b >> 32), bl = b & 0xffffffff,
		rh = (ah * bh), rl = (al * bl),
		rm1 = ah * bl, rm2 = al * bh,
		rm1h = rm1 >> 32, rm2h = rm2 >> 32,
		rm1l = rm1 & 0xffffffff, rm2l = rm2 & 0xffffffff,
		rmh = rm1h + rm2h, rml = rm1l + rm2l, c = ((rl >> 32) + rml) >> 32;

	rl = rl + (rml << 32);
	rh = rh + rmh + c;
	*lo = rl;
	*hi = rh;
}

static void adi_api_utils_mod_128(uint64_t ah, uint64_t al, uint64_t div,
	uint64_t* mod)
{
	uint64_t result = 0;
	uint64_t a;

	DIV_U64_REM(~0, div, &a);
	a += 1;

	DIV_U64_REM(ah, div, &ah);

	/*modular multiplication of (2^64*upper) % div */
	while (ah != 0) {
		if ((ah & 1) == 1) {
			result += a;
			if (result >= div)
				result -= div;
		}
		a <<= 1;
		if (a >= div)
			a -= div;
		ah >>= 1;
	}

	/* add up the 2 results and return the modulus */
	if (al > div)
		al -= div;

	DIV_U64_REM(al + result, div, mod);
}

static int adc_check_buffer_current(adc_adc_buff_curr_t buff_curr)
{
	int err;

	switch (buff_curr) {
	case AD9208_ADC_BUFF_CURR_400_UA:
	case AD9208_ADC_BUFF_CURR_500_UA:
	case AD9208_ADC_BUFF_CURR_600_UA:
	case AD9208_ADC_BUFF_CURR_700_UA:
	case AD9208_ADC_BUFF_CURR_800_UA:
	case AD9208_ADC_BUFF_CURR_1000_UA:
		err = API_ERROR_OK;
		break;
	default:
		err = API_ERROR_INVALID_PARAM;
	}
	return err;
}

static int adc_get_dec_filter_cfg(adc_adc_data_frmt_t op_data_format,
	uint8_t dcm_rate,
	uint8_t* filt_sel_val_0,
	uint8_t* filt_sel_val_1)
{
	uint8_t i = 0;

	if (op_data_format == AD9208_DATA_FRMT_COMPLEX) {
		for (i = 0; i < NO_OS_ARRAY_SIZE(ADI_DEC_FILTER_COMPLEX_TBL); i++) {
			if (ADI_DEC_FILTER_COMPLEX_TBL[i].dec_complex ==
				dcm_rate) {
				*filt_sel_val_0 =
					ADI_DEC_FILTER_COMPLEX_TBL[i].ctrl_reg_val;
				*filt_sel_val_1 =
					ADI_DEC_FILTER_COMPLEX_TBL[i].sel_reg_val;
				return API_ERROR_OK;
			}
		}
	}

	if (op_data_format == AD9208_DATA_FRMT_REAL) {
		for (i = 0; i < NO_OS_ARRAY_SIZE(ADI_DEC_FILTER_COMPLEX_TBL); i++) {
			if (ADI_DEC_FILTER_COMPLEX_TBL[i].dec_real == dcm_rate) {
				*filt_sel_val_0 =
					ADI_DEC_FILTER_COMPLEX_TBL[i].ctrl_reg_val;
				*filt_sel_val_1 =
					ADI_DEC_FILTER_COMPLEX_TBL[i].sel_reg_val;
				return API_ERROR_OK;
			}
		}
	}

	return API_ERROR_INVALID_PARAM;
}

int ADCApi::adc_adc_set_input_scale(adc_adc_scale_range_t full_scale_range)
{
	int err;
	uint8_t fs_val;

	if (h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;

	switch (full_scale_range) {
	case AD9208_ADC_SCALE_2P04_VPP:
		fs_val = 0;
		break;
	case AD9208_ADC_SCALE_1P13_VPP:
		fs_val = 0x8;
		break;
	case AD9208_ADC_SCALE_1P25_VPP:
		fs_val = 0x9;
		break;
	case AD9208_ADC_SCALE_1P7_VPP:
		fs_val = 0xD;
		break;
	case AD9208_ADC_SCALE_1P81_VPP:
		fs_val = 0xE;
		break;
	case AD9208_ADC_SCALE_1P93_VPP:
		fs_val = 0xF;
		break;
	default:
		return API_ERROR_INVALID_PARAM;
	}
	err = adc_register_write(
		AD9208_FULL_SCALE_CFG_REG,
		AD9208_TRM_VREF(fs_val));
	if (err != API_ERROR_OK)
		return err;

	return API_ERROR_OK;
}

static int adc_adc_set_vcm_export(uint8_t en)
{
	int err;
	uint32_t tmp_reg;

	if (ADCApi::h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;

	err = ADCApi::adc_register_read(AD9208_EXT_VCM_CTRL_REG, &tmp_reg);
	if (err != API_ERROR_OK)
		return err;
	if (en)
		tmp_reg |= AD9208_EXT_VCM_BUFF;
	else
		tmp_reg &= ~AD9208_EXT_VCM_BUFF;
	err = ADCApi::adc_register_write(AD9208_EXT_VCM_CTRL_REG, tmp_reg);
	if (err != API_ERROR_OK)
		return err;

	return API_ERROR_OK;
}

int ADCApi::adc_adc_set_input_cfg(
	signal_coupling_t analog_input_mode,
	uint8_t ext_vref,
	adc_adc_scale_range_t full_scale_range)
{
	int err;
	uint32_t tmp_reg;

	if (h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;
	if ((analog_input_mode >= COUPLING_UNKNOWN) || (ext_vref > 1))
		return API_ERROR_INVALID_PARAM;
	if ((analog_input_mode == COUPLING_DC) && (ext_vref == 1)) {
		/*Invalid Configuration DC Coupled Mode can't use external Vref */
		return API_ERROR_INVALID_PARAM;
	}
	/*Set Analog Input Mode Optimization */
	tmp_reg = analog_input_mode ? AD9208_DC_COUPLE_EN(1) :
		AD9208_DC_COUPLE_EN(0);
	err = adc_register_write(AD9208_ANALOG_CFG_REG, tmp_reg);
	if (err != API_ERROR_OK)
		return err;

	/*ac_coupled mode */
	if (analog_input_mode == COUPLING_AC) {
		/*Turn OFF VCM Export Buffer */
		err = adc_adc_set_vcm_export(0x0);
		if (err != API_ERROR_OK)
			return err;

		/*Set Internal or External Vref */
		tmp_reg = ext_vref ? AD9208_EXT_VREF_MODE : 0;
		err = adc_register_write(AD9208_VREF_CTRL_REG, tmp_reg);
		if (err != API_ERROR_OK)
			return err;
	}
	else {		/*dc_coupled mode */
		/*Disable External Vref Mode */
		err = adc_register_write(AD9208_VREF_CTRL_REG, 0x0);
		if (err != API_ERROR_OK)
			return err;
		err = adc_adc_set_vcm_export(0x1);
		if (err != API_ERROR_OK)
			return err;

	}

	err = adc_adc_set_input_scale(full_scale_range);
	if (err != API_ERROR_OK)
		return err;
	return API_ERROR_OK;
}

int ADCApi::adc_adc_set_input_buffer_cfg(
	adc_adc_buff_curr_t buff_curr_n,
	adc_adc_buff_curr_t buff_curr_p,
	adc_adc_buff_curr_t vcm_buff)
{
	int err;
	uint32_t tmp_reg;

	if (h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;
	//err = adc_check_buffer_current(buff_curr_n);
	//if (err != API_ERROR_OK)
	//	return err;
	//err = adc_check_buffer_current(buff_curr_p);
	//if (err != API_ERROR_OK)
	//	return err;

	//err = adc_check_buffer_current(vcm_buff);
	//if (err != API_ERROR_OK)
	//	return err;

	/*Optimize Buffer Settings for Common Mode Reference */
	err = adc_register_write(
		AD9208_BUFF_CFG_P_REG,
		AD9208_BUFF_CTRL_P(buff_curr_p));
	if (err != API_ERROR_OK)
		return err;
	err = adc_register_write(
		AD9208_BUFF_CFG_N_REG,
		AD9208_BUFF_CTRL_N(buff_curr_n));
	if (err != API_ERROR_OK)
		return err;
	/*Set Optimal VCM Buffer */
	err = adc_register_read(AD9208_EXT_VCM_CTRL_REG, &tmp_reg);
	if (err != API_ERROR_OK)
		return err;
	tmp_reg &= ~AD9208_EXT_VCM_BUFF_CURR(ALL);
	tmp_reg |= AD9208_EXT_VCM_BUFF_CURR(vcm_buff);
	err = adc_register_write(AD9208_EXT_VCM_CTRL_REG, tmp_reg);
	if (err != API_ERROR_OK)
		return err;
	return API_ERROR_OK;
}

int ADCApi::adc_adc_set_dc_offset_filt_en(uint8_t en)
{
	int err;
	uint32_t tmp_reg;

	if (h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;
	if (en > 1)
		return API_ERROR_INVALID_PARAM;

	err = adc_register_read(AD9208_DC_OFFSET_CAL_CTRL, &tmp_reg);
	if (err != API_ERROR_OK)
		return err;
	tmp_reg &= ~AD9208_DC_OFFSET_CAL_EN;
	tmp_reg |= (en) ? AD9208_DC_OFFSET_CAL_EN : 0;
	err = adc_register_write(AD9208_DC_OFFSET_CAL_CTRL, tmp_reg);
	if (err != API_ERROR_OK)
		return err;
	return API_ERROR_OK;
}

int ADCApi::adc_adc_set_fc_ch_mode(uint8_t fc_ch)
{
	int err;
	uint32_t tmp_reg, op_mode;

	if (h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;

	if ((fc_ch > AD9208_NOF_FC_MAX) || (fc_ch == 3))
		return API_ERROR_INVALID_PARAM;

	if (fc_ch == 4)
		op_mode = 0x3;
	else
		op_mode = fc_ch;

	err = adc_register_read(AD9208_ADC_MODE_REG, &tmp_reg);
	if (err != API_ERROR_OK)
		return err;
	tmp_reg &= ~AD9208_ADC_MODE(op_mode);
	tmp_reg |= AD9208_ADC_MODE(op_mode);
	err = adc_register_write(AD9208_ADC_MODE_REG, tmp_reg);
	if (err != API_ERROR_OK)
		return err;

	return API_ERROR_OK;
}

int ADCApi::adc_adc_set_dcm_mode(uint8_t dcm)
{
	int err;
	uint32_t tmp_reg;
	uint8_t dcm_mode;

	if (h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;

	err = adc_get_decimation_cfg(dcm, &dcm_mode);
	if (err != API_ERROR_OK)
		return err;

	err = adc_register_read(AD9208_ADC_DCM_REG, &tmp_reg);
	if (err != API_ERROR_OK)
		return err;

	tmp_reg &= ~AD9208_ADC_DCM_RATE(ALL);
	tmp_reg |= dcm_mode;

	err = adc_register_write(AD9208_ADC_DCM_REG, tmp_reg);
	if (err != API_ERROR_OK)
		return err;

	return API_ERROR_OK;
}

int ADCApi::adc_adc_set_data_format(
	adc_adc_data_frmt_t ip_data_frmt,
	adc_adc_data_frmt_t op_data_frmt)
{
	int err, offset;
	uint32_t tmp_reg, i;

	if (h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;
	if ((ip_data_frmt > AD9208_DATA_FRMT_COMPLEX) ||
		(op_data_frmt > AD9208_DATA_FRMT_COMPLEX))
		return API_ERROR_INVALID_PARAM;

	err = adc_register_read(AD9208_ADC_MODE_REG, &tmp_reg);
	if (err != API_ERROR_OK)
		return err;
	tmp_reg &= ~AD9208_ADC_Q_IGNORE;
	tmp_reg |= (op_data_frmt == AD9208_DATA_FRMT_COMPLEX) ? 0 :
		AD9208_ADC_Q_IGNORE;
	err = adc_register_write(AD9208_ADC_MODE_REG, tmp_reg);
	if (err != API_ERROR_OK)
		return err;

	/*TODO: Consider dependancy on FC */
	for (i = 0; i < AD9208_NOF_FC_MAX; i++) {
		offset = (AD9208_DDCX_REG_OFFSET * i);
		err = adc_register_read(AD9208_DDCX_CTRL0_REG + offset,
			&tmp_reg);
		if (err != API_ERROR_OK)
			return err;
		tmp_reg &= ~(AD9208_DDCX_MIXER_SEL |
			AD9208_DDCX_COMPLEX_TO_REAL);
		tmp_reg |= (ip_data_frmt == AD9208_DATA_FRMT_COMPLEX) ?
			AD9208_DDCX_MIXER_SEL : 0;
		tmp_reg |= (op_data_frmt == AD9208_DATA_FRMT_COMPLEX) ?
			0 : AD9208_DDCX_COMPLEX_TO_REAL;
		err = adc_register_write(AD9208_DDCX_CTRL0_REG + offset,
			tmp_reg);
		if (err != API_ERROR_OK)
			return err;
	}

	return API_ERROR_OK;
}

int ADCApi::adc_adc_set_sync_update_mode_enable(uint8_t en)
{
	int err;
	uint32_t tmp_reg;

	if (h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;
	if (en > 1)
		return API_ERROR_INVALID_PARAM;

	err = adc_register_read(AD9208_DDC_SYNC_CTRL_REG, &tmp_reg);
	if (err != API_ERROR_OK)
		return err;
	tmp_reg &= ~AD9208_DDC_UPDATE_MODE;
	tmp_reg |= (en) ? AD9208_DDC_UPDATE_MODE : 0;
	err = adc_register_write(AD9208_DDC_SYNC_CTRL_REG, tmp_reg);
	if (err != API_ERROR_OK)
		return err;

	return API_ERROR_OK;
}

int ADCApi::adc_adc_set_ddc_ip_mux(
	uint8_t ddc_ch, adc_adc_ch_t i_data_src,
	adc_adc_ch_t q_data_src)
{
	int err, offset;
	uint32_t tmp_reg;

	if (h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;
	if ((ddc_ch >= AD9208_NOF_FC_MAX) ||
		(i_data_src < AD9208_ADC_CH_A) || (i_data_src > AD9208_ADC_CH_B) ||
		(q_data_src < AD9208_ADC_CH_A) || (q_data_src > AD9208_ADC_CH_B))
		return API_ERROR_INVALID_PARAM;

	offset = (AD9208_DDCX_REG_OFFSET * ddc_ch);
	err =
		adc_register_read(AD9208_DDCX_DATA_SEL_REG + offset,
			&tmp_reg);
	if (err != API_ERROR_OK)
		return err;
	tmp_reg &= ~(AD9208_DDCX_Q_IP_CHB_SEL | AD9208_DDCX_I_IP_CHB_SEL);
	tmp_reg |=
		(i_data_src == AD9208_ADC_CH_B) ? AD9208_DDCX_I_IP_CHB_SEL : 0;
	tmp_reg |=
		(q_data_src == AD9208_ADC_CH_B) ? AD9208_DDCX_Q_IP_CHB_SEL : 0;

	err = adc_register_write(AD9208_DDCX_DATA_SEL_REG + offset,
		tmp_reg);
	if (err != API_ERROR_OK)
		return err;

	return API_ERROR_OK;

}

/*TODO: Resolve Mis-match in Chip DCM and DDC CH DCM MAX Values*/
int ADCApi::adc_adc_set_ddc_dcm(uint8_t ddc_ch, uint8_t dcm)
{
	int err, offset;
	uint32_t tmp_reg;
	uint8_t filt_sel_reg_0, filt_sel_reg_1;
	adc_adc_data_frmt_t op_data_format = AD9208_DATA_FRMT_COMPLEX;

	if (h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;
	if ((ddc_ch >= AD9208_NOF_FC_MAX) || (dcm > AD9208_ADC_DCM_MAX) ||
		(dcm < AD9208_ADC_DCM_MIN))
		return API_ERROR_INVALID_PARAM;

	offset = (AD9208_DDCX_REG_OFFSET * ddc_ch);
	err = adc_register_read(AD9208_DDCX_CTRL0_REG + offset, &tmp_reg);
	if (err != API_ERROR_OK)
		return err;
	if (tmp_reg & AD9208_DDCX_COMPLEX_TO_REAL)
		op_data_format = AD9208_DATA_FRMT_REAL;
	err = adc_get_dec_filter_cfg(op_data_format, dcm, &filt_sel_reg_0,
		&filt_sel_reg_1);
	if (err != API_ERROR_OK)
		return err;

	tmp_reg &= ~AD9208_DDCX_DCM_FILT_SEL_0(ALL);
	tmp_reg |= AD9208_DDCX_DCM_FILT_SEL_0(filt_sel_reg_0);
	err = adc_register_write(AD9208_DDCX_CTRL0_REG + offset, tmp_reg);
	if (err != API_ERROR_OK)
		return err;

	err = adc_register_read(AD9208_DDCX_DATA_SEL_REG + offset, &tmp_reg);
	if (err != API_ERROR_OK)
		return err;

	tmp_reg &= ~AD9208_DDCX_DCM_FILT_SEL_1(ALL);
	tmp_reg |= AD9208_DDCX_DCM_FILT_SEL_1(filt_sel_reg_1);
	err = adc_register_write(AD9208_DDCX_DATA_SEL_REG + offset,
		tmp_reg);
	if (err != API_ERROR_OK)
		return err;

	return API_ERROR_OK;
}

int ADCApi::adc_adc_set_ddc_gain(uint8_t ddc_ch, uint8_t gain_db)
{
	int err, offset;
	uint32_t tmp_reg;

	if (h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;
	if ((ddc_ch >= AD9208_NOF_FC_MAX) || ((gain_db != 0) && (gain_db != 6)))
		return API_ERROR_INVALID_PARAM;

	offset = (AD9208_DDCX_REG_OFFSET * ddc_ch);
	err = adc_register_read(AD9208_DDCX_CTRL0_REG + offset, &tmp_reg);
	if (err != API_ERROR_OK)
		return err;

	tmp_reg &= ~AD9208_DDCX_GAIN_SEL;
	tmp_reg |= (gain_db == 6) ? AD9208_DDCX_GAIN_SEL : 0;
	err = adc_register_write(AD9208_DDCX_CTRL0_REG + offset, tmp_reg);
	if (err != API_ERROR_OK)
		return err;

	return API_ERROR_OK;
}

int ADCApi::adc_adc_set_ddc_nco_mode(uint8_t ddc_ch, adc_adc_nco_mode_t mode)
{
	int err, offset;
	uint32_t tmp_reg;

	if (h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;

	if ((ddc_ch >= AD9208_NOF_FC_MAX) || (mode >= AD9208_ADC_NCO_INVLD))
		return API_ERROR_INVALID_PARAM;

	offset = (AD9208_DDCX_REG_OFFSET * ddc_ch);
	err = adc_register_read(AD9208_DDCX_CTRL0_REG + offset, &tmp_reg);
	if (err != API_ERROR_OK)
		return err;
	tmp_reg &= ~AD9208_DDCX_NCO_IF_MODE(ALL);
	tmp_reg |= AD9208_DDCX_NCO_IF_MODE(mode);
	err = adc_register_write(AD9208_DDCX_CTRL0_REG + offset, tmp_reg);
	if (err != API_ERROR_OK)
		return err;

	return API_ERROR_OK;
}

int ADCApi::adc_adc_set_ddc_nco_ftw(uint8_t ddc_ch, uint64_t ftw, uint64_t mod_a, uint64_t mod_b)
{
	int err, offset;
	uint32_t tmp_reg;

	if (h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;
	if (ddc_ch >= AD9208_NOF_FC_MAX)
		return API_ERROR_INVALID_PARAM;
	offset = (AD9208_DDCX_REG_OFFSET * ddc_ch);
	err = adc_register_write((AD9208_DDCX_FTW0_REG + offset),
		ADI_GET_BYTE(ftw, 0));
	if (err != API_ERROR_OK)
		return err;
	err = adc_register_write((AD9208_DDCX_FTW1_REG + offset),
		ADI_GET_BYTE(ftw, 8));
	if (err != API_ERROR_OK)
		return err;
	err = adc_register_write((AD9208_DDCX_FTW2_REG + offset),
		ADI_GET_BYTE(ftw, 16));
	if (err != API_ERROR_OK)
		return err;
	err = adc_register_write((AD9208_DDCX_FTW3_REG + offset),
		ADI_GET_BYTE(ftw, 24));
	if (err != API_ERROR_OK)
		return err;
	err = adc_register_write((AD9208_DDCX_FTW4_REG + offset),
		ADI_GET_BYTE(ftw, 32));
	if (err != API_ERROR_OK)
		return err;
	err = adc_register_write((AD9208_DDCX_FTW5_REG + offset),
		ADI_GET_BYTE(ftw, 40));
	if (err != API_ERROR_OK)
		return err;
	offset = (AD9208_DDCX_FRAC_REG_OFFSET * ddc_ch);
	if ((mod_a != 0) && (mod_b != 0)) {
		err = adc_register_write(AD9208_DDCX_MAW0_REG + offset,
			ADI_GET_BYTE(mod_a, 0));
		if (err != API_ERROR_OK)
			return err;
		err = adc_register_write(AD9208_DDCX_MAW1_REG + offset,
			ADI_GET_BYTE(mod_a, 8));
		if (err != API_ERROR_OK)
			return err;
		err = adc_register_write(AD9208_DDCX_MAW2_REG + offset,
			ADI_GET_BYTE(mod_a, 16));
		if (err != API_ERROR_OK)
			return err;
		err = adc_register_write(AD9208_DDCX_MAW3_REG + offset,
			ADI_GET_BYTE(mod_a, 24));
		if (err != API_ERROR_OK)
			return err;
		err = adc_register_write(AD9208_DDCX_MAW4_REG + offset,
			ADI_GET_BYTE(mod_a, 32));
		if (err != API_ERROR_OK)
			return err;
		err = adc_register_write(AD9208_DDCX_MAW5_REG + offset,
			ADI_GET_BYTE(mod_a, 40));
		if (err != API_ERROR_OK)
			return err;
		err = adc_register_write(AD9208_DDCX_MBW0_REG + offset,
			ADI_GET_BYTE(mod_b, 0));
		if (err != API_ERROR_OK)
			return err;
		err = adc_register_write(AD9208_DDCX_MBW1_REG + offset,
			ADI_GET_BYTE(mod_b, 8));
		if (err != API_ERROR_OK)
			return err;
		err = adc_register_write(AD9208_DDCX_MBW2_REG + offset,
			ADI_GET_BYTE(mod_b, 16));
		if (err != API_ERROR_OK)
			return err;
		err = adc_register_write(AD9208_DDCX_MBW3_REG + offset,
			ADI_GET_BYTE(mod_b, 24));
		if (err != API_ERROR_OK)
			return err;
		err = adc_register_write(AD9208_DDCX_MBW4_REG + offset,
			ADI_GET_BYTE(mod_b, 32));
		if (err != API_ERROR_OK)
			return err;
		err = adc_register_write(AD9208_DDCX_MBW5_REG + offset,
			ADI_GET_BYTE(mod_b, 40));
		if (err != API_ERROR_OK)
			return err;
	}

	ADCApi::adc_is_sync_spi_update_enabled(&tmp_reg);
	if (tmp_reg)
		ADCApi::adc_register_chip_transfer();
	return API_ERROR_OK;
}

int ADCApi::adc_adc_set_ddc_nco_phase(uint8_t ddc_ch, uint64_t po)
{
	int err, offset;

	if (h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;
	if (ddc_ch >= AD9208_NOF_FC_MAX)
		return API_ERROR_INVALID_PARAM;
	offset = (AD9208_DDCX_REG_OFFSET * ddc_ch);
	err = adc_register_write((AD9208_DDCX_PO0_REG + offset),
		ADI_GET_BYTE(po, 0));
	if (err != API_ERROR_OK)
		return err;
	err = adc_register_write((AD9208_DDCX_PO1_REG + offset),
		ADI_GET_BYTE(po, 8));
	if (err != API_ERROR_OK)
		return err;
	err = adc_register_write((AD9208_DDCX_PO2_REG + offset),
		ADI_GET_BYTE(po, 16));
	if (err != API_ERROR_OK)
		return err;
	err = adc_register_write((AD9208_DDCX_PO3_REG + offset),
		ADI_GET_BYTE(po, 24));
	if (err != API_ERROR_OK)
		return err;
	err = adc_register_write((AD9208_DDCX_PO4_REG + offset),
		ADI_GET_BYTE(po, 32));
	if (err != API_ERROR_OK)
		return err;
	err = adc_register_write((AD9208_DDCX_PO5_REG + offset),
		ADI_GET_BYTE(po, 40));
	if (err != API_ERROR_OK)
		return err;

	return API_ERROR_OK;
}

int ADCApi::adc_adc_set_ddc_nco(uint8_t ddc_ch, const int64_t carrier_freq_hz)
{
	uint64_t tmp_freq;
	int err;
	uint8_t is_pow2 = 0;

	if (h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;

	if (!carrier_freq_hz)
		return API_ERROR_INVALID_PARAM;

	//if (!((carrier_freq_hz >= (int64_t)(0ll - h->adc_clk_freq_hz / 2)) &&
	//	(carrier_freq_hz < (int64_t)(h->adc_clk_freq_hz / 2))))
	//	return API_ERROR_INVALID_PARAM;

	if (ddc_ch >= AD9208_NOF_FC_MAX)
		return API_ERROR_INVALID_PARAM;

	tmp_freq = carrier_freq_hz;
	while (tmp_freq <= h->adc_clk_freq_hz) {
		if ((tmp_freq) == h->adc_clk_freq_hz) {
			/* It is power of 2 */
			is_pow2 = 1;
			break;
		}
		tmp_freq *= 2;
	}

	if (is_pow2 == 1) {	/* Integer NCO mode */
		/*
		 * As we are in Integer NCO mode it guranteed the
		 *  value is integer power of 2
		 */
		tmp_freq = NO_OS_DIV_U64(h->adc_clk_freq_hz, carrier_freq_hz);
		tmp_freq = NO_OS_DIV_U64(ADI_POW2_48, tmp_freq);

		/* Write FTW */
		err = adc_adc_set_ddc_nco_ftw(ddc_ch, tmp_freq, 0, 0);
		if (err != API_ERROR_OK)
			return err;
	}
	else {		/* Programable Modulus Mode */
		int gcd;
		uint64_t ftw;
		uint64_t maw;
		uint64_t mbw;
		uint64_t M, N;

		uint64_t tmp_ah, tmp_al, /*tmp_bh, tmp_bl, tmp_fh, */ tmp_fl;

		gcd = adi_api_utils_gcd(carrier_freq_hz, h->adc_clk_freq_hz);
		M = NO_OS_DIV_U64(carrier_freq_hz, gcd);
		N = NO_OS_DIV_U64(h->adc_clk_freq_hz, gcd);

		if (M > NO_OS_S16_MAX) {
			uint64_t mask = U64MSB;
			int i = 0;

			while (((mask & M) == 0) && (mask != 1)) {
				mask >>= 1;
				i++;
			}
			ftw = NO_OS_DIV_U64(M * ((uint64_t)1u << i), N);
			ftw *= ((uint64_t)1u << (48 - i));
		}
		else
			ftw = NO_OS_DIV_U64(M * (ADI_POW2_48), N);

		adi_api_utils_mult_128(M, ADI_POW2_48, &tmp_ah, &tmp_al);
		adi_api_utils_mod_128(tmp_ah, tmp_al, N, &tmp_fl);

		maw = tmp_fl;
		mbw = N;

		gcd = adi_api_utils_gcd(maw, mbw);
		maw = NO_OS_DIV_U64(maw, gcd);
		mbw = NO_OS_DIV_U64(mbw, gcd);

		if ((maw > ADI_MAXUINT48) || (mbw > ADI_MAXUINT48))
			return API_ERROR_INVALID_PARAM;	/*out of Range */

		err = adc_adc_set_ddc_nco_ftw(ddc_ch, ftw, maw, mbw);
		if (err != API_ERROR_OK)
			return err;
	}
	return API_ERROR_OK;
}

int ADCApi::adc_adc_set_ddc_nco_reset()
{
	int err;
	uint32_t tmp_reg;

	if (h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;
	err = adc_register_read(AD9208_DDC_SYNC_CTRL_REG, &tmp_reg);
	if (err != API_ERROR_OK)
		return err;
	err = adc_register_write(AD9208_DDC_SYNC_CTRL_REG,
		tmp_reg | AD9208_NCO_SOFT_RESET);
	if (err != API_ERROR_OK)
		return err;
	
	if ((h->delay_us != NULL) && (NCO_RESET_PERIOD_US != 0)) {
		err = h->delay_us(h->user_data, NCO_RESET_PERIOD_US);
		if (err != 0)
			return API_ERROR_US_DELAY;
	}

	err = adc_register_write(AD9208_DDC_SYNC_CTRL_REG,
		(tmp_reg & ~AD9208_NCO_SOFT_RESET));
	if (err != API_ERROR_OK)
		return err;
	return API_ERROR_OK;
}

int ADCApi::adc_adc_set_clk_phase(uint8_t phase_adj)
{
	int err;

	if (h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;
	if (phase_adj > AD9208_IP_CLK_PHASE_ADJ(ALL))
		return API_ERROR_INVALID_PARAM;

	err = adc_register_write(AD9208_IP_CLK_PHASE_ADJ_REG, phase_adj);
	if (err != API_ERROR_OK)
		return err;

	return API_ERROR_OK;
}

int ADCApi::adc_adc_temp_sensor_set_enable(uint8_t en)
{
	int err;
	uint32_t tmp_reg;

	if (h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;
	if (en > 1)
		return API_ERROR_INVALID_PARAM;

	err = adc_adc_set_vcm_export(0x0);
	if (err != API_ERROR_OK)
		return err;
	/*Set Internal or External Vref */
	tmp_reg = (en == 1) ? AD9208_CENTRAL_DIODE_20X_EN : 0x0;
	err = adc_register_write(AD9208_TEMP_DIODE_CTRL_REG, tmp_reg);
	if (err != API_ERROR_OK)
		return err;
	return API_ERROR_OK;
}

void ADCApi::adc_set_channel(uint32_t channel)
{
	adc_register_write_direct(ADC_CHANNEL_CFG, channel);
}

bool ADCApi::get_adc_clockstate()
{
	uint32_t tmp_reg;
	adc_register_read(AD9208_IP_CLK_STAT_REG, &tmp_reg);
	if ((tmp_reg & 1) == 0)
	{
		return false;
	}
	else
	{
		return true;
	}
}
bool ADCApi::get_adc_status()
{
	union uint32_chararray status;
	adc_register_read_direct(ADC_FPGA_RESET_CFG, &status);
	if ((status.c[0] & 3) != 3)
	{
		return false;
	}
	else
	{
		return true;
	}
}

int ADCApi::adc_register_write(const uint16_t address, const uint32_t data)
{
	auto pcie_mem = Device::Device_MEM32::getInstance();
	uint32_t BASE = 0x000C2000;
	pcie_mem->SendData(BASE + address, data);
	//Sleep(1);
	return API_ERROR_OK;
}

int ADCApi::adc_filter_write(const uint16_t address, const uint32_t data)
{
	auto pcie_mem = Device::Device_MEM32::getInstance();
	uint32_t BASE = 0x000C2000;
	pcie_mem->SendData(BASE + address, data);
	return API_ERROR_OK;
}

int ADCApi::adc_register_write_direct(const uint32_t address, const uint32_t data)
{
	auto pcie_mem = Device::Device_MEM32::getInstance();
	pcie_mem->SendData(address, data);
	return API_ERROR_OK;
}

int ADCApi::adc_register_read(const uint16_t address, uint32_t* data)
{
	auto pcie_mem = Device::Device_MEM32::getInstance();
	uint32_t BASE = 0x000C2000;
	union uint32_chararray* uint32_char = (union uint32_chararray*)(data);
	pcie_mem->ReadBackData(BASE + address, 1, uint32_char->c);
	return API_ERROR_OK;
}

int ADCApi::adc_register_read_direct(const uint32_t address, union uint32_chararray* data)
{
	auto pcie_mem = Device::Device_MEM32::getInstance();
	pcie_mem->ReadBackData(address, 1, data->c);
	return API_ERROR_OK;
}

int ADCApi::adc_register_read_block(const uint16_t address, uint32_t* data, uint32_t count)
{
	int err;
	uint16_t i = 0;

	for (i = 0; i < count; i++) {
		err = ADCApi::adc_register_read((address + i), &data[i]);
		if (err != API_ERROR_OK)
			return err;
	}

	return API_ERROR_OK;
}

int ADCApi::adc_register_write_tbl(struct adi_reg_data* tbl, uint32_t count)
{
	uint16_t i = 0;
	int err;

	if (tbl == NULL)
		return API_ERROR_INVALID_PARAM;

	for (i = 0; i < count; i++) {
		err = ADCApi::adc_register_write(tbl[i].reg, tbl[i].val);
		if (err != API_ERROR_OK)
			return err;
	}

	return API_ERROR_OK;
}

int ADCApi::adc_is_sync_spi_update_enabled(uint32_t* enabled)
{
	int err;
	uint32_t tmp_reg;

	if (enabled == NULL)
		return API_ERROR_INVALID_PARAM;

	err = ADCApi::adc_register_read(AD9208_DDC_SYNC_CTRL_REG, &tmp_reg);
	if (err != API_ERROR_OK)
		return err;
	*enabled = (tmp_reg & AD9208_DDC_UPDATE_MODE) ? 0x1 : 0x0;

	return API_ERROR_OK;
}

int ADCApi::adc_register_chip_transfer()
{
	int err;

	err = ADCApi::adc_register_write(AD9208_CHIP_SPI_XFER_REG, AD9208_CHIP_TRIGGER_SPI_XFER);
	if (err != API_ERROR_OK)
		return err;

	return API_ERROR_OK;
}


static int get_jesd_serdes_vco_cfg(uint64_t slr_mbps, uint8_t* vco_cfg)
{
	/*Transport layer Parameter Ranges Table 22 */
	int i = 0x0;

	for (i = 0; i < NO_OS_ARRAY_SIZE(ADI_REC_SERDES_PLL_CFG); i++) {
		if ((slr_mbps >= ADI_REC_SERDES_PLL_CFG[i].slr_lwr_thres) &&
			(slr_mbps < ADI_REC_SERDES_PLL_CFG[i].slr_upr_thres)) {
			*vco_cfg = ADI_REC_SERDES_PLL_CFG[i].vco_cfg;
			return API_ERROR_OK;
		}
	}
	return API_ERROR_INVALID_PARAM;
}

static int check_jesd_params_range(jesd_param_t jesd_param)
{
	/*Transport layer Parameter Ranges Table 22 */
	if ((jesd_param.jesd_L != 1) &&
		(jesd_param.jesd_L != 2) &&
		(jesd_param.jesd_L != 4) && (jesd_param.jesd_L != 8)) {
		/*printf("API:AD9208:Err: Invalid JESD L \r\n"); */
		return API_ERROR_INVALID_PARAM;
	}
	if ((jesd_param.jesd_M != 1) &&
		(jesd_param.jesd_M != 2) &&
		(jesd_param.jesd_M != 4) && (jesd_param.jesd_M != 8)) {
		/*printf("API:AD9208:Err: Invalid JESD M \r\n"); */
		return API_ERROR_INVALID_PARAM;
	}
	if ((jesd_param.jesd_F != 1) &&
		(jesd_param.jesd_F != 2) &&
		(jesd_param.jesd_F != 4) && (jesd_param.jesd_F != 8)) {

		/*printf("API:AD9208:Err: Invalid JESD F \r\n"); */
		return API_ERROR_INVALID_PARAM;
	}
	if ((jesd_param.jesd_N < N_MIN) || (jesd_param.jesd_N > N_MAX)) {

		/*printf("API:AD9208:Err: Invalid JESD N \r\n"); */
		return API_ERROR_INVALID_PARAM;
	}
	if ((jesd_param.jesd_K < K_MIN) || (jesd_param.jesd_K > K_MAX) ||
		(jesd_param.jesd_K % 4 != 0)) {

		/*printf("API:AD9208:Err: Invalid JESD K \r\n"); */
		return API_ERROR_INVALID_PARAM;
	}
	if (jesd_param.jesd_CS > CS_MAX) {

		/*printf("API:AD9208:Err: Invalid JESD CS \r\n"); */
		return API_ERROR_INVALID_PARAM;
	}
	if (jesd_param.jesd_CF > CF_DEFAULT) {

		/*printf("API:AD9208:Err: Invalid JESD CF \r\n"); */
		return API_ERROR_INVALID_PARAM;
	}
	if ((jesd_param.jesd_NP != 8) && (jesd_param.jesd_NP != 16)) {

		/*printf("API:AD9208:Err: Invalid JESD NP \r\n"); */
		return API_ERROR_INVALID_PARAM;
	}

	return API_ERROR_OK;
}

int ADCApi::adc_jesd_enable_link(uint8_t en)
{
	int err;
	uint32_t tmp_reg;

	if (h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;

	if (en > 1)
		return API_ERROR_INVALID_PARAM;

	err = adc_register_read(AD9208_JESD_LINK_CTRL1_REG, &tmp_reg);
	if (err != API_ERROR_OK)
		return err;
	if (en) {
		err = ADCApi::adc_register_write_tbl(
			&ADI_REC_SERDES_INIT_TBL[0],
			NO_OS_ARRAY_SIZE
			(ADI_REC_SERDES_INIT_TBL));
		if (err != API_ERROR_OK)
			return err;
	}
	tmp_reg &= ~AD9208_JESD_LINK_PDN;
	tmp_reg |= en ? AD9208_JESD_LINK_PDN : 0;
	err = adc_register_write(AD9208_JESD_LINK_CTRL1_REG, tmp_reg);
	if (err != API_ERROR_OK)
		return err;

	//0x0572 JESD204B Link
	//	Control 2
	//	[7:6]	SYNCINB± pin control 00 Normal mode. 0x0 R / W
	//			10 Ignore SYNCINB±(force CGS).
	//			11 Ignore SYNCINB±(force ILAS / user data).
	//	5		SYNCINB± pin invert 0 SYNCINB± pin not inverted. 0x0 R / W
	//	1		SYNCINB± pin inverted.
	//	4		SYNCINB± pin type 0 LVDS differential pair SYNC input. 0x0 R / W
	//	1		CMOS single - ended SYNC input.
	//	3		Reserved Reserved. 0x0 R
	//	2		8 - bit / 10 - bit bypass 0 8 - bit / 10 - bit enabled. 0x0 R / W
	//	1		8 - bit / 10 - bit bypassed(the most
	//			significant 2 bits are 0).
	//	1		8 - bit / 10 - bit bit invert 0 Normal. 0x0 R / W
	//	1		Invert a, b, c, d, e, f, g, h, I, and j symbols.

	err = adc_register_write(AD9208_JESD_LINK_CTRL2_REG, 0);
	if (err != API_ERROR_OK)
		return err;


	return API_ERROR_OK;
}

int ADCApi::adc_jesd_set_if_config(jesd_param_t jesd_param, uint64_t* lane_rate_kbps)
{
	int err;
	uint32_t tmp_reg;
	uint8_t dcm, vco_cfg;
	uint64_t fout, slr, slr_mbps;

	if (h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;

	err = check_jesd_params_range(jesd_param);
	if (err != API_ERROR_OK)
		return err;

	/*Determine DCM and Fout */
	adc_get_decimation(&dcm);
	if (err != API_ERROR_OK)
		return err;

	if (h->adc_clk_freq_hz != 0)
		fout = NO_OS_DIV_U64(h->adc_clk_freq_hz, dcm);
	else {
		/*printf("API:AD9208: JESD :INVALID: CLK FREQ \r\n"); */
		return API_ERROR_INVALID_PARAM;
	}
	/*Calculate Lane Rate */
	slr = (((jesd_param.jesd_M * jesd_param.jesd_NP) * (10)) * fout);
	slr = NO_OS_DIV_U64(NO_OS_DIV_U64(slr, 8), jesd_param.jesd_L);
	slr_mbps = NO_OS_DIV_U64(slr, 1000000);

	if ((slr_mbps > LANE_RATE_MAX_MBPS) || (slr_mbps < LANE_RATE_MIN_MBPS)) {
		/*printf("API:AD9208: JESD :INVALID: SLR :%lld \r\n", slr_mbps); */
		return API_ERROR_INVALID_PARAM;
	}

	/*CFG SERDES PLL for SLR */
	err = get_jesd_serdes_vco_cfg(slr_mbps, &vco_cfg);
	if (err != API_ERROR_OK)
		return err;
	err = adc_register_read(AD9208_JESD_SERDES_PLL_CFG_REG, &tmp_reg);
	if (err != API_ERROR_OK)
		return err;
	tmp_reg &= AD9208_JESD_SLR_CTRL(ALL);
	tmp_reg |= AD9208_JESD_SLR_CTRL(vco_cfg);
	err = adc_register_write(AD9208_JESD_SERDES_PLL_CFG_REG, tmp_reg);
	if (err != API_ERROR_OK)
		return err;

	/*Set NOF Converters */
	err = adc_register_write(
		AD9208_JESD_M_CFG_REG,
		(jesd_param.jesd_M - 1));
	if (err != API_ERROR_OK)
		return err;

	/*Set Resolution and Sample and control bits */
	tmp_reg = (AD9208_JESD_CS(jesd_param.jesd_CS) |
		(AD9208_JESD_N(jesd_param.jesd_N - 1)));
	err = adc_register_write(AD9208_JESD_CS_N_CFG_REG, tmp_reg);
	if (err != API_ERROR_OK)
		return err;

	/*Set NP */
	err = adc_register_read(AD9208_JESD_SCV_NP_CFG_REG, &tmp_reg);
	if (err != API_ERROR_OK)
		return err;
	tmp_reg &= ~(AD9208_JESD_NP(ALL));
	tmp_reg |= AD9208_JESD_NP(jesd_param.jesd_NP - 1);
	err = adc_register_write(AD9208_JESD_SCV_NP_CFG_REG, tmp_reg);
	if (err != API_ERROR_OK)
		return err;

	/*Octets per Frame (F) and Frames per Multiframe (k) */
	err = adc_register_write(
		AD9208_JESD_F_CFG_REG,
		(AD9208_JESD_F(jesd_param.jesd_F - 1)));
	if (err != API_ERROR_OK)
		return err;
	err = adc_register_write(
		AD9208_JESD_K_CFG_REG,
		(AD9208_JESD_K(jesd_param.jesd_K - 1)));
	if (err != API_ERROR_OK)
		return err;
	/*Set Lanes */
	err = adc_register_read(AD9208_JESD_L_SCR_CFG_REG, &tmp_reg);
	if (err != API_ERROR_OK)
		return err;
	tmp_reg &= ~AD9208_JESD_LANES(ALL);
	tmp_reg |= AD9208_JESD_LANES(jesd_param.jesd_L - 1);
	err = adc_register_write(AD9208_JESD_L_SCR_CFG_REG, tmp_reg);
	if (err != API_ERROR_OK)
		return err;

	if (lane_rate_kbps != NULL)
		*lane_rate_kbps = NO_OS_DIV_U64(slr, 1000);

	return API_ERROR_OK;
}

int ADCApi::adc_jesd_get_cfg_param(jesd_param_t* jesd_param)
{

	int err, i;
	uint32_t tmp_reg[AD9208_JESD_CFG_REG_OFFSET];

	if (h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;

	if (jesd_param == NULL)
		return API_ERROR_INVALID_PARAM;

	/*Read All the JESD CFG Registers */
	for (i = 0; i < AD9208_JESD_CFG_REG_OFFSET; i++) {
		err =
			adc_register_read((AD9208_JESD_L_SCR_CFG_REG + i),
				&tmp_reg[i]);
		if (err != API_ERROR_OK)
			return err;
	}
	jesd_param->jesd_L = (AD9208_JESD_LANES(tmp_reg[0]) + 1);
	jesd_param->jesd_F = (AD9208_JESD_F(tmp_reg[1]) + 1);
	jesd_param->jesd_K = (AD9208_JESD_K(tmp_reg[2]) + 1);
	jesd_param->jesd_M = (AD9208_JESD_M(tmp_reg[3]) + 1);
	jesd_param->jesd_CS = ((tmp_reg[4] & 0xC) >> 6);
	jesd_param->jesd_N = (AD9208_JESD_N(tmp_reg[4]) + 1);
	jesd_param->jesd_NP = (AD9208_JESD_NP(tmp_reg[4]) + 1);
	jesd_param->jesd_S = (AD9208_JESD_S(tmp_reg[5]));

	jesd_param->jesd_HD = (tmp_reg[6] & AD9208_JESD_HD) ? 1 : 0;
	jesd_param->jesd_CF = (AD9208_JESD_CF(tmp_reg[6]));

	/*Read All the JESD CFG Registers */
	for (i = 0; i < AD9208_JESD_ID_CFG_REG_OFFSET; i++) {
		err =
			adc_register_read((AD9208_JESD_DID_CFG_REG + i),
				&tmp_reg[i]);
		if (err != API_ERROR_OK)
			return err;
	}

	jesd_param->jesd_DID = tmp_reg[0];
	jesd_param->jesd_BID = AD9208_JESD_BID(tmp_reg[1]);
	jesd_param->jesd_LID0 = AD9208_JESD_LID0(tmp_reg[2]);

	return API_ERROR_OK;
}

int ADCApi::adc_jesd_enable_scrambler(uint8_t en)
{
	int err;
	uint32_t tmp_reg;

	if (h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;

	if (en > 1)
		return API_ERROR_INVALID_PARAM;

	err = adc_register_read(AD9208_JESD_L_SCR_CFG_REG, &tmp_reg);
	if (err != API_ERROR_OK)
		return err;
	tmp_reg &= ~AD9208_JESD_SCR_EN;
	tmp_reg |= en ? AD9208_JESD_SCR_EN : 0;
	err = adc_register_write(AD9208_JESD_L_SCR_CFG_REG, tmp_reg);
	if (err != API_ERROR_OK)
		return err;

	return API_ERROR_OK;

}

int ADCApi::ad9695_jesd_set_lane_xbar(uint8_t logical_lane, uint8_t physical_lane)
{
	int err;
	uint32_t tmp_reg_val;
	uint16_t tmp_reg_addr;
	uint8_t tmp_nibble;

	if (h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;

	if ((physical_lane > (LANE_MAX - 1)) || (logical_lane > LANE_MAX - 1))
		return API_ERROR_INVALID_PARAM;

	switch (physical_lane) {
	case 0:
	case 1:
		tmp_reg_addr = AD9208_JESD_XBAR_CFG_REG;
		break;
	case 2:
	case 3:
		tmp_reg_addr = AD9208_JESD_XBAR_CFG_REG + 1;
		break;
	case 4:
	case 5:
		tmp_reg_addr = AD9208_JESD_XBAR_CFG_REG + 3;
		break;
	case 6:
	case 7:
		tmp_reg_addr = AD9208_JESD_XBAR_CFG_REG + 4;
		break;
	default:
		return API_ERROR_INVALID_PARAM;
	}

	tmp_nibble = (physical_lane % 2) ? 1 : 0;

	err = adc_register_read(tmp_reg_addr, &tmp_reg_val);
	if (err != API_ERROR_OK)
		return err;
	if (tmp_nibble == 0) {
		tmp_reg_val &= (~AD9695_JESD_XBAR_LN_EVEN(ALL));             //清除低三位
		tmp_reg_val |= AD9695_JESD_XBAR_LN_EVEN(logical_lane);      //设置低三位为logic lane
		tmp_reg_val &= (~AD9695_JESD_XBAR_LN_ODD(ALL));             //清除6-4位
	}

	err = adc_register_write(tmp_reg_addr, tmp_reg_val);
	if (err != API_ERROR_OK)
		return err;

	return API_ERROR_OK;
}

int ADCApi::adc_jesd_set_lane_xbar(uint8_t logical_lane, uint8_t physical_lane)
{
	int err;
	uint32_t tmp_reg_val;
	uint16_t tmp_reg_addr;
	uint8_t tmp_nibble;

	if (h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;

	if ((physical_lane > (LANE_MAX - 1)) || (logical_lane > LANE_MAX - 1))
		return API_ERROR_INVALID_PARAM;

	switch (physical_lane) {
	case 0:
	case 1:
		tmp_reg_addr = AD9208_JESD_XBAR_CFG_REG;
		break;
	case 2:
	case 3:
		tmp_reg_addr = AD9208_JESD_XBAR_CFG_REG + 1;
		break;
	case 4:
	case 5:
		tmp_reg_addr = AD9208_JESD_XBAR_CFG_REG + 3;
		break;
	case 6:
	case 7:
		tmp_reg_addr = AD9208_JESD_XBAR_CFG_REG + 4;
		break;
	default:
		return API_ERROR_INVALID_PARAM;
	}

	tmp_nibble = (physical_lane % 2) ? 1 : 0;

	err = adc_register_read(tmp_reg_addr, &tmp_reg_val);
	if (err != API_ERROR_OK)
		return err;
	if (tmp_nibble == 0) {
		tmp_reg_val &= (~AD9208_JESD_XBAR_LN_EVEN(ALL));             //清除低三位
		tmp_reg_val |= AD9208_JESD_XBAR_LN_EVEN(logical_lane);      //设置低三位为logic lane
	}
	else {
		tmp_reg_val &= (~AD9208_JESD_XBAR_LN_ODD(ALL));              //&10001111
		tmp_reg_val |= AD9208_JESD_XBAR_LN_ODD(logical_lane);
	}
	err = adc_register_write(tmp_reg_addr, tmp_reg_val);
	if (err != API_ERROR_OK)
		return err;

	return API_ERROR_OK;
}

int ADCApi::adc_jesd_get_lane_xbar(uint8_t* phy_log_map)
{
	int err, i;
	uint32_t tmp_reg[AD9208_JESD_XBAR_CFG_REG_OFFSET];

	if (h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;

	if (phy_log_map == NULL)
		return API_ERROR_INVALID_PARAM;

	for (i = 0; i < AD9208_JESD_XBAR_CFG_REG_OFFSET; i++) {
		err =
			adc_register_read(AD9208_JESD_XBAR_CFG_REG + i,
				&tmp_reg[i]);
		if (err != API_ERROR_OK)
			return err;
	}
	phy_log_map[0] = tmp_reg[0] & AD9208_JESD_XBAR_LN_EVEN(ALL);
	phy_log_map[1] = (tmp_reg[0] & AD9208_JESD_XBAR_LN_ODD(ALL)) >> 4;
	phy_log_map[2] = tmp_reg[1] & AD9208_JESD_XBAR_LN_EVEN(ALL);
	phy_log_map[3] = (tmp_reg[1] & AD9208_JESD_XBAR_LN_ODD(ALL)) >> 4;
	phy_log_map[4] = tmp_reg[3] & AD9208_JESD_XBAR_LN_EVEN(ALL);
	phy_log_map[5] = (tmp_reg[3] & AD9208_JESD_XBAR_LN_ODD(ALL)) >> 4;
	phy_log_map[6] = tmp_reg[4] & AD9208_JESD_XBAR_LN_EVEN(ALL);
	phy_log_map[7] = (tmp_reg[4] & AD9208_JESD_XBAR_LN_ODD(ALL)) >> 4;
	return API_ERROR_OK;
}

int ADCApi::adc_jesd_get_pll_status(uint8_t* pll_status)
{
	int err;
	uint32_t tmp_reg;

	if (h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;
	if (pll_status == NULL)
		return API_ERROR_INVALID_PARAM;

	err = adc_register_read(AD9208_JESD_SERDES_PLL_REG, &tmp_reg);
	if (err != API_ERROR_OK)
		return err;
	*pll_status = tmp_reg;

	return API_ERROR_OK;
}

int ADCApi::adc_jesd_subclass_set(uint8_t subclass)
{
	int err;
	uint32_t tmp_reg;

	if (h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;
	if (subclass >= JESD_SUBCLASS_INVALID)
		return API_ERROR_INVALID_PARAM;
	err = adc_register_read(AD9208_JESD_SCV_NP_CFG_REG, &tmp_reg);
	if (err != API_ERROR_OK)
		return err;
	tmp_reg &= ~AD9208_JESD_SUBCLASS(ALL);
	tmp_reg |= AD9208_JESD_SUBCLASS(subclass);
	err = adc_register_write(AD9208_JESD_SCV_NP_CFG_REG, tmp_reg);
	if (err != API_ERROR_OK)
		return err;

	return API_ERROR_OK;
}

int ADCApi::adc_jesd_syref_mode_set(jesd_sysref_mode_t mode, uint8_t sysref_count)
{
	int err;
	uint32_t tmp_reg;

	if (h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;
	if (mode >= SYSREF_MON)
		return API_ERROR_INVALID_PARAM;

	err = adc_register_read(AD9208_SYSREF_CTRL_0_REG, &tmp_reg);
	if (err != API_ERROR_OK)
		return err;
	tmp_reg &= ~AD9208_SYSREF_MODE_SEL(ALL);
	tmp_reg |= AD9208_SYSREF_MODE_SEL(mode);
	err = adc_register_write(AD9208_SYSREF_CTRL_0_REG, tmp_reg);
	if (err != API_ERROR_OK)
		return err;

	err = adc_register_read(AD9208_SYSREF_CTRL_1_REG, &tmp_reg);
	if (err != API_ERROR_OK)
		return err;
	tmp_reg &= ~AD9208_SYSREF_NSHOT_IGNORE(ALL);
	if (mode == SYSREF_ONESHOT)
		tmp_reg |= AD9208_SYSREF_NSHOT_IGNORE(sysref_count);
	else
		tmp_reg |= AD9208_SYSREF_NSHOT_IGNORE(0x0);
	err = adc_register_write(AD9208_SYSREF_CTRL_1_REG, tmp_reg);
	if (err != API_ERROR_OK)
		return err;

	return API_ERROR_OK;
}

int ADCApi::adc_jesd_syref_config_set(uint8_t sysref_edge_sel, uint8_t clk_edge_sel,
	uint8_t neg_window_skew,
	uint8_t pos_window_skew)
{
	int err;
	uint32_t tmp_reg;

	if (h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;
	if ((sysref_edge_sel > 1) || (clk_edge_sel > 1) ||
		(neg_window_skew > 0x3) || (pos_window_skew > 3))
		return API_ERROR_INVALID_PARAM;

	err = adc_register_read(AD9208_SYSREF_CTRL_0_REG, &tmp_reg);
	if (err != API_ERROR_OK)
		return err;
	tmp_reg &= ~AD9208_SYSREF_TRANSITION_SEL(ALL);
	tmp_reg &= ~AD9208_SYSREF_CLK_EDGE_SEL(ALL);
	tmp_reg |= AD9208_SYSREF_CLK_EDGE_SEL(clk_edge_sel);
	tmp_reg |= AD9208_SYSREF_TRANSITION_SEL(sysref_edge_sel);

	err = adc_register_write(AD9208_SYSREF_CTRL_0_REG, tmp_reg);
	if (err != API_ERROR_OK)
		return err;

	tmp_reg = AD9208_SYSREF_WIN_NEG(neg_window_skew) |
		AD9208_SYSREF_WIN_POS(pos_window_skew);
	err = adc_register_write(AD9208_SYSREF_CTRL_2_REG, tmp_reg);
	if (err != API_ERROR_OK)
		return err;

	return API_ERROR_OK;
}

int ADCApi::adc_jesd_syref_status_get(uint8_t* hold_status, uint8_t* setup_status,
	uint8_t* phase_status)
{
	int err;
	uint32_t tmp_reg;

	if (h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;
	if ((hold_status == NULL) ||
		(setup_status == NULL) || (phase_status == NULL))
		return API_ERROR_INVALID_PARAM;

	err = adc_register_read(AD9208_SYSREF_STAT_0_REG, &tmp_reg);
	if (err != API_ERROR_OK)
		return err;
	*hold_status = ((tmp_reg & 0xF0) >> 4);
	*setup_status = ((tmp_reg & 0x0F) >> 0);
	err = adc_register_read(AD9208_SYSREF_STAT_1_REG, &tmp_reg);
	if (err != API_ERROR_OK)
		return err;
	*phase_status = ((tmp_reg & 0xF) >> 0);

	return API_ERROR_OK;
}

int ADCApi::adc_jesd_sysref_timestamp_set(uint8_t timestamp_en, uint8_t control_bit, uint8_t delay)
{
	int err;
	uint32_t tmp_reg, tmp_nibble;
	uint16_t tmp_addr;

	if (h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;
	if ((timestamp_en > 0x1) ||
		(control_bit > 2) || (delay > AD9208_SYSREF_TS_DELAY(ALL)))
		return API_ERROR_INVALID_PARAM;

	if (timestamp_en == 0x1) {
		err = adc_register_write(AD9208_CHIP_SYNC_MODE_REG,
			AD9208_SYNC_TS_ENABLE);
		if (err != API_ERROR_OK)
			return err;

		if (err != API_ERROR_OK)
			return err;
		switch (control_bit) {
		case 0:
			tmp_addr = AD9208_OP_MODE_CTRL_1_REG;
			tmp_nibble = 0;
			break;
		case 1:
			tmp_addr = AD9208_OP_MODE_CTRL_1_REG;
			tmp_nibble = 1;
			break;
		case 2:
			tmp_addr = AD9208_OP_MODE_CTRL_2_REG;
			tmp_nibble = 0;
			break;
		default:
			return API_ERROR_INVALID_PARAM;
		}

		err = adc_register_read(tmp_addr, &tmp_reg);
		if (err != API_ERROR_OK)
			return err;
		tmp_reg &=
			((AD9208_OP_CONV_CTRL_BIT_SEL(ALL)) << (tmp_nibble * 4));
		tmp_reg |=
			((AD9208_OP_CONV_CTRL_BIT_SEL(AD9208_CB_SYSREF)) <<
				(tmp_nibble * 4));
		err = adc_register_write(tmp_addr, tmp_reg);
		if (err != API_ERROR_OK)
			return err;
		err = adc_register_write(AD9208_SYSREF_CTRL_3_REG,
			AD9208_SYSREF_TS_DELAY(delay));
		if (err != API_ERROR_OK)
			return err;
	}

	return API_ERROR_OK;
}

int ADCApi::adc_jesd_syref_lmfc_offset_set(uint8_t offset)
{
	int err;
	uint32_t tmp_reg;

	if (h == NULL)
		return API_ERROR_INVALID_HANDLE_PTR;
	if (offset > AD9208_JESD_LMFC_OFFSET(ALL))
		return API_ERROR_INVALID_PARAM;

	err = adc_register_read(AD9208_JESD_LMFC_OFFSET_REG, &tmp_reg);
	if (err != API_ERROR_OK)
		return err;
	tmp_reg &= AD9208_JESD_LMFC_OFFSET(ALL);
	tmp_reg |= AD9208_JESD_LMFC_OFFSET(offset);
	err = adc_register_write(AD9208_JESD_LMFC_OFFSET_REG, tmp_reg);
	if (err != API_ERROR_OK)
		return err;

	return API_ERROR_OK;
}

static int adc_spi_xfer(void* user_data, uint8_t* wbuf,
	uint8_t* rbuf, int len)
{
	struct no_os_spi_desc* spi = (struct no_os_spi_desc*)user_data;
	uint8_t* buffer = (uint8_t*)no_os_alloc::no_os_malloc(len);
	int32_t ret;

	memcpy(buffer, wbuf, 3);

	ret = no_os_spi::no_os_spi_write_and_read(spi, buffer, len);
	if (ret < 0)
		printf("Read Error %d", ret);
	else
		memcpy(rbuf, buffer, len);

	no_os_alloc::no_os_free(buffer);

	return ret;
}

/**
 * Set the test mode compatible with ad9208 API
 * @param st - The device structure
 * @param channel - Channel number
 * @param mode - Test mode. Accepted values:
 * 		 AD9208_TESTMODE_OFF
 * 		 AD9208_TESTMODE_MIDSCALE_SHORT
 * 		 AD9208_TESTMODE_POS_FULLSCALE
 * 		 AD9208_TESTMODE_NEG_FULLSCALE
 * 		 AD9208_TESTMODE_ALT_CHECKERBOARD
 * 		 AD9208_TESTMODE_PN23_SEQ
 * 		 AD9208_TESTMODE_PN9_SEQ
 * 		 AD9208_TESTMODE_ONE_ZERO_TOGGLE
 * 		 AD9208_TESTMODE_USER
 * 		 AD9208_TESTMODE_RAMP
 * @return 0 for success, any non-zero value indicates an error
 */
static int adc_testmode_set(struct adc_state* st, unsigned int chan, unsigned int mode)
{
	int ret;

	ret = ADCApi::adc_adc_set_channel_select(NO_OS_BIT(chan & 1));
	if (ret < 0)
		return ret;

	ret = ADCApi::adc_register_write(AD9208_REG_TEST_MODE, mode);
	if (ret < 0)
		return ret;

	return ADCApi::adc_adc_set_channel_select(AD9208_ADC_CH_ALL);
}

/**
 * Setup the device.
 * @param st - The device structure.
 * @return 0 in case of success, negative error code otherwise.
 */
static int32_t adc_setup(struct adc_state* st)
{
	ADCApi::h = st->adc_h;
	adc_adc_data_frmt_t input_fmt, output_fmt;
	API_DEF::adi_chip_id_t chip_id;
	uint64_t sample_rate, lane_rate_kbps;
	uint8_t dcm, pll_stat;
	int32_t timeout, i, ret;
	
	ret = ADCApi::adc_init();
	if (ret < 0) {
		printf("adc init failed (%d)\n", ret);
		return -ENODEV;
	}

	ret = ADCApi::adc_reset(0);
	if (ret < 0) {
		printf("adc reset failed (%d)\n", ret);
		ret = -ENODEV;
		goto error;
	}

	ret = ADCApi::adc_get_chip_id(&chip_id);
	if (ret < 0) {
		printf("adc_get_chip_id failed (%d)\n", ret);
		ret = -ENODEV;
		goto error;
	}

	if (chip_id.chip_type != AD9208_CHIP_TYPE) {
		printf("Wrong chip type (%X)\n", chip_id.chip_type);
		ret = -EINVAL;
		goto error;
	}

	ret = ADCApi::adc_adc_set_channel_select(AD9208_ADC_CH_ALL);
	if (ret < 0) {
		printf("Failed to select channels (%d)\n", ret);
		goto error;
	}

	ret = ADCApi::adc_set_pdn_pin_mode(st->powerdown_pin_en,
		(adc_pdn_mode_t)(st->powerdown_mode));
	if (ret < 0) {
		printf("Failed to set PWDN pin mode (%d)\n", ret);
		goto error;
	}

	ret = ADCApi::adc_set_input_clk_duty_cycle_stabilizer(st->duty_cycle_stabilizer_en);
	if (ret < 0) {
		printf("Failed to set clk duty cycle stabilizer (%d)\n", ret);
		goto error;
	}

	sample_rate = st->sampling_frequency_hz * st->input_div;

	ret = ADCApi::adc_set_input_clk_cfg(sample_rate, st->input_div);
	if (ret < 0) {
		printf("Failed to set input clk config (%d)\n", ret);
		goto error;
	}

	ret = ADCApi::adc_adc_set_input_cfg(
		st->analog_input_mode ? COUPLING_DC : COUPLING_AC,
		st->ext_vref_en, (adc_adc_scale_range_t)(st->current_scale));
	if (ret < 0) {
		printf("Failed to set adc input config: %d\n", ret);
		goto error;
	}

	ret = ADCApi::adc_adc_set_input_buffer_cfg((adc_adc_buff_curr_t)(st->buff_curr_n),
		(adc_adc_buff_curr_t)(st->buff_curr_p), (adc_adc_buff_curr_t)AD9208_BUFF_CURR_600_UA);
	if (ret < 0) {
		printf("Failed to set input buffer config: %d\n", ret);
		goto error;
	}

	ret = ADCApi::adc_adc_set_fc_ch_mode(st->fc_ch);
	if (ret < 0) {
		printf("Failed to set channel mode: %d\n", ret);
		goto error;
	}

	if (st->fc_ch == AD9208_FULL_BANDWIDTH_MODE) {
		dcm = 1; /* Full bandwidth */
	}
	else {
		dcm = st->ddc[0].decimation;
		for (i = 1; i < st->ddc_cnt; i++)
			dcm = no_os_min_t(uint8_t, dcm, st->ddc[i].decimation);
	}

	ret = ADCApi::adc_adc_set_dcm_mode(dcm);
	if (ret < 0) {
		printf("Failed to set decimation mode: %d\n", ret);
		goto error;
	}

	/* DDC Setup */
	if (st->ddc_input_format_real_en)
		input_fmt = AD9208_DATA_FRMT_REAL;
	else
		input_fmt = AD9208_DATA_FRMT_COMPLEX;

	if (st->ddc_output_format_real_en)
		output_fmt = AD9208_DATA_FRMT_REAL;
	else
		output_fmt = AD9208_DATA_FRMT_COMPLEX;

	ret = ADCApi::adc_adc_set_data_format(input_fmt, output_fmt);
	if (ret < 0) {
		printf("Failed to set data format: %d\n", ret);
		goto error;
	}

	for (i = 0; i < st->ddc_cnt; i++) {
		ret = ADCApi::adc_adc_set_ddc_gain(i, st->ddc[i].gain_db ? 6 : 0);
		if (ret < 0) {
			printf("Failed to set ddc gain: %d\n", ret);
			goto error;
		}

		ret = ADCApi::adc_adc_set_ddc_dcm(i, st->ddc[i].decimation);
		if (ret < 0) {
			printf("Failed to set ddc decimation mode: %d\n", ret);
			goto error;
		}

		ret = ADCApi::adc_adc_set_ddc_nco_mode(i, (adc_adc_nco_mode_t)(st->ddc[i].nco_mode));
		if (ret < 0) {
			printf("Failed to set ddc nco mode: %d\n", ret);
			goto error;
		}

		ret = ADCApi::adc_adc_set_ddc_nco(i, st->ddc[i].carrier_freq_hz);
		if (ret < 0) {
			printf("Failed to set ddc nco frequency: %d\n", ret);
			goto error;
		}

		ret = ADCApi::adc_adc_set_ddc_nco_phase(i, st->ddc[i].po);
		if (ret < 0) {
			printf("Failed to set ddc nco phase: %d\n", ret);
			goto error;
		}
	}

	ret = adc_testmode_set(st, 0, st->test_mode_ch0);
	if (ret < 0) {
		printf("Failed to set test mode for ch 0: %d\n", ret);
		goto error;
	}

	ret = adc_testmode_set(st, 1, st->test_mode_ch1);
	if (ret < 0) {
		printf("Failed to set test mode for ch 1: %d\n", ret);
		goto error;
	}

	ret = ADCApi::adc_jesd_syref_lmfc_offset_set(st->sysref_lmfc_offset);
	if (ret < 0) {
		printf("Failed to set SYSREF lmfc offset: %d\n", ret);
		goto error;
	}

	ret = ADCApi::adc_jesd_syref_config_set(st->sysref_edge_sel,
		st->sysref_clk_edge_sel,
		st->sysref_neg_window_skew,
		st->sysref_pos_window_skew);
	if (ret < 0) {
		printf("Failed to set SYSREF sig capture settings: %d\n", ret);
		goto error;
	}

	ret = ADCApi::adc_jesd_syref_mode_set((jesd_sysref_mode_t)st->sysref_mode,
		st->sysref_count);
	if (ret < 0) {
		printf("Failed to Set JESD SYNCHRONIZATION Mode: %d\n", ret);
		goto error;
	}

	ret = ADCApi::adc_jesd_set_if_config(*st->jesd_param, &lane_rate_kbps);
	if (ret < 0) {
		printf("Failed to set JESD204 interface config (%d)\n", ret);
		goto error;
	}

	ret = ADCApi::adc_jesd_subclass_set(st->jesd_subclass);
	if (ret < 0) {
		printf("Failed to set subclass (%d)\n", ret);
		goto error;
	}

	ret = ADCApi::adc_jesd_enable_scrambler(1);
	if (ret < 0) {
		printf("Failed to enable scrambler (%d)\n", ret);
		goto error;
	}

	ret = ADCApi::adc_jesd_enable_link(1);
	if (ret < 0) {
		printf("Failed to enabled JESD204 link (%d)\n", ret);
		goto error;
	}

	ret = ADCApi::ad9695_jesd_set_lane_xbar(3, 0);
	if (ret < 0) {
		printf("Failed to set lane xbar (%d)\n", ret);
		goto error;
	}

	ret = ADCApi::ad9695_jesd_set_lane_xbar(2, 2);
	if (ret < 0) {
		printf("Failed to set lane xbar (%d)\n", ret);
		goto error;
	}

	ret = ADCApi::ad9695_jesd_set_lane_xbar(1, 4);
	if (ret < 0) {
		printf("Failed to set lane xbar (%d)\n", ret);
		goto error;
	}

	ret = ADCApi::ad9695_jesd_set_lane_xbar(0, 6);
	if (ret < 0) {
		printf("Failed to set lane xbar (%d)\n", ret);
		goto error;
	}


	ret = ADCApi::adc_adc_set_dc_offset_filt_en(0);
	if (ret < 0) {
		printf("Failed to set dc offset filt en(%d)\n", ret);
		goto error;
	}

	timeout = 10;

	do {
		no_os_delay::no_os_mdelay(10);
		ret = ADCApi::adc_jesd_get_pll_status(&pll_stat);
		if (ret < 0) {
			printf("Failed to get pll status (%d)\n", ret);
			goto error;
		}
	} while (!(pll_stat & AD9208_JESD_PLL_LOCK_STAT) && timeout--);

	printf("ADC PLL %s\n", (pll_stat & AD9208_JESD_PLL_LOCK_STAT) ?
		"LOCKED" : "UNLOCKED");

	return 0;

error:
	ADCApi::adc_deinit();
	return ret;
}


/**
 * Delay microseconds, compatible with AD9208 API
 * Performs a blocking or sleep delay for the specified time in microseconds
 * @param user_data
 * @param us - time to delay/sleep in microseconds.
 */
static int adc_udelay(void* user_data, unsigned int us)
{
	no_os_delay::no_os_udelay(us);

	return 0;
}

/**
 * Initialize the device.
 * @param device - The device structure.
 * @param init_param - The structure that contains the device initial
 * 		       parameters.
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t ADCApi::adc_initialize(adc_dev* dev, adc_init_param* init_param)
{
	int32_t i, ret;
	struct adc_state* st = dev->st;

	if (!dev)
		return -ENOMEM;

	if (!dev->st) {
		ret = -ENOMEM;
		goto error;
	}

	st->adc_h->dev_xfer = adc_spi_xfer;
	st->adc_h->delay_us = adc_udelay;
	st->adc_h->reset_pin_ctrl = NULL;
	st->sampling_frequency_hz = init_param->sampling_frequency_hz;
	st->input_div = init_param->input_div;
	st->duty_cycle_stabilizer_en = init_param->duty_cycle_stabilizer_en;
	st->powerdown_mode = init_param->powerdown_mode;
	st->powerdown_pin_en = init_param->powerdown_pin_en;
	st->current_scale = init_param->current_scale;
	st->analog_input_mode = init_param->analog_input_mode;
	st->ext_vref_en = init_param->ext_vref_en;
	st->buff_curr_n = init_param->buff_curr_n;
	st->buff_curr_p = init_param->buff_curr_p;
	st->fc_ch = init_param->fc_ch;
	st->ddc_cnt = init_param->ddc_cnt;
	for (i = 0; i < st->ddc_cnt; i++) {
		st->ddc[i].decimation = init_param->ddc[i].decimation;
		st->ddc[i].nco_mode = init_param->ddc[i].nco_mode;
		st->ddc[i].carrier_freq_hz = init_param->ddc[i].carrier_freq_hz;
		st->ddc[i].gain_db = init_param->ddc[i].gain_db;
		st->ddc[i].po = init_param->ddc[i].po;
	}
	st->ddc_output_format_real_en = init_param->ddc_output_format_real_en;
	st->ddc_input_format_real_en = init_param->ddc_input_format_real_en;
	st->test_mode_ch0 = init_param->test_mode_ch0;
	st->test_mode_ch1 = init_param->test_mode_ch1;

	/* SYSREF Config */
	st->sysref_lmfc_offset = init_param->sysref_lmfc_offset;
	st->sysref_clk_edge_sel = init_param->sysref_clk_edge_sel;
	st->sysref_edge_sel = init_param->sysref_edge_sel;
	st->sysref_neg_window_skew = init_param->sysref_neg_window_skew;
	st->sysref_pos_window_skew = init_param->sysref_pos_window_skew;
	st->sysref_mode = init_param->sysref_mode;
	st->sysref_count = init_param->sysref_count;

	st->jesd_param = init_param->jesd_param;
	st->jesd_subclass = init_param->jesd_subclass;


	ret = adc_setup(st);
	if (ret < 0) {
		printf("Failed to setup device\n");
		goto error;
	}

	printf("ADC successfully initialized\n");

	return 0;

error:
	if (st->adc_h)
		no_os_alloc::no_os_free(st->adc_h);
	if (st->jesd_param)
		no_os_alloc::no_os_free(st->jesd_param);
	if (st)
		no_os_alloc::no_os_free(st);
	if (dev->gpio_powerdown)
		no_os_gpio::no_os_gpio_remove(dev->gpio_powerdown);
	if (dev->spi_desc)
		no_os_spi::no_os_spi_remove(dev->spi_desc);
	if (dev)
		no_os_alloc::no_os_free(dev);

	return ret;
}

int32_t ADCApi::adc_remove(adc_dev* device)
{
	int32_t ret;

	ret = no_os_gpio::no_os_gpio_remove(device->gpio_powerdown);
	ret |= no_os_spi::no_os_spi_remove(device->spi_desc);

	if (device->st->adc_h)
		no_os_alloc::no_os_free(device->st->adc_h);
	if (device->st)
		no_os_alloc::no_os_free(device->st);
	if (device)
		no_os_alloc::no_os_free(device);

	return ret;
}


int32_t ADCApi::adc_filter_init(uint32_t* filter,uint16_t mode)
{
	int ret;

	ret = ADCApi::adc_adc_set_channel_select(1);
	if (ret < 0) {
		printf("Failed to select channels (%d)\n", ret);
		goto error;
	}

	ret = ADCApi::filter_ctrl(mode);
	if (ret < 0) {
		printf("Failed to set filter ctrl (%d)\n", ret);
		goto error;
	}
	Sleep(1);

	ret = ADCApi::filter_x_coef(filter);
	if (ret < 0) {
		printf("Failed to set filter x coef (%d)\n", ret);
		goto error;
	}

	ret = ADCApi::filter_y_coef(filter);
	if (ret < 0) {
		printf("Failed to set filter y coef (%d)\n", ret);
		goto error;
	}
	ret = ADCApi::adc_adc_set_channel_select(2);
	if (ret < 0) {
		printf("Failed to select channels (%d)\n", ret);
		goto error;
	}

	ret = ADCApi::filter_ctrl(mode);
	if (ret < 0) {
		printf("Failed to set filter ctrl (%d)\n", ret);
		goto error;
	}
	Sleep(1);

	ret = ADCApi::filter_x_coef(filter);
	if (ret < 0) {
		printf("Failed to set filter x coef (%d)\n", ret);
		goto error;
	}

	ret = ADCApi::filter_y_coef(filter);
	if (ret < 0) {
		printf("Failed to set filter y coef (%d)\n", ret);
		goto error;
	}

	ret = ADCApi::adc_register_chip_transfer();
	if (ret < 0) {
		printf("Failed to set chip transfer (%d)\n", ret);
		goto error;
	}

error:
	return API_ERROR_INVALID_PARAM;
}

int ADCApi::set_rf_gain(int gain, int ch)
{
	uint32_t cmd = uint32_t(gain);
	if (ch == 0)
	{
		auto pcie_mem = Device::Device_MEM32::getInstance();
		pcie_mem->SendData(0x000C0000, cmd);
		pcie_mem->SendData(0x000C0001, cmd);
		pcie_mem->SendData(0x000C0002, cmd);
		pcie_mem->SendData(0x000C0003, cmd);
		return true;
	}
	else
	{

	}
}