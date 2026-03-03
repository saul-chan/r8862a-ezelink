#!/bin/bash

###
set -e

###CFR硬旁路
devmem 0x4c810008 32 0x02AAAAAA
devmem 0x4c818008 32 0x02AAAAAA

###DPD硬旁路
devmem 0x4c820000 32 0
devmem 0x4c820004 32 0
devmem 0x4c820018 32 0x0
devmem 0x4c82001C 32 0x0
devmem 0x4c820030 32 0x0
devmem 0x4c820034 32 0x0
devmem 0x4c820038 32 0x0
devmem 0x4c82007C 32 0
devmem 0x4c8201C4 32 2
devmem 0x4c8201C8 32 2
devmem 0x4c820284 32 2
devmem 0x4c8203C4 32 0x0
devmem 0x4c8203C8 32 0

devmem 0x4c822000 32 0
devmem 0x4c822004 32 0
devmem 0x4c822018 32 0x0
devmem 0x4c82201C 32 0x0
devmem 0x4c822030 32 0x0
devmem 0x4c822034 32 0x0
devmem 0x4c822038 32 0x0
devmem 0x4c82207C 32 0
devmem 0x4c8221C4 32 2
devmem 0x4c8221C8 32 2
devmem 0x4c822284 32 2
devmem 0x4c8223C4 32 0x0
devmem 0x4c8223C8 32 0


###修改异步FIFO读取等待时间
devmem 0x4c843148 32 2
devmem 0x4c842148 32 2

###修改PHY MDM配置
devmem 0x4AC00804 32 0x185C240d



###星座图功率基准表选择，符号组配置
devmem 0x4AC012A0 32 0xC50

cd /sys/kernel/debug/ieee80211/phy1/cls_wifi/irf

#RX SRC_ST1滤波器系数设置为单位系数，通道0
echo 0x4c846018 0 > irf_set_reg
echo 0x4c84601c 0 > irf_set_reg
echo 0x4c846020 0 > irf_set_reg
echo 0x4c846024 0x3ff > irf_set_reg

#RX SRC_ST1滤波器系数设置为单位系数，通道1
echo 0x4c847018 0 > irf_set_reg
echo 0x4c84701c 0 > irf_set_reg
echo 0x4c847020 0 > irf_set_reg
echo 0x4c847024 0x3ff > irf_set_reg


##RX SRC_ST2滤波器系数设置为单位系数，通道0
echo 0x4c84602c 0 > irf_set_reg
echo 0x4c846030 0 > irf_set_reg
echo 0x4c846034 0 > irf_set_reg
echo 0x4c846038 0 > irf_set_reg
echo 0x4c84603c 0 > irf_set_reg
echo 0x4c846040 0 > irf_set_reg
echo 0x4c846044 0 > irf_set_reg
echo 0x4c846048 0x3ff > irf_set_reg

#RX SRC_ST2滤波器系数设置为单位系数，通道1
echo 0x4c84702c 0 > irf_set_reg
echo 0x4c847030 0 > irf_set_reg
echo 0x4c847034 0 > irf_set_reg
echo 0x4c847038 0 > irf_set_reg
echo 0x4c84703c 0 > irf_set_reg
echo 0x4c847040 0 > irf_set_reg
echo 0x4c847044 0 > irf_set_reg
echo 0x4c847048 0x3ff > irf_set_reg

##RX SRC_ST3滤波器系数设置为单位系数，通道0
echo 0x4c846050 0 > irf_set_reg
echo 0x4c846054 0 > irf_set_reg
echo 0x4c846058 0 > irf_set_reg
echo 0x4c84605c 0 > irf_set_reg
echo 0x4c846060 0 > irf_set_reg
echo 0x4c846064 0 > irf_set_reg
echo 0x4c846068 0 > irf_set_reg
echo 0x4c84606c 0x3ff > irf_set_reg

##RX SRC_ST3滤波器系数设置为单位系数，通道1
echo 0x4c847050 0 > irf_set_reg
echo 0x4c847054 0 > irf_set_reg
echo 0x4c847058 0 > irf_set_reg
echo 0x4c84705c 0 > irf_set_reg
echo 0x4c847060 0 > irf_set_reg
echo 0x4c847064 0 > irf_set_reg
echo 0x4c847068 0 > irf_set_reg
echo 0x4c84706c 0x3ff > irf_set_reg

##RX SRC_ST4滤波器系数设置为单位系数，通道0
echo 0x4c846074 0 > irf_set_reg
echo 0x4c846078 0 > irf_set_reg
echo 0x4c84607c 0 > irf_set_reg
echo 0x4c846080 0 > irf_set_reg
echo 0x4c846084 0 > irf_set_reg
echo 0x4c846088 0 > irf_set_reg
echo 0x4c84608c 0 > irf_set_reg
echo 0x4c846090 0x3ff > irf_set_reg

##RX SRC_ST4滤波器系数设置为单位系数，通道1
echo 0x4c847074 0 > irf_set_reg
echo 0x4c847078 0 > irf_set_reg
echo 0x4c84707c 0 > irf_set_reg
echo 0x4c847080 0 > irf_set_reg
echo 0x4c847084 0 > irf_set_reg
echo 0x4c847088 0 > irf_set_reg
echo 0x4c84708c 0 > irf_set_reg
echo 0x4c847090 0x3ff > irf_set_reg


##RX SRC_ST5滤波器系数设置为单位系数，通道0
echo 0x4c846098 0 > irf_set_reg
echo 0x4c84609c 0 > irf_set_reg
echo 0x4c8460a0 0 > irf_set_reg
echo 0x4c8460a4 0 > irf_set_reg
echo 0x4c8460a8 0 > irf_set_reg
echo 0x4c8460ac 0 > irf_set_reg
echo 0x4c8460b0 0 > irf_set_reg
echo 0x4c8460b4 0x3ff > irf_set_reg

##RX SRC_ST5滤波器系数设置为单位系数，通道1
echo 0x4c847098 0 > irf_set_reg
echo 0x4c84709c 0 > irf_set_reg
echo 0x4c8470a0 0 > irf_set_reg
echo 0x4c8470a4 0 > irf_set_reg
echo 0x4c8470a8 0 > irf_set_reg
echo 0x4c8470ac 0 > irf_set_reg
echo 0x4c8470b0 0 > irf_set_reg
echo 0x4c8470b4 0x3ff > irf_set_reg

echo "============================ $0 done ======================================"
