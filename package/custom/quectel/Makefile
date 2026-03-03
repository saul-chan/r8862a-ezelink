include $(TOPDIR)/rules.mk

PKG_NAME:=quectel
PKG_VERSION:=1.6.4
PKG_BUILD_DIR:= $(BUILD_DIR)/$(PKG_NAME)


include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk

define Package/quectel
  CATEGORY:=Examples
  TITLE:=Quectel_QConnectManager_Linux_V1.6.4
  DEPENDS:=
endef

define Package/quectel/description
	If you can't figure out what this program does, you're probably  
	brain-dead and need immediate medical attention.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/quectel/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/quectel-atc-proxy $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/quectel-mbim-proxy $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/quectel-qmi-proxy $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/quectel-CM $(1)/usr/sbin/
endef


$(eval $(call BuildPackage,quectel))
