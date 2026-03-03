include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=cls-dpi
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define KernelPackage/cls-dpi
  SECTION:=clsemi
  CATEGORY:=ClourneySemi
  TITLE:=ClourneySemi DPI kernel module
  DEPENDS:=+kmod-nf-flow
  FILES:=$(PKG_BUILD_DIR)/cls_dpi.ko
  AUTOLOAD:=$(call AutoProbe,cls_dpi)
endef

define Build/Prepare
	$(call Build/Prepare/Default)
	./files/base64-swap.py files/etc/cls-dpi/app_feature.plain > $(PKG_BUILD_DIR)/app_feature
endef

define Build/Compile
	$(KERNEL_MAKE) M="$(PKG_BUILD_DIR)" modules
endef

define KernelPackage/cls-dpi/install
	$(INSTALL_DIR) $(1)/etc/cls-dpi/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/app_feature $(1)/etc/cls-dpi/app_feature
endef

$(eval $(call KernelPackage,cls-dpi))
