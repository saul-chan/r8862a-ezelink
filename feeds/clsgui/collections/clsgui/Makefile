#
# Copyright (C) 2008-2014 The LuCI Team <luci@lists.subsignal.org>
#
# This is free software, licensed under the Apache License, Version 2.0 .
#

include $(TOPDIR)/rules.mk

CLSGUI_BASENAME:=clsgui

CLSGUI_TITLE:=Clsgui interface with Uhttpd as Webserver
CLSGUI_DESCRIPTION:=Standard OpenWrt set including full admin with ppp support and the default argon theme
CLSGUI_DEPENDS:= \
	+uhttpd +uhttpd-mod-ubus +clsgui-theme-argon

PKG_LICENSE:=Apache-2.0
PKG_VERSION:=1.2.3

include ../../clsgui.mk

# call BuildPackage - OpenWrt buildroot signature
