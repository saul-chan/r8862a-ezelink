include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=pwm-drv
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define KernelPackage/pwm-drv
  SECTION:=clsemi
  CATEGORY:=ClourneySemi
  TITLE:=ClourneySemi PWM driver
  FILES:=$(PKG_BUILD_DIR)/dubhe2000_pwm.ko
endef

define Build/Compile
	(cd $(PKG_BUILD_DIR) && CROSS_COMPILE=$(TARGET_CROSS) ./build.sh $(LINUX_DIR) $(PKG_BUILD_DIR) dubhe2000)
endef

$(eval $(call KernelPackage,pwm-drv))
