#
# Copyright (C) 2008-2015 The LuCI Team <luci@lists.subsignal.org>
#
# This is free software, licensed under the Apache License, Version 2.0 .
#

include $(TOPDIR)/rules.mk

PKG_NAME:=clsgui-base
PKG_VERSION:=1.2.3

CLSGUI_BASENAME:=clsgui-base

CLSGUI_TITLE:=Clsgui core libraries
CLSGUI_DEPENDS:=+lua +luci-lib-nixio +luci-lib-ip +rpcd +libubus-lua +luci-lib-jsonc +liblucihttp-lua +luci-lib-base +rpcd-mod-file +rpcd-mod-luci +cgi-io

PKG_LICENSE:=MIT

HOST_BUILD_DIR:=$(BUILD_DIR_HOST)/$(PKG_NAME)
include $(INCLUDE_DIR)/host-build.mk
define Package/luci-base/conffiles
/etc/luci-uploads
/etc/config/luci
/etc/config/ucitrack
endef
include ../../clsgui.mk

define Host/Configure
endef

define Host/Compile
endef

define Host/Install
endef

$(eval $(call HostBuild))

# call BuildPackage - OpenWrt buildroot signature
