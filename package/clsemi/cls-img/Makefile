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

PKG_NAME:=cls-img
PKG_RELEASE:=1

PKG_FLAGS:=nonshared
PKG_BUILD_DEPENDS:=cls-img/host

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/host-build.mk

HOST_CFLAGS += -Wno-unused-result

define Package/cls-img
  SECTION:=clsemi
  CATEGORY:=ClourneySemi
  TITLE:=Utility for generating ClourneySemi Image
  DEPENDS:= +libc +zlib
endef

define Host/Prepare
	$(CP) ./src/* $(HOST_BUILD_DIR)
endef

define Host/Install
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/mkclsimg $(STAGING_DIR_HOST)/bin/
endef

define Package/cls-img/install
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/checkclsimg $(1)/sbin/
endef

$(eval $(call HostBuild))
$(eval $(call BuildPackage,cls-img))
