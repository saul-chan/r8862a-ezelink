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

PKG_NAME:=clsapi-sample
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/clsapi-sample
  SECTION:=clsemi
  CATEGORY:=ClourneySemi
  DEPENDS:= +libubox +libubus +libuci +cls-api +libnl-tiny +libiwinfo
  TITLE:=ClourneySemi sample codes for CLS-API
endef

define Package/clsapi-sample/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/* $(1)/bin/
endef

$(eval $(call BuildPackage,clsapi-sample))
