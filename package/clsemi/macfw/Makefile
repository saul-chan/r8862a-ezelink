include $(TOPDIR)/rules.mk

HWPLAT?=EVB-A
PKG_NAME:=macfw
PKG_RELEASE:=1
BUILD_SH:=build_dubhe2000.sh
WIFI0_FW:=out/fwmac_2G4/fmac_arc_karst/fmacfw.bin
WIFI1_FW:=out/fwmac_5G/fmac_arc_karst/fmacfw.bin
WIFI_CMN_FW:=out/fwmac_CMN/fmac_arc_karst/fmacfw.bin

include $(INCLUDE_DIR)/package.mk

define Package/macfw
  SECTION:=clsemi
  CATEGORY:=ClourneySemi
  TITLE:=ClourneySemi Wi-Fi MAC Firmware
endef

define Build/Compile
endef

define Package/macfw/install
	$(INSTALL_DIR) $(1)/lib/firmware
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/$(WIFI0_FW) $(1)/lib/firmware/wifi0.bin
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/$(WIFI1_FW) $(1)/lib/firmware/wifi1.bin
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/$(WIFI_CMN_FW) $(1)/lib/firmware/wificmn.bin
endef

$(eval $(call BuildPackage,macfw))
