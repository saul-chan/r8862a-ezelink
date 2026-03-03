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

PKG_NAME:=opencapwap
PKG_VERSION:=2.0

include $(INCLUDE_DIR)/package.mk

TARGET_CFLAGS += -I$(LINUX_DIR)/user_headers/include

define Package/opencapwap
	SECTION:=net
	CATEGORY:=Network
	DEPENDS:=+libopenssl +libpthread +libubox +libubus +libuci +cls-api +libiwinfo +libnl-tiny +@CAPWAP_IPC_ENABLE
	TITLE:=opencapwap open source
	URL:=https://github.com/vollero/openCAPWAP
endef

define Package/opencapwap/description
	Open source implementation of the CAPWAP protocol according to RFC 4515 and RFC 4516.
endef

define Package/opencapwap/install
	$(INSTALL_DIR) $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/AC $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/WTP $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/WUA $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/etc/init.d/
	$(INSTALL_BIN) ./files/wtp.init $(1)/etc/init.d/wtp
	$(INSTALL_DIR) $(1)/etc/wtp/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/config.wtp $(1)/etc/wtp/config.wtp
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/settings.wtp.txt $(1)/etc/wtp/settings.wtp.txt
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/config.ac $(1)/etc/wtp/config.ac
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/settings.ac.txt $(1)/etc/wtp/settings.ac.txt
	$(INSTALL_DIR) $(1)/etc/config/
	$(INSTALL_DATA) ./files/wtp.config $(1)/etc/config/wtp
endef

$(eval $(call BuildPackage,opencapwap))
