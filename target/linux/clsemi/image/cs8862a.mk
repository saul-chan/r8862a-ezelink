UBOOT_SCRIPT_SQUASHFS := $(KDIR)/uboot-script.squashfs
UBOOT_SCRIPT_INITRAMFS := $(KDIR)/uboot-script.initramfs

define Build/cls-img
	mkclsimg -f $@ -s $(realpath ./img-update.sh) -i $(STAGING_DIR_ROOT)/etc/openwrt_cls_info -o $@.new
	mv $@.new $@
endef

define Build/save-file
	rm -f $(1)
	cp $@ $(1)
endef

define Build/gen-squashfs-uboot-script
	rm -f $(UBOOT_SCRIPT_SQUASHFS)-$(DEVICE_MODEL)
	./gen-uboot-script.py $(UBOOT_SCRIPT_SQUASHFS)-$(DEVICE_MODEL) squashfs
endef

define Build/gen-initramfs-uboot-script
	rm -f $(UBOOT_SCRIPT_INITRAMFS)-$(DEVICE_MODEL)
	./gen-uboot-script.py $(UBOOT_SCRIPT_INITRAMFS)-$(DEVICE_MODEL) initramfs
endef

define Build/cls-gpt
       cp $@ $@.tmp 2>/dev/null || true
       ptgen -o $@.tmp -g \
                  -t 0x2e -N fw1 -p 32m@32k \
                  -t 0x2e -N fw2 -p 32m@32800k \
                  -t 0x83 -N rootfs_data -p 64m@65568k
       cat $@.tmp >> $@
       rm $@.tmp
endef

define Build/cls-create-ubi
	sh $(TOPDIR)/scripts/ubinize-image.sh \
		--kernel $(KDIR_TMP)/$(DEVICE_IMG_PREFIX)-$(FILESYSTEMS)-fit.bin \
		--clsfit \
		$(foreach part,$(UBINIZE_PARTS),--part $(part)) \
		$@ \
		-p $(NAND_BLOCKSIZE:%k=%KiB) -m $(NAND_PAGESIZE) \
		$(UBINIZE_OPTS)
endef

define Build/cls-append-fit
       dd if=$(KDIR_TMP)/$(DEVICE_IMG_PREFIX)-$(FILESYSTEMS)-fit.bin conv=sync >> $@
endef

define Device/cs8862a-rdk
    DEVICE_VENDOR := ClourneySemi
    DEVICE_MODEL := CS8862A-RDK
    IMAGE_SIZE := 20m
    KERNEL_NAME := vmlinux
    SUPPORTED_DEVICES += $$(shell awk 'NR>1 {print $$$$2}' supported_devices | xargs -n1 find $$(DTS_DIR)/clourney -name 2>/dev/null | xargs grep -m1 -w compatible | awk -F '"' '{print $$$$2}' | sort -u)
    SOC := $(DEFAULT_SOC)
    NAND_BLOCKSIZE := 128k
    NAND_PAGESIZE := 2048
    DEVICE_DTS_DIR := $(DTS_DIR)/clourney
    DEVICE_DTS := $$(shell awk 'NR>1 {print $$$$2}' supported_devices | xargs echo | sed 's/.dts//g')
    OUTPUT_DTB_SPACE := $$(addprefix $$(KDIR)/image-, $$(addsuffix .dtb, $$(DEVICE_DTS)))
    MULTI_DTB := $$(shell echo $$(OUTPUT_DTB_SPACE) | sed 's/ /?/g')
    KERNEL := kernel-bin | lzma
    KERNEL_INITRAMFS := gen-initramfs-uboot-script | kernel-bin | lzma | \
        fit_sign lzma $$(MULTI_DTB) none $$(UBOOT_SCRIPT_INITRAMFS)-$$(DEVICE_MODEL)
    KERNEL_INITRAMFS_SUFFIX := -firmware
    KERNEL_LOADADDR := 0x8000000
    FILESYSTEMS := squashfs
    IMAGES := fit.bin
    IMAGES += ubi-firmware
    IMAGES += emmc-factory.bin
    IMAGES += sysupgrade.bin
    CLS_FIRST_IMAGE := fit.bin
    KERNEL_INITRAMFS_SUFFIX := -firmware
    IMAGE/fit.bin := gen-squashfs-uboot-script | kernel-bin | fit_sign lzma $$(MULTI_DTB) external-static-with-rootfs $$(UBOOT_SCRIPT_SQUASHFS)-$$(DEVICE_MODEL)
    IMAGE/ubi-firmware := cls-create-ubi | save-file $$(BIN_DIR)/$$(DEVICE_IMG_PREFIX)-ubi-firmware
    IMAGE/emmc-factory.bin := cls-gpt | pad-to 32k | cls-append-fit | pad-to 32800k | cls-append-fit
    IMAGE/sysupgrade.bin := cls-append-fit | check-size | cls-img | append-metadata
endef
TARGET_DEVICES += cs8862a-rdk
