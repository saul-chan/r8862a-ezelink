include $(TOPDIR)/rules.mk

BOARD:=clsemi
BOARDNAME:=Clourneysemi
FEATURES:=fpu pci rtc usb ramdisk squashfs nand rootfs-part dt-overlay emmc
SUBTARGETS:=cs8862a

KERNEL_PATCHVER:=5.10

include $(INCLUDE_DIR)/target.mk

KERNELNAME:=Image dtbs

DEFAULT_PACKAGES += mkf2fs e2fsprogs cls-img

$(eval $(call BuildTarget))
