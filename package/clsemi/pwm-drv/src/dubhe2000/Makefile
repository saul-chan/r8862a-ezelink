# Copyright (c) 2021-2024, Clourney Semiconductor. All rights reserved.
#
# This software and/or documentation is licensed by Clourney Semiconductor under limited terms and conditions.
# Reproduction and redistribution in binary or source form, with or without modification,
# for use solely in conjunction with a Clourney Semiconductor chipset, is permitted in condition which
# must retain the above copyright notice.
#
# By using this software and/or documentation, you agree to the limited terms and conditions.

#
# Makefile for the Clourneysemi Dubhe1000 driver
#

obj-m += dubhe2000_pwm.o
dubhe2000_pwm-objs += pwm_drv.o

ccflags-y += -I$(PWD)

all: modules
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules_install INSTALL_MOD_STRIP=1 INSTALL_MOD_PATH=$(KERNELOUT)

modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) $@
