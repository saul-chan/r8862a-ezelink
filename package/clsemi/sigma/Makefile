include $(TOPDIR)/rules.mk

PKG_NAME:=sigma
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/sigma
  SECTION:=clsemi
  CATEGORY:=ClourneySemi
  TITLE:=ClourneySemi sigma tool
  DEPENDS:=+libpthread +libubox +libubus
endef

MAKE_PATH = clourneysemi

define Package/sigma/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/$(MAKE_PATH)/cls_ca $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/$(MAKE_PATH)/cls_dut $(1)/usr/sbin
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/cls-wfa.init $(1)/etc/init.d/cls-wfa
endef

$(eval $(call BuildPackage,sigma))
