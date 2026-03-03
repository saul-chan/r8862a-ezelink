#include <linux/delay.h>

#include "cls_wifi_irf.h"

#include "cls_wifi_msg_tx.h"
#include "cls_wifi_cali_debugfs.h"
#include "cls_wifi_soc.h"
#include "cls_wifi_cfgfile.h"

#include "afe_reg_def.h"
#include "cls_wifi_afe.h"
#include "cls_wifi_afe_init.h"
#include "cls_wifi_afe_com_top_config.h"

/*
static psensor_info_T psensor_type[PS_ARRAY_TYPE_NUM];
*/

const struct afe_reg_cfg com_top_bandgap_cfg_step1[] = {
	{AFE_RW_120, 1, BG_STU_EN_MASK, BG_STU_EN_LSB},
	{AFE_RW_120, 1, BG_FS_FLT_MASK, BG_FS_FLT_LSB},
	{AFE_RW_120, 0, BG_PD_MASK, BG_PD_LSB},
	{AFE_RW_120, 1, BG_VBK_EN_MASK, BG_VBK_EN_LSB},
};

const struct afe_reg_cfg com_top_bandgap_cfg_step2[] = {
	{AFE_RW_120, 0, BG_VBK_EN_MASK, BG_VBK_EN_LSB},
	{AFE_RW_120, 0, BG_FS_FLT_MASK, BG_FS_FLT_LSB},
	{AFE_RW_120, 1, BG_ROSC_EN_MASK, BG_ROSC_EN_LSB},
	{AFE_RW_120, 1, BG_CHOP_EN_MASK, BG_CHOP_EN_LSB},
};

const struct afe_reg_cfg com_top_rctune_rtune_ibias_cfg_step1[] = {
	{AFE_RW_120, 0, RCT_PD_MASK, RCT_PD_LSB},
	{AFE_RW_120, 1, RCT_EN_MASK, RCT_EN_LSB},
	{AFE_RW_120, 1, RCT_START_MASK, RCT_START_LSB},
};

const struct afe_reg_cfg com_top_rctune_rtune_ibias_cfg_step2[] = {
	{AFE_RW_120, 0, RCT_START_MASK, RCT_START_LSB},
	{AFE_RW_120, 0, RCT_EN_MASK, RCT_EN_LSB},
	{AFE_RW_120, 1, RCT_PD_MASK, RCT_PD_LSB},
};

const struct afe_reg_cfg com_top_rctune_rtune_ibias_cfg_step3[] = {
	{AFE_RW_120, 0, RT_PD_MASK, RT_PD_LSB},
	{AFE_RW_120, 1, RT_EN_MASK, RT_EN_LSB},
	{AFE_RW_120, 1, RT_START_MASK, RT_START_LSB},
};

const struct afe_reg_cfg com_top_rctune_rtune_ibias_cfg_step4[] = {
	{AFE_RW_120, 0, RT_START_MASK, RT_START_LSB},
	{AFE_RW_120, 0, RT_EN_MASK, RT_EN_LSB},
	{AFE_RW_120, 1, RT_PD_MASK, RT_PD_LSB},
};

const struct afe_reg_cfg com_top_psensor_cfg2[] = {
	{AFE_RW_124, 0, PS_EN_MASK, PS_EN_LSB},
	{AFE_RW_124, 1, PS_PD_MASK, PS_PD_LSB},
};

/* ver efuse addr */
#define SOC_EFUSE_REG_BASE 0x90414000
#define EFUSE_MAP_VER_LSB  8
#define EFUSE_MAP_VER_MASK 0xFF00

/* vbg efuse addr */
#define TOP_EFUSE_REG_BASE 0x90415000
#define COM_VBG_TRIM_LSB   0
#define COM_VBG_TRIM_MASK  0x00FF


int afe_com_top_bandgap_config(void)
{
	void __iomem *afe_reg_base;
	void __iomem *efuse_top_base;
	void __iomem *efuse_soc_base;

	uint32_t trimval = 0;

	pr_err("%s: start\n", __FUNCTION__);

	afe_reg_base   = ioremap(IO_LEFT_SRC_PARA_BASE, IO_LEFT_SRC_PARA_SIZE);
	efuse_soc_base = ioremap(SOC_EFUSE_REG_BASE, 0x10);
	efuse_top_base = ioremap(TOP_EFUSE_REG_BASE, 0x10);

	if (((CLS_REG_RAW_READ32(efuse_soc_base, 0) & EFUSE_MAP_VER_MASK) >> EFUSE_MAP_VER_LSB) > 0) {
		trimval = irf_reg_read_bits(efuse_top_base, 0, COM_VBG_TRIM_LSB, COM_VBG_TRIM_MASK);
		pr_err("afe efuse vbg trim = %d\n", trimval);
		irf_reg_write_bits(afe_reg_base, AFE_RW_120, trimval << BG_SEL_RVBG_LSB, BG_SEL_RVBG_MASK);
	}

	/* 1. bandgap step1 */
	AFE_ACTION(afe_reg_base, com_top_bandgap_cfg_step1);

#if (defined(DUBHE2000) && DUBHE2000)
	/* wait 800ms */
	mdelay(800);
#else
	/* wait 40ms */
	mdelay(40);
#endif

	/* 2. bandgap step2 */
	AFE_ACTION(afe_reg_base, com_top_bandgap_cfg_step2);

	iounmap(afe_reg_base);
	iounmap(efuse_soc_base);
	iounmap(efuse_top_base);

	pr_err("%s: finish\n", __FUNCTION__);

	return 0;
}

int afe_check_done_flag(void __iomem *base, uint32_t reg_addr, uint32_t lsb, uint32_t mask, uint32_t delay)
{
	uint32_t count = 0;

	while ((count < AFE_MAX_RUN_COUNT) && !irf_reg_read_bits(base, reg_addr, lsb, mask)) {
		udelay(delay);
		count++;
	}

	if (count >= AFE_MAX_RUN_COUNT)
		return -1;
	else
		return 0;
}

int afe_com_top_rctune_config(struct cls_wifi_plat *cls_wifi_plat, u8 radio_index)
{
	void __iomem *afe_reg_base;
	uint32_t rc_tune_value = 0;
	uint32_t i = 0;
	uint32_t sum = 0;
	char full_name[FS_PATH_MAX_LEN];
	uint8_t COM_TOP_BASE_REF_CLK = 48;
	uint8_t SELECT_48_OR_96M = 0; // 0: 48(40), 1: 96

	struct frequency_plan_subtbl *cfg = NULL;
	struct irf_comm_tbl *pll_tbl;
	struct frequency_plan_subtbl *sub_tbl;
	uint32_t *buf;
	uint32_t size;
	uint8_t version;

	pr_err("%s: start\n", __FUNCTION__);

	irf_get_fullname(cls_wifi_plat, full_name, PLL_PLAN_2G_DATA);
	size = cls_wifi_load_irf_binfile(cls_wifi_plat, radio_index, full_name,
		PLL_PLAN_2G_ADDR_D2K, PLL_PLAN_2G_SIZE_D2K, &version, 1);
	if (!size)
		return -1;

	buf = kzalloc(size * sizeof(u32), GFP_KERNEL);
	if (!buf)
		return -1;

	cls_wifi_plat->ep_ops->irf_table_readn(cls_wifi_plat, radio_index, PLL_PLAN_2G_ADDR_D2K, buf, size);

	pll_tbl = (struct irf_comm_tbl *)buf;
	sub_tbl = (struct frequency_plan_subtbl *)pll_tbl->data;

	for (i = 0; i < pll_tbl->head.sub_band_num; i++) {
		if (sub_tbl->center_freq == 0) {
			cfg = sub_tbl;
			break;
		}
		sub_tbl++;
	}
	if (!cfg)
		return -1;

	afe_reg_base = ioremap(IO_LEFT_SRC_PARA_BASE, IO_LEFT_SRC_PARA_SIZE);
	/* 1. according OSC's freq, set divider, we will use 48MHz OSC as first step, support 96MHz later. */
	irf_reg_write_bits(afe_reg_base, AFE_RW_120, SELECT_48_OR_96M << RCT_FRE_SEL_LSB, RCT_FRE_SEL_MASK);

	for (i = 0; i < AFE_TUNE_RUN_TIME; i++) {
		/* 2. start RC tune */
		AFE_ACTION(afe_reg_base, com_top_rctune_rtune_ibias_cfg_step1);

		/* wait 1ms */
		if (afe_check_done_flag(afe_reg_base, AFE_RO_04, RCT_DONE_LSB, RCT_DONE_MASK, 1000)) {
			pr_err("RC tune fail\n");
			iounmap(afe_reg_base);
			return -1;
		}
		rc_tune_value = irf_reg_read_bits(afe_reg_base, AFE_RO_04, RCT_DOUT_LSB, RCT_DOUT_MASK);
		sum += rc_tune_value;

		pr_err("%s: index [%u] rc_tune_value [%u]\n", __FUNCTION__, i, rc_tune_value);

		/* 5. stop RC tune */
		AFE_ACTION(afe_reg_base, com_top_rctune_rtune_ibias_cfg_step2);
	}

	pr_err("%s: sum of rc_tune_value [%u]\n", __FUNCTION__, sum);
	rc_tune_value = sum / AFE_TUNE_RUN_TIME;

	//no floating-point
	rc_tune_value = (uint32_t)((rc_tune_value * cfg->fref_MHz) / (SELECT_48_OR_96M + 1) / COM_TOP_BASE_REF_CLK );
	pr_err("ref_Mhz = %u, rc_tune_value = %u\n", cfg->fref_MHz, rc_tune_value);
	cls_wifi_plat->ep_ops->irf_table_writen(cls_wifi_plat, radio_index, IRF_SHARE_MEM_ADDR_D2K + offsetof(struct irf_share_data, rctune), &rc_tune_value,
											sizeof(rc_tune_value));

	kfree(buf);
	iounmap(afe_reg_base);

	pr_err("%s: finish\n", __FUNCTION__);

	return 0;
}

int afe_com_top_rtune_ibias_config(struct cls_wifi_plat *cls_wifi_plat, u8 radio_index)
{
	void __iomem *afe_reg_base;
	uint32_t r_tune_value = 0;
	uint32_t i = 0;
	uint32_t sum = 0;

	pr_err("%s: start\n", __FUNCTION__);

	afe_reg_base = ioremap(IO_LEFT_SRC_PARA_BASE, IO_LEFT_SRC_PARA_SIZE);
	for (i = 0; i < AFE_TUNE_RUN_TIME; i++) {
		/* 6. start R tune */
		AFE_ACTION(afe_reg_base, com_top_rctune_rtune_ibias_cfg_step3);

		/* wait 1ms */
		if (afe_check_done_flag(afe_reg_base, AFE_RO_04, RT_DONE_LSB, RT_DONE_MASK, 1000)) {
			pr_err("R tune fail\n");
			iounmap(afe_reg_base);
			return -1;
		}
		r_tune_value = irf_reg_read_bits(afe_reg_base, AFE_RO_04, RT_DOUT_LSB, RT_DOUT_MASK);
		sum += r_tune_value;

		pr_err("%s: index [%u] r_tune_value [%u]\n", __FUNCTION__, i, r_tune_value);

		/* 9. stop R tune */
		AFE_ACTION(afe_reg_base, com_top_rctune_rtune_ibias_cfg_step4);
	}

	pr_err("%s: sum of r_tune_value [%u]\n", __FUNCTION__, sum);
	r_tune_value = sum / AFE_TUNE_RUN_TIME;
	cls_wifi_plat->ep_ops->irf_table_writen(cls_wifi_plat, radio_index, IRF_SHARE_MEM_ADDR_D2K +
			offsetof(struct irf_share_data, rtune), &r_tune_value, sizeof(r_tune_value));
	pr_err("R tune value : %x\n", r_tune_value);

	/* 10. update RTUNE value to IBIAS */
	irf_reg_write_bits(afe_reg_base, AFE_RW_121, r_tune_value << IBIAS_R_SEL_LSB, IBIAS_R_SEL_MASK);

	/* 11. start IBIAS */
	irf_reg_write_bits(afe_reg_base, AFE_RW_121, 0 << IBIAS_PD_LSB, IBIAS_PD_MASK);

	iounmap(afe_reg_base);

	pr_err("%s: finish\n", __FUNCTION__);

	return 0;
}

int afe_com_top_psensor_config(struct cls_wifi_plat *cls_wifi_plat, u8 radio_index)
{
	void __iomem *afe_reg_base;
	uint32_t cur_sens_sel_value = 0;
	uint32_t n_sens_sel_value = 0;
	uint32_t p_sens_sel_value = 0;
	uint32_t n_sensitive_value = 0;
	uint32_t p_sensitive_value = 0;
	uint32_t psensor_array_idx = 0;
	uint32_t psensor_sen_sel_idx = 0;

	pr_err("%s: start\n", __FUNCTION__);

	afe_reg_base = ioremap(IO_LEFT_SRC_PARA_BASE, IO_LEFT_SRC_PARA_SIZE);
	for (psensor_array_idx = 0; psensor_array_idx < PS_ARRAY_TYPE_NUM; psensor_array_idx++) {
		for (cur_sens_sel_value = 0; cur_sens_sel_value < PS_SEN_SEL_NUM; cur_sens_sel_value++){
			cls_wifi_plat->ep_ops->irf_table_writen(cls_wifi_plat, radio_index, IRF_SHARE_MEM_ADDR_D2K +
					offsetof(struct irf_share_data, psensor_type[psensor_array_idx].psensor_sen_sel_idx[cur_sens_sel_value]),
					&psensor_sen_sel_idx, sizeof(psensor_sen_sel_idx));
		}
	}

	for (psensor_array_idx = 0; psensor_array_idx < PS_ARRAY_TYPE_NUM; psensor_array_idx++) {
		/* set psensor array */
		irf_reg_write_bits(afe_reg_base, AFE_RW_124, psensor_array_idx << PS_ARRAY_SEL_LSB, PS_ARRAY_SEL_MASK);

		/* set psensor sensor select to 0x7 by default */
		irf_reg_write_bits(afe_reg_base, AFE_RW_124, 0x7 << PS_SEN_SEL_LSB, PS_SEN_SEL_MASK);
		cur_sens_sel_value = 0x7;

		while (cur_sens_sel_value >= 0) {
			/* 1. N-Sensitive mode. */
			irf_reg_write_bits(afe_reg_base, AFE_RW_124, 56 << PS_CFG_SW_LSB, PS_CFG_SW_MASK);

			/* start Psensor. */
			irf_reg_write_bits(afe_reg_base, AFE_RW_124, 0 << PS_PD_LSB, PS_PD_MASK);
			udelay(100);
			irf_reg_write_bits(afe_reg_base, AFE_RW_124, 1 << PS_EN_LSB, PS_EN_MASK);

			/* wait 1ms */
			if (afe_check_done_flag(afe_reg_base, AFE_RO_04, PS_DONE_LSB, PS_DONE_MASK, 1000)) {
				pr_err("N mode : Psensor check done fail, type [%u] cur_sens_sel_value [%u]\n", psensor_array_idx, cur_sens_sel_value);
				/* 5. stop Psensor. */
				AFE_ACTION(afe_reg_base, com_top_psensor_cfg2);
				iounmap(afe_reg_base);
				return -1;
			}

			if (irf_reg_read_bits(afe_reg_base, AFE_RO_04, PS_OVERFLOW_LSB, PS_OVERFLOW_MASK)) {
				pr_err("N mode : Psensor overflow happen, cur_sens_sel_value [%u]\n", cur_sens_sel_value);
				if (cur_sens_sel_value > 0) {
					/* 5. stop Psensor. */
					AFE_ACTION(afe_reg_base, com_top_psensor_cfg2);
					cur_sens_sel_value--;
					irf_reg_write_bits(afe_reg_base, AFE_RW_124, cur_sens_sel_value << PS_SEN_SEL_LSB, PS_SEN_SEL_MASK);
					continue;
				} else {
					pr_err("N mode : Psensor overflow with least cur_sens_sel_value [%u], return now\n", cur_sens_sel_value);
					/* 5. stop Psensor. */
					AFE_ACTION(afe_reg_base, com_top_psensor_cfg2);
					iounmap(afe_reg_base);
					return -1;
				}
			} else {
				n_sensitive_value = irf_reg_read_bits(afe_reg_base, AFE_RO_04, PS_DOUT_LSB, PS_DOUT_MASK);
				pr_err("N mode : psensor_array_idx [%u] cur_sens_sel_value [%u] n_sensitive_value [%u]\n", psensor_array_idx, cur_sens_sel_value, n_sensitive_value);
				if (cur_sens_sel_value < 7) {
					n_sensitive_value = n_sensitive_value << (7 - cur_sens_sel_value);
					cls_wifi_plat->ep_ops->irf_table_writen(cls_wifi_plat, radio_index, IRF_SHARE_MEM_ADDR_D2K +
							offsetof(struct irf_share_data, psensor_type[psensor_array_idx].N_mode_val[7]),
							&n_sensitive_value, sizeof(n_sensitive_value));

					pr_err("Update N mode : psensor_array_idx [%u] n_sens_sel_value [7] n_sensitive_value [%u]\n",
					psensor_array_idx, n_sensitive_value);
				} else {
					cls_wifi_plat->ep_ops->irf_table_writen(cls_wifi_plat, radio_index, IRF_SHARE_MEM_ADDR_D2K +
							offsetof(struct irf_share_data, psensor_type[psensor_array_idx].N_mode_val[cur_sens_sel_value]),
							&n_sensitive_value, sizeof(n_sensitive_value));
				}
				n_sens_sel_value = cur_sens_sel_value;

				/* 5. stop Psensor. */
				AFE_ACTION(afe_reg_base, com_top_psensor_cfg2);

				/* 6. P-Sensitive mode. */
				irf_reg_write_bits(afe_reg_base, AFE_RW_124, 19 << PS_CFG_SW_LSB, PS_CFG_SW_MASK);

				/* start Psensor. */
				irf_reg_write_bits(afe_reg_base, AFE_RW_124, 0 << PS_PD_LSB, PS_PD_MASK);
				udelay(100);
				irf_reg_write_bits(afe_reg_base, AFE_RW_124, 1 << PS_EN_LSB, PS_EN_MASK);

				/* wait 1ms */
				if (afe_check_done_flag(afe_reg_base, AFE_RO_04, PS_DONE_LSB, PS_DONE_MASK, 1000)) {
					pr_err("P mode : Psensor check done fail, type [%u] cur_sens_sel_value [%u]\n", psensor_array_idx, cur_sens_sel_value);
					/* 5. stop Psensor. */
					AFE_ACTION(afe_reg_base, com_top_psensor_cfg2);
					iounmap(afe_reg_base);
					return -1;
				}

				if (irf_reg_read_bits(afe_reg_base, AFE_RO_04, PS_OVERFLOW_LSB, PS_OVERFLOW_MASK)) {
					pr_err("P mode : Psensor overflow happen, cur_sens_sel_value [%u]\n", cur_sens_sel_value);
					if (cur_sens_sel_value > 0) {
						/* 5. stop Psensor. */
						AFE_ACTION(afe_reg_base, com_top_psensor_cfg2);
						cur_sens_sel_value--;
						irf_reg_write_bits(afe_reg_base, AFE_RW_124, cur_sens_sel_value << PS_SEN_SEL_LSB, PS_SEN_SEL_MASK);
						continue;
					} else {
						pr_err("P mode : Psensor overflow with least cur_sens_sel_value [%u], return now\n", cur_sens_sel_value);
						/* 5. stop Psensor. */
						AFE_ACTION(afe_reg_base, com_top_psensor_cfg2);
						iounmap(afe_reg_base);
						return -1;
					}
				} else {
					p_sensitive_value = irf_reg_read_bits(afe_reg_base, AFE_RO_04, PS_DOUT_LSB, PS_DOUT_MASK);
					pr_err("P mode : psensor_array_idx [%u] cur_sens_sel_value [%u] p_sensitive_value [%u]\n", psensor_array_idx, cur_sens_sel_value, p_sensitive_value);
					if (cur_sens_sel_value < 7) {
						p_sensitive_value = p_sensitive_value << (7 - cur_sens_sel_value);
						cls_wifi_plat->ep_ops->irf_table_writen(cls_wifi_plat, radio_index, IRF_SHARE_MEM_ADDR_D2K +
								offsetof(struct irf_share_data, psensor_type[psensor_array_idx].P_mode_val[7]),
								&p_sensitive_value, sizeof(p_sensitive_value));
						pr_err("Update P mode : psensor_array_idx [%u] p_sens_sel_value [7] p_sensitive_value [%u]\n",
							psensor_array_idx, p_sensitive_value);
					} else {
						cls_wifi_plat->ep_ops->irf_table_writen(cls_wifi_plat, radio_index, IRF_SHARE_MEM_ADDR_D2K +
								offsetof(struct irf_share_data, psensor_type[psensor_array_idx].P_mode_val[cur_sens_sel_value]),
								&p_sensitive_value, sizeof(p_sensitive_value));
					}
					p_sens_sel_value = cur_sens_sel_value;
					/* 5. stop Psensor. */
					AFE_ACTION(afe_reg_base, com_top_psensor_cfg2);
					break;
				}
			}
		}
		pr_err("psensor_array_idx [%u] n_sens_sel_value [%u] p_sens_sel_value [%u]\n", psensor_array_idx, n_sens_sel_value, p_sens_sel_value);

		if (n_sens_sel_value != p_sens_sel_value) {
			/* use the minimum value between n_sens_sel_value and p_sens_sel_value. */
			if (n_sens_sel_value > p_sens_sel_value) {
				n_sens_sel_value = p_sens_sel_value;

				/* re-calculate code value for N mode */
				pr_err("psensor_array_idx [%u] re-calculate code value, n_sens_sel_value [%u]\n", psensor_array_idx, n_sens_sel_value);

				irf_reg_write_bits(afe_reg_base, AFE_RW_124, n_sens_sel_value << PS_SEN_SEL_LSB, PS_SEN_SEL_MASK);

				/* 1. N-Sensitive mode. */
				irf_reg_write_bits(afe_reg_base, AFE_RW_124, 56 << PS_CFG_SW_LSB, PS_CFG_SW_MASK);

				/* start Psensor. */
				irf_reg_write_bits(afe_reg_base, AFE_RW_124, 0 << PS_PD_LSB, PS_PD_MASK);
				udelay(100);
				irf_reg_write_bits(afe_reg_base, AFE_RW_124, 1 << PS_EN_LSB, PS_EN_MASK);

				/* wait 1ms */
				if (afe_check_done_flag(afe_reg_base, AFE_RO_04, PS_DONE_LSB, PS_DONE_MASK, 1000)) {
					pr_err("N mode : Psensor check done fail, type [%u] n_sens_sel_value [%u]\n", psensor_array_idx, n_sens_sel_value);
					/* 5. stop Psensor. */
					AFE_ACTION(afe_reg_base, com_top_psensor_cfg2);
					iounmap(afe_reg_base);
					return -1;
				}

				if (irf_reg_read_bits(afe_reg_base, AFE_RO_04, PS_OVERFLOW_LSB, PS_OVERFLOW_MASK)) {
					pr_err("N mode : Psensor overflow happen, n_sens_sel_value [%u]\n", n_sens_sel_value);
					/* 5. stop Psensor. */
					AFE_ACTION(afe_reg_base, com_top_psensor_cfg2);
					iounmap(afe_reg_base);
					return -1;
				} else {
					n_sensitive_value = irf_reg_read_bits(afe_reg_base, AFE_RO_04, PS_DOUT_LSB, PS_DOUT_MASK);
					pr_err("N mode : psensor_array_idx [%u] n_sens_sel_value [%u] n_sensitive_value [%u]\n", psensor_array_idx, n_sens_sel_value, n_sensitive_value);
					if (n_sens_sel_value < 7) {
						n_sensitive_value = n_sensitive_value << (7 - n_sens_sel_value);
						cls_wifi_plat->ep_ops->irf_table_writen(cls_wifi_plat, radio_index, IRF_SHARE_MEM_ADDR_D2K +
								offsetof(struct irf_share_data, psensor_type[psensor_array_idx].N_mode_val[7]),
								&n_sensitive_value, sizeof(n_sensitive_value));

						psensor_sen_sel_idx = 1;
						cls_wifi_plat->ep_ops->irf_table_writen(cls_wifi_plat, radio_index, IRF_SHARE_MEM_ADDR_D2K +
								offsetof(struct irf_share_data, psensor_type[psensor_array_idx].psensor_sen_sel_idx[7]),
								&psensor_sen_sel_idx, sizeof(psensor_sen_sel_idx));

						pr_err("Update N mode : psensor_array_idx [%u] n_sens_sel_value [7] n_sensitive_value [%u]\n",
								psensor_array_idx, n_sensitive_value);
					} else {
						cls_wifi_plat->ep_ops->irf_table_writen(cls_wifi_plat, radio_index, IRF_SHARE_MEM_ADDR_D2K +
								offsetof(struct irf_share_data, psensor_type[psensor_array_idx].N_mode_val[n_sens_sel_value]),
								&n_sensitive_value, sizeof(n_sensitive_value));

						psensor_sen_sel_idx = 1;
						cls_wifi_plat->ep_ops->irf_table_writen(cls_wifi_plat, radio_index, IRF_SHARE_MEM_ADDR_D2K +
								offsetof(struct irf_share_data, psensor_type[psensor_array_idx].psensor_sen_sel_idx[n_sens_sel_value]),
								&psensor_sen_sel_idx, sizeof(psensor_sen_sel_idx));
					}

					/* 5. stop Psensor. */
					AFE_ACTION(afe_reg_base, com_top_psensor_cfg2);
					continue;
				}
			} else {
				pr_err("Warning : n_sens_sel_value [%u] is less than p_sens_sel_value [%u], it should not happen\n", n_sens_sel_value, p_sens_sel_value);
				p_sens_sel_value = n_sens_sel_value;

				/* re-calculate code value for P mode */
				pr_err("psensor_array_idx [%u] re-calculate code value, p_sens_sel_value [%u]\n", psensor_array_idx, p_sens_sel_value);

				irf_reg_write_bits(afe_reg_base, AFE_RW_124, p_sens_sel_value << PS_SEN_SEL_LSB, PS_SEN_SEL_MASK);

				/* 6. P-Sensitive mode. */
				irf_reg_write_bits(afe_reg_base, AFE_RW_124, 19 << PS_CFG_SW_LSB, PS_CFG_SW_MASK);

				/* start Psensor. */
				irf_reg_write_bits(afe_reg_base, AFE_RW_124, 0 << PS_PD_LSB, PS_PD_MASK);
				udelay(100);
				irf_reg_write_bits(afe_reg_base, AFE_RW_124, 1 << PS_EN_LSB, PS_EN_MASK);

				/* wait 1ms */
				if (afe_check_done_flag(afe_reg_base, AFE_RO_04, PS_DONE_LSB, PS_DONE_MASK, 1000)) {
					pr_err("P mode : Psensor check done fail, type [%u] p_sens_sel_value [%u]\n", psensor_array_idx, p_sens_sel_value);
					/* 5. stop Psensor. */
					AFE_ACTION(afe_reg_base, com_top_psensor_cfg2);
					iounmap(afe_reg_base);
					return -1;
				}

				if (irf_reg_read_bits(afe_reg_base, AFE_RO_04, PS_OVERFLOW_LSB, PS_OVERFLOW_MASK)) {
					pr_err("P mode : Psensor overflow happen, p_sens_sel_value [%u]\n", p_sens_sel_value);
					/* 5. stop Psensor. */
					AFE_ACTION(afe_reg_base, com_top_psensor_cfg2);
					iounmap(afe_reg_base);
					return -1;
				} else {
					p_sensitive_value = irf_reg_read_bits(afe_reg_base, AFE_RO_04, PS_DOUT_LSB, PS_DOUT_MASK);
					pr_err("P mode : psensor_array_idx [%u] p_sens_sel_value [%u] p_sensitive_value [%u]\n", psensor_array_idx, p_sens_sel_value, p_sensitive_value);
					if (p_sens_sel_value < 7) {

					p_sensitive_value = p_sensitive_value << (7 - p_sens_sel_value);
					cls_wifi_plat->ep_ops->irf_table_writen(cls_wifi_plat, radio_index, IRF_SHARE_MEM_ADDR_D2K +
							offsetof(struct irf_share_data, psensor_type[psensor_array_idx].P_mode_val[7]),
							&p_sensitive_value, sizeof(p_sensitive_value));

					psensor_sen_sel_idx = 1;
					cls_wifi_plat->ep_ops->irf_table_writen(cls_wifi_plat, radio_index, IRF_SHARE_MEM_ADDR_D2K +
							offsetof(struct irf_share_data, psensor_type[psensor_array_idx].psensor_sen_sel_idx[7]),
							&psensor_sen_sel_idx, sizeof(psensor_sen_sel_idx));

					pr_err("Update P mode : psensor_array_idx [%u] p_sens_sel_value [7] p_sensitive_value [%u]\n",
						psensor_array_idx, p_sensitive_value);
					} else {
						cls_wifi_plat->ep_ops->irf_table_writen(cls_wifi_plat, radio_index, IRF_SHARE_MEM_ADDR_D2K +
								offsetof(struct irf_share_data, psensor_type[psensor_array_idx].P_mode_val[p_sens_sel_value]),
								&p_sensitive_value, sizeof(p_sensitive_value));

						psensor_sen_sel_idx = 1;
						cls_wifi_plat->ep_ops->irf_table_writen(cls_wifi_plat, radio_index, IRF_SHARE_MEM_ADDR_D2K +
								offsetof(struct irf_share_data, psensor_type[psensor_array_idx].psensor_sen_sel_idx[p_sens_sel_value]),
								&psensor_sen_sel_idx, sizeof(psensor_sen_sel_idx));
					}

					/* 5. stop Psensor. */
					AFE_ACTION(afe_reg_base, com_top_psensor_cfg2);
					continue;
				}
			}
		} else {
			if (n_sens_sel_value < 7){
				psensor_sen_sel_idx = 1;
				cls_wifi_plat->ep_ops->irf_table_writen(cls_wifi_plat, radio_index, IRF_SHARE_MEM_ADDR_D2K +
						offsetof(struct irf_share_data, psensor_type[psensor_array_idx].psensor_sen_sel_idx[7]),
						&psensor_sen_sel_idx, sizeof(psensor_sen_sel_idx));
			} else {
				psensor_sen_sel_idx = 1;
				cls_wifi_plat->ep_ops->irf_table_writen(cls_wifi_plat, radio_index, IRF_SHARE_MEM_ADDR_D2K +
						offsetof(struct irf_share_data, psensor_type[psensor_array_idx].psensor_sen_sel_idx[n_sens_sel_value]),
						&psensor_sen_sel_idx, sizeof(psensor_sen_sel_idx));
			}
			continue;
		}
	}

	for (psensor_array_idx = 0; psensor_array_idx < PS_ARRAY_TYPE_NUM; psensor_array_idx++) {
		psensor_sen_sel_idx = 0;
		n_sensitive_value = 0;
		p_sensitive_value = 0;
		for (cur_sens_sel_value = 0; cur_sens_sel_value < PS_SEN_SEL_NUM; cur_sens_sel_value++) {
			cls_wifi_plat->ep_ops->irf_table_readn(
				cls_wifi_plat, radio_index,
				IRF_SHARE_MEM_ADDR_D2K + offsetof(struct irf_share_data, psensor_type[psensor_array_idx].psensor_sen_sel_idx[cur_sens_sel_value]),
				&psensor_sen_sel_idx, sizeof(psensor_sen_sel_idx));

			pr_err("psensoridx = %u, cur_sens_sel = %u, psensor_sen_sel_idx = %u\n", psensor_array_idx, cur_sens_sel_value, psensor_sen_sel_idx);

			if (psensor_sen_sel_idx == 1) {
				cls_wifi_plat->ep_ops->irf_table_readn(
					cls_wifi_plat, radio_index,
					IRF_SHARE_MEM_ADDR_D2K + offsetof(struct irf_share_data, psensor_type[psensor_array_idx].N_mode_val[cur_sens_sel_value]),
					&n_sensitive_value, sizeof(n_sensitive_value));

				cls_wifi_plat->ep_ops->irf_table_readn(
					cls_wifi_plat, radio_index,
					IRF_SHARE_MEM_ADDR_D2K + offsetof(struct irf_share_data, psensor_type[psensor_array_idx].P_mode_val[cur_sens_sel_value]),
					&p_sensitive_value, sizeof(p_sensitive_value));

				cls_wifi_plat->ep_ops->irf_table_writen(cls_wifi_plat, radio_index,
														IRF_SHARE_MEM_ADDR_D2K + offsetof(struct irf_share_data, psensor_type[psensor_array_idx].N_value),
														&n_sensitive_value, sizeof(n_sensitive_value));

				cls_wifi_plat->ep_ops->irf_table_writen(cls_wifi_plat, radio_index,
														IRF_SHARE_MEM_ADDR_D2K + offsetof(struct irf_share_data, psensor_type[psensor_array_idx].P_value),
														&p_sensitive_value, sizeof(p_sensitive_value));
				break;
			}
		}
	}

	for (psensor_array_idx = 0; psensor_array_idx < PS_ARRAY_TYPE_NUM; psensor_array_idx++) {
		n_sensitive_value = 0;
		p_sensitive_value = 0;
		cls_wifi_plat->ep_ops->irf_table_readn(cls_wifi_plat, radio_index,
											   IRF_SHARE_MEM_ADDR_D2K + offsetof(struct irf_share_data, psensor_type[psensor_array_idx].N_value),
											   &n_sensitive_value, sizeof(n_sensitive_value));

		cls_wifi_plat->ep_ops->irf_table_readn(cls_wifi_plat, radio_index,
											   IRF_SHARE_MEM_ADDR_D2K + offsetof(struct irf_share_data, psensor_type[psensor_array_idx].P_value),
											   &p_sensitive_value, sizeof(p_sensitive_value));
		pr_err("Psensor index: %u, N_value = %u, P_value = %u\n", psensor_array_idx, n_sensitive_value, p_sensitive_value);
	}

	iounmap(afe_reg_base);
	pr_err("%s: finish\n", __FUNCTION__);

	return 0;
}
