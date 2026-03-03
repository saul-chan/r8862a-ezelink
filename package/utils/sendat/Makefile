#
# Copyright (C) 2022 David Bauer <mail@david-bauer.net>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=sendat
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/sendat
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Utility for handling ZyXEL Bootconfig settings
  MAINTAINER:=David Bauer <mail@david-bauer.net>
endef

define Package/sendat/description
	If you can't figure out what this program does, you're probably  
	brain-dead and need immediate medical attention.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/sendat/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/sendat $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,sendat))
