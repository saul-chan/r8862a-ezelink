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

PKG_NAME:=cls-mesh
PKG_RELEASE:=1
PKG_FORTIFY_SOURCE:=0
#TARGET_CFLAGS += -ggdb3

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

TARGET_CFLAGS += -I$(LINUX_DIR)/user_headers/include

define Package/cls-mesh
  SECTION:=clsemi
  CATEGORY:=ClourneySemi
  TITLE:=ClourneySemi Ultra Mesh Daemon
  DEPENDS+=+libopenssl +libubus +libubox +libuci +libnl-tiny
endef

define Package/cls-mesh/install
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./files/cls-mesh.config $(1)/etc/config/cls-mesh
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/cls-mesh.init $(1)/etc/init.d/cls-mesh
	$(INSTALL_DIR) $(1)/etc/sysctl.d
	$(INSTALL_DATA) ./files/sysctl-cls-mesh.conf $(1)/etc/sysctl.d/99-cls-mesh.conf
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/output/clmeshd $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,cls-mesh))
