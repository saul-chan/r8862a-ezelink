#
# Copyright (C) 2022 David Bauer <mail@david-bauer.net>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=slkgps-tcp
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/slkgps-tcp
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Utility for handling ZyXEL Bootconfig settings
  DEPENDS:=+libopenssl +libcurl +zlib
  MAINTAINER:=David Bauer <mail@david-bauer.net>
endef

define Package/slkgps-tcp/description
	If you can't figure out what this program does, you're probably  
	brain-dead and need immediate medical attention.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/slkgps-tcp/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/slkgps-tcp $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,slkgps-tcp))
