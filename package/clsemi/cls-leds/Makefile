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

PKG_NAME:=cls-leds
PKG_RELEASE:=1
PKG_LICENSE:=GPL-2.0

include $(INCLUDE_DIR)/package.mk

define Package/cls-leds
  SECTION:=clsemi
  CATEGORY:=ClourneySemi
  DEPENDS:=+@BUSYBOX_CONFIG_NSLOOKUP +@BUSYBOX_CONFIG_FEATURE_NSLOOKUP_BIG
  TITLE:=ClourneySemi LEDs management package
  DEPENDS:=+libubox +libubus
endef

define Package/cls-leds/description
	This package provides implementation for leds management
endef

define Package/cls-leds/install
	$(CP) ./files/* $(1)/
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/output/led_notify $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,cls-leds))
