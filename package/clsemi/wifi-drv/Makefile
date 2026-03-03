include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

HWPLAT?=EVB-A
PKG_NAME:=wifi-drv
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define KernelPackage/wifi-drv
  SECTION:=clsemi
  CATEGORY:=ClourneySemi
  TITLE:=ClourneySemi Wi-Fi driver
  DEPENDS:=+kmod-cfg80211 +@DRIVER_11AX_SUPPORT +@DRIVER_11AC_SUPPORT +@DRIVER_11N_SUPPORT
  FILES:=$(PKG_BUILD_DIR)/clsm_wifi.ko \
	 $(PKG_BUILD_DIR)/clsm_wifi_soc.ko \
	 $(PKG_BUILD_DIR)/clsm_3addr_br.ko
endef

define Build/Compile
	(cd $(PKG_BUILD_DIR) && PLAT_WORK_MODE=$(HWPLAT) CROSS_COMPILE=$(TARGET_CROSS) ./build.sh $(LINUX_DIR) $(PKG_BUILD_DIR) dubhe2000)
endef

define KernelPackage/wifi-drv/install
	$(INSTALL_DIR) $(1)/lib/firmware
	ln -s /tmp/.cls_wifi_settings.ini $(1)/lib/firmware/cls_wifi_settings.ini
	$(CP) ./files/* $(1)/
	$(CP) $(PKG_BUILD_DIR)/rootfs_addons/* $(1)/
endef

$(eval $(call KernelPackage,wifi-drv))
