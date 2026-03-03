include $(TOPDIR)/rules.mk

PKG_NAME:=cls-memload
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/cls-memload
  SECTION:=clsemi
  CATEGORY:=ClourneySemi
  TITLE:=ClourneySemi memload tool
endef

define Build/Compile
	(cd $(PKG_BUILD_DIR) && CROSS_COMPILE=$(TARGET_CROSS) ./build.sh)
endef

define Package/cls-memload/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/memload $(1)/usr/bin
endef

$(eval $(call BuildPackage,cls-memload))
