#
# Copyright (c) 2021-2025, Clourney Semiconductor. All rights reserved.
#
# This software and/or documentation is licensed by Clourney Semiconductor under limited terms and conditions.
# Reproduction and redistribution in binary or source form, with or without modification,
# for use solely in conjunction with a Clourney Semiconductor chipset, is permitted in condition which
# must retain the above copyright notice.
#
# By using this software and/or documentation, you agree to the limited terms and conditions.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=dns-hijack
PKG_RELEASE:=1
PKG_LICENSE:=GPL-2.0

include $(INCLUDE_DIR)/package.mk

define Package/dns-hijack
  SECTION:=clsemi
  CATEGORY:=ClourneySemi
  TITLE:=ClourneySemi DNS hijack
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/dns-hijack/install
	$(CP) ./files/* $(1)/
endef

$(eval $(call BuildPackage,dns-hijack))
