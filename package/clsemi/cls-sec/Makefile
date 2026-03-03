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

PKG_NAME:=cls-sec
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/cls-sec
  SECTION:=clsemi
  CATEGORY:=ClourneySemi
  TITLE:=ClourneySemi cls tools
  DEPENDS:= +libc
endef

define Package/cls-sec/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/cls $(1)/usr/sbin
	$(CP) ./files/* $(1)/
endef

$(eval $(call BuildPackage,cls-sec))
