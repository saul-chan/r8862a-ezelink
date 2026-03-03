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

PKG_NAME:=cls-opmode
PKG_RELEASE:=1
PKG_LICENSE:=GPL-2.0

include $(INCLUDE_DIR)/package.mk

define Package/cls-opmode
  SECTION:=clsemi
  CATEGORY:=ClourneySemi
  TITLE:=ClourneySemi operation mode(router/bridge/repeater)
endef

define Package/cls-opmode/description
	This package provides implementation for switch of operation mode
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/cls-opmode/install
	$(CP) ./files/* $(1)/
endef

$(eval $(call BuildPackage,cls-opmode))
