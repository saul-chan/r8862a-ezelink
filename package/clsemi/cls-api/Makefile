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

PKG_NAME:=cls-api
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

DESTDIR := $(PKG_INSTALL_DIR)
DEST_INC_DIR ?= $(DESTDIR)/usr/include
DEST_BIN_DIR ?= $(DESTDIR)/bin
DEST_LIB_DIR ?= $(DESTDIR)/usr/lib
CLSAPI_PLUGIN_DIR ?= /usr/lib/rpcd
DEST_PLUGIN_DIR ?= $(DESTDIR)/$(CLSAPI_PLUGIN_DIR)
CLSAPI_CLI_PLUGIN_DIR ?= /usr/lib/cls/clsapi_cli
DEST_CLI_PLUGIN_DIR ?= $(DESTDIR)/$(CLSAPI_CLI_PLUGIN_DIR)

TARGET_CFLAGS += -I$(LINUX_DIR)/user_headers/include

define Package/cls-api
  SECTION:=clsemi
  CATEGORY:=ClourneySemi
  DEPENDS:= +libubox +libubus +libjson-c +libuci +rpcd +libnl-tiny +libiwinfo +cls-hdr
  TITLE:=ClourneySemi APIs
endef

define Build/Compile
	$(call Build/Compile/Default, DESTDIR="$(DESTDIR)" CLSAPI_PLUGIN_DIR="$(CLSAPI_PLUGIN_DIR)" \
		CLSAPI_PLAT_OPENWRT=1 all)
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/
	$(INSTALL_BIN) $(DEST_INC_DIR)/*.h $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib/
	$(INSTALL_BIN) $(DEST_LIB_DIR)/*.so $(1)/usr/lib/
endef

define Package/cls-api/install
	$(INSTALL_DIR) $(1)/bin
	$(CP) -r $(DEST_BIN_DIR)/* $(1)/bin/
	rm -rf $(1)/bin/clsapid
	$(INSTALL_DIR) $(1)/$(CLSAPI_PLUGIN_DIR)
	$(INSTALL_DIR) $(1)/$(CLSAPI_CLI_PLUGIN_DIR)
	$(INSTALL_BIN) $(DEST_LIB_DIR)/*.so $(1)/usr/lib/
	$(INSTALL_BIN) $(DEST_PLUGIN_DIR)/*.so $(1)/$(CLSAPI_PLUGIN_DIR)
	$(INSTALL_BIN) $(DEST_CLI_PLUGIN_DIR)/*.so $(1)/$(CLSAPI_CLI_PLUGIN_DIR)
endef

$(eval $(call BuildPackage,cls-api))

