include $(TOPDIR)/rules.mk

PKG_NAME:=npe-cmd
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/npe-cmd
  SECTION:=clsemi
  CATEGORY:=ClourneySemi
  TITLE:=ClourneySemi NPE tool
endef

define Build/Compile
	(cd $(PKG_BUILD_DIR) && CROSS_COMPILE=$(TARGET_CROSS) ./build.sh dubhe2000)
endef

define Package/npe-cmd/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/npecmd $(1)/usr/sbin
endef

$(eval $(call BuildPackage,npe-cmd))
