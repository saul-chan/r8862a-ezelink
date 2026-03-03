include $(TOPDIR)/rules.mk

PKG_NAME:=cls-hdr
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

define Package/cls-hdr
  SECTION:=clsemi
  CATEGORY:=ClourneySemi
  TITLE:=Gather ClourneySemi header files from driver
endef

define Build/Compile
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/cls
	$(CP) $(PKG_BUILD_DIR)/cls_nl80211_vendor.h $(1)/usr/include/cls/
endef

define Package/cls-hdr/install
	# NOP to make sure pkg is installed, it's needed by pkgs who depends on cls-hdr
	:
endef

$(eval $(call BuildPackage,cls-hdr))
