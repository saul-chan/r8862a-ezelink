include $(TOPDIR)/rules.mk

PKG_NAME:=esw-cmd
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/esw-cmd
  SECTION:=clsemi
  CATEGORY:=ClourneySemi
  TITLE:=ClourneySemi external switch cmd tool
endef

define Build/Compile
	(cd $(PKG_BUILD_DIR) && CROSS_COMPILE=$(TARGET_CROSS) ./build.sh)
endef

define Package/esw-cmd/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/* $(1)/usr/sbin
endef

$(eval $(call BuildPackage,esw-cmd))
