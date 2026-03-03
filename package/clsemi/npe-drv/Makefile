include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

HWPLAT?=EVB-A
PKG_NAME:=npe-drv
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define KernelPackage/npe-drv
  SECTION:=clsemi
  CATEGORY:=ClourneySemi
  TITLE:=ClourneySemi NPE driver
  FILES:=$(PKG_BUILD_DIR)/dubhe2000/cls_npe.ko
  AUTOLOAD:=$(call AutoLoad,10,cls_npe.ko)
  MODPARAMS.cls_npe.ko:=switch_pause_mod=5
endef

define KernelPackage/npe-drv/config
  select KERNEL_PROFILING
endef

define Build/Compile
	(cd $(PKG_BUILD_DIR) && PLAT_WORK_MODE=$(HWPLAT) CROSS_COMPILE=$(TARGET_CROSS) ./build.sh $(LINUX_DIR) $(PKG_BUILD_DIR) dubhe2000)
endef

$(eval $(call KernelPackage,npe-drv))
