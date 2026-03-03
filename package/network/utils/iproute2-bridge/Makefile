#
# Copyright (C) 2006-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=iproute2
PKG_VERSION:=5.15.0
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@KERNEL/linux/utils/net/iproute2
PKG_HASH:=38e3e4a5f9a7f5575c015027a10df097c149111eeb739993128e5b2b35b291ff
PKG_BUILD_PARALLEL:=1
PKG_LICENSE:=GPL-2.0
PKG_CPE_ID:=cpe:/a:iproute2_project:iproute2

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk

define Package/iproute2-bridge
  SECTION:=net
  CATEGORY:=Network
  URL:=http://www.linuxfoundation.org/collaborate/workgroups/networking/iproute2
  SUBMENU:=Routing and Redirection
  MAINTAINER:=Russell Senior <russell@personaltelco.net>
  TITLE:=Bridge configuration utility from iproute2
  VARIANT:=bridge
  DEPENDS:=+libnl-tiny +libmnl
endef

define Build/Configure
	echo "static const char SNAPSHOT[] = \"$(PKG_VERSION)-$(PKG_RELEASE)-openwrt\";" \
		> $(PKG_BUILD_DIR)/include/SNAPSHOT.h
endef

TARGET_CFLAGS += -ffunction-sections -fdata-sections -flto
TARGET_LDFLAGS += -Wl,--gc-sections -Wl,--as-needed
TARGET_CPPFLAGS += -I$(STAGING_DIR)/usr/include/libnl-tiny

MAKE_FLAGS += \
	KERNEL_INCLUDE="$(LINUX_DIR)/include/uapi" \
	IP_CONFIG_TINY=y \
	BUILD_VARIANT=iptiny \
	LIBBPF_FORCE=off \
	HAVE_MNL=y \
	HAVE_CAP=$(HAVE_CAP) \
	TC_CONFIG_XT=n \
	TC_CONFIG_XT_OLD=n \
	TC_CONFIG_XT_OLD_H=n \
	TC_CONFIG_IPSET=n \
	FPIC="$(FPIC)" \
	$(if $(findstring c,$(OPENWRT_VERBOSE)),V=1,V='')

define Build/Compile
	+$(MAKE_VARS) $(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) $(MAKE_FLAGS)
endef

define Package/iproute2-bridge/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bridge/bridge $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,iproute2-bridge))
