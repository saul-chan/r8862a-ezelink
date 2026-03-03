#ifndef __CLS_WIFI_AFE_H__
#define __CLS_WIFI_AFE_H__

#include "cls_wifi_soc.h"
#include "cls_wifi_irf.h"
#include "cls_wifi_platform.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#define IO_LEFT_AFE_SRC_SFT_RST             0xa4
#define SFT_RST_AFE160_SUB_N_WIDTH          1
#define SFT_RST_AFE160_SUB_N_LSB            0
#define SFT_RST_AFE160_SUB_N_MASK           (((1<<SFT_RST_AFE160_SUB_N_WIDTH) - 1)<<SFT_RST_AFE160_SUB_N_LSB)
#define SFT_RST_AFE40_SUB_N_WIDTH           1
#define SFT_RST_AFE40_SUB_N_LSB             1
#define SFT_RST_AFE40_SUB_N_MASK            (((1<<SFT_RST_AFE40_SUB_N_WIDTH) - 1)<<SFT_RST_AFE40_SUB_N_LSB)
#define SFT_RST_PLL_2G_N_WIDTH              1
#define SFT_RST_PLL_2G_N_LSB                2
#define SFT_RST_PLL_2G_N_MASK               (((1<<SFT_RST_PLL_2G_N_WIDTH) - 1)<<SFT_RST_PLL_2G_N_LSB)
#define SFT_RST_PLL_5G_N_WIDTH              1
#define SFT_RST_PLL_5G_N_LSB                3
#define SFT_RST_PLL_5G_N_MASK               (((1<<SFT_RST_PLL_5G_N_WIDTH) - 1)<<SFT_RST_PLL_5G_N_LSB)
#define SFT_RST_PLL_SA_N_WIDTH              1
#define SFT_RST_PLL_SA_N_LSB                4
#define SFT_RST_PLL_SA_N_MASK               (((1<<SFT_RST_PLL_SA_N_WIDTH) - 1)<<SFT_RST_PLL_SA_N_LSB)
#define SFT_RST_REF_CFG_N_WIDTH             1
#define SFT_RST_REF_CFG_N_LSB               5
#define SFT_RST_REF_CFG_N_MASK              (((1<<SFT_RST_REF_CFG_N_WIDTH) - 1)<<SFT_RST_REF_CFG_N_LSB)
#define SFT_RST_REF_PARA_N_WIDTH            1
#define SFT_RST_REF_PARA_N_LSB              6
#define SFT_RST_REF_PARA_N_MASK             (((1<<SFT_RST_REF_PARA_N_WIDTH) - 1)<<SFT_RST_REF_PARA_N_LSB)

/********** D2K *********/
#define AFE_SUB_RST_PARA					0x2c
#define SFT_AFE_PLL_SA_N_WIDTH              1
#define SFT_AFE_PLL_SA_N_LSB                5
#define SFT_AFE_PLL_SA_N_MASK               (((1<<SFT_AFE_PLL_SA_N_WIDTH) - 1)<<SFT_AFE_PLL_SA_N_LSB)

#define IRF_MAX_ANT_NUM 2

struct afe_reg_cfg {
	uint32_t reg_name;
	uint32_t val;
	uint32_t mask;
	uint8_t lsb;
};

struct irf_tbl_head {
	char magic[4];
	uint8_t ver;
	uint8_t sub_band_num;
	uint8_t table_type;
	uint8_t level_num; //for gain table,indicate gain level num
	uint32_t crc;
	uint32_t len;
	int8_t temperature;
	uint8_t tbl_info;
	uint8_t res_b[2];
	uint32_t res[7];
};

struct irf_comm_tbl{
	struct irf_tbl_head head;
	uint8_t data[1];
};


struct frequency_plan_subtbl{
	unsigned center_freq : 16;
	unsigned kvco_default_vref_sel : 5;
	unsigned kvco_default_vctrl_sel : 4;
	unsigned doubler_en2 : 1;
	unsigned kvco_kreg_high : 6;

	unsigned dsm_order3_en : 1;
	unsigned dsm_prbs_initial : 23;
	unsigned didtc_os : 8;

	unsigned lpf_c3_sel : 8;
	unsigned lpf_c2_sel : 8;
	unsigned lpf_c1_sel : 8;
	unsigned is_band_6e : 1;
	unsigned lpf_en_pll : 1;
	unsigned lpf_r1_sel : 6;

	unsigned dsm_fcw_int : 8;
	unsigned kvco_vref_sel1_v : 5;
	unsigned kvco_vref_sel2_v : 5;
	unsigned kvco_vctrl_sel_set_en : 1;
	unsigned kvco_vctrl_sel_set_value : 4;
	unsigned set_vco_vref_sel_afc : 5;
	unsigned set_vctrl_sel_afc : 4;

	unsigned dcc_u : 5;
	unsigned lms_dly_sel : 3;
	unsigned lms_acc_len : 11;
	unsigned loop_cfg_dsm_en : 1;
	unsigned loop_cfg_didtc_en : 1;
	unsigned loop_cfg_dcc_en : 1;
	unsigned loop_cfg_dtc_lms_en : 1;
	unsigned dmydtc_code : 9;

	unsigned set_cbank_idle : 8;
	unsigned set_vco_vref_sel_idle : 5;
	unsigned set_vctrl_sel_idle : 4;
	unsigned set_div_p_idle : 6;
	unsigned set_div_s_idle : 5;
	unsigned cp_icode : 5;

	unsigned set_div_p_afc : 6;
	unsigned set_div_s_afc : 5;
	unsigned set_div_p_kvco : 6;
	unsigned set_div_s_kvco : 5;
	unsigned vco_vte_r2trim : 4;
	unsigned vco_vte_r1trim : 4;
	unsigned vco_vte_half_en : 1;
	unsigned vco_vte_en : 1;

	unsigned lockdet_ref_clk_ndiv : 24;
	unsigned lpf_r3_sel : 6;
	unsigned dtc_byp_en : 1;
	unsigned dtc_en : 1;

	unsigned afc_ref_clk_ndiv : 24;
	unsigned lms_u : 5;
	unsigned lms_gain_ini_high : 3;

	unsigned afc_k_base : 28; // 28bit
	unsigned buf_pri_cbank : 4;

	unsigned lockdet_kbase : 28; // 28bit
	unsigned buf_sec_cbank : 4;

	unsigned lockdet_locked_kdelta_th : 28; // 28bit
	unsigned ppa_pri_cbank : 4;

	unsigned lockdet_unlock_kdelta_th : 28; // 28bit
	unsigned ppa_sec_cbank : 4;

	unsigned lockdet_locked_win : 10; // 10bit
	unsigned iqg_r_sw : 6;    //iqg_r_sw_c2

	unsigned lockdet_unlock_win : 10; // 10bit
	unsigned iqg_r_sw_c3 : 6;

	unsigned iqg_c_sw : 5;   //iqg_c_sw_c2
	unsigned iqg_c_sw_c3 : 5;
	unsigned iqg_rx_fb_c_sw_c2 : 5;
	unsigned iqg_rx_fb_c_sw_c3 : 5;
	unsigned iqg_rx_fb_r_sw_c2 : 6;
	unsigned iqg_rx_fb_r_sw_c3 : 6;

	unsigned lockdet_locked_win_th : 10; // 10bit
	unsigned lockdet_unlock_win_th : 10; // 10bit
	unsigned fbrf_inter_cap_tune : 3;    // 3bit
	unsigned fbrf_input_cap_tune : 3;    // 3bit
	unsigned rxrf_cap_tune : 4;          // 4bit
	unsigned dsm_mode_sel : 2;

	unsigned rxrf_input_cap_tune : 3; // 3bit
	unsigned kvco_ref_clk_ndiv : 24;  // 24bit
	unsigned cp_offset_sel : 5;

	unsigned lms_os : 10;
	unsigned dtc_rsel : 5;
	unsigned didtc_rsel : 5;
	unsigned dmydtc_rsel : 5;
	unsigned fref_MHz : 7;

	uint32_t dsm_fcw_fr;       // 32bit
	uint32_t kvco_kreg_low;    // 32bit
	uint32_t lms_gain_ini_low; // 32bit

	/* new for d2k */
	uint32_t dtc_inl_coe2_ini_low;

	unsigned dtc_inl_coe2_ini_high:7;
	unsigned dtc_inl_en:1;
	unsigned afc2_ref_clk_ndiv:24;

	unsigned loop_cfg_psgen_en : 1;
	unsigned dtc_inl_u_a:3;
	unsigned afc2_k_base:28;

#if defined(CFG_MERAK3000)
	/* new for m3k */
	unsigned rxrf_cbank_control:13;
	unsigned rxrf_lnamode_en_5g:8;
	unsigned rxrf_lnamode_en_6e:8;
	unsigned bw:3;

	unsigned ckref_xo_sa_sel:4;
	unsigned clk_5glo_ref_sel:4;
	unsigned postdiv_5g_en:4;
	unsigned postdiv_5g_sel:8;
	unsigned postdiv_0p5_en:4;
	unsigned postdiv_0p5:8;
#endif
};

enum afe_pll_state {
#if (defined(DUBHE2000) && DUBHE2000)
	PLL_STATE_IDLE = 0,
	PLL_STATE_AFC = 1,
	PLL_STATE_AFC2 = 2,
	PLL_STATE_KVCO = 3,
	PLL_STATE_DONE = 4,
	PLL_STATE_LOOP = 6,
#else
	PLL_STATE_IDLE = 0,
	PLL_STATE_AFC = 1,
	PLL_STATE_LOOP = 2,
	PLL_STATE_KVCO = 3,
	PLL_STATE_LOCK_DET = 4,
	PLL_STATE_LOCKED = 6,
#endif
};

static inline void irf_reg_write_bits(void __iomem *base,uint32_t reg_addr, uint32_t bit_val,uint32_t mask)
{
	uint32_t val;

	val = CLS_REG_RAW_READ32(base,reg_addr);
	val &= ~mask;
	CLS_REG_RAW_WRITE32(base,reg_addr,(val | bit_val));
}


static inline void afe_reg_action(void __iomem *base, const struct afe_reg_cfg *pcfg,uint32_t elements)
{
	uint32_t i;

	for(i = 0;i<elements;i++,pcfg++){
		if(pcfg->mask != 0xffffffff){
			irf_reg_write_bits(base, pcfg->reg_name,pcfg->val << pcfg->lsb,pcfg->mask);
		}else{
			CLS_REG_RAW_WRITE32(base, pcfg->reg_name,pcfg->val);
		}
	}
	return;
}

static inline uint32_t irf_reg_read_bits(void __iomem *base, uint32_t reg_addr, uint32_t lsb, uint32_t mask)
{
	uint32_t val, ret;

	val = CLS_REG_RAW_READ32(base, reg_addr);
	ret = (val & mask) >> lsb;

	return ret;
}

#define AFE_ACTION(base,afe_reg_list)   afe_reg_action(base,afe_reg_list,NELEMENTS(afe_reg_list))

extern int pllsa_config(struct cls_wifi_plat *cls_wifi_plat, u8 radio_index);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __CLS_WIFI_AFE_H__ */
