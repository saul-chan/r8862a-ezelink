include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

HWPLAT?=EVB-A
PKG_NAME:=dt-overlay
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define KernelPackage/dt-overlay
  SECTION:=clsemi
  CATEGORY:=ClourneySemi
  TITLE:=device-tree overlay driver
  FILES:=$(PKG_BUILD_DIR)/dts_overlay.ko
endef

define KernelPackage/dt-overlay/config
  select KERNEL_PROFILING
endef

define Build/Compile
	(cd $(PKG_BUILD_DIR) && PLAT_WORK_MODE=$(HWPLAT) CROSS_COMPILE=$(TARGET_CROSS) ./build.sh $(LINUX_DIR) $(PKG_BUILD_DIR))
	(cd $(PKG_BUILD_DIR)/npe && dtc=$(LINUX_DIR)/scripts/dtc/dtc make)
endef

define  KernelPackage/dt-overlay/install
	$(INSTALL_DIR) $(1)/etc/npe_dtb
	$(INSTALL_DIR) $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/npe/npe_dtb/* $(1)/etc/npe_dtb/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/npe/script/* $(1)/usr/sbin/
endef

$(eval $(call KernelPackage,dt-overlay))
