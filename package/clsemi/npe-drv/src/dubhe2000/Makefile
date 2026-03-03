# Copyright (c) 2021-2024, Clourney Semiconductor. All rights reserved.
#
# This software and/or documentation is licensed by Clourney Semiconductor under limited terms and conditions.
# Reproduction and redistribution in binary or source form, with or without modification,
# for use solely in conjunction with a Clourney Semiconductor chipset, is permitted in condition which
# must retain the above copyright notice.
#
# By using this software and/or documentation, you agree to the limited terms and conditions.

#
# Makefile for the Clourneysemi Dubhe2000 driver
#
CONFIG_BOC_OPTIMIZE ?= n

ccflags-$(CONFIG_DUBHE2000_MODULE) += -DCONFIG_DUBHE2000_MODULE
ccflags-$(CONFIG_ARM_CCI) += -DCONFIG_ARM_CCI
ccflags-$(CONFIG_NET_POLL_CONTROLLER) += -DCONFIG_NET_POLL_CONTROLLER
ccflags-$(CONFIG_BOC_OPTIMIZE) += -DBOC_THROUGH_PUT_OPTIMIZE=1

#subdir-y := ../rtk
obj-m += cls_npe.o
cls_npe-objs += dubhe2000_main.o dubhe2000_edma.o dubhe2000_tag.o dubhe2000_debugfs.o dubhe2000_ethtool.o \
		dubhe2000_fwd.o dubhe2000_switch.o dubhe2000_mac.o dubhe2000_aging.o \
		dubhe2000_l2.o dubhe2000_l3.o dubhe2000_export.o \
		dubhe2000_vlan.o \
		dubhe2000_switch_stats.o dubhe2000_edma_stats.o dubhe2000_mac_stats.o dubhe2000_switch_dump.o \
		dubhe2000_tunnel.o dubhe2000_switch_generic.o \
		dubhe2000_acl.o dubhe2000_nat.o dubhe2000_tc.o dubhe2000_switch_pause.o

cls_npe-objs += switch_api/device_read_write.o switch_api/flexswitch.o \
		switch_api/common.o switch_api/hashes.o

ifeq ($(PLAT_WORK_MODE), EMU)
ccflags-y   += -DCONFIG_DUBHE2000_PLAT_EMU
ccflags-y	+= -DCONFIG_DUBHE2000_DEVLINK
else
RTK_DIR = $(shell realpath $(PWD)/../rtk)
cls_npe-objs += ../rtk/rtk_linux_register.o \
	../rtk/dal_phy.o \
	../rtk/dal_waMon.o \
	../rtk/hwp.o \
	../rtk/hwp_init.o \
	../rtk/identify.o \
	../rtk/memory.o \
	../rtk/miim.o \
	../rtk/miim_common_drv.o \
	../rtk/miim_debug.o \
	../rtk/phy.o \
	../rtk/phy_common.o \
	../rtk/phy_construct.o \
	../rtk/phy_driver_hwp.o \
	../rtk/phy_experimental_rtl8211fs.o \
	../rtk/phy_hal.o \
	../rtk/phy_probe.o \
	../rtk/phy_init.o \
	../rtk/phy_misc.o \
	../rtk/phy_patch.o \
	../rtk/phy_rtl8218b.o \
	../rtk/phy_rtl8218b_patch.o \
	../rtk/phy_rtl826xb.o \
	../rtk/phy_rtl826xb_patch.o \
	../rtk/phy_macsec.o \
	../rtk/aes.o \
	../rtk/phy_wrapped_miim.o \
	../rtk/rt_bitop.o \
	../rtk/sem.o \
	../rtk/thread.o \
	../rtk/time.o \
	../rtk/phy_rtl8226.o \
	../rtk/nic_rtl8226.o \
	../rtk/nic_rtl8226b.o \
	../rtk/nic_rtl8226_init.o \
	../rtk/nic_rtl8226b_init.o \
	../rtk/nic_rtl8226_port.o \
	../rtk/phy_rtl8224.o \
	../rtk/phy_rtl8224_patch.o
ccflags-y += -DPHY_ONLY -DCONFIG_SDK_WORDSIZE_64 -DCONFIG_SDK_RTL8226 -DCONFIG_SDK_RTL8224 -DCONFIG_SDK_RTL8218B -DCONFIG_SDK_SRC -I$(RTK_DIR)/inc
ccflags-y	+= -DCONFIG_DUBHE2000_PHYLINK -Wall -Werror
cls_npe-objs +=dubhe2000_mdio.o dubhe2000_xpcs.o
endif

ifneq ($(CONFIG_DUBHE2000_ENABLE_FWD_TEST),)
ccflags-y	:= -D__ENABLE_FWD_TEST__
cls_npe-objs += dubhe2000_fwd_test.o
endif

ifneq ($(CONFIG_DUBHE2000_ENABLE_LOOPBACK),)
ccflags-y	:= -D__ENABLE_LOOPBACK__
cls_npe-objs += dubhe2000_loopback.o
endif

#switch_api/ need -Wno-declaration-after-statement
ccflags-y += -I$(PWD) -Wno-declaration-after-statement

all: modules
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules_install INSTALL_MOD_STRIP=1 INSTALL_MOD_PATH=$(KERNELOUT)

modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) $@

