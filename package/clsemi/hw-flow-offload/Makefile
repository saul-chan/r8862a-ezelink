include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=hw-flow-offload
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define KernelPackage/hw-flow-offload
  SECTION:=clsemi
  CATEGORY:=ClourneySemi
  TITLE:=ClourneySemi HW flow offload driver
  DEPENDS:=+kmod-npe-drv +kmod-nf-flow
  FILES:=$(PKG_BUILD_DIR)/hw_flow_offload.ko
endef

define Build/Compile
	$(KERNEL_MAKE) \
		KBUILD_EXTRA_SYMBOLS="$(KERNEL_BUILD_DIR)/npe-drv/dubhe2000/Module.symvers" \
		M="$(PKG_BUILD_DIR)" modules
endef

define KernelPackage/hw-flow-offload/install
	$(CP) ./files/* $(1)/
endef

$(eval $(call KernelPackage,hw-flow-offload))
