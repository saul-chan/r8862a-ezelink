include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

define Image/Prepare
	if [ ! -s $(SECURE_BOOT_KEY).key -o ! -s $(SECURE_BOOT_KEY).crt ]; then \
		if PATH=$(PATH_WITHOUT_STAGING_HOST) which openssl; then \
			PATH=$(PATH_WITHOUT_STAGING_HOST) $(SCRIPT_DIR)/gen-secure-boot-key.sh $(SECURE_BOOT_KEY); \
		else \
			echo "Please install 'openssl'"; \
			false; \
		fi; \
	fi
endef

define Image/BuildKernel
	$(foreach k,$(filter zImage Image,$(KERNELNAME)), \
		cp $(KDIR)/$(KERNELNAME) $(BIN_DIR)/$(IMG_PREFIX)-$(k) \
	)
endef

define Image/Build/Initramfs
	$(foreach k,$(filter zImage Image,$(KERNELNAME)), \
		cp $(KDIR)/$(k)-initramfs $(BIN_DIR)/$(IMG_PREFIX)-$(k)-initramfs \
	)
endef

define Image/Build/gzip
	gzip -f9n $(BIN_DIR)/$(IMG_ROOTFS)-$(1).img
endef

define Image/Build
	$(call Image/Build/$(1))
endef

define Device/Default
    PROFILES := Default
    KERNEL_DEPENDS = $$(wildcard $(DTS_DIR)/$$(DEVICE_DTS).dts)
    DEVICE_DTS = $$(SOC)-$(lastword $(subst _, ,$(1)))
    DEVICE_DTS_CONFIG := clsemi
    IMAGES := sysupgrade.bin
    IMAGE/sysupgrade.bin = sysupgrade-tar | append-metadata
    IMAGE/sysupgrade.bin/squashfs :=
endef

include $(SUBTARGET).mk

$(eval $(call BuildImage))
