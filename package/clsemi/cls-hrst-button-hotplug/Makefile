#
# Copyright (c) 2021-2024, Clourney Semiconductor. All rights reserved.
#
# This software and/or documentation is licensed by Clourney Semiconductor under limited terms and conditions.
# Reproduction and redistribution in binary or source form, with or without modification,
# for use solely in conjunction with a Clourney Semiconductor chipset, is permitted in condition which
# must retain the above copyright notice.
#
# By using this software and/or documentation, you agree to the limited terms and conditions.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=cls-hrst-button-hotplug
PKG_RELEASE:=3
PKG_LICENSE:=GPL-2.0

include $(INCLUDE_DIR)/package.mk

define KernelPackage/cls-hrst-button-hotplug
  SECTION:=clsemi
  CATEGORY:=ClourneySemi
  TITLE:=ClourneySemi simple hardware reset button hotplug driver
  FILES:=$(PKG_BUILD_DIR)/cls-hrst-button-hotplug.ko
  AUTOLOAD:=$(call AutoLoad,30,cls-hrst-button-hotplug,1)
  KCONFIG:=
endef

define KernelPackage/cls-hrst-button-hotplug/description
 This is a replacement for the following in-kernel drivers:
 1) gpio_keys (KEYBOARD_GPIO)
 2) gpio_keys_polled (KEYBOARD_GPIO_POLLED)

 Instead of generating input events (like in-kernel drivers do) it generates
 uevent-s and broadcasts them. This allows disabling input subsystem which is
 an overkill for OpenWrt simple needs.
endef

define Build/Compile
	$(KERNEL_MAKE) M="$(PKG_BUILD_DIR)" modules
endef

$(eval $(call KernelPackage,cls-hrst-button-hotplug))
