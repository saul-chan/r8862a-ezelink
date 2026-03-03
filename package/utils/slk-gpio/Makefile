#
# Copyright (C) 2022 David Bauer <mail@david-bauer.net>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=slk-gpio
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/slk-gpio
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Utility for handling ZyXEL Bootconfig settings
  DEPENDS:=+libpthread 
  MAINTAINER:=David Bauer <mail@david-bauer.net>
endef

define Package/slk-gpio/description
	If you can't figure out what this program does, you're probably  
	brain-dead and need immediate medical attention.
endef
define Package/slk-gpio/extra_provides
	echo 'libstdc++.so.6'; \
	echo 'libm.so.6'; \
    echo 'libc.so.6';
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/slk-gpio/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/slk-gpio $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,slk-gpio))
