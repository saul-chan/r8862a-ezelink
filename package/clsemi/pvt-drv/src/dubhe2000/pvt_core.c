/*
 *  Copyright (c) 2021-2024, Clourney Semiconductor. All rights reserved.
 *
 *  This software and/or documentation is licensed by Clourney Semiconductor under limited terms and conditions.
 *  Reproduction and redistribution in binary or source form, with or without modification,
 *  for use solely in conjunction with a Clourney Semiconductor chipset, is permitted in condition which
 *  must retain the above copyright notice.
 *
 *  By using this software and/or documentation, you agree to the limited terms and conditions.
 *
 *  PVT-SONSOR driver for PVT
 *
 */

#include <linux/types.h>
#include <linux/of.h>
#include <linux/debugfs.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <stddef.h>
#include <pvt_sensor.h>
#include <linux/types.h>
#include <linux/io.h>

#define sensor_io_read(v)        readl(v)
#define sensor_io_write(a, v)    writel(v, a)

int tmp_rdata[T_SENSOR_NUM];

static const uint32_t loop_freq[] =
{
	810000000, 1770000000, 1480000000, 1150000000,
	290000000, 810000000, 810000000, 810000000,
};

static uint32_t create_low_12_bits_variable(uint32_t val)
{
	uint32_t mask = TS_ADC_BITMASK;
	return val & mask;
}

static uint32_t create_low_14_bits_variable(uint32_t val)
{
	uint32_t mask = VM_ADC_BITMASK;
	return val & mask;
}

static int sensor_get_response(struct clourney_pvt_dev *pvt_dev)
{
	int retry_times = 0, ret = 0, looptime = 100;

	while ((PVTS_HW_SW_LOCK_MASK & sensor_io_read(pvt_dev->reg_base + 0x14)) != 0) {
		retry_times++;
		if (retry_times > looptime) {
			ret = -1;
			break;
		}
	}

	return ret;
}

static int tsens_check_sdif_status(struct clourney_pvt_dev *pvt_dev)
{
	int retry_times = 0, ret = 0, looptime = 100;

	while ((PVT_SDIF_BUSY_MASK &
		sensor_io_read(pvt_dev->reg_base + TS_COMMON_OFFSET + COMMON_SDIF_STATUS)) != 0) {
		retry_times++;
		if (retry_times > looptime) {
			ret = -1;
			break;
		}
	}
	return ret;
}

static int psens_check_sdif_status(struct clourney_pvt_dev *pvt_dev)
{
	int retry_times = 0, ret = 0, looptime = 100;

	while ((PVT_SDIF_BUSY_MASK &
		sensor_io_read(pvt_dev->reg_base + PD_COMMON_OFFSET + COMMON_SDIF_STATUS)) != 0) {
		retry_times++;
		if (retry_times > looptime) {
			ret = -1;
			break;
		}
	}
	return ret;
}

static int vsens_check_sdif_status(struct clourney_pvt_dev *pvt_dev)
{
	int retry_times = 0, ret = 0, looptime = 100;

	while ((PVT_SDIF_BUSY_MASK &
		sensor_io_read(pvt_dev->reg_base + VM_COMMON_OFFSET + COMMON_SDIF_STATUS)) != 0) {
		retry_times++;
		if (retry_times > looptime) {
			ret = -1;
			break;
		}
	}
	return ret;
}

static uint32_t nbs_calc1(uint32_t f, uint32_t pd_cfg1, uint32_t pd_cfg2, uint32_t pd_cfg3, uint32_t pd_data, uint32_t flag_sel)
{
	// real eq1;
	uint32_t nbs_calc1_value;
	uint32_t pre_diva = 0;
	uint32_t pre_divb = 0;
	uint32_t win_size = 0;
	float dout_proc = 0;
	uint32_t dout_fault = 0;

	if (((pd_cfg2 >> 5) & 0x7) == 7)
		pre_divb = 1;
	else
		pre_divb = 4;

	if ((pd_cfg3 & 0xF) == 0)
		pre_diva = 4;
	else if ((pd_cfg3 & 0xF) == 1)
		pre_diva = 8;
	else if ((pd_cfg3 & 0xF) == 2)
		pre_diva = 16;
	else if ((pd_cfg3 & 0xF) == 3)
		pre_diva = 1;

	if (((pd_cfg3 >> 4) & 0xF) == 0)
		win_size = 255;
	else if (((pd_cfg3 >> 4) & 0xF) == 1)
		win_size = 127;
	else if (((pd_cfg3 >> 4) & 0xF) == 2)
		win_size = 63;
	else if (((pd_cfg3 >> 4) & 0xF) == 3)
		win_size = 31;

	// case (pd_cfg2[7:5])
	if (((pd_cfg2 >> 5) & 0x7) == 0)
		dout_proc = 0;
	else if (((pd_cfg2 >> 5) & 0x7) == 1)
		dout_proc = (float)(1.0 * loop_freq[1] * win_size) / (f * pre_diva * pre_divb);
	else if (((pd_cfg2 >> 5) & 0x7) == 2)
		dout_proc = (float)(1.0 * loop_freq[2] * win_size) / (f * pre_diva * pre_divb);
	else if (((pd_cfg2 >> 5) & 0x7) == 3)
		dout_proc = (float)(1.0 * loop_freq[3] * win_size) / (f * pre_diva * pre_divb);
	else if (((pd_cfg2 >> 5) & 0x7) == 4)
		dout_proc = (float)(1.0 * loop_freq[4] * win_size) / (f * pre_diva * pre_divb);
	else if (((pd_cfg2 >> 5) & 0x7) == 5)
		dout_proc = (float)(1.0 * loop_freq[5] * win_size) / (f * pre_diva * pre_divb);
	else if (((pd_cfg2 >> 5) & 0x7) == 6)
		dout_proc = (float)(1.0 * loop_freq[6] * win_size) / (f * pre_diva * pre_divb);
	else if (((pd_cfg2 >> 5) & 0x7) == 7)
		dout_proc = (float)(1.0 * loop_freq[7] * win_size) / (f * pre_diva * pre_divb);

	if ((pd_cfg1 & 0xF) == 0)
		nbs_calc1_value = dout_proc;
	else if ((pd_cfg1 & 0xF) == 4)
		nbs_calc1_value = dout_fault;
	else if ((pd_cfg1 & 0xF) == 8)
		nbs_calc1_value = 0xBA2;
	else if ((pd_cfg1 & 0xF) == 9)
		nbs_calc1_value = 0x45D;
	else if ((pd_cfg1 & 0xF) == 10)
		nbs_calc1_value = 0xBA2;
	else if ((pd_cfg1 & 0xF) == 11)
		nbs_calc1_value = 0x45D;
	else
		nbs_calc1_value = 0;
	return (pd_data * pre_diva * pre_divb * f) / win_size;
}

static int tempcal(uint32_t datain, uint32_t modecalc)
{
	int DataCal;

	DataCal = (220500 * datain - 279661140);

	DataCal = DataCal / 4094;

	return DataCal;
}

static int volcal(uint32_t datain, uint32_t resolution)
{
	int voltage = (1200 * ((6 * datain) - 3 - (1 << 14))) / ((1 << 14) * 5);
	return voltage;
}

static uint32_t nbs_calc(int t, uint32_t run_mode)
{
	uint32_t nbs_calc_v = 0.0;

	if (run_mode == 0)
		nbs_calc_v = (t * 40940000 - 1710473200) / 2205 + 2047000;
	if (run_mode == 1)
	 	nbs_calc_v = (t * 40940000 - 2380251600) / 2028 + 2047000;

	return nbs_calc_v / 1000;
}

static void sensor_pg00(struct clourney_pvt_dev *pvt_dev)
{
	uint32_t rdata = 0;
	if (debug_pr_info == 1)
		printk(KERN_INFO "pg00...\n");

	rdata = sensor_io_read(pvt_dev->reg_base + 0x0);
	if (debug_pr_info == 1) {
		printk(KERN_INFO "PVT MOORTEC ID:0x%04x COMPONENT ID:0x%02x \n", rdata >> 18, (rdata >> 12) & 0x3F);
		printk(KERN_INFO "PVT Ver:%d.%d \n", (rdata >> 5) & 0x1F, (rdata >> 0) & 0x1F);
	}
	rdata = sensor_io_read(pvt_dev->reg_base + 0x4);
	if (debug_pr_info == 1) {
		printk(KERN_INFO "PVT VM:%d  CH:%d \n", (rdata >> 16) & 0xFF, (rdata >> 24) & 0xFF);
		printk(KERN_INFO "PVT PD:%d  TS:%d \n", (rdata >> 8) & 0xFF, (rdata >> 0) & 0xFF);
	}
	rdata = sensor_io_read(pvt_dev->reg_base + 0x8);
	if (debug_pr_info == 1)
		printk(KERN_INFO "Device ID:0x%08x \n", rdata);

	rdata = sensor_io_read(pvt_dev->reg_base + 0xc);
	if (debug_pr_info == 1)
		printk(KERN_INFO "scratch reg check addr=0x%px infor=0x%08x \n", pvt_dev->reg_base + 0xc, rdata);

	sensor_io_write(pvt_dev->reg_base + 0xc, 0x5A5A5A5A);
	sensor_get_response(pvt_dev);
	rdata = sensor_io_read(pvt_dev->reg_base + 0xc);
	if (debug_pr_info == 1)
		printk(KERN_INFO "scratch reg check again addr=0x%px infor=0x%08x \n", (pvt_dev->reg_base + 0xc), rdata);
	return;
}

static void pvt_sensor_irq_parameter_update(struct clourney_pvt_dev *pvt_dev)
{
	int cnt = 0;

	for (cnt = 0; cnt < T_SENSOR_NUM; cnt++) {
		// Ts0~2
		// set alarma
		sensor_io_write(pvt_dev->reg_base + TS_MACRO_OFFSET + ARLARMA_CFG_REG + 0 * MACRO_OFFSET, pvt_dev->temp_rise_alarm1);//90-->110 C rising alarm (0x0c360c00)
		sensor_get_response(pvt_dev);
		sensor_io_write(pvt_dev->reg_base + TS_MACRO_OFFSET + ARLARMA_CFG_REG + 1 * MACRO_OFFSET, pvt_dev->temp_rise_alarm2);//90-->110 C rising alarm (0x0c360c00)
		sensor_get_response(pvt_dev);
		sensor_io_write(pvt_dev->reg_base + TS_MACRO_OFFSET + ARLARMA_CFG_REG + 2 * MACRO_OFFSET, pvt_dev->temp_rise_alarm3);//90-->110 C rising alarm (0x0c360c00)
		// set alarmb
		sensor_io_write(pvt_dev->reg_base + TS_MACRO_OFFSET + ARLARMB_CFG_REG + 0 * MACRO_OFFSET, pvt_dev->temp_fall_alarm1);//-20<---10C  falling alarm (0x0d000c36)
		sensor_get_response(pvt_dev);
		sensor_io_write(pvt_dev->reg_base + TS_MACRO_OFFSET + ARLARMB_CFG_REG + 1 * MACRO_OFFSET, pvt_dev->temp_fall_alarm2);//-20<---10C  falling alarm (0x0d000c36)
		sensor_get_response(pvt_dev);
		sensor_io_write(pvt_dev->reg_base + TS_MACRO_OFFSET + ARLARMB_CFG_REG + 2 * MACRO_OFFSET, pvt_dev->temp_fall_alarm3);//-20<---10C  falling alarm (0x0d000c36)
		// PD0~2
		// set alarma
		sensor_io_write(pvt_dev->reg_base + PD_MACRO_OFFSET + ARLARMA_CFG_REG + cnt * MACRO_OFFSET, pvt_dev->process_alarmA);
		sensor_get_response(pvt_dev);
		// set alarmb
		sensor_io_write(pvt_dev->reg_base + PD_MACRO_OFFSET + ARLARMB_CFG_REG + cnt * MACRO_OFFSET, pvt_dev->process_alarmB);
		sensor_get_response(pvt_dev);
	}

	// VM0
	// 7 alarm
	for (cnt = 0; cnt < V_SENSOR_POINT_NUM; cnt++) {
		if (cnt==0) {
			// set alarma
			sensor_io_write(pvt_dev->reg_base + VM_MACRO_OFFSET + VM_ARLARMA_OFFSET_REG + cnt * VM_CHANNEL_OFFSET, pvt_dev->volt_rise_alarm1);//1.2v 阈值
			sensor_get_response(pvt_dev);
			// set alarmb
			sensor_io_write(pvt_dev->reg_base + VM_MACRO_OFFSET + VM_ARLARMB_OFFSET_REG + cnt * VM_CHANNEL_OFFSET, pvt_dev->volt_fall_alarm1);
			sensor_get_response(pvt_dev);
		} else {
			 // set alarma
			sensor_io_write(pvt_dev->reg_base + VM_MACRO_OFFSET + VM_ARLARMA_OFFSET_REG + cnt * VM_CHANNEL_OFFSET, pvt_dev->volt_rise_alarm2);//0.76<---0.77v FAIILING Alarm
			sensor_get_response(pvt_dev);
			// set alarmb
			sensor_io_write(pvt_dev->reg_base + VM_MACRO_OFFSET + VM_ARLARMB_OFFSET_REG + cnt * VM_CHANNEL_OFFSET, pvt_dev->volt_fall_alarm2);
			sensor_get_response(pvt_dev);
		}
	}
}

static void sensor_pg01(struct clourney_pvt_dev *pvt_dev)
{
	int cnt = 0;
	sensor_io_write(pvt_dev->reg_base + IRQ_GLB_EN_REG, 0x0e); // global irq_enable
	sensor_get_response(pvt_dev);
	//Temperature
	for (cnt = 0; cnt < T_SENSOR_NUM; cnt++) {
		sensor_io_write(pvt_dev->reg_base + TS_MACRO_OFFSET + IRQ_ENABLE_REG + cnt * MACRO_OFFSET, 0x18);
		sensor_get_response(pvt_dev);
	}
	//Process Detector
	for (cnt = 0; cnt < P_SENSOR_NUM; cnt++) {
		sensor_io_write(pvt_dev->reg_base + PD_MACRO_OFFSET + IRQ_ENABLE_REG + cnt * MACRO_OFFSET, 0x18);
		sensor_get_response(pvt_dev);
	}

	//Voltage Monitor
	sensor_io_write(pvt_dev->reg_base + VM_MACRO_OFFSET + IRQ_ENABLE_REG, 0x0); // vm_irq_enable
	sensor_get_response(pvt_dev);
	sensor_io_write(pvt_dev->reg_base + VM_MACRO_OFFSET + VM_ARLARMA_ENABLE_REG, 0x7e); // vm_irq_alarma_enable
	sensor_get_response(pvt_dev);
	sensor_io_write(pvt_dev->reg_base + VM_MACRO_OFFSET + VM_ARLARMB_ENABLE_REG, 0x7e); // vm_irq_alarmb_enable
	sensor_get_response(pvt_dev);

	for (cnt = 0; cnt < T_SENSOR_NUM; cnt++) {
		// Ts0~2
		// set alarma
		sensor_io_write(pvt_dev->reg_base + TS_MACRO_OFFSET + ARLARMA_CFG_REG + 0 * MACRO_OFFSET, pvt_dev->temp_rise_alarm1);//90-->110 C rising alarm (0x0c360c00)
		sensor_get_response(pvt_dev);
		sensor_io_write(pvt_dev->reg_base + TS_MACRO_OFFSET + ARLARMA_CFG_REG + 1 * MACRO_OFFSET, pvt_dev->temp_rise_alarm2);//90-->110 C rising alarm (0x0c360c00)
		sensor_get_response(pvt_dev);
		sensor_io_write(pvt_dev->reg_base + TS_MACRO_OFFSET + ARLARMA_CFG_REG + 2 * MACRO_OFFSET, pvt_dev->temp_rise_alarm3);//90-->110 C rising alarm (0x0c360c00)
		// set alarmb
		sensor_io_write(pvt_dev->reg_base + TS_MACRO_OFFSET + ARLARMB_CFG_REG + 0 * MACRO_OFFSET, pvt_dev->temp_fall_alarm1);//-20<---10C  falling alarm (0x0d000c36)
		sensor_get_response(pvt_dev);
		sensor_io_write(pvt_dev->reg_base + TS_MACRO_OFFSET + ARLARMB_CFG_REG + 1 * MACRO_OFFSET, pvt_dev->temp_fall_alarm2);//-20<---10C  falling alarm (0x0d000c36)
		sensor_get_response(pvt_dev);
		sensor_io_write(pvt_dev->reg_base + TS_MACRO_OFFSET + ARLARMB_CFG_REG + 2 * MACRO_OFFSET, pvt_dev->temp_fall_alarm3);//-20<---10C  falling alarm (0x0d000c36)
		// PD0~2
		// set alarma
		sensor_io_write(pvt_dev->reg_base + PD_MACRO_OFFSET + ARLARMA_CFG_REG + cnt * MACRO_OFFSET, pvt_dev->process_alarmA);
		sensor_get_response(pvt_dev);
		// set alarmb
		sensor_io_write(pvt_dev->reg_base + PD_MACRO_OFFSET + ARLARMB_CFG_REG + cnt * MACRO_OFFSET, pvt_dev->process_alarmB);
		sensor_get_response(pvt_dev);
	}

	// VM0
	// 7 alarm
	for (cnt = 0; cnt < V_SENSOR_POINT_NUM; cnt++) {
		if (cnt==0) {
			// set alarma
			sensor_io_write(pvt_dev->reg_base + VM_MACRO_OFFSET + VM_ARLARMA_OFFSET_REG + cnt * VM_CHANNEL_OFFSET, pvt_dev->volt_rise_alarm1);//1.2v 阈值
			sensor_get_response(pvt_dev);
			// set alarmb
			sensor_io_write(pvt_dev->reg_base + VM_MACRO_OFFSET + VM_ARLARMB_OFFSET_REG + cnt * VM_CHANNEL_OFFSET, pvt_dev->volt_fall_alarm1);
			sensor_get_response(pvt_dev);
		} else {
			 // set alarma
			sensor_io_write(pvt_dev->reg_base + VM_MACRO_OFFSET + VM_ARLARMA_OFFSET_REG + cnt * VM_CHANNEL_OFFSET, pvt_dev->volt_rise_alarm2);//0.76<---0.77v FAIILING Alarm
			sensor_get_response(pvt_dev);
			// set alarmb
			sensor_io_write(pvt_dev->reg_base + VM_MACRO_OFFSET + VM_ARLARMB_OFFSET_REG + cnt * VM_CHANNEL_OFFSET, pvt_dev->volt_fall_alarm2);
			sensor_get_response(pvt_dev);
		}
	}

	// PVT Sample Control
	// ts group
	sensor_io_write(pvt_dev->reg_base + TS_COMMON_OFFSET + COMMON_SMPL_CTRL, 0x0);
	sensor_get_response(pvt_dev);

	// pd group
	sensor_io_write(pvt_dev->reg_base + PD_COMMON_OFFSET + COMMON_SMPL_CTRL, 0x0);
	sensor_get_response(pvt_dev);
	// vm group
	sensor_io_write(pvt_dev->reg_base + VM_COMMON_OFFSET + COMMON_SMPL_CTRL, 0x0);
	sensor_get_response(pvt_dev);
	// PVT Sample HiLo Control
	// ts0~2
	for (cnt = 0; cnt < T_SENSOR_NUM; cnt++) {
		sensor_io_write(pvt_dev->reg_base + TS_MACRO_OFFSET + HILO_RESET_REG + cnt * MACRO_OFFSET, 0x3);
		sensor_get_response(pvt_dev);
	}
	// pd0~2
	for (cnt = 0; cnt < P_SENSOR_NUM; cnt++) {
		sensor_io_write(pvt_dev->reg_base + PD_MACRO_OFFSET + HILO_RESET_REG + cnt * MACRO_OFFSET, 0x3);
		sensor_get_response(pvt_dev);
	}
	// vm0
	for (cnt = 1; cnt < V_SENSOR_POINT_NUM; cnt++) {
		sensor_io_write(pvt_dev->reg_base + VM_MACRO_OFFSET + VM_HILO_RESET_REG + cnt * VM_CHANNEL_OFFSET, 0x3);
		sensor_get_response(pvt_dev);
	}

	return;
}

static void sensor_pg02(struct clourney_pvt_dev *pvt_dev)
{
	if (debug_pr_info == 1)
		printk(KERN_INFO "pg02...\n");

	// ts0~2
	// set sdif halt ts common
	sensor_io_write(pvt_dev->reg_base + TS_COMMON_OFFSET + COMMON_SDIF_HALT, 0x0);
	sensor_get_response(pvt_dev);
	// set sdif halt pd common
	sensor_io_write(pvt_dev->reg_base + PD_COMMON_OFFSET + COMMON_SDIF_HALT, 0x0);
	sensor_get_response(pvt_dev);
	// set sdif halt vm common
	sensor_io_write(pvt_dev->reg_base + VM_COMMON_OFFSET + COMMON_SDIF_HALT, 0x0);
	sensor_get_response(pvt_dev);

	// ts0~2
	// set sdif disable
	sensor_io_write(pvt_dev->reg_base + TS_COMMON_OFFSET + COMMON_SDIF_DISABLE, 0x7);
	sensor_get_response(pvt_dev);
	// set sdif disable to 0
	sensor_io_write(pvt_dev->reg_base + TS_COMMON_OFFSET + COMMON_SDIF_DISABLE, 0x0);
	sensor_get_response(pvt_dev);

	// pd0~2
	// set sdif disable
	sensor_io_write(pvt_dev->reg_base + PD_COMMON_OFFSET + COMMON_SDIF_DISABLE, 0x7);
	sensor_get_response(pvt_dev);
	// set sdif disable to 0
	sensor_io_write(pvt_dev->reg_base + PD_COMMON_OFFSET + COMMON_SDIF_DISABLE, 0x0);
	sensor_get_response(pvt_dev);

	// vm0
	// set sdif disable
	sensor_io_write(pvt_dev->reg_base + VM_COMMON_OFFSET + COMMON_SDIF_DISABLE, 0x1);
	sensor_get_response(pvt_dev);
	// set sdif disable to 0
	sensor_io_write(pvt_dev->reg_base + VM_COMMON_OFFSET + COMMON_SDIF_DISABLE, 0x0);
	sensor_get_response(pvt_dev);

	// ts
	// set clk synth ref_clk=48MHz
	sensor_io_write(pvt_dev->reg_base + TS_COMMON_OFFSET + COMMON_CLK_SYNTH, ((1 << 24) | (1 << 16) | (3 << 8) | (3 << 0)));
	sensor_get_response(pvt_dev);

	// pd
	// set clk synth ref_clk=48MHz
	sensor_io_write(pvt_dev->reg_base + PD_COMMON_OFFSET + COMMON_CLK_SYNTH, ((1 << 24) | (1 << 16) | (3 << 8) | (3 << 0)));
	sensor_get_response(pvt_dev);
	// vm
	// set clk synth ref_clk=48MHz
	sensor_io_write(pvt_dev->reg_base + VM_COMMON_OFFSET + COMMON_CLK_SYNTH, ((1 << 24) | (1 << 16) | (3 << 8) | (3 << 0)));
	sensor_get_response(pvt_dev);

	return;
}

static void sensor_pg03(struct clourney_pvt_dev *pvt_dev, uint8_t ts_cfg, uint8_t an_en, uint8_t cfga, uint8_t no_ip_ctrl,
				uint8_t pd_cfg1, uint8_t pd_cfg2, uint8_t pd_cfg3, uint8_t vm_cfg1, uint8_t vm_channel)
{
	uint32_t rdata = 0, wdata = 0; int cnt = 0;
	if (debug_pr_info == 1)
		printk(KERN_INFO "pg03...\n");

	// PG03: Program the SDA registers
	tsens_check_sdif_status(pvt_dev);
	psens_check_sdif_status(pvt_dev);
	vsens_check_sdif_status(pvt_dev);

	// set sdif, ip_tmr
	sensor_io_write(pvt_dev->reg_base + TS_COMMON_OFFSET + COMMON_SDIF, ((1 << 31) | (1 << 27) | (5 << 24) | (0x100 << 0)));
	sensor_get_response(pvt_dev);
	sensor_io_write(pvt_dev->reg_base + PD_COMMON_OFFSET + COMMON_SDIF, ((1 << 31) | (1 << 27) | (5 << 24) | (0 << 0)));
	sensor_get_response(pvt_dev);
	sensor_io_write(pvt_dev->reg_base + VM_COMMON_OFFSET + COMMON_SDIF, ((1 << 31) | (1 << 27) | (5 << 24) | (0x40 << 0)));
	sensor_get_response(pvt_dev);

	rdata = sensor_io_read(pvt_dev->reg_base + TS_COMMON_OFFSET + COMMON_SDIF_STATUS);
	if (debug_pr_info == 1)
		printk(KERN_INFO "TS sdif status addr=0x%p infor=0x%08x\n",
			(pvt_dev->reg_base + TS_COMMON_OFFSET + COMMON_SDIF_STATUS), rdata);

	rdata = sensor_io_read(pvt_dev->reg_base + PD_COMMON_OFFSET + COMMON_SDIF_STATUS);
		if (debug_pr_info == 1)
			printk(KERN_INFO "PD sdif status addr=0x%p infor=0x%08x\n",
				(pvt_dev->reg_base + PD_COMMON_OFFSET + COMMON_SDIF_STATUS), rdata);

	rdata = sensor_io_read(pvt_dev->reg_base + VM_COMMON_OFFSET + COMMON_SDIF_STATUS);
	if (debug_pr_info == 1)
		printk(KERN_INFO "VM sdif status addr=0x%p infor=0x%08x\n",
			(pvt_dev->reg_base + VM_COMMON_OFFSET + COMMON_SDIF_STATUS), rdata);

	// set sdif, ip_tmr
	sensor_io_write(pvt_dev->reg_base + TS_COMMON_OFFSET + COMMON_SDIF, ((1 << 31) | (0 << 27) | (5 << 24) | (0x100 << 0)));
	sensor_get_response(pvt_dev);
	sensor_io_write(pvt_dev->reg_base + PD_COMMON_OFFSET + COMMON_SDIF, ((1 << 31) | (0 << 27) | (5 << 24) | (0 << 0)));
	sensor_get_response(pvt_dev);
	sensor_io_write(pvt_dev->reg_base + VM_COMMON_OFFSET + COMMON_SDIF, ((1 << 31) | (0 << 27) | (5 << 24) | (0x40 << 0)));
	sensor_get_response(pvt_dev);

	for (cnt = 0; cnt < T_SENSOR_NUM; cnt++) {
		rdata = sensor_io_read(pvt_dev->reg_base + TS_MACRO_OFFSET + SDIF_RDATA_REG + cnt * MACRO_OFFSET);
		if (debug_pr_info == 1)
			printk(KERN_INFO "TS%d SDIF RDATA addr=0x%px infor=0x%08x\n", cnt,
				(pvt_dev->reg_base + TS_MACRO_OFFSET + SDIF_RDATA_REG + cnt * MACRO_OFFSET), rdata);
	}

	for (cnt = 0; cnt < P_SENSOR_NUM; cnt++) {
		rdata = sensor_io_read(pvt_dev->reg_base + PD_MACRO_OFFSET + SDIF_RDATA_REG + cnt * MACRO_OFFSET);
		if (debug_pr_info == 1)
			printk(KERN_INFO "PD%d SDIF RDATA addr=0x%px infor=0x%08x\n", cnt,
				(pvt_dev->reg_base + PD_MACRO_OFFSET + SDIF_RDATA_REG + cnt * MACRO_OFFSET), rdata);
	}

	rdata = sensor_io_read(pvt_dev->reg_base + VM_MACRO_OFFSET + VM_SDIF_RDATA_REG);
	if (debug_pr_info == 1)
		printk(KERN_INFO "VM0 SDIF RDATA addr=0x%px infor=0x%08x\n",
			(pvt_dev->reg_base + VM_MACRO_OFFSET + VM_SDIF_RDATA_REG), rdata);

	// set sdif, ip_cfg
	wdata = (1 << 31 | 1 << 27 | 1 << 24 | 0xAA << 16 | 0xBB << 8 | ts_cfg);
	sensor_io_write(pvt_dev->reg_base + TS_COMMON_OFFSET + COMMON_SDIF, wdata);
	sensor_get_response(pvt_dev);

	wdata = (1 << 31 | 1 << 27 | 1 << 24 | pd_cfg3 << 16 | pd_cfg2 << 8 | pd_cfg1);
	sensor_io_write(pvt_dev->reg_base + PD_COMMON_OFFSET + COMMON_SDIF, wdata);
	sensor_get_response(pvt_dev);

	wdata = (1 << 31 | 1 << 27 | 1 << 24 | vm_channel << 16 | vm_cfg1 << 8 | 0x0 << 0);
	sensor_io_write(pvt_dev->reg_base + VM_COMMON_OFFSET + COMMON_SDIF, wdata);
	sensor_get_response(pvt_dev);

	rdata = sensor_io_read(pvt_dev->reg_base + TS_COMMON_OFFSET + COMMON_SDIF_STATUS);
	if (debug_pr_info == 1)
		printk(KERN_INFO "TS sdif status addr=0x%p infor=0x%08x\n",
			(pvt_dev->reg_base + TS_COMMON_OFFSET + COMMON_SDIF_STATUS), rdata);

	rdata = sensor_io_read(pvt_dev->reg_base + PD_COMMON_OFFSET + COMMON_SDIF_STATUS);
	if (debug_pr_info == 1)
		printk(KERN_INFO "PD sdif status addr=0x%p infor=0x%08x\n",
			(pvt_dev->reg_base + PD_COMMON_OFFSET + COMMON_SDIF_STATUS), rdata);

	rdata = sensor_io_read(pvt_dev->reg_base + VM_COMMON_OFFSET + COMMON_SDIF_STATUS);
	if (debug_pr_info == 1)
		printk(KERN_INFO "VM sdif status addr=0x%p infor=0x%08x\n",
			(pvt_dev->reg_base + VM_COMMON_OFFSET + COMMON_SDIF_STATUS), rdata);

	// read SDA register ip_cfg
	sensor_io_write(pvt_dev->reg_base + TS_COMMON_OFFSET + COMMON_SDIF, ((1 << 31) | (0 << 27) | (1 << 24) | (0 << 0)));
	sensor_get_response(pvt_dev);

	sensor_io_write(pvt_dev->reg_base + PD_COMMON_OFFSET + COMMON_SDIF, ((1 << 31) | (0 << 27) | (1 << 24) | (0 << 0)));
	sensor_get_response(pvt_dev);

	sensor_io_write(pvt_dev->reg_base + VM_COMMON_OFFSET + COMMON_SDIF, ((1 << 31) | (0 << 27) | (1 << 24) | (0 << 0)));
	sensor_get_response(pvt_dev);

	for (cnt = 0; cnt < T_SENSOR_NUM; cnt++) {
		rdata = sensor_io_read(pvt_dev->reg_base + TS_MACRO_OFFSET + SDIF_RDATA_REG + cnt * MACRO_OFFSET);
		if (debug_pr_info == 1)
			printk(KERN_INFO "TS%d SDIF RDATA addr=0x%p infor=0x%08x\n", cnt,
				(pvt_dev->reg_base + TS_MACRO_OFFSET + SDIF_RDATA_REG + cnt * MACRO_OFFSET), rdata);
	}

	for (cnt = 0; cnt < P_SENSOR_NUM; cnt++) {
		rdata = sensor_io_read(pvt_dev->reg_base + PD_MACRO_OFFSET + SDIF_RDATA_REG + cnt * MACRO_OFFSET);
		if (debug_pr_info == 1)
			printk(KERN_INFO "PD%d SDIF RDATA addr=0x%p infor=0x%08x\n", cnt,
				(pvt_dev->reg_base + PD_MACRO_OFFSET + SDIF_RDATA_REG + cnt * MACRO_OFFSET), rdata);
	}

	rdata = sensor_io_read(pvt_dev->reg_base + VM_MACRO_OFFSET + VM_SDIF_RDATA_REG);
	if (debug_pr_info == 1)
		printk(KERN_INFO "VM0 SDIF RDATA addr=0x%p infor=0x%08x\n",
				(pvt_dev->reg_base + VM_MACRO_OFFSET + VM_SDIF_RDATA_REG), rdata);

	sensor_get_response(pvt_dev);

	// set sdif, ip_cfga
	wdata = (1 << 31 | 1 << 27 | 2 << 24 | 0 << 16 | an_en << 8 | cfga);
	sensor_io_write(pvt_dev->reg_base + TS_COMMON_OFFSET + COMMON_SDIF, wdata);
	sensor_get_response(pvt_dev);

	wdata = (1 << 31 | 1 << 27 | 2 << 24 | 0 << 16 | an_en << 8 | cfga);
	sensor_io_write(pvt_dev->reg_base + PD_COMMON_OFFSET + COMMON_SDIF, wdata);
	sensor_get_response(pvt_dev);

	wdata = (1 << 31 | 1 << 27 | 2 << 24 | 0 << 16 | an_en << 8 | cfga);
	sensor_io_write(pvt_dev->reg_base + VM_COMMON_OFFSET + COMMON_SDIF, wdata);
	sensor_get_response(pvt_dev);

	rdata = sensor_io_read(pvt_dev->reg_base + TS_COMMON_OFFSET + COMMON_SDIF_STATUS);
	if (debug_pr_info == 1)
		printk(KERN_INFO "TS sdif status addr=0x%p infor=0x%08x\n",
			(pvt_dev->reg_base + TS_COMMON_OFFSET + COMMON_SDIF_STATUS), rdata);

	rdata = sensor_io_read(pvt_dev->reg_base + PD_COMMON_OFFSET + COMMON_SDIF_STATUS);
	if (debug_pr_info == 1)
		printk(KERN_INFO "PD sdif status addr=0x%p infor=0x%08x\n",
			(pvt_dev->reg_base + PD_COMMON_OFFSET + COMMON_SDIF_STATUS), rdata);

	rdata = sensor_io_read(pvt_dev->reg_base + VM_COMMON_OFFSET + COMMON_SDIF_STATUS);
	if (debug_pr_info == 1)
		printk(KERN_INFO "VM sdif status addr=0x%p infor=0x%08x\n",
			(pvt_dev->reg_base + VM_COMMON_OFFSET + COMMON_SDIF_STATUS), rdata);

	// read SDA register ip_cfg
	sensor_io_write(pvt_dev->reg_base + TS_COMMON_OFFSET + COMMON_SDIF, (1 << 31 | 0 << 27 | 2 << 24 | 0 << 16 | 0 << 8 | 0));
	sensor_get_response(pvt_dev);

	sensor_io_write(pvt_dev->reg_base + PD_COMMON_OFFSET + COMMON_SDIF, (1 << 31 | 0 << 27 | 2 << 24 | 0 << 16 | 0 << 8 | 0));
	sensor_get_response(pvt_dev);

	sensor_io_write(pvt_dev->reg_base + VM_COMMON_OFFSET + COMMON_SDIF, (1 << 31 | 0 << 27 | 2 << 24 | 0 << 16 | 0 << 8 | 0));
	sensor_get_response(pvt_dev);

	for (cnt = 0; cnt < T_SENSOR_NUM; cnt++) {
		rdata = sensor_io_read(pvt_dev->reg_base + TS_MACRO_OFFSET + SDIF_RDATA_REG + cnt * MACRO_OFFSET);
		if (debug_pr_info == 1)
			printk(KERN_INFO "TS%d SDIF RDATA addr=0x%p infor=0x%08x\n", cnt,
				(pvt_dev->reg_base + TS_MACRO_OFFSET + SDIF_RDATA_REG + cnt * MACRO_OFFSET), rdata);
	}

	for (cnt = 0; cnt < P_SENSOR_NUM; cnt++) {
		rdata = sensor_io_read(pvt_dev->reg_base + PD_MACRO_OFFSET + SDIF_RDATA_REG + cnt * MACRO_OFFSET);
		if (debug_pr_info == 1)
			printk(KERN_INFO "PD%d SDIF RDATA addr=0x%p infor=0x%08x\n", cnt,
				(pvt_dev->reg_base + PD_MACRO_OFFSET + SDIF_RDATA_REG + cnt * MACRO_OFFSET), rdata);
	}

	rdata = sensor_io_read(pvt_dev->reg_base + VM_MACRO_OFFSET + VM_SDIF_RDATA_REG);
	if (debug_pr_info == 1)
		printk(KERN_INFO "VM0 SDIF RDATA addr=0x%p infor=0x%08x\n",
			(pvt_dev->reg_base + VM_MACRO_OFFSET + VM_SDIF_RDATA_REG), rdata);

	if (no_ip_ctrl == 0) {
		sensor_io_write(pvt_dev->reg_base + TS_COMMON_OFFSET + COMMON_SDIF, (1 << 31 | 1 << 27 | 0 << 24 | 0 << 16 | 0 << 8 | 0));
		sensor_get_response(pvt_dev);

		sensor_io_write(pvt_dev->reg_base + PD_COMMON_OFFSET + COMMON_SDIF, (1 << 31 | 1 << 27 | 0 << 24 | 0 << 16 | 0 << 8 | 0));
		sensor_get_response(pvt_dev);

		sensor_io_write(pvt_dev->reg_base + VM_COMMON_OFFSET + COMMON_SDIF, (1 << 31 | 1 << 27 | 0 << 24 | 0 << 16 | 0 << 8 | 0));
		sensor_get_response(pvt_dev);

		rdata = sensor_io_read(pvt_dev->reg_base + TS_COMMON_OFFSET + COMMON_SDIF_STATUS);
		if (debug_pr_info == 1)
			printk(KERN_INFO "TS sdif status addr=0x%p infor=0x%08x\n",
				(pvt_dev->reg_base + TS_COMMON_OFFSET + COMMON_SDIF_STATUS), rdata);

		rdata = sensor_io_read(pvt_dev->reg_base + PD_COMMON_OFFSET + COMMON_SDIF_STATUS);
		if (debug_pr_info == 1)
			printk(KERN_INFO "PD sdif status addr=0x%p infor=0x%08x\n",
				(pvt_dev->reg_base + PD_COMMON_OFFSET + COMMON_SDIF_STATUS), rdata);

		rdata = sensor_io_read(pvt_dev->reg_base + VM_COMMON_OFFSET + COMMON_SDIF_STATUS);
		if (debug_pr_info == 1)
			printk(KERN_INFO "VM sdif status addr=0x%p infor=0x%08x\n",
				(pvt_dev->reg_base + VM_COMMON_OFFSET + COMMON_SDIF_STATUS), rdata);

		sensor_io_write(pvt_dev->reg_base + TS_COMMON_OFFSET + COMMON_SDIF, (1 << 31 | 0 << 27 | 0 << 24 | 0 << 16 | 0 << 8 | 0));
		sensor_get_response(pvt_dev);

		sensor_io_write(pvt_dev->reg_base + PD_COMMON_OFFSET + COMMON_SDIF, (1 << 31 | 0 << 27 | 0 << 24 | 0 << 16 | 0 << 8 | 0));
		sensor_get_response(pvt_dev);

		sensor_io_write(pvt_dev->reg_base + VM_COMMON_OFFSET + COMMON_SDIF, (1 << 31 | 0 << 27 | 0 << 24 | 0 << 16 | 0 << 8 | 0));
		sensor_get_response(pvt_dev);

		for (cnt = 0; cnt < T_SENSOR_NUM; cnt++) {
			rdata = sensor_io_read(pvt_dev->reg_base + TS_MACRO_OFFSET + SDIF_RDATA_REG + cnt * MACRO_OFFSET);
			if (debug_pr_info == 1)
				printk(KERN_INFO "TS%d SDIF RDATA addr=0x%p infor=0x%08x\n", cnt,
					(pvt_dev->reg_base + TS_MACRO_OFFSET + SDIF_RDATA_REG + cnt * MACRO_OFFSET),
						rdata);
		}

		for (cnt = 0; cnt < P_SENSOR_NUM; cnt++) {
			rdata = sensor_io_read(pvt_dev->reg_base + PD_MACRO_OFFSET + SDIF_RDATA_REG + cnt * MACRO_OFFSET);
			if (debug_pr_info == 1)
				printk(KERN_INFO "PD%d SDIF RDATA addr=0x%p infor=0x%08x\n", cnt,
					(pvt_dev->reg_base + PD_MACRO_OFFSET + SDIF_RDATA_REG + cnt * MACRO_OFFSET),
						rdata);
		}

		rdata = sensor_io_read(pvt_dev->reg_base + VM_MACRO_OFFSET + VM_SDIF_RDATA_REG);
		if (debug_pr_info == 1)
			printk(KERN_INFO "VM0 SDIF RDATA addr=0x%p infor=0x%08x\n",
				(pvt_dev->reg_base + VM_MACRO_OFFSET + VM_SDIF_RDATA_REG), rdata);
	}
}

static void sensor_irq_clean(struct clourney_pvt_dev *pvt_dev)
{
	uint32_t rdata = 0, wdata = 0, cnt = 0;
	uint32_t ts_rdata[T_SENSOR_NUM], vm_radta[V_SENSOR_POINT_NUM];

	rdata = sensor_io_read(pvt_dev->reg_base + IRQ_TS_STATUS_REG);
	if (rdata) {
		printk(KERN_INFO "TS irq status, infor=0x%08x \n", rdata);
	}

	rdata = sensor_io_read(pvt_dev->reg_base + IRQ_VM_STATUS_REG);
	if (rdata) {
		printk(KERN_INFO "VM irq status, infor=0x%08x \n", rdata);
	}

	rdata = sensor_io_read(pvt_dev->reg_base + IRQ_PD_STATUS_REG);
	if (rdata) {
		printk(KERN_INFO "PD irq status, infor=0x%08x \n", rdata);
	}

	for (cnt = 0; cnt < T_SENSOR_NUM; cnt++) {
		rdata = sensor_io_read(pvt_dev->reg_base + TS_MACRO_OFFSET + IRQ_STATUS_REG + cnt * MACRO_OFFSET);
		if (rdata) {
			/* mask T0/T1/T2 sensor irq */
			sensor_io_write((pvt_dev->reg_base + IRQ_TS_MASK_REG), cnt << 1);
			sensor_get_response(pvt_dev);

			ts_rdata[cnt] = sensor_io_read(pvt_dev->reg_base + TS_MACRO_OFFSET + SDIF_DATA_REG + cnt * MACRO_OFFSET);
			ts_rdata[cnt] = create_low_12_bits_variable(ts_rdata[cnt]);
			printk(KERN_INFO "TS%d Temperature[%d.%03d C] modecal%d ts_rdata[0x%08x], irq infor=0x%08x \n", cnt, tempcal(ts_rdata[cnt], 1) / 1000, abs(tempcal(ts_rdata[cnt], 1) % 1000), 1, ts_rdata[cnt], rdata);
		}
		wdata = 1 << 4 | 1 << 3 | 1 << 1 | 1 << 0;
		sensor_io_write((pvt_dev->reg_base + TS_MACRO_OFFSET + IRQ_CLEAR_REG + cnt * MACRO_OFFSET), wdata);
		sensor_get_response(pvt_dev);
	}

	for (cnt = 0; cnt < P_SENSOR_NUM; cnt++) {
		rdata = sensor_io_read(pvt_dev->reg_base + PD_MACRO_OFFSET + IRQ_STATUS_REG + cnt * MACRO_OFFSET);
		if (rdata) {
			/* mask P0/P1/P2 sensor irq */
			sensor_io_write((pvt_dev->reg_base + IRQ_PD_MASK_REG), cnt << 1);
			sensor_get_response(pvt_dev);
			printk(KERN_INFO "PD%d irq status RDATA, irq infor=0x%08x \n", cnt, rdata);
		}
		wdata = 1 << 4 | 1 << 3 | 1 << 1 | 1 << 0;
		sensor_io_write((pvt_dev->reg_base + PD_MACRO_OFFSET + IRQ_CLEAR_REG + cnt * MACRO_OFFSET), wdata);
		sensor_get_response(pvt_dev);
	}

	rdata = sensor_io_read(pvt_dev->reg_base + VM_MACRO_OFFSET + VM_IRQ_STATUS);
	if (rdata) {
		/* mask v sensor irq */
		sensor_io_write((pvt_dev->reg_base +  IRQ_VM_MASK_REG), 0x01);
		sensor_get_response(pvt_dev);

		for (cnt = 0; cnt < V_SENSOR_POINT_NUM; cnt++) {
			sensor_pg03(pvt_dev, 0, 0, 0, 1, 0, (1 << 5), 0, 0, cnt);
			vm_radta[cnt] = sensor_io_read(pvt_dev->reg_base + VM_MACRO_OFFSET + VM_SDIF_DATA_REG);
			vm_radta[cnt] = create_low_14_bits_variable(vm_radta[cnt]);
			printk(KERN_INFO "VM0 CH%d, Monitor Voltage : [%d.%03d V], VM_Code[0x%08x], infor =0x%08x \n", cnt, volcal(vm_radta[cnt] & 0xffff, 14) / 1000, abs(volcal(vm_radta[cnt] & 0xffff, 14) % 1000), vm_radta[cnt], rdata);
		}
	}

	rdata = sensor_io_read(pvt_dev->reg_base + VM_MACRO_OFFSET + VM_IRQ_ALARMA_STATUS);
	if (rdata) {
		/* mask v sensor irq */
		sensor_io_write((pvt_dev->reg_base +  IRQ_VM_MASK_REG), 0x01);
		sensor_get_response(pvt_dev);
		printk("VM irq alarma status RDATA addr=0x%px, infor =0x%08x \n", (pvt_dev->reg_base + VM_MACRO_OFFSET + VM_IRQ_ALARMA_STATUS), rdata);
	}

	rdata = sensor_io_read(pvt_dev->reg_base + VM_MACRO_OFFSET + VM_IRQ_ALARMB_STATUS);
	if (rdata) {
		/* mask v sensor irq */
		sensor_io_write((pvt_dev->reg_base +  IRQ_VM_MASK_REG), 0x01);
		sensor_get_response(pvt_dev);
		printk("VM irq alarmb status RDATA addr=0x%px, infor =0x%08x \n", (pvt_dev->reg_base + VM_MACRO_OFFSET + VM_IRQ_ALARMB_STATUS), rdata);
	}

	//VM_IRQ_ALARMA_CLR
	wdata = 0x1FF;//9 VM point
	sensor_io_write((pvt_dev->reg_base + VM_MACRO_OFFSET + VM_IRQ_ALARMA_CLEAR), wdata);
	sensor_get_response(pvt_dev);

	//VM_IRQ_ALARMB_CLR
	wdata = 0x1FF;//9 VM point
	sensor_io_write((pvt_dev->reg_base + VM_MACRO_OFFSET + VM_IRQ_ALARMB_CLEAR), wdata);
	sensor_get_response(pvt_dev);

	//VM IRQ_CLR
	wdata = 1 << 1 | 1 << 0;
	sensor_io_write((pvt_dev->reg_base + VM_MACRO_OFFSET + VM_IRQ_CLEAR), wdata);
	sensor_get_response(pvt_dev);

	return;
}

static void __used sensor_pg05(struct clourney_pvt_dev *pvt_dev)
{
	if (debug_pr_info == 1)
		printk(KERN_INFO "pg05...\n");

	tsens_check_sdif_status(pvt_dev);
	psens_check_sdif_status(pvt_dev);
	vsens_check_sdif_status(pvt_dev);
}

static void __used sensor_pg06(struct clourney_pvt_dev *pvt_dev, uint8_t ts_cfg, uint8_t an_en, uint8_t cfga, int temperature, uint32_t vm_channel)
{
	int cnt;
	uint32_t golden_nbs = 0;
	uint32_t rdata = 0, wdata = 0, frequency = 5, ts_rdata[T_SENSOR_NUM], pd_rdata[P_SENSOR_NUM], vm_radta[V_SENSOR_POINT_NUM];
	uint32_t pd_cfg1 = (0 << 4) | (0 << 2) | (0 << 0), pd_cfg2 = (1 << 5), pd_cfg3 = (0 << 4) | (0 << 0);
	uint32_t vm_cfg1 = 0x00; //vm_channel:channel 0-0. 1-1. ,vm_channel = 0/1/2/3/4/5/6/
	if (debug_pr_info == 1)
		printk(KERN_INFO "pg06...\n");

	sensor_pg03(pvt_dev, ts_cfg, an_en, cfga, 1, pd_cfg1, pd_cfg2, pd_cfg3, vm_cfg1, vm_channel);

	tsens_check_sdif_status(pvt_dev);
	psens_check_sdif_status(pvt_dev);
	vsens_check_sdif_status(pvt_dev);

	wdata = 1 << 31 | 1 << 27 | 0 << 24 | 0x0104 << 0;
	sensor_io_write(pvt_dev->reg_base + TS_COMMON_OFFSET + COMMON_SDIF, wdata);
	sensor_get_response(pvt_dev);

	wdata = 1 << 31 | 1 << 27 | 0 << 24 | 0x0104 << 0;
	sensor_io_write(pvt_dev->reg_base + PD_COMMON_OFFSET + COMMON_SDIF, wdata);
	sensor_get_response(pvt_dev);

	wdata = 1 << 31 | 1 << 27 | 0 << 24 | 0x0104 << 0;
	sensor_io_write(pvt_dev->reg_base + VM_COMMON_OFFSET + COMMON_SDIF, wdata);
	sensor_get_response(pvt_dev);

	// read smpl cnt
	// PG06_1_* Wait for a sample done IRQ
	for (cnt = 0; cnt < T_SENSOR_NUM; cnt++) {
		rdata = sensor_io_read(pvt_dev->reg_base + TS_MACRO_OFFSET + SDIF_DONE_REG + cnt * MACRO_OFFSET);
		if (debug_pr_info == 1)
			printk(KERN_INFO "TS%d SDIF done addr=0x%px infor=0x%08x \n", cnt, (pvt_dev->reg_base + TS_MACRO_OFFSET + SDIF_DONE_REG + cnt * MACRO_OFFSET), rdata);
	}

	for (cnt = 0; cnt < P_SENSOR_NUM; cnt++) {
		rdata = sensor_io_read(pvt_dev->reg_base + PD_MACRO_OFFSET + SDIF_DONE_REG + cnt * MACRO_OFFSET);
		if (debug_pr_info == 1)
			printk(KERN_INFO "PD%d SDIF done addr=0x%px infor=0x%08x \n", cnt, (pvt_dev->reg_base + PD_MACRO_OFFSET + SDIF_DONE_REG + cnt * MACRO_OFFSET), rdata);
	}

	msleep(50);

	rdata = sensor_io_read(pvt_dev->reg_base + VM_MACRO_OFFSET + VM_SDIF_DONE_REG);
	if (debug_pr_info == 1)
		printk(KERN_INFO "VM0 SDIF done addr=0x%px infor=0x%08x \n", (pvt_dev->reg_base + VM_MACRO_OFFSET + VM_SDIF_DONE_REG), rdata);

	// read smpl cnt
	rdata = sensor_io_read(pvt_dev->reg_base + TS_COMMON_OFFSET + COMMON_SMPL_CNT);
	sensor_get_response(pvt_dev);
	if (debug_pr_info == 2)
		printk(KERN_INFO "TS rdaddr[%px]:smpl cnt=0x%08x \n", (pvt_dev->reg_base + TS_COMMON_OFFSET + COMMON_SMPL_CNT), rdata);

	rdata = sensor_io_read(pvt_dev->reg_base + PD_COMMON_OFFSET + COMMON_SMPL_CNT);
	sensor_get_response(pvt_dev);
	if (debug_pr_info == 2)
		printk(KERN_INFO "PD rdaddr[%px]:smpl cnt=0x%08x \n", (pvt_dev->reg_base + PD_COMMON_OFFSET + COMMON_SMPL_CNT), rdata);

	rdata = sensor_io_read(pvt_dev->reg_base + VM_COMMON_OFFSET + COMMON_SMPL_CNT);
	sensor_get_response(pvt_dev);
	if (debug_pr_info == 2)
		printk(KERN_INFO "VM rdaddr[%px]:smpl cnt=0x%08x \n", (pvt_dev->reg_base + VM_COMMON_OFFSET + COMMON_SMPL_CNT),rdata);

	// read TS SDIF DATA
	for (cnt = 0; cnt < T_SENSOR_NUM; cnt++) {
		ts_rdata[cnt] = sensor_io_read(pvt_dev->reg_base + TS_MACRO_OFFSET + SDIF_DATA_REG + cnt * MACRO_OFFSET);
		ts_rdata[cnt] = create_low_12_bits_variable(ts_rdata[cnt]);

		if (vm_channel == 0) {
			if((ts_cfg & 0xF) == 1)
				printk(KERN_INFO "TS%d Temperature[%d.%03d C] modecal%d ts_rdata[0x%08x]\n", cnt, tempcal(ts_rdata[cnt], 2) / 1000, abs(tempcal(ts_rdata[cnt], 2) % 1000), 2, ts_rdata[cnt]);
			else
				printk(KERN_INFO "TS%d Temperature[%d.%03d C] modecal%d ts_rdata[0x%08x]\n", cnt, tempcal(ts_rdata[cnt], 1) / 1000, abs(tempcal(ts_rdata[cnt], 1) % 1000), 1, ts_rdata[cnt]);
		}
		tmp_rdata[cnt] = tempcal(ts_rdata[cnt], 1);
		if (debug_pr_info == 1)
			printk(KERN_INFO "Gold temperature = %d C \n", temperature);

		if (((ts_cfg & 0xF) == 0) && (an_en == 0))
			golden_nbs = nbs_calc(temperature, 0);
		else if (((ts_cfg & 0xF) == 0) && (an_en == 1) &&((cfga & 0xF) == 0))
			golden_nbs = nbs_calc(temperature, 0);
		else if (((ts_cfg & 0xF) == 1) && (an_en == 0))
			golden_nbs = nbs_calc(temperature, 1);
		else if (((ts_cfg & 0xF) == 1) && (an_en == 1) && ((cfga & 0xF) == 0))
			golden_nbs = nbs_calc(temperature, 1);
		else if (((ts_cfg & 0xF) == 0) && (an_en == 1) && ((cfga & 0xF) == 3))
			golden_nbs = nbs_calc(temperature, 2);

		golden_nbs = golden_nbs | an_en << 16;

		if (debug_pr_info == 1)
			printk(KERN_INFO "golden_nbs = 0x%08x \n", golden_nbs);

		if (golden_nbs != ts_rdata[cnt]) {
			if (debug_pr_info == 1)
				printk(KERN_INFO "Golden nbs != Dout \n");
		}
	}

	if (vm_channel == 0) {
		for (cnt = 0; cnt < P_SENSOR_NUM; cnt++) {
			pd_rdata[cnt] = sensor_io_read(pvt_dev->reg_base + PD_MACRO_OFFSET + SDIF_DATA_REG + cnt * MACRO_OFFSET);
			pd_rdata[cnt] = create_low_14_bits_variable(pd_rdata[cnt]);
			nbs_calc1(frequency, pd_cfg1, pd_cfg2, pd_cfg3, pd_rdata[cnt], 0);
		}
	}

	cnt = 0;
	vm_radta[cnt] = sensor_io_read(pvt_dev->reg_base + VM_MACRO_OFFSET + VM_SDIF_DATA_REG + cnt * 0x4);
	vm_radta[cnt] = create_low_14_bits_variable(vm_radta[cnt]);
	if (vm_channel == 0) {
		printk(KERN_INFO "VM0 VDD(IOVolt) : [%d.%03d V], VM_Code[0x%08x]\n", volcal(vm_radta[cnt] & 0xffff, 14) / 1000, abs(volcal(vm_radta[cnt] & 0xffff, 14) % 1000), vm_radta[cnt]);
	} else if (vm_channel == 1) {
		printk(KERN_INFO "VM0 VDDQ(CoreVolt) : [%d.%03d V], VM_Code[0x%08x]\n", volcal(vm_radta[cnt] & 0xffff, 14) / 1000, abs(volcal(vm_radta[cnt] & 0xffff, 14) % 1000), vm_radta[cnt]);
	}
}

static void pvt_print(struct clourney_pvt_dev *pvt_dev)
{
	int cnt;
	uint32_t Pn, rdata = 0, frequency = 5, ts_rdata[T_SENSOR_NUM], pd_rdata[P_SENSOR_NUM], vm_radta[V_SENSOR_POINT_NUM];
	uint32_t pd_loop_freq = 0, pd_cfg1 = (0 << 4) | (0 << 2) | (0 << 0), pd_cfg2 = (1 << 5), pd_cfg3 = (0 << 4) | (0 << 0);

	// SDIF_SMPL_DONE
	// PG06_1_* Wait for a sample done IRQ
	for (cnt = 0; cnt < T_SENSOR_NUM; cnt++) {
		rdata = sensor_io_read(pvt_dev->reg_base + TS_MACRO_OFFSET + SDIF_DONE_REG + cnt * MACRO_OFFSET);
		if (debug_pr_info == 1)
			printk(KERN_INFO "TS%d SDIF done addr=0x%px infor =0x%08x \n", cnt, (pvt_dev->reg_base + TS_MACRO_OFFSET + SDIF_DONE_REG + cnt * MACRO_OFFSET), rdata);
	}

	for (cnt = 0; cnt < P_SENSOR_NUM; cnt++) {
		rdata = sensor_io_read(pvt_dev->reg_base + PD_MACRO_OFFSET + SDIF_DONE_REG + cnt * MACRO_OFFSET);
		if (debug_pr_info == 1)
			printk(KERN_INFO "PD%d SDIF done addr=0x%px infor =0x%08x \n", cnt, (pvt_dev->reg_base + PD_MACRO_OFFSET + SDIF_DONE_REG + cnt * MACRO_OFFSET), rdata);
	}

    msleep(50);

	rdata = sensor_io_read(pvt_dev->reg_base + VM_MACRO_OFFSET + VM_SDIF_DONE_REG);
	if (debug_pr_info == 1)
		printk(KERN_INFO "VM0 SDIF done addr=0x%px infor =0x%08x \n", (pvt_dev->reg_base + VM_MACRO_OFFSET + VM_SDIF_DONE_REG), rdata);

	// SMPL_COUNT check  read smpl cnt
	rdata = sensor_io_read(pvt_dev->reg_base + TS_COMMON_OFFSET + COMMON_SMPL_CNT);
	sensor_get_response(pvt_dev);
	if (debug_pr_info == 1)
		printk(KERN_INFO "TS rdaddr[0x%px]:smpl cnt=0x%08x \n", (pvt_dev->reg_base + TS_COMMON_OFFSET + COMMON_SMPL_CNT), rdata);

	rdata = sensor_io_read(pvt_dev->reg_base + PD_COMMON_OFFSET + COMMON_SMPL_CNT);
	sensor_get_response(pvt_dev);
	if (debug_pr_info == 1)
		printk(KERN_INFO "PD rdaddr[0x%px]:smpl cnt=0x%08x \n", (pvt_dev->reg_base + PD_COMMON_OFFSET + COMMON_SMPL_CNT), rdata);

	rdata = sensor_io_read(pvt_dev->reg_base + VM_COMMON_OFFSET + COMMON_SMPL_CNT);
	sensor_get_response(pvt_dev);
	if (debug_pr_info == 1)
		printk(KERN_INFO "VM rdaddr[0x%px]:smpl cnt=0x%08x \n", (pvt_dev->reg_base + VM_COMMON_OFFSET + COMMON_SMPL_CNT),rdata);

	//RD IP sample data type and fault values: read TS SDIF DATA
	for (cnt = 0; cnt < T_SENSOR_NUM; cnt++) {
		ts_rdata[cnt] = sensor_io_read(pvt_dev->reg_base + TS_MACRO_OFFSET + SDIF_DATA_REG + cnt * MACRO_OFFSET);
		ts_rdata[cnt] = create_low_12_bits_variable(ts_rdata[cnt]);
		//printk(KERN_INFO "rdaddr[0x%px]:ts_rdata=0x%08x \n", (pvt_dev->reg_base + TS_MACRO_OFFSET + SDIF_DATA_REG + cnt * MACRO_OFFSET), ts_rdata[cnt]);
		printk(KERN_INFO "TS%d Temperature[%d.%03d C] modecal%d ts_rdata[0x%08x]\n", cnt, tempcal(ts_rdata[cnt], 1) / 1000, abs(tempcal(ts_rdata[cnt], 1) % 1000), 1, ts_rdata[cnt]);
		tmp_rdata[cnt] = tempcal(ts_rdata[cnt], 1);
	}

	for (Pn = 1; Pn <= P_SENSOR_POINT_NUM; Pn++) {
		pd_cfg2 = pd_cfg2 &(~(0xf<<5));
		pd_cfg2 = (Pn << 5);
		sensor_pg03(pvt_dev, 0, 0, 0, 1, 0, pd_cfg2, 0, 0, 0);
		msleep(50);
		// read PD SDIF DATA
		for (cnt = 0; cnt < P_SENSOR_NUM; cnt++) {
			pd_rdata[cnt] = sensor_io_read(pvt_dev->reg_base + PD_MACRO_OFFSET + SDIF_DATA_REG + cnt * MACRO_OFFSET);
			pd_rdata[cnt] = create_low_14_bits_variable(pd_rdata[cnt]);
			//printk(KERN_INFO "rdaddr[0x%px]:pd_rdata=0x%08x \n", (pvt_dev->reg_base + PD_MACRO_OFFSET + SDIF_DATA_REG + cnt * MACRO_OFFSET), pd_rdata[cnt]);
			pd_loop_freq = nbs_calc1(frequency, pd_cfg1, pd_cfg2, pd_cfg3, pd_rdata[cnt], 0);
			if (((pd_cfg2 >> 5) & 0x7) == 1)
				printk(KERN_INFO "osc chain[SVT 16nm thin oxide]: pd_rdata=0x%08x, pd_loop_freq = %d MHz\n", pd_rdata[cnt], pd_loop_freq);
			else if (((pd_cfg2 >> 5) & 0x7) == 2)
				printk(KERN_INFO "osc chain[LVT 16nm thin oxide]: pd_rdata=0x%08x, pd_loop_freq = %d MHz\n", pd_rdata[cnt], pd_loop_freq);
			else if (((pd_cfg2 >> 5) & 0x7) == 3)
				printk(KERN_INFO "osc chain[ULVT 16nm thin oxide]: pd_rdata=0x%08x, pd_loop_freq = %d MHz\n", pd_rdata[cnt], pd_loop_freq);
			else if (((pd_cfg2 >> 5) & 0x7) == 4)
				printk(KERN_INFO "osc chain[SVT 20nm thin oxide]: pd_rdata=0x%08x, pd_loop_freq = %d MHz\n", pd_rdata[cnt], pd_loop_freq);
			else if (((pd_cfg2 >> 5) & 0x7) == 5)
				printk(KERN_INFO "osc chain[LVT 20nm thin oxide]: pd_rdata=0x%08x, pd_loop_freq = %d MHz\n", pd_rdata[cnt], pd_loop_freq);
			else if (((pd_cfg2 >> 5) & 0x7) == 6)
				printk(KERN_INFO "osc chain[ULVT 20nm thin oxide]: pd_rdata=0x%08x, pd_loop_freq = %d MHz\n", pd_rdata[cnt], pd_loop_freq);
			else
				printk(KERN_INFO "pd do not support pd_cfg2[7:5] as 0 or 0x7 \n");
		}
	}

	// read VM SDIF DATA
	for (cnt = 0; cnt < V_SENSOR_POINT_NUM; cnt++) {
		sensor_pg03(pvt_dev, 0, 0, 0, 1, 0, (1 << 5), 0, 0, cnt);
		msleep(50);
		vm_radta[cnt] = sensor_io_read(pvt_dev->reg_base + VM_MACRO_OFFSET + VM_SDIF_DATA_REG);
		vm_radta[cnt] = create_low_14_bits_variable(vm_radta[cnt]);
		printk(KERN_INFO "VM0 CH%d, Monitor Voltage : [%d.%03d V], VM_Code[0x%08x]\n", cnt, volcal(vm_radta[cnt] & 0xffff, 14) / 1000, abs(volcal(vm_radta[cnt] & 0xffff, 14) % 1000), vm_radta[cnt]);
	}

	msleep(50);

	return;
}

static void pvt_init(struct clourney_pvt_dev *pvt_dev, uint8_t ts_cfg, uint8_t an_en, uint8_t cfga, uint8_t vm_cfg2 ,int temperature)
{
	uint32_t rdata = 0, wdata = 0;
	int cnt;
	uint32_t pd_cfg1 = (0 << 4) | (0 << 2) | (0 << 0), pd_cfg2 = (0 << 5), pd_cfg3 = (0 << 4) | (0 << 0);
	uint32_t vm_cfg1 = 0x00;// vm_cfg2 = 0x1;

	sensor_pg00(pvt_dev);
	sensor_pg01(pvt_dev);
	sensor_pg02(pvt_dev);
	sensor_pg03(pvt_dev, ts_cfg, an_en, cfga, 1, pd_cfg1, pd_cfg2, pd_cfg3, vm_cfg1, vm_cfg2);

	tsens_check_sdif_status(pvt_dev);
	psens_check_sdif_status(pvt_dev);
	vsens_check_sdif_status(pvt_dev);

	wdata = 1 << 31 | 1 << 27 | 0 << 24 | 0x0108 << 0;
	sensor_io_write(pvt_dev->reg_base + TS_COMMON_OFFSET + COMMON_SDIF, wdata);
	sensor_get_response(pvt_dev);

	wdata = 1 << 31 | 1 << 27 | 0 << 24 | 0x0108 << 0;
	sensor_io_write(pvt_dev->reg_base + PD_COMMON_OFFSET + COMMON_SDIF, wdata);
	sensor_get_response(pvt_dev);

	wdata = 1 << 31 | 1 << 27 | 0 << 24 | 0x0108 << 0;
	sensor_io_write(pvt_dev->reg_base + VM_COMMON_OFFSET + COMMON_SDIF, wdata);
	sensor_get_response(pvt_dev);

	// SDIF_SMPL_DONE
	// PG06_1_* Wait for a sample done IRQ
	for (cnt = 0; cnt < T_SENSOR_NUM; cnt++) {
		rdata = sensor_io_read(pvt_dev->reg_base + TS_MACRO_OFFSET + SDIF_DONE_REG + cnt * MACRO_OFFSET);
	}

	for (cnt = 0; cnt < P_SENSOR_NUM; cnt++) {
		rdata = sensor_io_read(pvt_dev->reg_base + PD_MACRO_OFFSET + SDIF_DONE_REG + cnt * MACRO_OFFSET);
	}

	msleep(50);

	rdata = sensor_io_read(pvt_dev->reg_base + VM_MACRO_OFFSET + VM_SDIF_DONE_REG);

	// SMPL_COUNT check  read smpl cnt
	rdata = sensor_io_read(pvt_dev->reg_base + TS_COMMON_OFFSET + COMMON_SMPL_CNT);
	sensor_get_response(pvt_dev);

	rdata = sensor_io_read(pvt_dev->reg_base + PD_COMMON_OFFSET + COMMON_SMPL_CNT);
	sensor_get_response(pvt_dev);

	rdata = sensor_io_read(pvt_dev->reg_base + VM_COMMON_OFFSET + COMMON_SMPL_CNT);
	sensor_get_response(pvt_dev);

	msleep(50);

	return;
}

static int pvt_sensor_init(struct clourney_pvt_dev *pvt_dev)
{
	pvt_print(pvt_dev);
	return 0;
}

static int pvt_sensor_deinit(struct clourney_pvt_dev *pvt_dev)
{
	return 0;
}

static int read_process_value(struct clourney_pvt_dev *pvt_dev)
{
	return 0;
}

static int read_voltage_value(struct clourney_pvt_dev *pvt_dev)
{
	return 0;
}

static int read_temperature_value(struct clourney_pvt_dev *pvt_dev)
{
	return 0;
}

struct clourney_pvt_ops pvt_ops = {
	.init = pvt_sensor_init,
	.deinit = pvt_sensor_deinit,
	.start = NULL,
	.stop = NULL,
	.read_process = read_process_value,
	.read_vol = read_voltage_value,
	.read_temp = read_temperature_value,
};

int pvt_sensor_get_value(int *rdata)
{
	if (pvt_dev == NULL) {
		pr_info("ERROR: pvt_sensor_get_value pvt_dev point is NULL!!!\n");
		return -1;
	}

	if (rdata == NULL) {
		pr_info("ERROR: pvt_sensor_get_value rdata point is NULL!!!\n");
		return -1;
	}
	pvt_sensor_init(pvt_dev);
	memcpy(rdata, tmp_rdata, sizeof(tmp_rdata));
	return 0;
}
EXPORT_SYMBOL_GPL(pvt_sensor_get_value);

void pvt_pop_jobs (unsigned long data)
{
	struct clourney_pvt_priv *pvt_priv = (struct clourney_pvt_priv *)data;

	/* unmask p/v/t sensors irq */
	sensor_io_write((pvt_priv->pvt_dev->reg_base + IRQ_TS_MASK_REG), 0);
	sensor_get_response(pvt_dev);

	sensor_io_write((pvt_priv->pvt_dev->reg_base + IRQ_PD_MASK_REG), 0);
	sensor_get_response(pvt_dev);

	sensor_io_write((pvt_priv->pvt_dev->reg_base + IRQ_VM_MASK_REG), 0);
	sensor_get_response(pvt_dev);
	/* display pvt value */
}

/* a function to run callbacks in the IRQ handler */
irqreturn_t pvt_irq_handler(int irq, void *dev)
{
	struct clourney_pvt_priv *pvt_priv = platform_get_drvdata(to_platform_device(dev));
	unsigned long lock_flag;

	spin_lock_irqsave(&pvt_priv->hw_lock, lock_flag);
	/* clear interrupt pin */
	sensor_irq_clean(pvt_priv->pvt_dev);
	/* run tasklet to pop jobs off fifo */
	tasklet_schedule(&pvt_priv->pop_jobs);
	spin_unlock_irqrestore(&pvt_priv->hw_lock, lock_flag);

   return IRQ_HANDLED;
}

void pvt_sensor_interrupt_init(struct clourney_pvt_dev *pvt_dev)
{
	if (!pvt_dev) {
		pr_info("ERROR: pvt_dev point is NULL!!!\n");
		return;
	}
	mutex_lock(&pvt_dev->my_mutex);
	pvt_init(pvt_dev, 0, 0 ,0, 1, 25);
	mutex_unlock(&pvt_dev->my_mutex);
}

void pvt_sensor_parameter_update(struct clourney_pvt_dev *pvt_dev)
{
	if (!pvt_dev) {
		pr_info("ERROR: pvt_dev point is NULL!!!\n");
		return;
	}

	mutex_lock(&pvt_dev->my_mutex);
	pvt_sensor_irq_parameter_update(pvt_dev);
	mutex_unlock(&pvt_dev->my_mutex);
}

