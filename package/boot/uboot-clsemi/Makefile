include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_VERSION:=2022.01
PKG_RELEASE:=$(AUTORELEASE)
PKG_SOURCE_PROTO:=none

include $(INCLUDE_DIR)/u-boot.mk
include $(INCLUDE_DIR)/package.mk

define U-Boot/Default
  BUILD_TARGET:=clsemi
  UBOOT_IMAGE:=u-boot.bin
endef

define U-Boot/cs8862a-demo-a
  BUILD_DEVICES:=cs8862a-rdk
  BUILD_SUBTARGET:=cs8862a
  NAME:=ClourneySemi CS8862A DEMO-A
  UBOOT_CONFIG:=dubhe2000_demo-a
endef

define U-Boot/cs8862a-demo-b
  BUILD_DEVICES:=cs8862a-rdk
  BUILD_SUBTARGET:=cs8862a
  NAME:=ClourneySemi CS8862A DEMO-B
  UBOOT_CONFIG:=dubhe2000_demo-b
endef

define U-Boot/cs8862a-demo-o
  BUILD_DEVICES:=cs8862a-rdk
  BUILD_SUBTARGET:=cs8862a
  NAME:=ClourneySemi CS8862A DEMO-O
  UBOOT_CONFIG:=dubhe2000_demo-o
endef

define U-Boot/cs8862a-demo-r
  BUILD_DEVICES:=cs8862a-rdk
  BUILD_SUBTARGET:=cs8862a
  NAME:=ClourneySemi CS8862A DEMO-R
  UBOOT_CONFIG:=dubhe2000_demo-r
endef

define U-Boot/cs8862a-evb-a
  BUILD_DEVICES:=cs8862a-rdk
  BUILD_SUBTARGET:=cs8862a
  NAME:=ClourneySemi CS8862A EVB-A
  UBOOT_CONFIG:=dubhe2000_evb-a
endef

define U-Boot/cs8862a-evb-b
  BUILD_DEVICES:=cs8862a-rdk
  BUILD_SUBTARGET:=cs8862a
  NAME:=ClourneySemi CS8862A EVB-B
  UBOOT_CONFIG:=dubhe2000_evb-b
endef

define U-Boot/cs8862a-rdk
  BUILD_DEVICES:=cs8862a-rdk
  BUILD_SUBTARGET:=cs8862a
  NAME:=ClourneySemi CS8862A RDK
  UBOOT_CONFIG:=dubhe2000_rdk
endef

define U-Boot/cs8862a-rdk-v24
  BUILD_DEVICES:=cs8862a-rdk
  BUILD_SUBTARGET:=cs8862a
  NAME:=ClourneySemi CS8862A RDK-V24
  UBOOT_CONFIG:=dubhe2000_rdk-v24
endef

CLS_CSMEU_TOOL=csmeu_plus
CLS_UBOOT_KEY_FILE=rot_dbg_rsa2048.pem
UBOOT_MAKE_FLAGS+=BUILD_UBOOT_MFST=y
UBOOT_MAKE_FLAGS+=CLS_CSMEU_TOOL=$(CLS_CSMEU_TOOL)
UBOOT_MAKE_FLAGS+=CLS_UBOOT_KEY_FILE=$(CLS_UBOOT_KEY_FILE)
UBOOT_MAKE_FLAGS+=CLS_UBOOT_KEY=1234567812345678

ifeq ($(CONFIG_UBOOT_CHECK_FIT_SIGN-$(BUILD_VARIANT)),y)
UBOOT_CONFIG_FILE=$(PKG_BUILD_DIR)/configs/$(UBOOT_CONFIG)_defconfig
UBOOT_DTS_FILE=$(PKG_BUILD_DIR)/arch/arm/dts/clourneysemi-dubhe2000-$(subst dubhe2000_,,$(UBOOT_CONFIG)).dts
endif

UBOOT_TARGETS := \
	cs8862a-demo-a \
	cs8862a-demo-b \
	cs8862a-demo-o \
	cs8862a-demo-r \
	cs8862a-evb-a \
	cs8862a-evb-b \
	cs8862a-rdk \
	cs8862a-rdk-v24

NOW_TS := $(shell date +%s)
UTC_TS := $(shell echo $$(( $(NOW_TS) + 8*3600 )))
SOURCE_DATE_EPOCH := ${UTC_TS}

ifeq ($(CONFIG_UBOOT_CHECK_FIT_SIGN-$(BUILD_VARIANT)),y)
define Build/Configure
	if [ ! -s $(SECURE_BOOT_KEY).key -o ! -s $(SECURE_BOOT_KEY).crt ]; then \
		if PATH=$(PATH_WITHOUT_STAGING_HOST) which openssl; then \
			PATH=$(PATH_WITHOUT_STAGING_HOST) $(SCRIPT_DIR)/gen-secure-boot-key.sh $(SECURE_BOOT_KEY); \
		else \
			echo "Please install 'openssl'"; \
			false; \
		fi; \
	fi
	$(DTC) -I dts -O dtb dummy.dts > $(PKG_BUILD_DIR)/dummy.dtb
	$(CP) dummy.its $(PKG_BUILD_DIR)/dummy.its
	sed -i "s/\$$$${key_name}/$(notdir $(SECURE_BOOT_KEY))/" $(PKG_BUILD_DIR)/dummy.its
	$(STAGING_DIR_HOST)/bin/mkimage -f $(PKG_BUILD_DIR)/dummy.its -k $(dir $(SECURE_BOOT_KEY)) -K $(PKG_BUILD_DIR)/dummy.dtb $(PKG_BUILD_DIR)/dummy.img
	$(DTC) -I dtb -O dts $(PKG_BUILD_DIR)/dummy.dtb > $(PKG_BUILD_DIR)/dummy.dts
	sed -i '1d' $(PKG_BUILD_DIR)/dummy.dts
	awk '/algo = "sha1,rsa2048"/{print "\t\t\trequired = \"conf\";"}1' $(PKG_BUILD_DIR)/dummy.dts >> $(PKG_BUILD_DIR)/cls-sign-check.dts
	cat $(PKG_BUILD_DIR)/cls-sign-check.dts >> $(UBOOT_DTS_FILE)
	echo CONFIG_FIT_SIGNATURE=y >> $(UBOOT_CONFIG_FILE)
	echo CONFIG_FIT_SIGNATURE_MAX_SIZE=0x10000000 >> $(UBOOT_CONFIG_FILE)
	echo CONFIG_ECDSA=y >> $(UBOOT_CONFIG_FILE)
	echo CONFIG_ECDSA_VERIFY=y >> $(UBOOT_CONFIG_FILE)
	echo CONFIG_RSA=y >> $(UBOOT_CONFIG_FILE)
	echo CONFIG_RSA_VERIFY=y >> $(UBOOT_CONFIG_FILE)
	echo CONFIG_RSA_SOFTWARE_EXP=y >> $(UBOOT_CONFIG_FILE)
	$(call Build/Configure/U-Boot)
endef
endif

define Package/u-boot/install
	$(CP) $(patsubst %,$(PKG_BUILD_DIR)/%,UBOOT_MFST.bin) $(1)/
endef

define Build/InstallDev
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/$(UBOOT_IMAGE) $(STAGING_DIR_IMAGE)/$(VARIANT)-$(UBOOT_IMAGE)
endef

$(eval $(call BuildPackage/U-Boot))
