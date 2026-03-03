include $(TOPDIR)/rules.mk

PKG_NAME:=slkgps
PKG_VERSION:=1.0
PKG_BUILD_DIR:= $(BUILD_DIR)/$(PKG_NAME)


include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk

define Package/slkgps
  CATEGORY:=Examples
  TITLE:=slkgps sdk
  DEPENDS:=+libcurl
endef

define Package/slkgps/description
	If you can't figure out what this program does, you're probably  
	brain-dead and need immediate medical attention.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/slkgps/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/slkgpsinit $(1)/etc/init.d/slkgpsinit
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/slkgps $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,slkgps))
