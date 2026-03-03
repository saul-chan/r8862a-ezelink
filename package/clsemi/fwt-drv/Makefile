include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=fwt-drv
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define KernelPackage/fwt-drv
  SECTION:=clsemi
  CATEGORY:=ClourneySemi
  TITLE:=ClourneySemi FWT driver
  FILES:=$(PKG_BUILD_DIR)/fwt_drv.ko
  AUTOLOAD:=$(call AutoLoad,09,fwt_drv.ko)
endef

define Build/Compile
	(cd $(PKG_BUILD_DIR) && CROSS_COMPILE=$(TARGET_CROSS) ./build.sh $(LINUX_DIR) $(PKG_BUILD_DIR))
endef

$(eval $(call KernelPackage,fwt-drv))
