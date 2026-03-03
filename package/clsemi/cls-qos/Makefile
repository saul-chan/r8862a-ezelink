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

PKG_NAME:=cls-qos
PKG_RELEASE:=1

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define KernelPackage/cls-qos
  SECTION:=clsemi
  CATEGORY:=ClourneySemi
  TITLE:=ClourneySemi Ultra QoS kernel module
  DEPENDS:=+kmod-nf-flow
  FILES:=$(PKG_BUILD_DIR)/kernel/cls_qos.ko
  AUTOLOAD:=$(call AutoProbe,cls_qos)
endef

define Package/cls-qos
  SECTION:=clsemi
  CATEGORY:=ClourneySemi
  TITLE:=ClourneySemi Ulrta QoS
  DEPENDS:=+kmod-nft-netdev +kmod-nft-bridge +kmod-cls-qos +nftables +conntrack +rpcd +libubox +libubus +libuci +cls-hdr
endef

define Build/Compile
	$(KERNEL_MAKE) M="$(PKG_BUILD_DIR)/kernel" modules
endef

define Package/cls-qos/install
	$(INSTALL_DIR) $(1)/usr/lib/rpcd
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/rpcd/* $(1)/usr/lib/rpcd/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/* $(1)/usr/sbin/
	$(CP) ./files/* $(1)/
endef

$(eval $(call KernelPackage,cls-qos))
$(eval $(call BuildPackage,cls-qos))
