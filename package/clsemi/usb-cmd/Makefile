include $(TOPDIR)/rules.mk

PKG_NAME:=usb-cmd
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/usb-cmd
  SECTION:=clsemi
  CATEGORY:=ClourneySemi
  TITLE:=ClourneySemi usbcmd tool
endef

define Build/Compile
	(cd $(PKG_BUILD_DIR) && CROSS_COMPILE=$(TARGET_CROSS) ./build.sh)
endef

define Package/usb-cmd/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(CP) $(PKG_BUILD_DIR)/bin/* $(1)/usr/sbin
endef

$(eval $(call BuildPackage,usb-cmd))
