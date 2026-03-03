# SPDX-License-Identifier: GPL-2.0
#
# Makefile for the Clourneysemi network device drivers.
#

obj-$(CONFIG_CLS_NPE) += npe/
#obj-y += rtl8367/

ifeq ($(CONFIG_CLS_PHYLINK)$(CONFIG_CLS_RTKSDK),yy)
obj-$(CONFIG_CLS_PHYLINK) += rtlphy_sdk/
endif

obj-$(CONFIG_CLS_PHYLINK) += yt8614/

ifeq ($(CONFIG_CLS_PHYLINK)$(CONFIG_CLS_YTSWITCH),yy)
obj-y += yt_switch/
endif
