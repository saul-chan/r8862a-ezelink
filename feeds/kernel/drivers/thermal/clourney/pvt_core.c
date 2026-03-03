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
#include <linux/types.h>
#include <linux/io.h>
#include "pvt_tsens.h"

#define sensor_io_read(v)        readl(v)
#define sensor_io_write(a, v)    writel(v, a)

#define SENSOR_DISPLAY_ENABLE  0
#define SENSOR_DISPLAY_DISABLE 1

struct cls_tsens_dev *tsens_dev;
EXPORT_SYMBOL(tsens_dev);
unsigned int debug_pr_info;
EXPORT_SYMBOL(debug_pr_info);

int tmp_rdata[T_SENSOR_NUM];

static void tsens_alarm_set(struct cls_tsens_dev *tsens_dev,
			uint8_t alarm_type, uint8_t alarm_no)
{
	spin_lock(&tsens_dev->alarm_lock);
	if (alarm_type == TSEN_ALARM_RISE) {
		tsens_dev->alarma_flag &= ~(1 << alarm_no);
		tsens_dev->alarma_flag |= (1 << alarm_no);
	}

	if (alarm_type == TSEN_ALARM_FALL) {
		tsens_dev->alarmb_flag &= ~(1 << alarm_no);
		tsens_dev->alarmb_flag |= (1 << alarm_no);
	}
	spin_unlock(&tsens_dev->alarm_lock);
}

static uint32_t create_low_12_bits_variable(uint32_t val)
{
	uint32_t mask = TS_ADC_BITMASK;

	return val & mask;
}

static int tsens_check_sdif_status(struct cls_tsens_dev *tsens_dev)
{
	int retry_times = 0, ret = 0, looptime = 100;

	while ((TS_SDIF_BUSY_MASK &
		sensor_io_read(tsens_dev->reg_base + TS_COMMON_OFFSET + COMMON_SDIF_STATUS)) != 0) {
		retry_times++;
		if (retry_times > looptime) {
			ret = -1;
			break;
		}
	}
	return ret;
}

static int tempcal(uint32_t datain, uint32_t modecalc)
{
	int DataCal;

	DataCal = (220500 * datain - 279661140);

	DataCal = DataCal / 4094;

	return DataCal;
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

static void sensor_pg00(struct cls_tsens_dev *tsens_dev)
{
	uint32_t rdata = 0;

	if (debug_pr_info == 1)
		printk(KERN_INFO "pg00...\n");

	rdata = sensor_io_read(tsens_dev->reg_base + 0x0);

	if (debug_pr_info == 1) {
		printk(KERN_INFO "PVT MOORTEC ID:0x%04x COMPONENT ID:0x%02x\n", rdata >> 18, (rdata >> 12) & 0x3F);
		printk(KERN_INFO "PVT Ver:%d.%d\n", (rdata >> 5) & 0x1F, (rdata >> 0) & 0x1F);
	}
	rdata = sensor_io_read(tsens_dev->reg_base + 0x4);
	if (debug_pr_info == 1) {
		printk(KERN_INFO "PVT VM:%d  CH:%d\n", (rdata >> 16) & 0xFF, (rdata >> 24) & 0xFF);
		printk(KERN_INFO "PVT PD:%d  TS:%d\n", (rdata >> 8) & 0xFF, (rdata >> 0) & 0xFF);
	}
	rdata = sensor_io_read(tsens_dev->reg_base + 0x8);
	if (debug_pr_info == 1)
		printk(KERN_INFO "Device ID:0x%08x\n", rdata);

	rdata = sensor_io_read(tsens_dev->reg_base + 0xc);
	if (debug_pr_info == 1)
		printk(KERN_INFO "scratch reg check addr=0x%p infor=0x%08x\n", tsens_dev->reg_base + 0xc, rdata);

	sensor_io_write(tsens_dev->reg_base + 0xc, 0x5A5A5A5A);
	rdata = sensor_io_read(tsens_dev->reg_base + 0xc);
	if (debug_pr_info == 1)
		printk(KERN_INFO "scratch reg check again addr=0x%p infor=0x%08x\n",
			(tsens_dev->reg_base + 0xc), rdata);
}

static void tsens_irq_parameter_update(struct cls_tsens_dev *tsens_dev)
{
	tsens_dev->temp_rise_alarm0 =
		(temperature_code_value_conversion(tsens_dev->temp_rise_alarm0_H) << 16)
		| temperature_code_value_conversion(tsens_dev->temp_rise_alarm0_L);
	tsens_dev->temp_rise_alarm1 =
		(temperature_code_value_conversion(tsens_dev->temp_rise_alarm1_H) << 16)
		| temperature_code_value_conversion(tsens_dev->temp_rise_alarm1_L);
	tsens_dev->temp_rise_alarm2 =
		(temperature_code_value_conversion(tsens_dev->temp_rise_alarm2_H) << 16)
		| temperature_code_value_conversion(tsens_dev->temp_rise_alarm2_L);
	tsens_dev->temp_fall_alarm0 =
		(temperature_code_value_conversion(tsens_dev->temp_fall_alarm0_L) << 16)
		| temperature_code_value_conversion(tsens_dev->temp_fall_alarm0_H);
	tsens_dev->temp_fall_alarm1 =
		(temperature_code_value_conversion(tsens_dev->temp_fall_alarm1_L) << 16)
		| temperature_code_value_conversion(tsens_dev->temp_fall_alarm1_H);
	tsens_dev->temp_fall_alarm2 =
		(temperature_code_value_conversion(tsens_dev->temp_fall_alarm2_L) << 16)
		| temperature_code_value_conversion(tsens_dev->temp_fall_alarm2_H);

	// Ts0~2
	// set alarma
	sensor_io_write(tsens_dev->reg_base + TS_MACRO_OFFSET + ARLARMA_CFG_REG + 0 * MACRO_OFFSET,
			tsens_dev->temp_rise_alarm0);
	sensor_io_write(tsens_dev->reg_base + TS_MACRO_OFFSET + ARLARMA_CFG_REG + 1 * MACRO_OFFSET,
			tsens_dev->temp_rise_alarm1);
	sensor_io_write(tsens_dev->reg_base + TS_MACRO_OFFSET + ARLARMA_CFG_REG + 2 * MACRO_OFFSET,
			tsens_dev->temp_rise_alarm2);
	// set alarmb
	sensor_io_write(tsens_dev->reg_base + TS_MACRO_OFFSET + ARLARMB_CFG_REG + 0 * MACRO_OFFSET,
			tsens_dev->temp_fall_alarm0);
	sensor_io_write(tsens_dev->reg_base + TS_MACRO_OFFSET + ARLARMB_CFG_REG + 1 * MACRO_OFFSET,
			tsens_dev->temp_fall_alarm1);
	sensor_io_write(tsens_dev->reg_base + TS_MACRO_OFFSET + ARLARMB_CFG_REG + 2 * MACRO_OFFSET,
			tsens_dev->temp_fall_alarm2);
}

static void sensor_pg01(struct cls_tsens_dev *tsens_dev)
{
	int cnt = 0;

	sensor_io_write(tsens_dev->reg_base + IRQ_GLB_EN_REG, (1 << TS_IRQ_ENABLE_BIT)); // global irq_enable
	//Temperature
	for (cnt = 0; cnt < T_SENSOR_NUM; cnt++)
		sensor_io_write(tsens_dev->reg_base + TS_MACRO_OFFSET + IRQ_ENABLE_REG + cnt * MACRO_OFFSET, 0x18);

	for (cnt = 0; cnt < T_SENSOR_NUM; cnt++) {
		// Ts0~2
		// set alarma
		// 90-->110 C rising alarm (0x0c360c00)
		sensor_io_write(tsens_dev->reg_base + TS_MACRO_OFFSET + ARLARMA_CFG_REG + 0 * MACRO_OFFSET,
				tsens_dev->temp_rise_alarm0);
		// 90-->110 C rising alarm (0x0c360c00)
		sensor_io_write(tsens_dev->reg_base + TS_MACRO_OFFSET + ARLARMA_CFG_REG + 1 * MACRO_OFFSET,
				tsens_dev->temp_rise_alarm1);
		// 90-->110 C rising alarm (0x0c360c00)
		sensor_io_write(tsens_dev->reg_base + TS_MACRO_OFFSET + ARLARMA_CFG_REG + 2 * MACRO_OFFSET,
				tsens_dev->temp_rise_alarm2);
		// set alarmb
		// -20<---10C  falling alarm (0x0d000c36)
		sensor_io_write(tsens_dev->reg_base + TS_MACRO_OFFSET + ARLARMB_CFG_REG + 0 * MACRO_OFFSET,
				tsens_dev->temp_fall_alarm0);
		// -20<---10C  falling alarm (0x0d000c36)
		sensor_io_write(tsens_dev->reg_base + TS_MACRO_OFFSET + ARLARMB_CFG_REG + 1 * MACRO_OFFSET,
				tsens_dev->temp_fall_alarm1);
		// -20<---10C  falling alarm (0x0d000c36)
		sensor_io_write(tsens_dev->reg_base + TS_MACRO_OFFSET + ARLARMB_CFG_REG + 2 * MACRO_OFFSET,
				tsens_dev->temp_fall_alarm2);
	}

	// PVT Sample Control
	// ts group
	sensor_io_write(tsens_dev->reg_base + TS_COMMON_OFFSET + COMMON_SMPL_CTRL, 0x0);
	// PVT Sample HiLo Control
	// ts0~2
	for (cnt = 0; cnt < T_SENSOR_NUM; cnt++)
		sensor_io_write(tsens_dev->reg_base + TS_MACRO_OFFSET + HILO_RESET_REG + cnt * MACRO_OFFSET, 0x3);
}

static void sensor_pg02(struct cls_tsens_dev *tsens_dev)
{
	if (debug_pr_info == 1)
		printk(KERN_INFO "pg02...\n");

	// ts0~2
	// set sdif halt ts common
	sensor_io_write(tsens_dev->reg_base + TS_COMMON_OFFSET + COMMON_SDIF_HALT, 0x0);

	// ts0~2
	// set sdif disable
	sensor_io_write(tsens_dev->reg_base + TS_COMMON_OFFSET + COMMON_SDIF_DISABLE, 0x7);
	// set sdif disable to 0
	sensor_io_write(tsens_dev->reg_base + TS_COMMON_OFFSET + COMMON_SDIF_DISABLE, 0x0);

	// ts
	// set clk synth ref_clk=48MHz
	sensor_io_write(tsens_dev->reg_base + TS_COMMON_OFFSET + COMMON_CLK_SYNTH,
			((1 << 24) | (1 << 16) | (3 << 8) | (3 << 0)));
}

static void sensor_pg03(struct cls_tsens_dev *tsens_dev,
			uint8_t ts_cfg, uint8_t an_en, uint8_t cfga, uint8_t no_ip_ctrl)
{
	uint32_t rdata = 0, wdata = 0; int cnt = 0;

	if (debug_pr_info == 1)
		printk(KERN_INFO "pg03...\n");

	// PG03: Program the SDA registers
	tsens_check_sdif_status(tsens_dev);

	// set sdif, ip_tmr
	sensor_io_write(tsens_dev->reg_base + TS_COMMON_OFFSET + COMMON_SDIF,
			((1 << 31) | (1 << 27) | (5 << 24) | (0x100 << 0)));

	tsens_check_sdif_status(tsens_dev);

	// set sdif, ip_tmr
	sensor_io_write(tsens_dev->reg_base + TS_COMMON_OFFSET + COMMON_SDIF,
			((1 << 31) | (0 << 27) | (5 << 24) | (0x100 << 0)));

	for (cnt = 0; cnt < T_SENSOR_NUM; cnt++) {
		rdata = sensor_io_read(tsens_dev->reg_base + TS_MACRO_OFFSET + SDIF_RDATA_REG + cnt * MACRO_OFFSET);
		if (debug_pr_info == 1)
			printk(KERN_INFO "TS%d SDIF RDATA addr=0x%p infor=0x%08x\n", cnt,
				(tsens_dev->reg_base + TS_MACRO_OFFSET + SDIF_RDATA_REG + cnt * MACRO_OFFSET), rdata);
	}

	// set sdif, ip_cfg
	wdata = (1 << 31 | 1 << 27 | 1 << 24 | 0xAA << 16 | 0xBB << 8 | ts_cfg);
	sensor_io_write(tsens_dev->reg_base + TS_COMMON_OFFSET + COMMON_SDIF, wdata);

	tsens_check_sdif_status(tsens_dev);

	// read SDA register ip_cfg
	sensor_io_write(tsens_dev->reg_base + TS_COMMON_OFFSET + COMMON_SDIF,
			((1 << 31) | (0 << 27) | (1 << 24) | (0 << 0)));

	for (cnt = 0; cnt < T_SENSOR_NUM; cnt++) {
		rdata = sensor_io_read(tsens_dev->reg_base + TS_MACRO_OFFSET + SDIF_RDATA_REG + cnt * MACRO_OFFSET);
		if (debug_pr_info == 1)
			printk(KERN_INFO "TS%d SDIF RDATA addr=0x%p infor=0x%08x\n", cnt,
				(tsens_dev->reg_base + TS_MACRO_OFFSET + SDIF_RDATA_REG + cnt * MACRO_OFFSET), rdata);
	}

	// set sdif, ip_cfga
	wdata = (1 << 31 | 1 << 27 | 2 << 24 | 0 << 16 | an_en << 8 | cfga);
	sensor_io_write(tsens_dev->reg_base + TS_COMMON_OFFSET + COMMON_SDIF, wdata);

	tsens_check_sdif_status(tsens_dev);

	// read SDA register ip_cfg
	sensor_io_write(tsens_dev->reg_base + TS_COMMON_OFFSET + COMMON_SDIF,
			(1 << 31 | 0 << 27 | 2 << 24 | 0 << 16 | 0 << 8 | 0));

	for (cnt = 0; cnt < T_SENSOR_NUM; cnt++) {
		rdata = sensor_io_read(tsens_dev->reg_base + TS_MACRO_OFFSET + SDIF_RDATA_REG + cnt * MACRO_OFFSET);
		if (debug_pr_info == 1)
			printk(KERN_INFO "TS%d SDIF RDATA addr=0x%p infor=0x%08x\n", cnt,
				(tsens_dev->reg_base + TS_MACRO_OFFSET + SDIF_RDATA_REG + cnt * MACRO_OFFSET), rdata);
	}

	if (no_ip_ctrl == 0) {
		sensor_io_write(tsens_dev->reg_base + TS_COMMON_OFFSET + COMMON_SDIF,
				(1 << 31 | 1 << 27 | 0 << 24 | 0 << 16 | 0 << 8 | 0));

		tsens_check_sdif_status(tsens_dev);

		sensor_io_write(tsens_dev->reg_base + TS_COMMON_OFFSET + COMMON_SDIF,
				(1 << 31 | 0 << 27 | 0 << 24 | 0 << 16 | 0 << 8 | 0));

		for (cnt = 0; cnt < T_SENSOR_NUM; cnt++) {
			rdata = sensor_io_read(tsens_dev->reg_base +
						TS_MACRO_OFFSET + SDIF_RDATA_REG + cnt * MACRO_OFFSET);
			if (debug_pr_info == 1)
				printk(KERN_INFO "TS%d SDIF RDATA addr=0x%p infor=0x%08x\n", cnt,
					(tsens_dev->reg_base + TS_MACRO_OFFSET + SDIF_RDATA_REG + cnt * MACRO_OFFSET),
					rdata);
		}
	}
}

static void sensor_irq_clean(struct cls_tsens_dev *tsens_dev)
{
	uint32_t rdata = 0, wdata = 0, cnt = 0;
	uint32_t ts_rdata[T_SENSOR_NUM];

	rdata = sensor_io_read(tsens_dev->reg_base + IRQ_TS_STATUS_REG);
	if (rdata) {
		//printk(KERN_INFO "TS irq status, infor=0x%08x\n", rdata);
		//Update All t-sensor temp data
		for (cnt = 0; cnt < T_SENSOR_NUM; cnt++) {
			ts_rdata[cnt] = sensor_io_read(tsens_dev->reg_base +
				TS_MACRO_OFFSET + SDIF_DATA_REG + cnt * MACRO_OFFSET);
			ts_rdata[cnt] = create_low_12_bits_variable(ts_rdata[cnt]);
			tsens_dev->tsens_data[cnt] = tempcal(ts_rdata[cnt], 1);
		}

		for (cnt = 0; cnt < T_SENSOR_NUM; cnt++) {
			rdata = sensor_io_read(tsens_dev->reg_base + TS_MACRO_OFFSET
					+ IRQ_STATUS_REG + cnt * MACRO_OFFSET);
			if (rdata) {
				/* mask T0/T1/T2 sensor irq */
				sensor_io_write((tsens_dev->reg_base + IRQ_TS_MASK_REG), cnt << 1);

				ts_rdata[cnt] = sensor_io_read(tsens_dev->reg_base +
								TS_MACRO_OFFSET + SDIF_DATA_REG + cnt * MACRO_OFFSET);
				ts_rdata[cnt] = create_low_12_bits_variable(ts_rdata[cnt]);

				printk(KERN_INFO "TS%d Temperature[%d.%03d C] modecal%d ts_rdata[0x%08x], irq infor=0x%08x\n",
					cnt, tempcal(ts_rdata[cnt], 1) / 1000,
					abs(tempcal(ts_rdata[cnt], 1) % 1000), 1, ts_rdata[cnt], rdata);

				if (rdata & IRQ_STATUS_ALARMA)
					tsens_alarm_set(tsens_dev, TSEN_ALARM_RISE, cnt);

				if (rdata & IRQ_STATUS_ALARMB)
					tsens_alarm_set(tsens_dev, TSEN_ALARM_FALL, cnt);
			}
			wdata = 1 << 4 | 1 << 3 | 1 << 1 | 1 << 0;
			sensor_io_write((tsens_dev->reg_base + TS_MACRO_OFFSET
						+ IRQ_CLEAR_REG + cnt * MACRO_OFFSET), wdata);
		}

		if (tsens_dev->alarm_work_enable)
			schedule_work(&tsens_dev->tsens_work);
	}
}

static void __used sensor_pg05(struct cls_tsens_dev *tsens_dev)
{
	uint32_t wdata = 0;

	if (debug_pr_info == 1)
		printk(KERN_INFO "pg05...\n");

	tsens_check_sdif_status(tsens_dev);

	wdata = 1 << 31 | 1 << 27 | 0 << 24 | 1 << 0;
	sensor_io_write(tsens_dev->reg_base + TS_COMMON_OFFSET + COMMON_SDIF, wdata);

	wdata = 0 << 24;
	sensor_io_write(tsens_dev->reg_base + TS_COMMON_OFFSET + COMMON_CLK_SYNTH, wdata);

	tsens_check_sdif_status(tsens_dev);
}

static void __used sensor_pg06(struct cls_tsens_dev *tsens_dev,
				uint8_t ts_cfg, uint8_t an_en, uint8_t cfga, int temperature)
{
	int cnt;
	uint32_t golden_nbs = 0;
	uint32_t rdata = 0, wdata = 0, ts_rdata[T_SENSOR_NUM];

	if (debug_pr_info == 1)
		printk(KERN_INFO "pg06...\n");

	sensor_pg03(tsens_dev, ts_cfg, an_en, cfga, 1);

	tsens_check_sdif_status(tsens_dev);

	wdata = 1 << 31 | 1 << 27 | 0 << 24 | 0x0104 << 0;
	sensor_io_write(tsens_dev->reg_base + TS_COMMON_OFFSET + COMMON_SDIF, wdata);

	// read smpl cnt
	// PG06_1_* Wait for a sample done IRQ
	for (cnt = 0; cnt < T_SENSOR_NUM; cnt++) {
		rdata = sensor_io_read(tsens_dev->reg_base + TS_MACRO_OFFSET + SDIF_DONE_REG + cnt * MACRO_OFFSET);
		if (debug_pr_info == 1)
			printk(KERN_INFO "TS%d SDIF done addr=0x%p infor=0x%08x\n", cnt,
				(tsens_dev->reg_base + TS_MACRO_OFFSET + SDIF_DONE_REG + cnt * MACRO_OFFSET), rdata);
	}

	msleep(50);

	// read smpl cnt
	rdata = sensor_io_read(tsens_dev->reg_base + TS_COMMON_OFFSET + COMMON_SMPL_CNT);
	if (debug_pr_info == 2)
		printk(KERN_INFO "TS rdaddr[%p]:smpl cnt=0x%08x\n",
			(tsens_dev->reg_base + TS_COMMON_OFFSET + COMMON_SMPL_CNT), rdata);

	// read TS SDIF DATA
	for (cnt = 0; cnt < T_SENSOR_NUM; cnt++) {
		ts_rdata[cnt] = sensor_io_read(tsens_dev->reg_base +
						TS_MACRO_OFFSET + SDIF_DATA_REG + cnt * MACRO_OFFSET);
		ts_rdata[cnt] = create_low_12_bits_variable(ts_rdata[cnt]);

		if ((ts_cfg & 0xF) == 1)
			printk(KERN_INFO "TS%d Temperature[%d.%03d C] modecal%d ts_rdata[0x%08x]\n",
				cnt, tempcal(ts_rdata[cnt], 2) / 1000,
				abs(tempcal(ts_rdata[cnt], 2) % 1000), 2, ts_rdata[cnt]);
		else
			printk(KERN_INFO "TS%d Temperature[%d.%03d C] modecal%d ts_rdata[0x%08x]\n",
				cnt, tempcal(ts_rdata[cnt], 1) / 1000,
				abs(tempcal(ts_rdata[cnt], 1) % 1000), 1, ts_rdata[cnt]);

		tmp_rdata[cnt] = tempcal(ts_rdata[cnt], 1);
		if (debug_pr_info == 1)
			printk(KERN_INFO "Gold temperature = %d C\n", temperature);

		if (((ts_cfg & 0xF) == 0) && (an_en == 0))
			golden_nbs = nbs_calc(temperature, 0);
		else if (((ts_cfg & 0xF) == 0) && (an_en == 1) && ((cfga & 0xF) == 0))
			golden_nbs = nbs_calc(temperature, 0);
		else if (((ts_cfg & 0xF) == 1) && (an_en == 0))
			golden_nbs = nbs_calc(temperature, 1);
		else if (((ts_cfg & 0xF) == 1) && (an_en == 1) && ((cfga & 0xF) == 0))
			golden_nbs = nbs_calc(temperature, 1);
		else if (((ts_cfg & 0xF) == 0) && (an_en == 1) && ((cfga & 0xF) == 3))
			golden_nbs = nbs_calc(temperature, 2);

		golden_nbs = golden_nbs | an_en << 16;

		if (debug_pr_info == 1)
			printk(KERN_INFO "golden_nbs = 0x%08x\n", golden_nbs);

		if (golden_nbs != ts_rdata[cnt]) {
			if (debug_pr_info == 1)
				printk(KERN_INFO "Golden nbs != Dout\n");
		}
	}
}

static void tsens_print(struct cls_tsens_dev *tsens_dev, uint32_t tsensor_display_flag)
{
	int cnt;
	uint32_t rdata = 0, ts_rdata[T_SENSOR_NUM];

	mutex_lock(&tsens_dev->my_mutex);
	// SDIF_SMPL_DONE
	// PG06_1_* Wait for a sample done IRQ
	for (cnt = 0; cnt < T_SENSOR_NUM; cnt++) {
		rdata = sensor_io_read(tsens_dev->reg_base + TS_MACRO_OFFSET + SDIF_DONE_REG + cnt * MACRO_OFFSET);
		if ((debug_pr_info == 1) && (tsensor_display_flag == SENSOR_DISPLAY_ENABLE))
			printk(KERN_INFO "TS%d SDIF done addr=0x%p infor =0x%08x\n", cnt,
				(tsens_dev->reg_base + TS_MACRO_OFFSET + SDIF_DONE_REG + cnt * MACRO_OFFSET), rdata);
	}
	msleep(50);

	// SMPL_COUNT check  read smpl cnt
	rdata = sensor_io_read(tsens_dev->reg_base + TS_COMMON_OFFSET + COMMON_SMPL_CNT);
	if ((debug_pr_info == 1) && (tsensor_display_flag == SENSOR_DISPLAY_ENABLE))
		printk(KERN_INFO "TS rdaddr[0x%p]:smpl cnt=0x%08x\n",
			(tsens_dev->reg_base + TS_COMMON_OFFSET + COMMON_SMPL_CNT), rdata);

	//RD IP sample data type and fault values: read TS SDIF DATA
	for (cnt = 0; cnt < T_SENSOR_NUM; cnt++) {
		ts_rdata[cnt] = sensor_io_read(tsens_dev->reg_base +
						TS_MACRO_OFFSET + SDIF_DATA_REG + cnt * MACRO_OFFSET);
		ts_rdata[cnt] = create_low_12_bits_variable(ts_rdata[cnt]);
		if (tsensor_display_flag == SENSOR_DISPLAY_ENABLE)
			printk(KERN_INFO "TS%d Temperature[%d.%03d C] modecal%d ts_rdata[0x%08x]\n",
				cnt, tempcal(ts_rdata[cnt], 1) / 1000,
				abs(tempcal(ts_rdata[cnt], 1) % 1000), 1, ts_rdata[cnt]);
		tmp_rdata[cnt] = tempcal(ts_rdata[cnt], 1);
	}
	msleep(50);
	mutex_unlock(&tsens_dev->my_mutex);
}

static void tsens_init(struct cls_tsens_dev *tsens_dev, uint8_t ts_cfg, uint8_t an_en, uint8_t cfga)
{
	uint32_t rdata = 0, wdata = 0;
	int cnt;

	sensor_pg00(tsens_dev);
	sensor_pg01(tsens_dev);
	sensor_pg02(tsens_dev);
	sensor_pg03(tsens_dev, ts_cfg, an_en, cfga, 1);

	tsens_check_sdif_status(tsens_dev);

	wdata = 1 << 31 | 1 << 27 | 0 << 24 | 0x0108 << 0;
	sensor_io_write(tsens_dev->reg_base + TS_COMMON_OFFSET + COMMON_SDIF, wdata);

	// SDIF_SMPL_DONE
	// PG06_1_* Wait for a sample done IRQ
	for (cnt = 0; cnt < T_SENSOR_NUM; cnt++)
		rdata = sensor_io_read(tsens_dev->reg_base + TS_MACRO_OFFSET + SDIF_DONE_REG + cnt * MACRO_OFFSET);

	msleep(50);

	// SMPL_COUNT check  read smpl cnt
	rdata = sensor_io_read(tsens_dev->reg_base + TS_COMMON_OFFSET + COMMON_SMPL_CNT);
}

static int cls_tsens_init(struct cls_tsens_dev *tsens_dev)
{
	tsens_print(tsens_dev, SENSOR_DISPLAY_ENABLE);
	return 0;
}

static int cls_tsens_deinit(struct cls_tsens_dev *tsens_dev)
{
	return 0;
}

static int read_process_value(struct cls_tsens_dev *tsens_dev)
{
	return 0;
}

static int read_voltage_value(struct cls_tsens_dev *tsens_dev)
{
	return 0;
}

static int read_temperature_value(struct cls_tsens_dev *tsens_dev)
{
	return 0;
}

struct cls_tsens_ops tsens_ops = {
	.init = cls_tsens_init,
	.deinit = cls_tsens_deinit,
	.start = NULL,
	.stop = NULL,
	.read_process = read_process_value,
	.read_vol = read_voltage_value,
	.read_temp = read_temperature_value,
};
EXPORT_SYMBOL(tsens_ops);

int cls_tsens_get_value(int *rdata)
{
	if (tsens_dev == NULL) {
		pr_info("ERROR: %s tsens_dev point is NULL!!!\n", __func__);
		return -1;
	}

	if (rdata == NULL) {
		pr_info("ERROR: %s rdata point is NULL!!!\n", __func__);
		return -1;
	}

	tsens_print(tsens_dev, SENSOR_DISPLAY_DISABLE);
	memcpy(rdata, tmp_rdata, sizeof(tmp_rdata));
	return 0;
}
EXPORT_SYMBOL_GPL(cls_tsens_get_value);

void tsens_pop_jobs(unsigned long data)
{
	struct cls_tsens_priv *tsens_priv = (struct cls_tsens_priv *)data;

	/* unmask p/v/t sensors irq */
	sensor_io_write((tsens_priv->tsens_dev->reg_base + IRQ_TS_MASK_REG), 0);
}
EXPORT_SYMBOL_GPL(tsens_pop_jobs);

/* a function to run callbacks in the IRQ handler */
irqreturn_t tsens_irq_handler(int irq, void *dev)
{
	struct cls_tsens_priv *tsens_priv = platform_get_drvdata(to_platform_device(dev));
	unsigned long lock_flag;

	spin_lock_irqsave(&tsens_priv->hw_lock, lock_flag);
	/* clear interrupt pin */
	sensor_irq_clean(tsens_priv->tsens_dev);
	/* run tasklet to pop jobs off fifo */
	tasklet_schedule(&tsens_priv->pop_jobs);
	spin_unlock_irqrestore(&tsens_priv->hw_lock, lock_flag);

	return IRQ_HANDLED;
}
EXPORT_SYMBOL(tsens_irq_handler);

void tsens_interrupt_init(struct cls_tsens_dev *tsens_dev)
{
	if (!tsens_dev) {
		pr_info("ERROR: tsens_dev point is NULL!!!\n");
		return;
	}
	mutex_lock(&tsens_dev->my_mutex);
	tsens_init(tsens_dev, 0, 0, 0);
	mutex_unlock(&tsens_dev->my_mutex);
}
EXPORT_SYMBOL(tsens_interrupt_init);

void tsens_parameter_update(struct cls_tsens_dev *tsens_dev)
{
	if (!tsens_dev) {
		pr_info("ERROR: tsens_dev point is NULL!!!\n");
		return;
	}

	mutex_lock(&tsens_dev->my_mutex);
	tsens_irq_parameter_update(tsens_dev);
	mutex_unlock(&tsens_dev->my_mutex);
}
EXPORT_SYMBOL(tsens_parameter_update);
