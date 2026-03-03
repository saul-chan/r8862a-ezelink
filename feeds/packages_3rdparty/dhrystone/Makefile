#
# Copyright (C) 2006-2009 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=dhrystone
PKG_VERSION:=2.1
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)
include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=3rdparty
  CATEGORY:=Third Party
  TITLE:=Dhrystone benchmark
endef

define Package/$(PKG_NAME)/description
  Dhrystone benchmark
endef

# overwrite the OpenWrt default -Os option
TARGET_CFLAGS = -O2 -funroll-all-loops --param max-inline-insns-auto=550

define Build/Prepare
	$(INSTALL_DIR) $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/gcc_dry2 $(1)/usr/bin/dhrystone
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
