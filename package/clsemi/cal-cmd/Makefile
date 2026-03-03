include $(TOPDIR)/rules.mk

PKG_NAME:=cal-cmd
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/cal-cmd
  SECTION:=clsemi
  CATEGORY:=ClourneySemi
  TITLE:=ClourneySemi calibration tool
endef

define Package/cal-cmd/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/calcmd $(1)/usr/sbin
	$(CP) ./files/* $(1)/
endef

$(eval $(call BuildPackage,cal-cmd))
