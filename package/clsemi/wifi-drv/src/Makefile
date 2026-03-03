THIS_DIR := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))
#use following if the make is older than 3.8
#THIS_DIR := $(realpath $(dir $(lastword $(MAKEFILE_LIST))))
-include $(THIS_DIR)/platform.make

CLS_WIFI_VERS_NUM=1.0.0.0

#
# DEBUG OPTIONS
CONFIG_CLS_WIFI_UM_HELPER_DFLT ?= "/usr/bin/cls_wifi_umh.sh"

# Support of P2P DebugFS for enabling/disabling NoA and OppPS
CONFIG_CLS_WIFI_P2P_DEBUGFS ?= y

#
# } // WAITING FOR KCONFIG
#

subdir-ccflags-$(CONFIG_DEBUG_FS) += -DCONFIG_CLS_WIFI_DEBUGFS
subdir-ccflags-$(CONFIG_DEBUG_FS) += -DCONFIG_CLS_WIFI_UM_HELPER_DFLT=\"$(CONFIG_CLS_WIFI_UM_HELPER_DFLT)\"
subdir-ccflags-$(CONFIG_CLS_WIFI_P2P_DEBUGFS) += -DCONFIG_CLS_WIFI_P2P_DEBUGFS

# FW VARS
subdir-ccflags-y += -DCLS_TX_MAX_RATES=4
subdir-ccflags-y += -DCLS_CHAN_CTXT_CNT=3

all: modules
	$(MAKE) -C $(KERNELDIR) O=$(KBUILDDIR) M=$(PWD) modules_install INSTALL_MOD_STRIP=1 INSTALL_MOD_PATH=$(KERNELOUT)

modules clean:
	$(MAKE) -C $(KERNELDIR) O=$(KBUILDDIR) M=$(PWD) $@

# Enable A-MSDU support (need FW support)
## Select this if FW is compiled with AMSDU support
CONFIG_CLS_WIFI_SPLIT_TX_BUF ?= y
## Select this TO send AMSDU
CONFIG_CLS_WIFI_AMSDUS_TX ?= y

# Enable BFMER support (need FW support)
CONFIG_CLS_WIFI_BFMER ?= y

# Support for HE MU (need FW support)
CONFIG_CLS_WIFI_HEMU_TX ?= n

# Compile driver for HE-AP MACHW (need FW support)
CONFIG_CLS_WIFI_MACHW_HE_AP ?= $(CONFIG_CLS_WIFI_HEMU_TX)

ifeq ($(CONFIG_CLS_WIFI_HEMU_TX), y)
CONFIG_CLS_WIFI_USER_MAX ?= 4
else
CONFIG_CLS_WIFI_USER_MAX = 1
endif

# Enable handling of radar event
CONFIG_CLS_WIFI_RADAR ?= y

# Enable HW queue for Broadcast/Multicast traffic (need FW support)
CONFIG_CLS_WIFI_BCMC ?= y

# Enable Monitor+Data interface support (need FW support)
CONFIG_CLS_WIFI_MON_DATA ?= n

# extra DEBUG config
CONFIG_CLS_WIFI_SW_PROFILING ?= n
CONFIG_CLS_WIFI_DBG ?= n

CONFIG_PLATFORM_DUBHE2000 ?= n
CONFIG_PLATFORM_MERAK2000 ?= n
CONFIG_PLATFORM_MERAK3000 ?= n
CONFIG_CLS_3ADDR_BR ?= n
CONFIG_CLS_EMU ?= n
CONFIG_CLS_FPGA ?= n
CONFIG_CLS_FWT ?= y
CONFIG_CLS_DUBHE_ETH ?= y
CONFIG_CLS_DBGCNT_HOST ?= y

ifeq ($(CONFIG_PLATFORM_MERAK3000), y)
CONFIG_CLS_PCI_SHM ?= y
else
CONFIG_CLS_PCI_SHM ?= n
endif

ifeq ($(CONFIG_PLATFORM_DUBHE2000), y)
CONFIG_CLS_WIFI_TX_SCH ?= y
else ifeq ($(CONFIG_PLATFORM_MERAK3000), y)
CONFIG_CLS_WIFI_TX_SCH ?= y
CONFIG_CLS_WIFI_AMSDU_TX_OFFLOAD=0
else
CONFIG_CLS_WIFI_TX_SCH ?= y
endif

ifeq ($(CONFIG_DEBUG_FS), y)
CONFIG_WIFI_TRAFFIC_MONITOR ?= y
endif

obj-m := clsm_wifi.o
clsm_wifi-y := cls_wifi_msg_tx.o		  \
			   cls_wifi_msg_rx.o		  \
			   cls_wifi_utils.o		   \
			   cls_wifi_cmds.o			\
			   cls_wifi_cfgfile.o		 \
			   cls_wifi_strs.o			\
			   cls_wifi_rx.o			  \
			   cls_wifi_tx.o			  \
			   cls_wifi_txq.o			 \
			   cls_wifi_main.o			\
			   cls_wifi_mod_params.o	  \
			   cls_wifi_mesh.o			\
			   cls_wifi_platform.o		  \
			   cls_wifi_cali.o			\
			   cls_wifi_cali_struct_def.o \
			   cls_wifi_cali_debugfs.o	\
			   cls_wifi_version.o		 \
			   cls_wifi_tdls.o			  \
			   ipc_host.o				 \
			   hal_desc.o				 \
			   cls_wifi_irf.o			\
			   vbss.o				 \
			   vendor.o					  \
			   cls_wifi_atf.o                 \
			   cls_wifi_dif_sm.o          \
			   cls_wifi_csi.o             \
			   cls_wifi_power_tbl.o             \
			   clsm_wifi_dfx.o \
			   nac.o \
			   cls_wifi_heartbeat.o


clsm_wifi-$(CONFIG_PLATFORM_DUBHE2000)		+= cls_wifi_afe.o
clsm_wifi-$(CONFIG_PLATFORM_DUBHE2000)		+= cls_wifi_afe_init.o
clsm_wifi-$(CONFIG_PLATFORM_DUBHE2000)		+= cls_wifi_afe_com_top_config.o
clsm_wifi-$(CONFIG_CLS_WIFI_RADAR)		+= cls_wifi_radar.o
clsm_wifi-$(CONFIG_DEBUG_FS)			+= cls_wifi_debugfs.o
clsm_wifi-$(CONFIG_DEBUG_FS)			+= cls_wifi_fw_dump.o
clsm_wifi-$(CONFIG_DEBUG_FS)			+= cls_wifi_fw_trace.o
clsm_wifi-$(CONFIG_WIFI_TRAFFIC_MONITOR)	+= cls_wifi_tm.o
clsm_wifi-$(CONFIG_NL80211_TESTMODE)	+= cls_wifi_testmode.o
clsm_wifi-$(CONFIG_CLS_WIFI_BFMER)		+= cls_wifi_bfmer.o
clsm_wifi-$(CONFIG_CLS_WIFI_HEMU_TX)	+= cls_wifi_he_mu.o
clsm_wifi-$(CONFIG_CLS_MSGQ)			+= cls_wifi_msgq.o
clsm_wifi-y	 += cls_wifi_core.o

ifeq ($(CONFIG_PLATFORM_MERAK2000), y)
obj-m += clsm_wifi_pci.o
clsm_wifi_pci-y := cls_wifi_pci.o
clsm_wifi-$(CONFIG_CLS_PCI_SHM) += cls_wifi_pci_shm.o
endif

ifeq ($(CONFIG_CLS_3ADDR_BR), y)
obj-m += clsm_3addr_br.o
clsm_3addr_br-y := cls_3addr_bridge.o cls_3addr_sta.o
endif

ifeq ($(CONFIG_PLATFORM_DUBHE2000), y)
obj-m += clsm_wifi_soc.o
clsm_wifi_soc-y := cls_wifi_soc.o
endif

ccflags-y += -DCLS_WIFI_VERS_NUM=$(CLS_WIFI_VERS_NUM)
ccflags-y += -I$(src)
ccflags-y += -Werror
ccflags-y += -I$(KERNELDIR) -Wall
ccflags-y += -DCONFIG_CLS_WIFI_AMSDU_TX_OFFLOAD=$(CONFIG_CLS_WIFI_AMSDU_TX_OFFLOAD)
ccflags-$(CONFIG_DYNAMIC_DEBUG) += -DDEBUG
ccflags-$(CONFIG_CLS_WIFI_RADAR) += -DCONFIG_CLS_WIFI_RADAR
ccflags-$(CONFIG_CLS_WIFI_MON_DATA) += -DCONFIG_CLS_WIFI_MON_DATA
ccflags-$(CONFIG_CLS_WIFI_BFMER) += -DCONFIG_CLS_WIFI_BFMER
ccflags-$(CONFIG_CLS_WIFI_SPLIT_TX_BUF) += -DCONFIG_CLS_WIFI_SPLIT_TX_BUF
ifeq ($(CONFIG_CLS_WIFI_SPLIT_TX_BUF), y)
ccflags-$(CONFIG_CLS_WIFI_AMSDUS_TX) += -DCONFIG_CLS_WIFI_AMSDUS_TX
endif
ccflags-$(CONFIG_CLS_WIFI_DBG) += -DCONFIG_CLS_WIFI_DBG
ccflags-$(CONFIG_CLS_WIFI_SW_PROFILING) += -DCONFIG_CLS_WIFI_SW_PROFILING
ccflags-$(CONFIG_CLS_WIFI_HEMU_TX) += -DCONFIG_CLS_WIFI_HEMU_TX
ccflags-$(CONFIG_CLS_WIFI_MACHW_HE_AP) += -DCONFIG_CLS_WIFI_MACHW_HE_AP
ccflags-$(CONFIG_DEBUG_IRF) += -DCONFIG_DEBUG_IRF
ccflags-$(CONFIG_CLS_MSGQ) += -DCONFIG_CLS_MSGQ
ccflags-$(CONFIG_CLS_MSGQ_TEST) += -DCONFIG_CLS_MSGQ_TEST
ccflags-$(CONFIG_PLATFORM_DUBHE2000) += -DDUBHE2000=1
ccflags-$(CONFIG_PLATFORM_MERAK2000) += -DMERAK2000=1
ccflags-$(CONFIG_PLATFORM_MERAK3000) += -DCFG_MERAK3000
ccflags-$(CONFIG_CLS_PCI_SHM) += -DCFG_PCIE_SHM
ifeq ($(CONFIG_PLATFORM_MERAK3000), y)
ccflags-$(CONFIG_CLS_FPGA) += -DCFG_M3K_FPGA
endif
ccflags-$(CONFIG_CLS_3ADDR_BR) += -DCONFIG_CLS_3ADDR_BR=1
ccflags-$(CONFIG_CLS_VBSS) += -DCONFIG_CLS_VBSS
ccflags-$(CONFIG_CLS_EMU) += -DCONFIG_CLS_EMU_ADAPTER=1
ccflags-$(CONFIG_CMCC_TEST) += -DCONFIG_CMCC_TEST
ccflags-$(CONFIG_CLS_FWT) += -DCONFIG_CLS_FWT
ccflags-$(CONFIG_CLS_WIFI_TX_SCH) += -DCONFIG_CLS_WIFI_TX_SCH
ccflags-$(CONFIG_CLS_DUBHE_ETH) += -DCLS_WIFI_DUBHE_ETH
ccflags-$(CONFIG_CLS_DBGCNT_HOST) += -DCLS_WIFI_DBGCNT_HOST
ccflags-$(CONFIG_WIFI_TRAFFIC_MONITOR) += -DCONFIG_WIFI_TRAFFIC_MONITOR
ccflags-$(CONFIG_CLS_NAC) += -DCONFIG_CLS_NAC
ccflags-$(CONFIG_CLS_SMTANT) += -DCONFIG_CLS_SMTANT
ccflags-$(CONFIG_CLS_HIGH_TEMP) += -DCONFIG_CLS_HIGH_TEMP

ifeq ($(CONFIG_CLS_WIFI_BCMC), y)
ccflags-y += -DCLS_TXQ_CNT=5
else
ccflags-y += -DCLS_TXQ_CNT=4
endif

ccflags-y += -DCLS_MU_USER_MAX=$(CONFIG_CLS_WIFI_USER_MAX)
