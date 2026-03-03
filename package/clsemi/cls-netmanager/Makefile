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

PKG_NAME:=cls-netmanager
PKG_RELEASE:=1
PKG_FORTIFY_SOURCE:=0

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

TARGET_CFLAGS += -I$(LINUX_DIR)/user_headers/include

define Package/cls-netmanager
  SECTION:=clsemi
  CATEGORY:=ClourneySemi
  TITLE:=ClourneySemi net manager Daemon
  DEPENDS:=+libpthread +libubox +libubus +libuci +cls-api +libnl-tiny +libiwinfo +cls-mesh +cls-opmode
endef

define Package/cls-netmanager/postinst
	if [ $(CONFIG_EXTRA_VERSION_NUMBER) = "Mesh" ]; then
		# Change UCI default
		if [ -f $(TARGET_DIR)/etc/config/cls-mesh ]; then
			sed -i "s/option mode 'agent'/option mode 'auto'/" $(TARGET_DIR)/etc/config/cls-mesh
		fi
		if [ -f $(TARGET_DIR)/etc/config/cls-opmode ]; then
			sed -i "s/option mode 'router'/option mode 'bridge'/" $(TARGET_DIR)/etc/config/cls-opmode
		fi
	fi
endef

define Package/cls-netmanager/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/output/clsnetmanager $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/etc/init.d/
	$(INSTALL_BIN) ./files/cls-netmanager.init $(1)/etc/init.d/cls-netmanager
	$(INSTALL_DIR) $(1)/etc/config/
	$(INSTALL_DATA) ./files/cls-netmanager.config $(1)/etc/config/cls-netmanager
endef

$(eval $(call BuildPackage,cls-netmanager))
