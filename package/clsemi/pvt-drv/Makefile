include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=pvt-drv
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define KernelPackage/pvt-drv
  SECTION:=clsemi
  CATEGORY:=ClourneySemi
  TITLE:=ClourneySemi PVT driver
  FILES:=$(PKG_BUILD_DIR)/dubhe2000_pvt.ko
endef

define Build/Compile
	(cd $(PKG_BUILD_DIR) && CROSS_COMPILE=$(TARGET_CROSS) ./build.sh $(LINUX_DIR) $(PKG_BUILD_DIR) dubhe2000)
endef

$(eval $(call KernelPackage,pvt-drv))
