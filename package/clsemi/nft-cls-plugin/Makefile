include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=nft-cls-plugin
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define KernelPackage/nft-cls-plugin
  SECTION:=clsemi
  CATEGORY:=ClourneySemi
  TITLE:=ClourneySemi plguin for netfilter
  DEPENDS:=+kmod-nft-core
  FILES:=$(PKG_BUILD_DIR)/nft_cls_plugin.ko
  AUTOLOAD:=$(call AutoProbe,nft_cls_plugin)
endef

define Build/Compile
	$(KERNEL_MAKE) M="$(PKG_BUILD_DIR)" modules
endef

$(eval $(call KernelPackage,nft-cls-plugin))
