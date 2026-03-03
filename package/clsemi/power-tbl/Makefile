include $(TOPDIR)/rules.mk

PKG_NAME:=power-tbl
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/power-tbl
  SECTION:=clsemi
  CATEGORY:=ClourneySemi
  TITLE:=ClourneySemi Power Table tool
endef

define Build/Compile
	(cd $(PKG_BUILD_DIR) && CROSS_COMPILE=$(TARGET_CROSS) ./build.sh)
endef

define Package/power-tbl/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/power_tbl $(1)/usr/sbin
endef

$(eval $(call BuildPackage,power-tbl))
