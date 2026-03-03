#
# Copyright (c) 2021-2024, Clourney Semiconductor. All rights reserved.
#
# This software and/or documentation is licensed by Clourney Semiconductor under limited terms and conditions.
# Reproduction and redistribution in binary or source form, with or without modification,
# for use solely in conjunction with a Clourney Semiconductor chipset, is permitted in condition which
# must retain the above copyright notice.
#
# By using this software and/or documentation, you agree to the limited terms and conditions.
#

include $(TOPDIR)/rules.mk

LUCI_SECTION=clsemi
LUCI_CATEGORY=ClourneySemi
LUCI_SUBMENU_FORCED=none
LUCI_TITLE:=LuCI for ClourneySemi Ultra QoS
LUCI_DEPENDS:=+luci-base +cls-qos

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature
