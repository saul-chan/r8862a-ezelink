include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=ytsw-drv
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define KernelPackage/ytsw-drv
  SECTION:=clsemi
  CATEGORY:=ClourneySemi
  TITLE:=ClourneySemi yt_switch driver
  FILES:=$(PKG_BUILD_DIR)/yt_switch.ko
  AUTOLOAD:=$(call AutoLoad,11,yt_switch.ko)
endef

define Build/Compile
	(cd $(PKG_BUILD_DIR) && \
		CPU_ARCH=arm64 \
		LINUX_SRC_PATH=$(LINUX_DIR) \
		KERNELOUT=$(PKG_BUILD_DIR) \
		CROSS_COMPILE=$(TARGET_CROSS) \
		make )
endef

$(eval $(call KernelPackage,ytsw-drv))
