#include <linux/kernel.h>
#include <linux/kmod.h>
#include <linux/debugfs.h>
#include <linux/string.h>
#include <linux/delay.h>
#include "cls_wifi_cali_debugfs.h"
#include "cls_wifi_soc.h"
#include "cls_wifi_msg_tx.h"
#include "cls_wifi_cfgfile.h"
#include "cls_wifi_irf.h"
#include "cls_wifi_afe.h"

#include "afe_reg_def_d2k.h"
#include "pllcali_reg_def_d2k.h"

#include "cls_wifi_afe_com_top_config.h"
#include "cls_wifi_dubhe2000.h"


#define AFE_RW_BASE		 (0x90422000)
#define AFE_PLL_SA_BASE	 (0x90425000)


#define RETRY_TIMES      6


const struct afe_reg_cfg pll_sa_ldo_cfg[] = {
	{AFE_RW_125, 0, IBIAS_PD_PLLSA_MASK, IBIAS_PD_PLLSA_LSB},
	{AFE_RW_125, 0, PFDLDO_PD_PLLSA_MASK, PFDLDO_PD_PLLSA_LSB},
	{AFE_RW_126, 0, CKOLDO_PD_PLLSA_MASK, CKOLDO_PD_PLLSA_LSB},
	{AFE_RW_126, 0, VCOLDO_PD_PLLSA_MASK, VCOLDO_PD_PLLSA_LSB},
	{AFE_RW_126, 0, CPLDO_PD_PLLSA_MASK, CPLDO_PD_PLLSA_LSB},
};

const struct afe_reg_cfg pllsa_osc_cfg[] = {
	{AFE_RW_012, 15,    XO_DRV_PLL5G_MASK,    XO_DRV_PLL5G_LSB},
	{AFE_RW_013,  1, OSCLDO_FS_PLL5G_MASK, OSCLDO_FS_PLL5G_LSB},
	{AFE_RW_012,  0,   XO_CSEL_PLL5G_MASK,   XO_CSEL_PLL5G_LSB}
};

const struct afe_reg_cfg pllsa_refclk_cfg[] = {
	{AFE_RW_013,  1,  CKREF_CML_EN_PLL5G_MASK,  CKREF_CML_EN_PLL5G_LSB},
	{AFE_RW_013,  4, CKREF_CML_DRV_PLL5G_MASK, CKREF_CML_DRV_PLL5G_LSB},
	{AFE_RW_125,  1,     IB_VBGSEL_PLLSA_MASK,     IB_VBGSEL_PLLSA_LSB},
	{AFE_RW_125,  1,    IB_V06SELN_PLLSA_MASK,    IB_V06SELN_PLLSA_LSB},
	{AFE_RW_125, 18,    PFDLDO_SEL_PLLSA_MASK,    PFDLDO_SEL_PLLSA_LSB},
};
const struct afe_reg_cfg pllsa_refclk_cfg_1[] = {
	{AFE_RW_125, 1,   PFDLDO_FS_PD_PLLSA_MASK,   PFDLDO_FS_PD_PLLSA_LSB},
	{AFE_RW_127, 1,         CML_EN_PLLSA_MASK,         CML_EN_PLLSA_LSB},
	{AFE_RW_127, 4,    CML_DRV_SEL_PLLSA_MASK,    CML_DRV_SEL_PLLSA_LSB},
	{AFE_RW_127, 3,      CML_VBSEL_PLLSA_MASK,      CML_VBSEL_PLLSA_LSB},
	{AFE_RW_127, 1, REF_CLK2COM_EN_PLLSA_MASK, REF_CLK2COM_EN_PLLSA_LSB}
};
/*
const struct afe_reg_cfg pllsa_double_en_cfg[]=
{
	{AFE_RW_125,   31, PFDLDO_SEL_PLLSA_MASK,           PFDLDO_SEL_PLLSA_LSB},
	{AFE_RW_125,    1, PFDLDO_FS_PD_PLLSA_MASK,         PFDLDO_FS_PD_PLLSA_LSB},
	{AFE_RW_127,    1, DOUBLER_EN_PLLSA_MASK,           DOUBLER_EN_PLLSA_LSB}
};
*/

const struct afe_reg_cfg pllsa_vco_cfg[] = {
	{AFE_RW_132, 1, VCO_VTE_FS_EN_PLLSA_MASK, VCO_VTE_FS_EN_PLLSA_LSB},
	{AFE_RW_131, 1,    VCO_REF_EN_PLLSA_MASK,    VCO_REF_EN_PLLSA_LSB},
	{AFE_RW_131, 1,       VCO1_EN_PLLSA_MASK,       VCO1_EN_PLLSA_LSB}
};

const struct afe_reg_cfg pllsa_pfdcp_dtc_cfg[] = {
	{AFE_RW_127, 15,    PFD_RST_SEL_PLLSA_MASK,    PFD_RST_SEL_PLLSA_LSB},
	{AFE_RW_127,  1,        PFD_RST_PLLSA_MASK,        PFD_RST_PLLSA_LSB},
	{AFE_RW_128,  0,    CP_COMP_PDW_PLLSA_MASK,    CP_COMP_PDW_PLLSA_LSB},
	{AFE_RW_128,  1,     CP_COMP_EN_PLLSA_MASK,     CP_COMP_EN_PLLSA_LSB},
	{AFE_RW_128,  3,   CP_VBGEN_SEL_PLLSA_MASK,   CP_VBGEN_SEL_PLLSA_LSB},
	{AFE_RW_128,  1,    CP_UNITX_EN_PLLSA_MASK,    CP_UNITX_EN_PLLSA_LSB},
	{AFE_RW_128,  0,     CP_LPF_SEL_PLLSA_MASK,     CP_LPF_SEL_PLLSA_LSB},
	{AFE_RW_128,  4,       CP_IBIAS_PLLSA_MASK,       CP_IBIAS_PLLSA_LSB},
	{AFE_RW_128,  1,       CP_FS_EN_PLLSA_MASK,       CP_FS_EN_PLLSA_LSB},
	{AFE_RW_128,  1,     CP_OPA2_EN_PLLSA_MASK,     CP_OPA2_EN_PLLSA_LSB},
	{AFE_RW_128,  1,     CP_OPA1_EN_PLLSA_MASK,     CP_OPA1_EN_PLLSA_LSB},
	{AFE_RW_128,  0,          CP_EN_PLLSA_MASK,          CP_EN_PLLSA_LSB},
	{AFE_RW_130,  1,       NDIV_RST_PLLSA_MASK,       NDIV_RST_PLLSA_LSB},
};

const struct afe_reg_cfg pllsa_pfdcp_dtc_cfg1[] = {
	{AFE_RW_132,  0,   CP_OFFSET_EN_PLLSA_MASK,   CP_OFFSET_EN_PLLSA_LSB},
	{AFE_RW_135,  1,      DIDTC_RST_PLLSA_MASK,      DIDTC_RST_PLLSA_LSB},
	{AFE_RW_135,  0,       DIDTC_EN_PLLSA_MASK,       DIDTC_EN_PLLSA_LSB},
	{AFE_RW_135,  0,      DMYDTC_EN_PLLSA_MASK,      DMYDTC_EN_PLLSA_LSB},
	{AFE_RW_134,  3, LOCKDET_DFFSEL_PLLSA_MASK, LOCKDET_DFFSEL_PLLSA_LSB},
	{AFE_RW_134,  13,LOCKDET_DLYSEL_PLLSA_MASK, LOCKDET_DLYSEL_PLLSA_LSB},
	{AFE_RW_134,  0,   LOCKDET_RSTB_PLLSA_MASK,   LOCKDET_RSTB_PLLSA_LSB}
};

const struct afe_reg_cfg pllsa_alalog_clock_cfg[] = {
	{AFE_RW_127, 1, REF_DIG_CLK_EN_PLLSA_MASK, REF_DIG_CLK_EN_PLLSA_LSB},
	{AFE_RW_131, 1,     VCO_BUF_EN_PLLSA_MASK,     VCO_BUF_EN_PLLSA_LSB},
	{AFE_RW_130, 1,    CKO_BUF1_EN_PLLSA_MASK,    CKO_BUF1_EN_PLLSA_LSB},
	{AFE_RW_130, 1,     CLK_DIV_EN_PLLSA_MASK,     CLK_DIV_EN_PLLSA_LSB},
	{AFE_RW_130, 1,    CLK_TEST_EN_PLLSA_MASK,    CLK_TEST_EN_PLLSA_LSB},
	{AFE_RW_130, 0,       NDIV_RST_PLLSA_MASK,       NDIV_RST_PLLSA_LSB},
	{AFE_RW_130, 1,     DSM_CLK_EN_PLLSA_MASK,     DSM_CLK_EN_PLLSA_LSB},
	{AFE_RW_125, 1,         A2D_EN_PLLSA_MASK,         A2D_EN_PLLSA_LSB},
	{AFE_RW_133, 1,         DIV_EN_PLLSA_MASK,         DIV_EN_PLLSA_LSB},
	{AFE_RW_133, 0,        DIV2_EN_PLLSA_MASK,        DIV2_EN_PLLSA_LSB},
	{AFE_RW_133, 1,    POSTDIV_SEL_PLLSA_MASK,    POSTDIV_SEL_PLLSA_LSB},
	{AFE_RW_133, 0,      DRV_P_SEL_PLLSA_MASK,      DRV_P_SEL_PLLSA_LSB},
	{AFE_RW_133, 0,    CNT_CLK_SEL_PLLSA_MASK,    CNT_CLK_SEL_PLLSA_LSB}
};

const struct afe_reg_cfg pllsa_loop_mode[] = {
	{AFE_RW_127, 0,      PFD_RST_PLLSA_MASK,      PFD_RST_PLLSA_LSB},
	{AFE_RW_135, 0,    DIDTC_RST_PLLSA_MASK,    DIDTC_RST_PLLSA_LSB},
	{AFE_RW_135, 1,     DIDTC_EN_PLLSA_MASK,     DIDTC_EN_PLLSA_LSB},
	{AFE_RW_135, 1,    DMYDTC_EN_PLLSA_MASK,    DMYDTC_EN_PLLSA_LSB},
	{AFE_RW_128, 1,        CP_EN_PLLSA_MASK,        CP_EN_PLLSA_LSB},
	{AFE_RW_132, 1, CP_OFFSET_EN_PLLSA_MASK, CP_OFFSET_EN_PLLSA_LSB},
	{AFE_RW_131, 0,   VCO_REF_EN_PLLSA_MASK,   VCO_REF_EN_PLLSA_LSB}
};

const struct afe_reg_cfg pllsa_to_abb[] = {
	{AFE_RW_130, 1, CKO_BUF2_EN_PLLSA_MASK, CKO_BUF2_EN_PLLSA_LSB},
	{AFE_RW_130, 1,  CLK_SA1_EN_PLLSA_MASK,  CLK_SA1_EN_PLLSA_LSB},
	{AFE_RW_130, 1,  CLK_SA2_EN_PLLSA_MASK,  CLK_SA2_EN_PLLSA_LSB},
	{AFE_RW_133, 1,     DIV8_EN_PLLSA_MASK,     DIV8_EN_PLLSA_LSB},
};

int pll_digital_cfg(void __iomem *pll_base, struct frequency_plan_subtbl *cfg)
{
	irf_reg_write_bits(pll_base, DSM_FCW_INT, cfg->dsm_fcw_int<<DSM_FCW_INT_LSB, DSM_FCW_INT_MASK);
	irf_reg_write_bits(pll_base,DSM_FCW_FR, cfg->dsm_fcw_fr<<DSM_FCW_FR_LSB, DSM_FCW_FR_MASK);
	irf_reg_write_bits(pll_base,DSM_CFG, cfg->dsm_mode_sel<<DSM_MODE_SEL_LSB, DSM_MODE_SEL_MASK);
	irf_reg_write_bits(pll_base, DSM_CFG, cfg->dsm_order3_en<<DSM_ORDER3_EN_LSB, DSM_ORDER3_EN_MASK);
	irf_reg_write_bits(pll_base, DSM_CFG, cfg->dsm_prbs_initial<<DSM_PRBS_INITIAL_LSB, DSM_PRBS_INITIAL_MASK);
	irf_reg_write_bits(pll_base, AFC_REF_CLK_NDIV, cfg->afc_ref_clk_ndiv<<AFC_REF_CLK_NDIV_LSB, AFC_REF_CLK_NDIV_MASK);
	irf_reg_write_bits(pll_base, AFC_K_BASE, cfg->afc_k_base<<AFC_K_BASE_LSB, AFC_K_BASE_MASK);
	irf_reg_write_bits(pll_base, KVCO_DEFAULT_VREF_VCTRL, cfg->kvco_default_vref_sel<<KVCO_DEFAULT_VREF_SEL_LSB, KVCO_DEFAULT_VREF_SEL_MASK);
	irf_reg_write_bits(pll_base, KVCO_DEFAULT_VREF_VCTRL, cfg->kvco_default_vctrl_sel<<KVCO_DEFAULT_VCTRL_SEL_LSB, KVCO_DEFAULT_VCTRL_SEL_MASK);
	irf_reg_write_bits(pll_base, KVCO_REF_CLK_NDIV, cfg->kvco_ref_clk_ndiv<<KVCO_REF_CLK_NDIV_LSB, KVCO_REF_CLK_NDIV_MASK);
	irf_reg_write_bits(pll_base, KVCO_VREF_CFG, cfg->kvco_vref_sel1_v<<KVCO_VREF_SEL1_V_LSB, KVCO_VREF_SEL1_V_MASK);
	irf_reg_write_bits(pll_base, KVCO_VREF_CFG, cfg->kvco_vref_sel2_v<<KVCO_VREF_SEL2_V_LSB, KVCO_VREF_SEL2_V_MASK);
	irf_reg_write_bits(pll_base, KVCO_KREG_LOW, cfg->kvco_kreg_low<<KVCO_KREG_LOW_LSB, KVCO_KREG_LOW_MASK);
	irf_reg_write_bits(pll_base, KVCO_KREG_HIGH, cfg->kvco_kreg_high<<KVCO_KREG_HIGH_LSB, KVCO_KREG_HIGH_MASK);
	irf_reg_write_bits(pll_base, KVCO_FORCE_OUT, cfg->kvco_vctrl_sel_set_en<<KVCO_VCTRL_SEL_SET_EN_LSB, KVCO_VCTRL_SEL_SET_EN_MASK);
	irf_reg_write_bits(pll_base, KVCO_FORCE_OUT, cfg->kvco_vctrl_sel_set_value<<KVCO_VCTRL_SEL_SET_VALUE_LSB, KVCO_VCTRL_SEL_SET_VALUE_MASK);
#if (defined(DUBHE2000) && DUBHE2000)
	irf_reg_write_bits(pll_base, AFC2_REF_CLK_NDIV, cfg->afc2_ref_clk_ndiv << AFC2_REF_CLK_NDIV_LSB, AFC2_REF_CLK_NDIV_MASK);
	irf_reg_write_bits(pll_base, AFC2_K_BASE, cfg->afc2_k_base << AFC2_K_BASE_LSB, AFC2_K_BASE_MASK);
#endif
	irf_reg_write_bits(pll_base, DCC_CFG, cfg->dcc_u<<DCC_U_LSB, DCC_U_MASK);
	irf_reg_write_bits(pll_base, DCC_CFG,  0<<DCC_INV_LSB, DCC_INV_MASK);
	irf_reg_write_bits(pll_base, DIDTC_CFG, cfg->didtc_os<<DIDTC_OS_LSB, DIDTC_OS_MASK);
	irf_reg_write_bits(pll_base, LMS_1CFG, cfg->lms_dly_sel<<LMS_DLY_SEL_LSB, LMS_DLY_SEL_MASK);
	irf_reg_write_bits(pll_base, LMS_1CFG, cfg->lms_acc_len<<LMS_ACC_LEN_LSB, LMS_ACC_LEN_MASK);
	irf_reg_write_bits(pll_base, LMS_2CFG, cfg->lms_u<<LMS_U_LSB, LMS_U_MASK);
	irf_reg_write_bits(pll_base, LMS_2CFG, cfg->lms_os<<LMS_OS_LSB, LMS_OS_MASK);
	irf_reg_write_bits(pll_base, LMS_GAIN_LOW, cfg->lms_gain_ini_low<<LMS_GAIN_INI_LOW_LSB, LMS_GAIN_INI_LOW_MASK);
	irf_reg_write_bits(pll_base, LMS_GAIN_HIGH, cfg->lms_gain_ini_high<<LMS_GAIN_INI_HIGH_LSB, LMS_GAIN_INI_HIGH_MASK);
#if (defined(DUBHE2000) && DUBHE2000)
	irf_reg_write_bits(pll_base, INL_1CFG, cfg->dtc_inl_u_a << DTC_INL_U_A_LSB, DTC_INL_U_A_MASK);
	irf_reg_write_bits(pll_base, INL_3CFG, cfg->dtc_inl_coe2_ini_low << DTC_INL_COE2_INI_LOW_LSB, DTC_INL_COE2_INI_LOW_MASK);
	irf_reg_write_bits(pll_base, INL_4CFG, cfg->dtc_inl_coe2_ini_high << DTC_INL_COE2_INI_HIGH_LSB, DTC_INL_COE2_INI_HIGH_MASK);
#endif
	irf_reg_write_bits(pll_base, LOCKDET_REF_CLK_NDIV, cfg->lockdet_ref_clk_ndiv<<LOCKDET_REF_CLK_NDIV_LSB, LOCKDET_REF_CLK_NDIV_MASK);
	irf_reg_write_bits(pll_base, LOCKDET_KBASE, cfg->lockdet_kbase<<LOCKDET_KBASE_LSB, LOCKDET_KBASE_MASK);
	irf_reg_write_bits(pll_base, LOCKDET_LOCKED_KDELTA_TH, cfg->lockdet_locked_kdelta_th<<LOCKDET_LOCKED_KDELTA_TH_LSB, LOCKDET_LOCKED_KDELTA_TH_MASK);
	irf_reg_write_bits(pll_base, LOCKDET_UNLOCK_KDELTA_TH, cfg->lockdet_unlock_kdelta_th<<LOCKDET_UNLOCK_KDELTA_TH_LSB, LOCKDET_UNLOCK_KDELTA_TH_MASK);
	irf_reg_write_bits(pll_base, LOCKDET_LOCKED_WIN, cfg->lockdet_locked_win<<LOCKDET_LOCKED_WIN_LSB, LOCKDET_LOCKED_WIN_MASK);
	irf_reg_write_bits(pll_base, LOCKDET_UNLOCK_WIN, cfg->lockdet_unlock_win<<LOCKDET_UNLOCK_WIN_LSB, LOCKDET_UNLOCK_WIN_MASK);
	irf_reg_write_bits(pll_base, LOCKDET_LOCKED_WIN_TH, cfg->lockdet_locked_win_th<<LOCKDET_LOCKED_WIN_TH_LSB, LOCKDET_LOCKED_WIN_TH_MASK);
	irf_reg_write_bits(pll_base, LOCKDET_UNLOCK_WIN_TH, cfg->lockdet_unlock_win_th<<LOCKDET_UNLOCK_WIN_TH_LSB, LOCKDET_UNLOCK_WIN_TH_MASK);
	irf_reg_write_bits(pll_base, OUT_IDLE_DFLT_CBANK_VCO_VCTRL_DIDTC, cfg->set_cbank_idle<<SET_CBANK_IDLE_LSB, SET_CBANK_IDLE_MASK);
	irf_reg_write_bits(pll_base, OUT_IDLE_DFLT_CBANK_VCO_VCTRL_DIDTC, cfg->set_vco_vref_sel_idle<<SET_VCO_VREF_SEL_IDLE_LSB, SET_VCO_VREF_SEL_IDLE_MASK);
	irf_reg_write_bits(pll_base, OUT_IDLE_DFLT_CBANK_VCO_VCTRL_DIDTC, cfg->set_vctrl_sel_idle<<SET_VCTRL_SEL_IDLE_LSB, SET_VCTRL_SEL_IDLE_MASK);
	irf_reg_write_bits(pll_base, OUT_IDLE_DFLT_P_S_DTC, cfg->set_div_p_idle<<SET_DIV_P_IDLE_LSB, SET_DIV_P_IDLE_MASK);
	irf_reg_write_bits(pll_base, OUT_IDLE_DFLT_P_S_DTC, cfg->set_div_s_idle<<SET_DIV_S_IDLE_LSB, SET_DIV_S_IDLE_MASK);
	irf_reg_write_bits(pll_base, OUT_AFC_DFLT_CBANK_VCO_VCTRL_DIDTC, cfg->set_vco_vref_sel_afc<<SET_VCO_VREF_SEL_AFC_LSB, SET_VCO_VREF_SEL_AFC_MASK);
	irf_reg_write_bits(pll_base, OUT_AFC_DFLT_CBANK_VCO_VCTRL_DIDTC, cfg->set_vctrl_sel_afc<<SET_VCTRL_SEL_AFC_LSB, SET_VCTRL_SEL_AFC_MASK);
	irf_reg_write_bits(pll_base, OUT_AFC_DFLT_P_S_DTC, cfg->set_div_p_afc<<SET_DIV_P_AFC_LSB, SET_DIV_P_AFC_MASK);
	irf_reg_write_bits(pll_base, OUT_AFC_DFLT_P_S_DTC, cfg->set_div_s_afc<<SET_DIV_S_AFC_LSB, SET_DIV_S_AFC_MASK);
	irf_reg_write_bits(pll_base, OUT_KVCO_DFLT_P_S_DTC, cfg->set_div_p_kvco<<SET_DIV_P_KVCO_LSB, SET_DIV_P_KVCO_MASK);
	irf_reg_write_bits(pll_base, OUT_KVCO_DFLT_P_S_DTC, cfg->set_div_s_kvco<<SET_DIV_S_KVCO_LSB, SET_DIV_S_KVCO_MASK);
	irf_reg_write_bits(pll_base, LOOPSTATE_WAIT_TIME, 4294967295<<LOOP_WORK_WT_TIME_LSB, LOOP_WORK_WT_TIME_MASK);

	return 0;
}

int pll_afc_config(void __iomem *pll_base)
{
	int i;
	uint32_t status;
	uint32_t main_current_state;
	uint32_t open_afc_sta_ind;

	irf_reg_write_bits(pll_base, MAIN_STATE_NOJUMP,1<<CFG_OPEN_AFC_NOJUMP_LSB,CFG_OPEN_AFC_NOJUMP_MASK);
	irf_reg_write_bits(pll_base,  MAIN_STATE_START,1<<CFG_OPEN_AFC_START_LSB,CFG_OPEN_AFC_START_MASK);
	udelay(1);
	irf_reg_write_bits(pll_base, MAIN_STATE_START,0,CFG_OPEN_AFC_START_MASK);

	for(i = 0;i < RETRY_TIMES; i++){
		udelay(10000);
		status = CLS_REG_RAW_READ32(pll_base, MAIN_STATE_RD);
		main_current_state = (status & MAIN_CURRENT_STATE_MASK) >> MAIN_CURRENT_STATE_LSB;
		open_afc_sta_ind = (status & OPEN_AFC_STA_IND_MASK) >> OPEN_AFC_STA_IND_LSB;
		if((PLL_STATE_AFC == main_current_state ) &&(!open_afc_sta_ind)){
			return 0;
		}
	}
	pr_err("[AFC]PLLSA main status:0x%x\n",status);
	return -1;
}


int pll_kvco_config(void __iomem *pll_base)
{
	int i;
	uint32_t status;
	uint32_t main_current_state;
	uint32_t open_kvco_sta_ind;

	irf_reg_write_bits(pll_base, MAIN_STATE_NOJUMP,1<<CFG_OPENKVCO_NOJUMP_LSB,CFG_OPENKVCO_NOJUMP_MASK);
	irf_reg_write_bits(pll_base, MAIN_STATE_START,1<<CFG_OPENKVCO_START_LSB,CFG_OPENKVCO_START_MASK);
	udelay(1);
	irf_reg_write_bits(pll_base, MAIN_STATE_START,0,CFG_OPENKVCO_START_MASK);

	for(i = 0;i < RETRY_TIMES; i++){
		udelay(10000);
		status = CLS_REG_RAW_READ32(pll_base, MAIN_STATE_RD);
		main_current_state = (status & MAIN_CURRENT_STATE_MASK) >> MAIN_CURRENT_STATE_LSB;
		open_kvco_sta_ind = (status & OPEN_KVCO_STA_IND_MASK) >> OPEN_KVCO_STA_IND_LSB;
		if((PLL_STATE_KVCO == main_current_state ) &&(!open_kvco_sta_ind)){
			return 0;
		}
	}

	pr_err("[KVCO]PLLSA main status:0x%x\n",status);
	return 0;
}


#if !(defined(DUBHE2000) && DUBHE2000)
int pll_manul_start_kdet(void __iomem *pll_base, uint32_t *kdet)
{
	uint8_t i;
	uint32_t kdet_result;

	irf_reg_write_bits(pll_base, AFC_MANUAL_CTRL,1<<AFC_MANUAL_START_LSB,AFC_MANUAL_START_MASK);
	udelay(1);
	irf_reg_write_bits(pll_base, AFC_MANUAL_CTRL,0<<AFC_MANUAL_START_LSB,AFC_MANUAL_START_MASK);
	for(i = 0;i<RETRY_TIMES;i++){
		kdet_result = CLS_REG_RAW_READ32(pll_base, AFC_MANUAL_RSULT);
		if(kdet_result & AFC_MANUAL_KDET_VALID_MASK){
			*kdet = kdet_result &AFC_MANUAL_KDET_MASK;
			return 0;
		}
		udelay(10000);
	}

	pr_err("kdet continuous detect failure.\n");

	return -1;
}



int pll_cbank_search(void __iomem *pll_base, struct frequency_plan_subtbl *cfg)
{
	uint32_t analog_vctrl_sel;
	uint8_t cbank,cbank_step,cbank1,cbank2;
	uint32_t kdet,kdet1,kdet2;

	cbank = 64;
	cbank_step = 32;

	irf_reg_write_bits(pll_base, MT_ANALOG_RD_PULSE,1<<MT_TO_SOFT_RD_PULSE_LSB,MT_TO_SOFT_RD_PULSE_MASK);

	analog_vctrl_sel = (CLS_REG_RAW_READ32(pll_base, ANALOG_CBANK_VCO_VCTRL_DIDTC)>>ANALOG_VCTRL_SEL_LSB) & 0xf;

	irf_reg_write_bits(pll_base, OUT_AFC_DFLT_CBANK_VCO_VCTRL_DIDTC,
		analog_vctrl_sel<<SET_VCTRL_SEL_AFC_LSB,SET_VCTRL_SEL_AFC_MASK);
	//TODO:should index table by Tsensor
	irf_reg_write_bits(pll_base, KVCO_DEFAULT_VREF_VCTRL,
		cfg->set_vco_vref_sel_afc<<KVCO_DEFAULT_VREF_SEL_LSB,KVCO_DEFAULT_VREF_SEL_MASK);
	irf_reg_write_bits(pll_base, AFC_MANUAL_CTRL,1<<AFC_MANUAL_MODE_LSB,AFC_MANUAL_MODE_MASK);
	irf_reg_write_bits(pll_base, AFC_MANUAL_CTRL,(cbank&0x7f)<<AFC_MANUAL_CBANK_LSB,AFC_MANUAL_CBANK_MASK);

	do{
		if(0 != pll_manul_start_kdet(pll_base,&kdet)){
			return -1;
		}

		if(kdet < cfg->afc_k_base){
			if(cbank_step >= 1){
				cbank = cbank + cbank_step;
				cbank_step = cbank_step/2;
				irf_reg_write_bits(pll_base, AFC_MANUAL_CTRL,(cbank&0x7f)<<AFC_MANUAL_CBANK_LSB,AFC_MANUAL_CBANK_MASK);
				continue;
			} else {
				kdet1 = kdet;
				cbank1 = cbank;
				cbank2 = cbank1+1;
				irf_reg_write_bits(pll_base, AFC_MANUAL_CTRL,(cbank2&0x7f)<<AFC_MANUAL_CBANK_LSB,AFC_MANUAL_CBANK_MASK);

				if(0 != pll_manul_start_kdet(pll_base,&kdet2)){
					return -1;
				}
				if(abs((int)(kdet1 - cfg->afc_k_base)) <= abs((int)(kdet2 - cfg->afc_k_base))){
					irf_reg_write_bits(pll_base, AFC_MANUAL_CTRL,(cbank1&0x7f)<<AFC_MANUAL_CBANK_LSB,AFC_MANUAL_CBANK_MASK);
				}
				/* cbank2 been writed before */
				/*
				else {
					irf_reg_write_bits(pll_base, AFC_MANUAL_CTRL,(cbank2&0x7f)<<AFC_MANUAL_CBANK_LSB,AFC_MANUAL_CBANK_MASK);
				}*/

				return 0;
			}
		} else {
			if(kdet > cfg->afc_k_base){
				if(cbank_step >= 1){
					cbank = cbank - cbank_step;
					cbank_step = cbank_step/2;
					irf_reg_write_bits(pll_base, AFC_MANUAL_CTRL,(cbank&0x7f)<<AFC_MANUAL_CBANK_LSB,AFC_MANUAL_CBANK_MASK);
					continue;
				} else {
					kdet1 = kdet;
					cbank1 = cbank;
					cbank2 = cbank1 - 1;
					irf_reg_write_bits(pll_base, AFC_MANUAL_CTRL,(cbank2&0x7f)<<AFC_MANUAL_CBANK_LSB,AFC_MANUAL_CBANK_MASK);

					if(0 != pll_manul_start_kdet(pll_base,&kdet2)){
						return -1;
					}
					if(abs((int)(kdet1 - cfg->afc_k_base)) <= abs((int)(kdet2 - cfg->afc_k_base))){
						irf_reg_write_bits(pll_base, AFC_MANUAL_CTRL,(cbank1&0x7f)<<AFC_MANUAL_CBANK_LSB,AFC_MANUAL_CBANK_MASK);
					}
					/* cbank2 been writed before */
					/*
					else {
						irf_reg_write_bits(pll_base, AFC_MANUAL_CTRL,(cbank2&0x7f)<<AFC_MANUAL_CBANK_LSB,AFC_MANUAL_CBANK_MASK);
					}*/

					return 0;
				}
			} else{
				return 0;
			}
		}

	}while(1);

	return 0;
}
#endif

int pllsa_doubler_adjust(void __iomem *afe_base)
{
	uint8_t sel_pllsa = 0;
	uint32_t i;

	for(i = 0;i<=255;i++){
		irf_reg_write_bits(afe_base, AFE_RW_127,sel_pllsa,DOUBLER_SEL_PLLSA_MASK);
		if(CLS_REG_RAW_READ32(afe_base, AFE_RO_15) & CO_BIT(DOUBLER_DET_PLLSA_LSB)){
			irf_reg_write_bits(afe_base, AFE_RW_127,sel_pllsa/2,DOUBLER_SEL_PLLSA_MASK);
			return 0;
		}
		sel_pllsa++;
	}

	pr_err("PLLSA doubler adjust over 255.\n");
	return 1;
}

#if (defined(DUBHE2000) && DUBHE2000)
int pll_afc2_config(void __iomem *pll_base)
{
	int i;
	uint32_t status;
	uint32_t main_current_state;
	uint32_t open_afc2_sta_ind;

	irf_reg_write_bits(pll_base, MAIN_STATE_NOJUMP, 1 << CFG_OPENAFC2_NOJUMP_LSB, CFG_OPENAFC2_NOJUMP_MASK);
	irf_reg_write_bits(pll_base, MAIN_STATE_START, 1 << CFG_OPENAFC2_START_LSB, CFG_OPENAFC2_START_MASK);
	udelay(1);
	irf_reg_write_bits(pll_base, MAIN_STATE_START , 0, CFG_OPENAFC2_START_MASK);

	for (i = 0; i < RETRY_TIMES; i++) {
		udelay(10000);
		status = CLS_REG_RAW_READ32(pll_base, MAIN_STATE_RD);
		main_current_state = (status & MAIN_CURRENT_STATE_MASK) >> MAIN_CURRENT_STATE_LSB;
		open_afc2_sta_ind = (status & OPEN_AFC2_STA_IND_MASK) >> OPEN_AFC2_STA_IND_LSB;
		if ((PLL_STATE_AFC2 == main_current_state) && (!open_afc2_sta_ind))
			return 0;
	}

	pr_err("[AFC2]PLLSA main status:0x%x\n", status);

	return -1;
}
#endif

int pllsa_config(struct cls_wifi_plat *cls_wifi_plat, u8 radio_index)
{
	struct frequency_plan_subtbl *cfg = NULL;
	uint32_t pll_lock_ind;
	uint32_t lock_ind;
	void __iomem *afe_base;
	void __iomem *pll_base;
	int ret = 0;
	uint32_t loop_cfg_val;
	uint32_t loop_cfg_mask;
	void __iomem *rst_para_addr;
	struct irf_comm_tbl *pll_tbl;
	struct frequency_plan_subtbl *sub_tbl;
	uint32_t *buf;
	uint32_t size;
	int i;
	uint8_t version;
	char full_name[FS_PATH_MAX_LEN];
	uint32_t N_value;
	uint32_t P_value;
	uint32_t vco_ldo = 0;

	irf_get_fullname(cls_wifi_plat, full_name, PLL_PLAN_2G_DATA);
	size = cls_wifi_load_irf_binfile(cls_wifi_plat, radio_index, full_name,
			PLL_PLAN_2G_ADDR_D2K, PLL_PLAN_2G_SIZE_D2K, &version, 1);
	if (!size)
		return -1;

	buf = kzalloc(size * sizeof(u32), GFP_KERNEL);
	if (!buf)
		return -1;

	cls_wifi_plat->ep_ops->irf_table_readn(cls_wifi_plat, radio_index, PLL_PLAN_2G_ADDR_D2K, buf,
			size);

	pll_tbl = (struct irf_comm_tbl*)buf;
	sub_tbl = (struct frequency_plan_subtbl*)pll_tbl->data;

	for (i = 0; i <pll_tbl->head.sub_band_num; i++) {
		if(sub_tbl->center_freq == 0){ // center_freq == 0: pllsa
			cfg = sub_tbl;
			break;
		}
		sub_tbl++;
	}
	if (!cfg)
		return -1;

	afe_base = ioremap(AFE_RW_BASE, 0x1000);
	pll_base = ioremap(AFE_PLL_SA_BASE, 0x1000);

	rst_para_addr = ioremap(CLS_DUBHE2000_TOP_RST_PARA_BASE, CLS_DUBHE2000_TOP_RST_PARA_SIZE);

	//0. ldo config
	AFE_ACTION(afe_base, pll_sa_ldo_cfg);

	//1. osc config
	AFE_ACTION(afe_base, pllsa_osc_cfg);

	//start bandgap
	afe_com_top_bandgap_config();

	//2. ref clock config
	AFE_ACTION(afe_base, pllsa_refclk_cfg);
	udelay(10);
	AFE_ACTION(afe_base, pllsa_refclk_cfg_1);

	//3. start RC tune/R tune/Ibias/Psensor
	afe_com_top_rctune_config(cls_wifi_plat, radio_index);
	afe_com_top_rtune_ibias_config(cls_wifi_plat, radio_index);
	afe_com_top_psensor_config(cls_wifi_plat, radio_index);

	//4. doubler enable, duty cycle calibration
	if(cfg->doubler_en2){
		irf_reg_write_bits(afe_base, AFE_RW_127, 1<<DOUBLER_EN_PLLSA_LSB, DOUBLER_EN_PLLSA_MASK);
		irf_reg_write_bits(afe_base, AFE_RW_127, 0, DOUBLER_RSTB_PLLSA_MASK);
		udelay(1);
		irf_reg_write_bits(afe_base, AFE_RW_127, 1<<DOUBLER_RSTB_PLLSA_LSB, DOUBLER_RSTB_PLLSA_MASK);
		pllsa_doubler_adjust(afe_base);
	}

	// 5.1 alalog power config
	irf_reg_write_bits(afe_base, AFE_RW_126, 18 << CKOLDO_SEL_PLLSA_LSB, CKOLDO_SEL_PLLSA_MASK);
	udelay(10);
	irf_reg_write_bits(afe_base, AFE_RW_126, 1 << CKOLDO_FS_PD_PLLSA_LSB, CKOLDO_FS_PD_PLLSA_MASK);

	cls_wifi_plat->ep_ops->irf_table_readn(cls_wifi_plat, radio_index,
										   IRF_SHARE_MEM_ADDR_D2K + offsetof(struct irf_share_data, psensor_type[PSENSOR_LVT].N_value), &N_value,
										   sizeof(N_value));

	cls_wifi_plat->ep_ops->irf_table_readn(cls_wifi_plat, radio_index,
										   IRF_SHARE_MEM_ADDR_D2K + offsetof(struct irf_share_data, psensor_type[PSENSOR_LVT].P_value), &P_value,
										   sizeof(P_value));

	// VCOLDO_SEL_PLLSA=MAX(18,MIN(28,18+ROUND((P_lvt_code+N_lvt_code-1120)/60,0)));

	// vco_ldo = max(18, min(28, 18 + ROUND((int32_t)(P_value + N_value) - 1120) / 60, 0)));

	vco_ldo = max(18, min(28, (((int32_t)(P_value + N_value) - 1120) / 60 + 18)));

	pr_err("PSENSOR_LVT: N_value = %u, P_value = %u, vco_ldo = %u\n", N_value, P_value, vco_ldo);

	irf_reg_write_bits(afe_base, AFE_RW_126, vco_ldo << VCOLDO_SEL_PLLSA_LSB, VCOLDO_SEL_PLLSA_MASK);
	udelay(10);
	irf_reg_write_bits(afe_base, AFE_RW_126, 1<<VCOLDO_FS_PD_PLLSA_LSB, VCOLDO_FS_PD_PLLSA_MASK);
	irf_reg_write_bits(afe_base, AFE_RW_126, 1<<CPLDO_RES_SHTEN_PLLSA_LSB, CPLDO_RES_SHTEN_PLLSA_MASK);
	irf_reg_write_bits(afe_base, AFE_RW_126, 7<<CPLDO_SEL_PLLSA_LSB, CPLDO_SEL_PLLSA_MASK);
	udelay(10);
	irf_reg_write_bits(afe_base, AFE_RW_126, 1<<CPLDO_FS_PD_PLLSA_LSB, CPLDO_FS_PD_PLLSA_MASK);

	//5.2 PLL BW config
	irf_reg_write_bits(afe_base, AFE_RW_129, cfg->lpf_c3_sel<<LPF_C3_SEL_PLLSA_LSB, LPF_C3_SEL_PLLSA_MASK);
	irf_reg_write_bits(afe_base, AFE_RW_129, cfg->lpf_c2_sel<<LPF_C2_SEL_PLLSA_LSB, LPF_C2_SEL_PLLSA_MASK);
	irf_reg_write_bits(afe_base, AFE_RW_129, cfg->lpf_c1_sel<<LPF_C1_SEL_PLLSA_LSB, LPF_C1_SEL_PLLSA_MASK);
	irf_reg_write_bits(afe_base, AFE_RW_129, 1<<LPF_EN_PLLSA_LSB, LPF_EN_PLLSA_MASK);
	irf_reg_write_bits(afe_base, AFE_RW_129, cfg->lpf_r1_sel<<LPF_R1_SEL_PLLSA_LSB, LPF_R1_SEL_PLLSA_MASK);
	irf_reg_write_bits(afe_base, AFE_RW_130, 0, LPF_R3_BYP_EN_PLLSA_MASK);
	irf_reg_write_bits(afe_base, AFE_RW_130, cfg->lpf_r3_sel, LPF_R3_SEL_PLLSA_MASK);
	irf_reg_write_bits(afe_base, AFE_RW_128, cfg->cp_icode<<CP_ICODE_PLLSA_LSB, CP_ICODE_PLLSA_MASK);

	//5.3 vco config
#if (defined(DUBHE2000) && DUBHE2000)
	irf_reg_write_bits(afe_base, AFE_RW_131, 3 << VBIAS_CON_SEL_PLLSA_LSB, VBIAS_CON_SEL_PLLSA_MASK);
#endif
	irf_reg_write_bits(afe_base, AFE_RW_132,cfg->vco_vte_en<<VCO_VTE_EN_PLLSA_LSB,VCO_VTE_EN_PLLSA_MASK);
	irf_reg_write_bits(afe_base, AFE_RW_132,cfg->vco_vte_half_en<<VCO_VTE_HALF_EN_PLLSA_LSB,VCO_VTE_HALF_EN_PLLSA_MASK);
	irf_reg_write_bits(afe_base, AFE_RW_132,18<<VCO_VTE_VREF_SEL_PLLSA_LSB,VCO_VTE_VREF_SEL_PLLSA_MASK);
	irf_reg_write_bits(afe_base, AFE_RW_132,cfg->vco_vte_r2trim<<VCO_VTE_R2TRIM_PLLSA_LSB,VCO_VTE_R2TRIM_PLLSA_MASK);
	irf_reg_write_bits(afe_base, AFE_RW_132,cfg->vco_vte_r1trim<<VCO_VTE_R1TRIM_PLLSA_LSB,VCO_VTE_R1TRIM_PLLSA_MASK);
	AFE_ACTION(afe_base, pllsa_vco_cfg);

	//5.4 PFDCP&DTC&DIDTC&LOCKDE config
	AFE_ACTION(afe_base, pllsa_pfdcp_dtc_cfg);
	irf_reg_write_bits(afe_base, AFE_RW_132, cfg->cp_offset_sel << CP_OFFSET_SEL_PLLSA_LSB, CP_OFFSET_SEL_PLLSA_MASK);
	AFE_ACTION(afe_base, pllsa_pfdcp_dtc_cfg1);

	//5.5 digital config
	pll_digital_cfg(pll_base, cfg);

	irf_reg_write_bits(rst_para_addr, AFE_SUB_RST_PARA, 0 << SFT_AFE_PLL_SA_N_LSB, SFT_AFE_PLL_SA_N_MASK);

	//5.6 alalog clock config
	AFE_ACTION(afe_base, pllsa_alalog_clock_cfg);
	udelay(10);

	/* release PLL reset  */
	irf_reg_write_bits(rst_para_addr, AFE_SUB_RST_PARA, 1 << SFT_AFE_PLL_SA_N_LSB, SFT_AFE_PLL_SA_N_MASK);

	udelay(10);

	//5.8 update PLL digital registers
#if (defined(DUBHE2000) && DUBHE2000)
	irf_reg_write_bits(pll_base, LOOPSTATE_LOOPFUNC_CFG, 0, LOOP_CFG_UP_MASK);
	irf_reg_write_bits(pll_base, DCC_CFG, 0, DCC_CFG_UP_MASK);
	irf_reg_write_bits(pll_base, DIDTC_CFG, 0, DIDTC_CFG_UP_MASK);
	irf_reg_write_bits(pll_base, DSM_CFG, 0, DSM_CFG_UP_MASK);
	udelay(10000);
	irf_reg_write_bits(pll_base, LOOPSTATE_LOOPFUNC_CFG, 1 << LOOP_CFG_UP_LSB, LOOP_CFG_UP_MASK);
	irf_reg_write_bits(pll_base, DCC_CFG, 1 << DCC_CFG_UP_LSB, DCC_CFG_UP_MASK);
	irf_reg_write_bits(pll_base, DIDTC_CFG, 1 << DIDTC_CFG_UP_LSB, DIDTC_CFG_UP_MASK);
	irf_reg_write_bits(pll_base, DSM_CFG, 1 << DSM_CFG_UP_LSB, DSM_CFG_UP_MASK);
#endif

	//6.0 AFC calibration
	pll_afc_config(pll_base);

	//7.0 KVCO calibration
	pll_kvco_config(pll_base);

#if (defined(DUBHE2000) && DUBHE2000)
	//8.0 afc2 calibration
	pll_afc2_config(pll_base);
#else
	//8.0 cbank search
	pll_cbank_search(pll_base,cfg);

	irf_reg_write_bits(pll_base, AFC_MANUAL_CTRL, 1<<AFC_MANUAL_START_LSB, AFC_MANUAL_START_MASK);
	udelay(1);
	irf_reg_write_bits(pll_base, AFC_MANUAL_CTRL, 0<<AFC_MANUAL_START_LSB, AFC_MANUAL_START_MASK);
#endif

	//9.0 PLL LOOP mode
	irf_reg_write_bits(pll_base, LMS_2CFG, 1<<LMS_CFGOK_LSB,LMS_CFGOK_MASK);
	udelay(1);
	irf_reg_write_bits(pll_base, LMS_2CFG, 0<<LMS_CFGOK_LSB,LMS_CFGOK_MASK);
	irf_reg_write_bits(pll_base, LMS_GAIN_RDWREN, 1<<LMS_GAIN_WREN_LSB,LMS_GAIN_WREN_MASK);
#if (defined(DUBHE2000) && DUBHE2000)
	irf_reg_write_bits(pll_base, INL_2CFG, 1 << DTC_INL_COE2_WREN_LSB, DTC_INL_COE2_WREN_MASK);
#endif
	udelay(1);
	irf_reg_write_bits(pll_base, LMS_GAIN_RDWREN, 0<<LMS_GAIN_WREN_LSB,LMS_GAIN_WREN_MASK);
#if (defined(DUBHE2000) && DUBHE2000)
	irf_reg_write_bits(pll_base, INL_2CFG, 0, DTC_INL_COE2_WREN_MASK);
#endif

	irf_reg_write_bits(pll_base, LOOPSTATE_MANUAL_CFG,1<<LOOP_MANUAL_EN_LSB,LOOP_MANUAL_EN_MASK);
	irf_reg_write_bits(pll_base, MAIN_STATE_START,1<<CFG_LOOPWORK_START_LSB,CFG_LOOPWORK_START_MASK);
	udelay(1);
	irf_reg_write_bits(pll_base, MAIN_STATE_START,0<<CFG_LOOPWORK_START_LSB,CFG_LOOPWORK_START_MASK);
	AFE_ACTION(afe_base, pllsa_loop_mode);
	irf_reg_write_bits(pll_base, PD_LOOP_START, 1<<MANUAL_LOOPSTART_LSB, MANUAL_LOOPSTART_MASK);
	irf_reg_write_bits(pll_base, PD_LOOP_START, 1<<MANUAL_LOOPSTART_EN_LSB, MANUAL_LOOPSTART_EN_MASK);

	loop_cfg_val = (cfg->loop_cfg_dsm_en<<LOOP_CFG_DSM_EN_LSB) | (cfg->loop_cfg_didtc_en<<LOOP_CFG_DIDTC_EN_LSB) |
					(cfg->loop_cfg_dcc_en<<LOOP_CFG_DCC_EN_LSB) | (cfg->loop_cfg_dtc_lms_en<<LOOP_CFG_DTC_LMS_EN_LSB) |
					(cfg->loop_cfg_psgen_en<<LOOP_CFG_PSGEN_EN_LSB);

	loop_cfg_mask = LOOP_CFG_DSM_EN_MASK | LOOP_CFG_DIDTC_EN_MASK | LOOP_CFG_DCC_EN_MASK |
					LOOP_CFG_DTC_LMS_EN_MASK | LOOP_CFG_PSGEN_EN_MASK;
	irf_reg_write_bits(pll_base, LOOPSTATE_LOOPFUNC_CFG, loop_cfg_val, loop_cfg_mask);

#if (defined(DUBHE2000) && DUBHE2000)
	irf_reg_write_bits(pll_base, INL_1CFG, cfg->dtc_inl_en << DTC_INL_EN_LSB, DTC_INL_EN_MASK);
#endif

	irf_reg_write_bits(pll_base, MAIN_STATE_NOJUMP, 1<<CFG_LOOPWORK_NOJUMP_LSB,CFG_LOOPWORK_NOJUMP_MASK);

	//10. close FS EN
	udelay(1);
	irf_reg_write_bits(afe_base, AFE_RW_128, 0<<CP_FS_EN_PLLSA_LSB, CP_FS_EN_PLLSA_MASK);
	irf_reg_write_bits(afe_base, AFE_RW_132, 0<<VCO_VTE_FS_EN_PLLSA_LSB, VCO_VTE_FS_EN_PLLSA_MASK);

	/* 10.2 open lock detect  */
	irf_reg_write_bits(pll_base, MAIN_STATE_START, 1<<CFG_LOCK_DET_START_LSB, CFG_LOCK_DET_START_MASK);
	udelay(10);
	irf_reg_write_bits(pll_base, MAIN_STATE_START, 0<<CFG_LOCK_DET_START_LSB, CFG_LOCK_DET_START_MASK);

	//11. detect lock
	udelay(10000);
	irf_reg_write_bits(afe_base, AFE_RW_134,1<<LOCKDET_RSTB_PLLSA_LSB,LOCKDET_RSTB_PLLSA_MASK);
	/* add 10us delay */
	udelay(10);

	pll_lock_ind = CLS_REG_RAW_READ32(afe_base, AFE_RO_15);
	if((pll_lock_ind & LOCK_PLLSA_MASK) && (!(pll_lock_ind & UNLOCK_PLLSA_MASK))){
		pr_err("PLLSA locked, trans clock to ABB!\n");
		//12. trans clock to ABB
		AFE_ACTION(afe_base, pllsa_to_abb);
	} else {
		lock_ind = CLS_REG_RAW_READ32(pll_base, LOCKDET_LOCKED_IND);
		pr_err("PLLSA unlock, lock status:0x%x, 0x%x\n",pll_lock_ind,lock_ind);
		ret = -1;
	}

	kfree(buf);
	iounmap(afe_base);
	iounmap(pll_base);
	iounmap(rst_para_addr);

	return ret;
}
