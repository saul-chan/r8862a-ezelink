#
# Copyright (C) 2023 Clourney Semiconductor - All Rights Reserved.
# SPDX-License-Identifier: BSD-3-Clause
# https://spdx.org/licenses
#

include $(TOPDIR)/rules.mk

PKG_NAME:=netcwmp
PKG_VERSION:=c0f7cb0
PKG_RELEASE:=1.3

include $(INCLUDE_DIR)/package.mk

define Package/netcwmp
	SECTION:=net
	CATEGORY:=Network
	TITLE:=netcwmp
	DEPENDS+=+libubox +luci +libjson-c +zlib +libpthread +libopenssl +libubox +libubus +libuci +cls-api +libnl-tiny +libiwinfo
	URL:=https://github.com/netcwmp/netcwmp
endef

define Package/netcwmp/description
	NetCWMP is a protocol for remote management of network devices.
endef

define Package/netcwmp/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/cwmpd/src/cwmpd $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/etc/init.d/
	$(INSTALL_BIN) ./files/cwmpd.init $(1)/etc/init.d/cwmpd
	$(INSTALL_DIR) $(1)/etc/cwmpd/
	$(INSTALL_DATA) ./files/cwmp.conf $(1)/etc/cwmpd/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/conf/device.xml $(1)/etc/cwmpd/
	$(INSTALL_DIR) $(1)/etc/config/
	$(INSTALL_DATA) ./files/netcwmp.config $(1)/etc/config/netcwmp
endef

$(eval $(call BuildPackage,netcwmp))
