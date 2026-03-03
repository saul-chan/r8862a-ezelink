#!/bin/bash
soc_debug_static() {
	echo "#### close soc_debug: START!"

	echo "## Step1: reset ATB、CTI、CTM、ETF、ETR"
	echo "# Setting CPU_SUB_RST_PARA_ADDR(0x90442800), bit 6 to clear"
	CPU_SUB_RST_PARA_ADDR=0x90442800
	orig_value=$(devmem $CPU_SUB_RST_PARA_ADDR)
	mask=0xFFFFFFFFFFFFFFBF
	new_value=$(($orig_value & $mask))
	echo "# update $CPU_SUB_RST_PARA_ADDR: $orig_value ->  $new_value"
	devmem $CPU_SUB_RST_PARA_ADDR 32 $new_value

	echo "## Step2: close ATB_S clock"
	echo "# Setting CPU_SUB_CG_PARA_ADDR(0x90443000), bit 3 to clear"
	CPU_SUB_CG_PARA_ADDR=0x90443000
	orig_value=$(devmem $CPU_SUB_CG_PARA_ADDR)
	mask=0xFFFFFFFFFFFFFFF7
	new_value=$(($orig_value & $mask))
	echo "# update $CPU_SUB_CG_PARA_ADDR: $orig_value ->  $new_value"
	devmem $CPU_SUB_CG_PARA_ADDR 32 $new_value

	echo "## Step3: close ATB_M clock"
	echo "# Setting CPU_SUB_CG_PARA_ADDR(0x90443000), bit 5 to clear"
	CPU_SUB_CG_PARA_ADDR=0x90443000
	orig_value=$(devmem $CPU_SUB_CG_PARA_ADDR)
	mask=0xFFFFFFFFFFFFFFDF
	new_value=$(($orig_value & $mask))
	echo "# update $CPU_SUB_CG_PARA_ADDR: $orig_value ->  $new_value"
	devmem $CPU_SUB_CG_PARA_ADDR 32 $new_value

	echo "## Step4: close CTI、CTM、ETF、ETR"
	echo "# Setting PERI_SUB_CG_PARA_ADDR(0x90443008), bit 0 to clear"
	PERI_SUB_CG_PARA_ADDR=0x90443008
	orig_value=$(devmem $PERI_SUB_CG_PARA_ADDR)
	mask=0xFFFFFFFFFFFFFFFE
	new_value=$(($orig_value & $mask))
	echo "# update $PERI_SUB_CG_PARA_ADDR: $orig_value ->  $new_value"
	devmem $PERI_SUB_CG_PARA_ADDR 32 $new_value

	echo "## Step5: ETR RAM shutdown"
	echo "# Setting PERI_LSYS_IP_RAM_POWER0_ADDR(0x90000088), bit 1 to set"
	PERI_LSYS_IP_RAM_POWER0_ADDR=0x90000088
	orig_value=$(devmem $PERI_LSYS_IP_RAM_POWER0_ADDR)
	mask=0x00000002
	new_value=$((orig_value | $mask))
	echo "# update $PERI_LSYS_IP_RAM_POWER0_ADDR: $orig_value ->  $new_value"
	devmem $PERI_LSYS_IP_RAM_POWER0_ADDR 32 $new_value

	echo "#### close soc_debug: END!"
}

pcie_static() {
	echo "#### close pcie: START!"

	echo "## Step1: close PCIE clock"
	echo "# Setting PERI_SUB_CG_PARA0_ADDR(0x90443008), bit 9 to clear"
	PERI_SUB_CG_PARA0_ADDR=0x90443008
	orig_value=$(devmem $PERI_SUB_CG_PARA0_ADDR)
	mask=0xFFFFFFFFFFFFFDFF
	new_value=$(($orig_value & $mask))
	echo "# update $PERI_SUB_CG_PARA0_ADDR: $orig_value ->  $new_value"
	devmem $PERI_SUB_CG_PARA0_ADDR 32 $new_value

	echo "## Step2: PCIE SP RAM shutdown"
	echo "# Setting PERI_LSYS_IP_RAM_POWER1_ADDR(0x9000008c), bit 0 to set"
	PERI_LSYS_IP_RAM_POWER1_ADDR=0x9000008c
	orig_value=$(devmem $PERI_LSYS_IP_RAM_POWER1_ADDR)
	mask=0x00000001
	new_value=$((orig_value | $mask))
	echo "# update $PERI_LSYS_IP_RAM_POWER1_ADDR: $orig_value ->  $new_value"
	devmem $PERI_LSYS_IP_RAM_POWER1_ADDR 32 $new_value

	echo "## Step3: PCIE TP RAM shutdown"
	echo "# Setting PERI_LSYS_IP_RAM_POWER1_ADDR(0x9000008c), bit 8 to set"
	PERI_LSYS_IP_RAM_POWER1_ADDR=0x9000008c
	orig_value=$(devmem $PERI_LSYS_IP_RAM_POWER1_ADDR)
	mask=0x00000100
	new_value=$((orig_value | $mask))
	echo "# update $PERI_LSYS_IP_RAM_POWER1_ADDR: $orig_value ->  $new_value"
	devmem $PERI_LSYS_IP_RAM_POWER1_ADDR 32 $new_value

	echo "## Step4: set pciephy_ref_usd_pad"
	echo "# Setting PERI_LSYS_PCIE_PHY_CLK_CONTRL_ADDR(0x900C0828), bit 1 to set"
	PERI_LSYS_PCIE_PHY_CLK_CONTRL_ADDR=0x900C0828
	orig_value=$(devmem $PERI_LSYS_PCIE_PHY_CLK_CONTRL_ADDR)
	mask=0x00000002
	new_value=$((orig_value | $mask))
	echo "# update $PERI_LSYS_PCIE_PHY_CLK_CONTRL_ADDR: $orig_value ->  $new_value"
	devmem $PERI_LSYS_PCIE_PHY_CLK_CONTRL_ADDR 32 $new_value

	echo "#### close pcie: END!"
}

usb_static() {
	echo "#### close usb: START!"

	echo "## Step1: reset usb"
	echo "# Setting PERI_SUB_RST_PARA0_ADDR(0x90442808), bit 3 to clear"
	PERI_SUB_RST_PARA0_ADDR=0x90442808
	orig_value=$(devmem $PERI_SUB_RST_PARA0_ADDR)
	mask=0xFFFFFFFFFFFFFFF7
	new_value=$(($orig_value & $mask))
	echo "# update $PERI_SUB_RST_PARA0_ADDR: $orig_value ->  $new_value"
	devmem $PERI_SUB_RST_PARA0_ADDR 32 $new_value

	echo "## Step2: close usb clock"
	echo "# Setting PERI_SUB_CG_PARA1_ADDR(0x9044300c), bit 3 to clear"
	PERI_SUB_CG_PARA1_ADDR=0x9044300c
	orig_value=$(devmem $PERI_SUB_CG_PARA1_ADDR)
	mask=0xFFFFFFFFFFFFFFF7
	new_value=$(($orig_value & $mask))
	echo "# update $PERI_SUB_CG_PARA1_ADDR: $orig_value ->  $new_value"
	devmem $PERI_SUB_CG_PARA1_ADDR 32 $new_value

	echo "## Step3: USB SP RAM shutdown"
	echo "# Setting PERI_LSYS_IP_RAM_POWER0_ADDR(0x90000088), bit 8 to set"
	PERI_LSYS_IP_RAM_POWER0_ADDR=0x90000088
	orig_value=$(devmem $PERI_LSYS_IP_RAM_POWER0_ADDR)
	mask=0x00000100
	new_value=$((orig_value | $mask))
	echo "# update $PERI_LSYS_IP_RAM_POWER0_ADDR: $orig_value ->  $new_value"
	devmem $PERI_LSYS_IP_RAM_POWER0_ADDR 32 $new_value

	echo "## Step4: reset USB PHY"
	echo "# Setting PERI_SUB_RST_PARA0_ADDR(0x90442808), bit 4 to clear"
	PERI_SUB_RST_PARA0_ADDR=0x90442808
	orig_value=$(devmem $PERI_SUB_RST_PARA0_ADDR)
	mask=0xFFFFFFFFFFFFFFEF
	new_value=$(($orig_value & $mask))
	echo "# update $PERI_SUB_RST_PARA0_ADDR: $orig_value ->  $new_value"
	devmem $PERI_SUB_RST_PARA0_ADDR 32 $new_value

	echo "## Step5: test_powerdown_ssp"
	echo "# Setting USB_LSYS_PHY_MISC3_ADDR(0x900A0028), bit 10 to set"
	USB_LSYS_PHY_MISC3_ADDR=0x900A0028
	orig_value=$(devmem $USB_LSYS_PHY_MISC3_ADDR)
	mask=0x00000400
	new_value=$((orig_value | $mask))
	echo "# update $USB_LSYS_PHY_MISC3_ADDR: $orig_value ->  $new_value"
	devmem $USB_LSYS_PHY_MISC3_ADDR 32 $new_value

	echo "## Step6: test_powerdown_hsp"
	echo "# Setting USB_LSYS_PHY_MISC3_ADDR(0x900A0028), bit 11 to set"
	USB_LSYS_PHY_MISC3_ADDR=0x900A0028
	orig_value=$(devmem $USB_LSYS_PHY_MISC3_ADDR)
	mask=0x00000800
	new_value=$((orig_value | $mask))
	echo "# update $USB_LSYS_PHY_MISC3_ADDR: $orig_value ->  $new_value"
	devmem $USB_LSYS_PHY_MISC3_ADDR 32 $new_value

	echo "#### close usb: END!"
}

pnand_static() {
	echo "#### close pand: START!"

	echo "## Step1: reset pnand"
	echo "# Setting PERI_SUB_RST_PARA0_ADDR(0x90442808), bit 6 to clear"
	PERI_SUB_RST_PARA0_ADDR=0x90442808
	orig_value=$(devmem $PERI_SUB_RST_PARA0_ADDR)
	mask=0xFFFFFFFFFFFFFFBF
	new_value=$(($orig_value & $mask))
	echo "# update $PERI_SUB_RST_PARA0_ADDR: $orig_value ->  $new_value"
	devmem $PERI_SUB_RST_PARA0_ADDR 32 $new_value

	echo "## Step2: close pnand clock"
	echo "# Setting PERI_SUB_CG_PARA0_ADDR(0x90443008), bit 11 to clear"
	PERI_SUB_CG_PARA0_ADDR=0x90443008
	orig_value=$(devmem $PERI_SUB_CG_PARA0_ADDR)
	mask=0xFFFFFFFFFFFFF7FF
	new_value=$(($orig_value & $mask))
	echo "# update $PERI_SUB_CG_PARA0_ADDR: $orig_value ->  $new_value"
	devmem $PERI_SUB_CG_PARA0_ADDR 32 $new_value

	echo "## Step3: PNAND SP RAM shutdown"
	echo "# Setting PERI_LSYS_IP_RAM_POWER2_ADDR(0x90000090), bit 0 to set"
	PERI_LSYS_IP_RAM_POWER2_ADDR=0x90000090
	orig_value=$(devmem $PERI_LSYS_IP_RAM_POWER2_ADDR)
	mask=0x00000001
	new_value=$((orig_value | $mask))
	echo "# update $PERI_LSYS_IP_RAM_POWER2_ADDR: $orig_value ->  $new_value"
	devmem $PERI_LSYS_IP_RAM_POWER2_ADDR 32 $new_value

	echo "## Step4: PNAND DP RAM shutdown"
	echo "# Setting PERI_LSYS_IP_RAM_POWER2_ADDR(0x90000090), bit 8 to set"
	PERI_LSYS_IP_RAM_POWER2_ADDR=0x90000090
	orig_value=$(devmem $PERI_LSYS_IP_RAM_POWER2_ADDR)
	mask=0x00000100
	new_value=$((orig_value | $mask))
	echo "# update $PERI_LSYS_IP_RAM_POWER2_ADDR: $orig_value ->  $new_value"
	devmem $PERI_LSYS_IP_RAM_POWER2_ADDR 32 $new_value

	echo "#### close pand: END!"
}

sdemmc_static() {
	echo "#### close sdemmc: START!"

	echo "## Step1: reset sdemmc"
	echo "# Setting PERI_SUB_RST_PARA0_ADDR(0x90442808), bit 8 to clear"
	PERI_SUB_RST_PARA0_ADDR=0x90442808
	orig_value=$(devmem $PERI_SUB_RST_PARA0_ADDR)
	mask=0xFFFFFFFFFFFFFEFF
	new_value=$(($orig_value & $mask))
	echo "# update $PERI_SUB_RST_PARA0_ADDR: $orig_value ->  $new_value"
	devmem $PERI_SUB_RST_PARA0_ADDR 32 $new_value

	echo "## Step2: close sdemmc clock"
	echo "# Setting PERI_SUB_CG_PARA0_ADDR(0x90443008), bit 18 to clear"
	PERI_SUB_CG_PARA0_ADDR=0x90443008
	orig_value=$(devmem $PERI_SUB_CG_PARA0_ADDR)
	mask=0xFFFFFFFFFFFBFFFF
	new_value=$(($orig_value & $mask))
	echo "# update $PERI_SUB_CG_PARA0_ADDR: $orig_value ->  $new_value"
	devmem $PERI_SUB_CG_PARA0_ADDR 32 $new_value

	echo "## Step3: SDEMMC SP RAM shutdown"
	echo "# Setting PERI_LSYS_IP_RAM_POWER0_ADDR(0x90000088), bit 16 to clear"
	PERI_LSYS_IP_RAM_POWER0_ADDR=0x90000088
	orig_value=$(devmem $PERI_LSYS_IP_RAM_POWER0_ADDR)
	mask=0xFFFFFFFFFFFEFFFF
	new_value=$(($orig_value & $mask))
	echo "# update $PERI_LSYS_IP_RAM_POWER0_ADDR: $orig_value ->  $new_value"
	devmem $PERI_LSYS_IP_RAM_POWER0_ADDR 32 $new_value

	echo "#### close sdemmc: END!"
}

spacc_static() {
	echo "#### close spacc: START!"

	echo "## Step1: reset spacc"
	echo "# Setting PERI_SUB_RST_PARA0_ADDR(0x90442808), bit 20 to clear"
	PERI_SUB_RST_PARA0_ADDR=0x90442808
	orig_value=$(devmem $PERI_SUB_RST_PARA0_ADDR)
	mask=0xFFFFFFFFFFEFFFFF
	new_value=$(($orig_value & $mask))
	echo "# update $PERI_SUB_RST_PARA0_ADDR: $orig_value ->  $new_value"
	devmem $PERI_SUB_RST_PARA0_ADDR 32 $new_value

	echo "## Step2: close spacc clock"
	echo "# Setting PERI_SUB_CG_PARA0_ADDR(0x90443008), bit 20 to clear"
	PERI_SUB_CG_PARA0_ADDR=0x90443008
	orig_value=$(devmem $PERI_SUB_CG_PARA0_ADDR)
	mask=0xFFFFFFFFFFEFFFFF
	new_value=$(($orig_value & $mask))
	echo "# update $PERI_SUB_CG_PARA0_ADDR: $orig_value ->  $new_value"
	devmem $PERI_SUB_CG_PARA0_ADDR 32 $new_value

	echo "## Step3: SPACC SP RAM shutdown"
	echo "# Setting PERI_LSYS_IP_RAM_POWER2_ADDR(0x90000090), bit 16 to set"
	PERI_LSYS_IP_RAM_POWER2_ADDR=0x90000090
	orig_value=$(devmem $PERI_LSYS_IP_RAM_POWER2_ADDR)
	mask=0x00010000
	new_value=$((orig_value | $mask))
	echo "# update $PERI_LSYS_IP_RAM_POWER2_ADDR: $orig_value ->  $new_value"
	devmem $PERI_LSYS_IP_RAM_POWER2_ADDR 32 $new_value

	echo "## Step4: SPACC TP RAM shutdown"
	echo "# Setting PERI_LSYS_IP_RAM_POWER2_ADDR(0x90000090), bit 24 to set"
	PERI_LSYS_IP_RAM_POWER2_ADDR=0x90000090
	orig_value=$(devmem $PERI_LSYS_IP_RAM_POWER2_ADDR)
	mask=0x01000000
	new_value=$((orig_value | $mask))
	echo "# update $PERI_LSYS_IP_RAM_POWER2_ADDR: $orig_value ->  $new_value"
	devmem $PERI_LSYS_IP_RAM_POWER2_ADDR 32 $new_value

	echo "#### close spacc: END!"
}

soc_ip_static() {
	echo "#### close soc_ip: START!"

	#soc_debug_static
	pcie_static
	usb_static
	pnand_static
	sdemmc_static
	spacc_static
	echo "#### close soc_ip: END!"
}

fwd_serder_static() {
	echo "#### close serdes: START!"

	echo "## Step1: close serdes0 clock"
	echo "# Setting FWD_SUB_CG_PARA0_ADDR(0x9044304c), bit 1 to clear"
	FWD_SUB_CG_PARA0_ADDR=0x9044304c
	orig_value=$(devmem $FWD_SUB_CG_PARA0_ADDR)
	mask=0xFFFFFFFFFFFFFFFD
	new_value=$(($orig_value & $mask))
	echo "# update $FWD_SUB_CG_PARA0_ADDR: $orig_value ->  $new_value"
	devmem $FWD_SUB_CG_PARA0_ADDR 32 $new_value

	echo "## Step2: close serdes1 clock"
	echo "# Setting FWD_SUB_CG_PARA0_ADDR(0x9044304c), bit 2 to clear"
	FWD_SUB_CG_PARA0_ADDR=0x9044304c
	orig_value=$(devmem $FWD_SUB_CG_PARA0_ADDR)
	mask=0xFFFFFFFFFFFFFFFB
	new_value=$(($orig_value & $mask))
	echo "# update $FWD_SUB_CG_PARA0_ADDR: $orig_value ->  $new_value"
	devmem $FWD_SUB_CG_PARA0_ADDR 32 $new_value

	echo "#### close serdes: END!"
}

radio_2g_static() {
	echo "#### close 2G: START!"

	echo "## Step1: close wifi 2g clock"
	echo "# Setting WIFI_2G_SUB_CG_PARA0_ADDR(0x90443018), bit 0 to clear"
	WIFI_2G_SUB_CG_PARA0_ADDR=0x90443018
	orig_value=$(devmem $WIFI_2G_SUB_CG_PARA0_ADDR)
	mask=0xFFFFFFFFFFFFFFFE
	new_value=$(($orig_value & $mask))
	echo "# update $WIFI_2G_SUB_CG_PARA0_ADDR: $orig_value ->  $new_value"
	devmem $WIFI_2G_SUB_CG_PARA0_ADDR 32 $new_value

	echo "## Step2: close wifi 2g sp ram"
	echo "# Setting WIFI_2G_SUB_RST_PARA_ADDR(0x90443800), bit 0 to set"
	WIFI_2G_SUB_RST_PARA_ADDR=0x90443800
	orig_value=$(devmem $WIFI_2G_SUB_RST_PARA_ADDR)
	mask=0x00000001
	new_value=$((orig_value | $mask))
	echo "# update $WIFI_2G_SUB_RST_PARA_ADDR: $orig_value ->  $new_value"
	devmem $WIFI_2G_SUB_RST_PARA_ADDR 32 $new_value

	echo "## Step3: close wifi 2g tp ram"
	echo "# Setting WIFI_2G_SUB_RST_PARA_ADDR(0x90443800), bit 8 to set"
	WIFI_2G_SUB_RST_PARA_ADDR=0x90443800
	orig_value=$(devmem $WIFI_2G_SUB_RST_PARA_ADDR)
	mask=0x00000100
	new_value=$((orig_value | $mask))
	echo "# update $WIFI_2G_SUB_RST_PARA_ADDR: $orig_value ->  $new_value"
	devmem $WIFI_2G_SUB_RST_PARA_ADDR 32 $new_value

	echo "## Step4: close dif 2g clock"
	echo "#(Setting DIF_2G_SUB_CG_PARA0_ADDR(0x90443028), value 0)"
	DIF_2G_SUB_CG_PARA0_ADDR=0x90443028
	orig_value=$(devmem $DIF_2G_SUB_CG_PARA0_ADDR)
	new_value=0
	echo "# update $DIF_2G_SUB_CG_PARA0_ADDR: $orig_value ->  $new_value"
	devmem $DIF_2G_SUB_CG_PARA0_ADDR 32 $new_value

	echo "## Step5: close dpd 2g clock"
	echo "# Setting DPD_SUB_CG_PARA0_ADDR(0x90443038), mask 0xFFFFFFF3, value 0x0"
	DPD_SUB_CG_PARA0_ADDR=0x90443038
	orig_value=$(devmem $DPD_SUB_CG_PARA0_ADDR)
	mask=0xFFFFFFF3
	value=0x0
	new_value=$((($orig_value & $mask) | $value))
	echo "# update $DPD_SUB_CG_PARA0_ADDR: $orig_value ->  $new_value"
	devmem $DPD_SUB_CG_PARA0_ADDR 32 $new_value

	echo "## Step6: close dif 2g sp ram"
	echo "# Setting DIF_2G_SUB_RST_PARA_ADDR(0x90443808), bit 0 to set"
	DIF_2G_SUB_RST_PARA_ADDR=0x90443808
	orig_value=$(devmem $DIF_2G_SUB_RST_PARA_ADDR)
	mask=0x00000001
	new_value=$((orig_value | $mask))
	echo "# update $DIF_2G_SUB_RST_PARA_ADDR: $orig_value ->  $new_value"
	devmem $DIF_2G_SUB_RST_PARA_ADDR 32 $new_value

	echo "## Step7: close dif 2g tp ram"
	echo "# Setting DIF_2G_SUB_RST_PARA_ADDR(0x90443808), bit 8 to set"
	DIF_2G_SUB_RST_PARA_ADDR=0x90443808
	orig_value=$(devmem $DIF_2G_SUB_RST_PARA_ADDR)
	mask=0x00000100
	new_value=$((orig_value | $mask))
	echo "# update $DIF_2G_SUB_RST_PARA_ADDR: $orig_value ->  $new_value"
	devmem $DIF_2G_SUB_RST_PARA_ADDR 32 $new_value

	echo "## Step8: close dpd 2g sp ram"
	echo "# Setting DPD_SUB_RST_PARA0_ADDR(0x90443810), bit 0 to set"
	DPD_SUB_RST_PARA0_ADDR=0x90443810
	orig_value=$(devmem $DPD_SUB_RST_PARA0_ADDR)
	mask=0x00000001
	new_value=$((orig_value | $mask))
	echo "# update $DPD_SUB_RST_PARA0_ADDR: $orig_value ->  $new_value"
	devmem $DPD_SUB_RST_PARA0_ADDR 32 $new_value

	echo "## Step9: close dpd 2g tp ram"
	echo "# Setting DPD_SUB_RST_PARA0_ADDR(0x90443810), bit 8 to set"
	DPD_SUB_RST_PARA0_ADDR=0x90443810
	orig_value=$(devmem $DPD_SUB_RST_PARA0_ADDR)
	mask=0x00000100
	new_value=$((orig_value | $mask))
	echo "# update $DPD_SUB_RST_PARA0_ADDR: $orig_value ->  $new_value"
	devmem $DPD_SUB_RST_PARA0_ADDR 32 $new_value

	echo "##Step10: close AFE C0"
	sh /root/afe/afe_close_ch0.sh

	echo "##Step11: close AFE C1"
	sh /root/afe/afe_close_ch1.sh

	echo "#### close 2G: END!"
}

radio_2g_dyn() {
	echo "#### 2G dynamic: START!"

	echo "## Step1: enable wifi 2g dynamic cg"
	echo "#(Setting WIFI_2G_SUB_CG_PARA1_ADDR(0x9044301C), value 0xE00)"
	WIFI_2G_SUB_CG_PARA1_ADDR=0x9044301C
	orig_value=$(devmem $WIFI_2G_SUB_CG_PARA1_ADDR)
	new_value=0xE00
	echo "# update $WIFI_2G_SUB_CG_PARA1_ADDR: $orig_value ->  $new_value"
	devmem $WIFI_2G_SUB_CG_PARA1_ADDR 32 $new_value

	echo "## Step2: enable wifi 2g ram low_power"
	echo "#(Setting WIFI_2G_SUB_RST_PARA_ADDR(0x90443800), value 0x5C1C)"
	WIFI_2G_SUB_RST_PARA_ADDR=0x90443800
	orig_value=$(devmem $WIFI_2G_SUB_RST_PARA_ADDR)
	new_value=0x5C1C
	echo "# update $WIFI_2G_SUB_RST_PARA_ADDR: $orig_value ->  $new_value"
	devmem $WIFI_2G_SUB_RST_PARA_ADDR 32 $new_value

	echo "## Step3: enable wmac 2g ram low_power"
	echo "# Setting UNKNOWN_ADDR(0x48B040AC), mask 0xFCFFFFFF, value 0x02000000"
	UNKNOWN_ADDR=0x48B040AC
	orig_value=$(devmem $UNKNOWN_ADDR)
	mask=0xFCFFFFFF
	value=0x02000000
	new_value=$((($orig_value & $mask) | $value))
	echo "# update $UNKNOWN_ADDR: $orig_value ->  $new_value"
	devmem $UNKNOWN_ADDR 32 $new_value

	echo "## Step4: enable phy 2g ram low_power"
	echo "#(Setting UNKNOWN_ADDR(0x48CO8244), value 0x03200000)"
	UNKNOWN_ADDR=0x48C08244
	orig_value=$(devmem $UNKNOWN_ADDR)
	new_value=0x03200000
	echo "# update $UNKNOWN_ADDR: $orig_value ->  $new_value"
	devmem $UNKNOWN_ADDR 32 $new_value

	echo "## Step5: enable dif 2g dynamic cg"
	echo "#(Setting DIF_2G_SUB_CG_PARA1_ADDR(0x9044302C), value 0x1FF00)"
	DIF_2G_SUB_CG_PARA1_ADDR=0x9044302C
	orig_value=$(devmem $DIF_2G_SUB_CG_PARA1_ADDR)
	new_value=0x1FF00
	echo "# update $DIF_2G_SUB_CG_PARA1_ADDR: $orig_value ->  $new_value"
	devmem $DIF_2G_SUB_CG_PARA1_ADDR 32 $new_value

	echo "## Step6: enable dif 2g ram low power"
	echo "#(Setting DIF_2G_SUB_RST_PARA_ADDR(0x90443808), value 0x5C1C)"
	DIF_2G_SUB_RST_PARA_ADDR=0x90443808
	orig_value=$(devmem $DIF_2G_SUB_RST_PARA_ADDR)
	new_value=0x5C1C
	echo "# update $DIF_2G_SUB_RST_PARA_ADDR: $orig_value ->  $new_value"
	devmem $DIF_2G_SUB_RST_PARA_ADDR 32 $new_value

	echo "## Step7: enable dpd 2g ram low_power"
	echo "# Setting DPD_SUB_RST_PARA0_ADDR(0x90443810), mask 0xFFFF0000, value 0x5C1C"
	DPD_SUB_RST_PARA0_ADDR=0x90443810
	orig_value=$(devmem $DPD_SUB_RST_PARA0_ADDR)
	mask=0xFFFF0000
	value=0x5C1C
	new_value=$((($orig_value & $mask) | $value))
	echo "# update $DPD_SUB_RST_PARA0_ADDR: $orig_value ->  $new_value"
	devmem $DPD_SUB_RST_PARA0_ADDR 32 $new_value

	echo "## Step8: enable dpd_com ram low power"
	echo "#(Setting DPD_SUB_RST_PARA1_PARA0_ADDR(0x90443814), value 0x5C1C)"
	DPD_SUB_RST_PARA1_PARA0_ADDR=0x90443814
	orig_value=$(devmem $DPD_SUB_RST_PARA1_PARA0_ADDR)
	new_value=0x5C1C
	echo "# update $DPD_SUB_RST_PARA1_PARA0_ADDR: $orig_value ->  $new_value"
	devmem $DPD_SUB_RST_PARA1_PARA0_ADDR 32 $new_value

	echo "## Step9: enable dif_com ram low power"
	echo "#(Setting DIF_COM_SUB_RST_PARA0_ADDR(0x90443818), value 0x5C5C5C)"
	DIF_COM_SUB_RST_PARA0_ADDR=0x90443818
	orig_value=$(devmem $DIF_COM_SUB_RST_PARA0_ADDR)
	new_value=0x5C5C5C
	echo "# update $DIF_COM_SUB_RST_PARA0_ADDR: $orig_value ->  $new_value"
	devmem $DIF_COM_SUB_RST_PARA0_ADDR 32 $new_value

	echo "## Step10: enable dif 2g ram low power"
	echo "#(Setting CFR_LP_PARA_2G_ADDR(0x40200080), value 0x2008200)"
	CFR_LP_PARA_2G_ADDR=0x40200080
	orig_value=$(devmem $CFR_LP_PARA_2G_ADDR)
	new_value=0x2008200
	echo "# update $CFR_LP_PARA_2G_ADDR: $orig_value ->  $new_value"
	devmem $CFR_LP_PARA_2G_ADDR 32 $new_value

	echo "## Step11: enable dpd ram low power"
	echo "#(Setting DPD_FWD0_LP_PARA_ADDR(0x40500070), value 0x2008200)"
	DPD_FWD0_LP_PARA_ADDR=0x40500070
	orig_value=$(devmem $DPD_FWD0_LP_PARA_ADDR)
	new_value=0x2008200
	echo "# update $DPD_FWD0_LP_PARA_ADDR: $orig_value ->  $new_value"
	devmem $DPD_FWD0_LP_PARA_ADDR 32 $new_value

	echo "## Step12: enable dpd ram low power"
	echo "#(Setting DPD_FWD1_LP_PARA_ADDR(0x40500074), value 0x2008200)"
	DPD_FWD1_LP_PARA_ADDR=0x40500074
	orig_value=$(devmem $DPD_FWD1_LP_PARA_ADDR)
	new_value=0x2008200
	echo "# update $DPD_FWD1_LP_PARA_ADDR: $orig_value ->  $new_value"
	devmem $DPD_FWD1_LP_PARA_ADDR 32 $new_value

	echo "## Step13: enable dpd ram low power"
	echo "#(Setting DPD_FB_CTRL_LP_PARA__ADDR(0x4040007c), value 0x2008200)"
	DPD_FB_CTRL_LP_PARA__ADDR=0x4040007c
	orig_value=$(devmem $DPD_FB_CTRL_LP_PARA__ADDR)
	new_value=0x2008200
	echo "# update $DPD_FB_CTRL_LP_PARA__ADDR: $orig_value ->  $new_value"
	devmem $DPD_FB_CTRL_LP_PARA__ADDR 32 $new_value

	echo "## Step14: enable dpd ram low power"
	echo "#(Setting DPD_LMS_CTRL_LP_PARA_ADDR(0x40400080), value 0x2008200)"
	DPD_LMS_CTRL_LP_PARA_ADDR=0x40400080
	orig_value=$(devmem $DPD_LMS_CTRL_LP_PARA_ADDR)
	new_value=0x2008200
	echo "# update $DPD_LMS_CTRL_LP_PARA_ADDR: $orig_value ->  $new_value"
	devmem $DPD_LMS_CTRL_LP_PARA_ADDR 32 $new_value

	echo "## Step15: enable sample 2G CSI ram low power"
	echo "#(Setting UNKNOWN_ADDR(0x4062002c), value 0x2008200)"
	UNKNOWN_ADDR=0x4062002c
	orig_value=$(devmem $UNKNOWN_ADDR)
	new_value=0x2008200
	echo "# update $UNKNOWN_ADDR: $orig_value ->  $new_value"
	devmem $UNKNOWN_ADDR 32 $new_value

	echo "#### 2G dynamic: END!"
}

radio_5g_static() {
	echo "#### close 5G: START!"

	echo "## Step1: close wifi 5g clock"
	echo "# Setting WIFI_5G_SUB_CG_PARA0_ADDR(0x90443020), bit 0 to clear"
	WIFI_5G_SUB_CG_PARA0_ADDR=0x90443020
	orig_value=$(devmem $WIFI_5G_SUB_CG_PARA0_ADDR)
	mask=0xFFFFFFFFFFFFFFFE
	new_value=$(($orig_value & $mask))
	echo "# update $WIFI_5G_SUB_CG_PARA0_ADDR: $orig_value ->  $new_value"
	devmem $WIFI_5G_SUB_CG_PARA0_ADDR 32 $new_value

	echo "## Step2: close wifi 5g sp ram"
	echo "# Setting WIFI_5G_SUB_RST_PARA_ADDR(0x90443804), bit 0 to set"
	WIFI_5G_SUB_RST_PARA_ADDR=0x90443804
	orig_value=$(devmem $WIFI_5G_SUB_RST_PARA_ADDR)
	mask=0x00000001
	new_value=$((orig_value | $mask))
	echo "# update $WIFI_5G_SUB_RST_PARA_ADDR: $orig_value ->  $new_value"
	devmem $WIFI_5G_SUB_RST_PARA_ADDR 32 $new_value

	echo "## Step3: close wifi 5g tp ram"
	echo "# Setting WIFI_5G_SUB_RST_PARA_ADDR(0x90443804), bit 8 to set"
	WIFI_5G_SUB_RST_PARA_ADDR=0x90443804
	orig_value=$(devmem $WIFI_5G_SUB_RST_PARA_ADDR)
	mask=0x00000100
	new_value=$((orig_value | $mask))
	echo "# update $WIFI_5G_SUB_RST_PARA_ADDR: $orig_value ->  $new_value"
	devmem $WIFI_5G_SUB_RST_PARA_ADDR 32 $new_value

	echo "## Step4: close dif 5g clock"
	echo "#(Setting DIF_5G_SUB_CG_PARA0_ADDR(0x90443030), value 0)"
	DIF_5G_SUB_CG_PARA0_ADDR=0x90443030
	orig_value=$(devmem $DIF_5G_SUB_CG_PARA0_ADDR)
	new_value=0
	echo "# update $DIF_5G_SUB_CG_PARA0_ADDR: $orig_value ->  $new_value"
	devmem $DIF_5G_SUB_CG_PARA0_ADDR 32 $new_value

	echo "## Step5: close dpd 5g clock"
	echo "# Setting DPD_SUB_CG_PARA0_ADDR(0x90443038), mask 0xFFFFFFFC, value 0x0"
	DPD_SUB_CG_PARA0_ADDR=0x90443038
	orig_value=$(devmem $DPD_SUB_CG_PARA0_ADDR)
	mask=0xFFFFFFFC
	value=0x0
	new_value=$((($orig_value & $mask) | $value))
	echo "# update $DPD_SUB_CG_PARA0_ADDR: $orig_value ->  $new_value"
	devmem $DPD_SUB_CG_PARA0_ADDR 32 $new_value

	echo "## Step6: close dif 5g sp ram"
	echo "# Setting DIF_5G_SUB_RST_PARA_ADDR(0x9044380C), bit 0 to set"
	DIF_5G_SUB_RST_PARA_ADDR=0x9044380C
	orig_value=$(devmem $DIF_5G_SUB_RST_PARA_ADDR)
	mask=0x00000001
	new_value=$((orig_value | $mask))
	echo "# update $DIF_5G_SUB_RST_PARA_ADDR: $orig_value ->  $new_value"
	devmem $DIF_5G_SUB_RST_PARA_ADDR 32 $new_value

	echo "## Step7: close dif 5g tp ram"
	echo "# Setting DIF_5G_SUB_RST_PARA_ADDR(0x9044380C), bit 8 to set"
	DIF_5G_SUB_RST_PARA_ADDR=0x9044380C
	orig_value=$(devmem $DIF_5G_SUB_RST_PARA_ADDR)
	mask=0x00000100
	new_value=$((orig_value | $mask))
	echo "# update $DIF_5G_SUB_RST_PARA_ADDR: $orig_value ->  $new_value"
	devmem $DIF_5G_SUB_RST_PARA_ADDR 32 $new_value

	echo "## Step8: close dpd 5g sp ram"
	echo "# Setting DPD_SUB_RST_PARA0_ADDR(0x90443810), bit 16 to set"
	DPD_SUB_RST_PARA0_ADDR=0x90443810
	orig_value=$(devmem $DPD_SUB_RST_PARA0_ADDR)
	mask=0x00010000
	new_value=$((orig_value | $mask))
	echo "# update $DPD_SUB_RST_PARA0_ADDR: $orig_value ->  $new_value"
	devmem $DPD_SUB_RST_PARA0_ADDR 32 $new_value

	echo "## Step9: close dpd 5g tp ram"
	echo "# Setting DPD_SUB_RST_PARA0_ADDR(0x90443810), bit 24 to set"
	DPD_SUB_RST_PARA0_ADDR=0x90443810
	orig_value=$(devmem $DPD_SUB_RST_PARA0_ADDR)
	mask=0x01000000
	new_value=$((orig_value | $mask))
	echo "# update $DPD_SUB_RST_PARA0_ADDR: $orig_value ->  $new_value"
	devmem $DPD_SUB_RST_PARA0_ADDR 32 $new_value

	echo "##Step10: close AFE C2"
	sh /root/afe/afe_close_ch2.sh

	echo "##Step11: close AFE C3"
	sh /root/afe/afe_close_ch3.sh

	echo "#### close 5G: END!"
}

radio_5g_dyn() {
	echo "#### 5G dynamic: START!"

	echo "## Step1: enable wifi 5g dynamic cg"
	echo "#(Setting WIFI_5G_SUB_CG_PARA1_ADDR(0x90443024), value 0xE00)"
	WIFI_5G_SUB_CG_PARA1_ADDR=0x90443024
	orig_value=$(devmem $WIFI_5G_SUB_CG_PARA1_ADDR)
	new_value=0xE00
	echo "# update $WIFI_5G_SUB_CG_PARA1_ADDR: $orig_value ->  $new_value"
	devmem $WIFI_5G_SUB_CG_PARA1_ADDR 32 $new_value

	echo "## Step2: enable wifi 5g ram low_power"
	echo "#(Setting WIFI_5G_SUB_RST_PARA_ADDR(0x90443804), value 0x5C1C)"
	WIFI_5G_SUB_RST_PARA_ADDR=0x90443804
	orig_value=$(devmem $WIFI_5G_SUB_RST_PARA_ADDR)
	new_value=0x5C1C
	echo "# update $WIFI_5G_SUB_RST_PARA_ADDR: $orig_value ->  $new_value"
	devmem $WIFI_5G_SUB_RST_PARA_ADDR 32 $new_value

	echo "## Step3: enable wmac 5g ram low_power"
	echo "# Setting UNKOWN_ADDR(0x4AB040AC), mask 0xFCFFFFFF, value 0x02000000"
	UNKOWN_ADDR=0x4AB040AC
	orig_value=$(devmem $UNKOWN_ADDR)
	mask=0xFCFFFFFF
	value=0x02000000
	new_value=$((($orig_value & $mask) | $value))
	echo "# update $UNKOWN_ADDR: $orig_value ->  $new_value"
	devmem $UNKOWN_ADDR 32 $new_value

	echo "## Step4: enable phy 5g ram low_power"
	echo "#(Setting UNKNOWN_ADDR(0x4ACO8244), value 0x03200000)"
	UNKNOWN_ADDR=0x4AC08244
	orig_value=$(devmem $UNKNOWN_ADDR)
	new_value=0x03200000
	echo "# update $UNKNOWN_ADDR: $orig_value ->  $new_value"
	devmem $UNKNOWN_ADDR 32 $new_value

	echo "## Step5: enable dif 5g dynamic cg"
	echo "#(Setting DIF_5G_SUB_CG_PARA1_ADDR(0x90443034), value 0x1FF00)"
	DIF_5G_SUB_CG_PARA1_ADDR=0x90443034
	orig_value=$(devmem $DIF_5G_SUB_CG_PARA1_ADDR)
	new_value=0x1FF00
	echo "# update $DIF_5G_SUB_CG_PARA1_ADDR: $orig_value ->  $new_value"
	devmem $DIF_5G_SUB_CG_PARA1_ADDR 32 $new_value

	echo "## Step6: enable dif 5g ram low power"
	echo "#(Setting DIF_5G_SUB_RST_PARA_ADDR(0x9044380C), value 0x5C1C5C1C)"
	DIF_5G_SUB_RST_PARA_ADDR=0x9044380C
	orig_value=$(devmem $DIF_5G_SUB_RST_PARA_ADDR)
	new_value=0x5C1C5C1C
	echo "# update $DIF_5G_SUB_RST_PARA_ADDR: $orig_value ->  $new_value"
	devmem $DIF_5G_SUB_RST_PARA_ADDR 32 $new_value

	echo "## Step7: enable dpd 5g ram low_power"
	echo "# Setting DPD_SUB_RST_PARA0_ADDR(0x90443810), mask 0x0000FFFF, value 0x5C1C0000"
	DPD_SUB_RST_PARA0_ADDR=0x90443810
	orig_value=$(devmem $DPD_SUB_RST_PARA0_ADDR)
	mask=0x0000FFFF
	value=0x5C1C0000
	new_value=$((($orig_value & $mask) | $value))
	echo "# update $DPD_SUB_RST_PARA0_ADDR: $orig_value ->  $new_value"
	devmem $DPD_SUB_RST_PARA0_ADDR 32 $new_value

	echo "## Step8: enable dpd_com ram low power"
	echo "#(Setting DPD_SUB_RST_PARA1_PARA0_ADDR(0x90443814), value 0x5C1C)"
	DPD_SUB_RST_PARA1_PARA0_ADDR=0x90443814
	orig_value=$(devmem $DPD_SUB_RST_PARA1_PARA0_ADDR)
	new_value=0x5C1C
	echo "# update $DPD_SUB_RST_PARA1_PARA0_ADDR: $orig_value ->  $new_value"
	devmem $DPD_SUB_RST_PARA1_PARA0_ADDR 32 $new_value

	echo "## Step9: enable dif_com ram low power"
	echo "#(Setting DIF_COM_SUB_RST_PARA0_ADDR(0x90443818), value 0x5C5C5C)"
	DIF_COM_SUB_RST_PARA0_ADDR=0x90443818
	orig_value=$(devmem $DIF_COM_SUB_RST_PARA0_ADDR)
	new_value=0x5C5C5C
	echo "# update $DIF_COM_SUB_RST_PARA0_ADDR: $orig_value ->  $new_value"
	devmem $DIF_COM_SUB_RST_PARA0_ADDR 32 $new_value

	echo "## Step10: enable dif 5g ram low power"
	echo "#(Setting CFR_LP_PARA_5G_ADDR(0x40000080), value 0x2008200)"
	CFR_LP_PARA_5G_ADDR=0x40000080
	orig_value=$(devmem $CFR_LP_PARA_5G_ADDR)
	new_value=0x2008200
	echo "# update $CFR_LP_PARA_5G_ADDR: $orig_value ->  $new_value"
	devmem $CFR_LP_PARA_5G_ADDR 32 $new_value

	echo "## Step11: enable dpd ram low power"
	echo "#(Setting DPD_FWD0_LP_PARA_ADDR(0x40400070), value 0x2008200)"
	DPD_FWD0_LP_PARA_ADDR=0x40400070
	orig_value=$(devmem $DPD_FWD0_LP_PARA_ADDR)
	new_value=0x2008200
	echo "# update $DPD_FWD0_LP_PARA_ADDR: $orig_value ->  $new_value"
	devmem $DPD_FWD0_LP_PARA_ADDR 32 $new_value

	echo "## Step12: enable dpd ram low power"
	echo "#(Setting DPD_FWD1_LP_PARA_ADDR(0x40400074), value 0x2008200)"
	DPD_FWD1_LP_PARA_ADDR=0x40400074
	orig_value=$(devmem $DPD_FWD1_LP_PARA_ADDR)
	new_value=0x2008200
	echo "# update $DPD_FWD1_LP_PARA_ADDR: $orig_value ->  $new_value"
	devmem $DPD_FWD1_LP_PARA_ADDR 32 $new_value

	echo "## Step13: enable dpd ram low power"
	echo "#(Setting DPD_FB_CTRL_LP_PARA_ADDR(0x4040007c), value 0x2008200)"
	DPD_FB_CTRL_LP_PARA_ADDR=0x4040007c
	orig_value=$(devmem $DPD_FB_CTRL_LP_PARA_ADDR)
	new_value=0x2008200
	echo "# update $DPD_FB_CTRL_LP_PARA_ADDR: $orig_value ->  $new_value"
	devmem $DPD_FB_CTRL_LP_PARA_ADDR 32 $new_value

	echo "## Step14: enable dpd ram low power"
	echo "#(Setting DPD_LMS_CTRL_LP_PARA_ADDR(0x40400080), value 0x2008200)"
	DPD_LMS_CTRL_LP_PARA_ADDR=0x40400080
	orig_value=$(devmem $DPD_LMS_CTRL_LP_PARA_ADDR)
	new_value=0x2008200
	echo "# update $DPD_LMS_CTRL_LP_PARA_ADDR: $orig_value ->  $new_value"
	devmem $DPD_LMS_CTRL_LP_PARA_ADDR 32 $new_value

	echo "## Step15: enable sample 5G CSI ram low power"
	echo "#(Setting UNKNOWN_ADDR(0x40620030), value 0x2008200)"
	UNKNOWN_ADDR=0x40620030
	orig_value=$(devmem $UNKNOWN_ADDR)
	new_value=0x2008200
	echo "# update $UNKNOWN_ADDR: $orig_value ->  $new_value"
	devmem $UNKNOWN_ADDR 32 $new_value

	echo "#### 5G dynamic: END!"
}

dpd_static() {
	echo "#### close dpd: START!"

	echo "## Step1: close dpd clock"
	echo "# Setting DPD_SUB_CG_PARA0_ADDR(0x90443038), bit 4 to clear"
	DPD_SUB_CG_PARA0_ADDR=0x90443038
	orig_value=$(devmem $DPD_SUB_CG_PARA0_ADDR)
	mask=0xFFFFFFFFFFFFFFEF
	new_value=$(($orig_value & $mask))
	echo "# update $DPD_SUB_CG_PARA0_ADDR: $orig_value ->  $new_value"
	devmem $DPD_SUB_CG_PARA0_ADDR 32 $new_value

	echo "## Step2: close dpd ram"
	echo "#(Setting DPD_SUB_RST_PARA1_PARA0_ADDR(0x90443814), value 0x0101)"
	DPD_SUB_RST_PARA1_PARA0_ADDR=0x90443814
	orig_value=$(devmem $DPD_SUB_RST_PARA1_PARA0_ADDR)
	new_value=0x0101
	echo "# update $DPD_SUB_RST_PARA1_PARA0_ADDR: $orig_value ->  $new_value"
	devmem $DPD_SUB_RST_PARA1_PARA0_ADDR 32 $new_value

	echo "#### close dpd: END!"
}

smp_static() {
	echo "#### close smp: START!"

	echo "## Step1: close serdes0 clock"
	echo "# Setting DIF_COM_SUB_CG_PARA0_ADDR(0x90443040), bit 3 to clear"
	DIF_COM_SUB_CG_PARA0_ADDR=0x90443040
	orig_value=$(devmem $DIF_COM_SUB_CG_PARA0_ADDR)
	mask=0xFFFFFFFFFFFFFFF7
	new_value=$(($orig_value & $mask))
	echo "# update $DIF_COM_SUB_CG_PARA0_ADDR: $orig_value ->  $new_value"
	devmem $DIF_COM_SUB_CG_PARA0_ADDR 32 $new_value

	echo "## Step2: close serdes0 clock"
	echo "# Setting DIF_COM_SUB_RST_PARA0_ADDR(0x90443818), mask 0x0000FFFF, value 0x00010000"
	DIF_COM_SUB_RST_PARA0_ADDR=0x90443818
	orig_value=$(devmem $DIF_COM_SUB_RST_PARA0_ADDR)
	mask=0x0000FFFF
	value=0x00010000
	new_value=$((($orig_value & $mask) | $value))
	echo "# update $DIF_COM_SUB_RST_PARA0_ADDR: $orig_value ->  $new_value"
	devmem $DIF_COM_SUB_RST_PARA0_ADDR 32 $new_value

	echo "#### close smp: END!"
}

intfndet_static() {
	echo "#### close intfndet: START!"

	echo "## Step1: close serdes0 clock"
	echo "# Setting DIF_COM_SUB_RST_PARA0_ADDR(0x90443818), mask 0xFFFFFF00, value 0x1"
	DIF_COM_SUB_RST_PARA0_ADDR=0x90443818
	orig_value=$(devmem $DIF_COM_SUB_RST_PARA0_ADDR)
	mask=0xFFFFFF00
	value=0x1
	new_value=$((($orig_value & $mask) | $value))
	echo "# update $DIF_COM_SUB_RST_PARA0_ADDR: $orig_value ->  $new_value"
	devmem $DIF_COM_SUB_RST_PARA0_ADDR 32 $new_value

	echo "#### close intfndet: END!"
}

radar_static() {
	echo "#### close radar: START!"

	echo "## Step1: close radar ram"
	echo "# Setting DIF_5G_SUB_RST_PARA_ADDR(0x9044380C), mask 0x0000FFFF, value 0x01010000"
	DIF_5G_SUB_RST_PARA_ADDR=0x9044380C
	orig_value=$(devmem $DIF_5G_SUB_RST_PARA_ADDR)
	mask=0x0000FFFF
	value=0x01010000
	new_value=$((($orig_value & $mask) | $value))
	echo "# update $DIF_5G_SUB_RST_PARA_ADDR: $orig_value ->  $new_value"
	devmem $DIF_5G_SUB_RST_PARA_ADDR 32 $new_value

	echo "#### close radar: END!"
}

bist_fft_static() {
	echo "#### close bist_fft: START!"

	echo "## Step1: close bist_fft_ clock"
	echo "# Setting DIF_COM_SUB_CG_PARA0_ADDR(0x90443040), bit 2 to clear"
	DIF_COM_SUB_CG_PARA0_ADDR=0x90443040
	orig_value=$(devmem $DIF_COM_SUB_CG_PARA0_ADDR)
	mask=0xFFFFFFFFFFFFFFFB
	new_value=$(($orig_value & $mask))
	echo "# update $DIF_COM_SUB_CG_PARA0_ADDR: $orig_value ->  $new_value"
	devmem $DIF_COM_SUB_CG_PARA0_ADDR 32 $new_value

	echo "#### close bist_fft: END!"
}

dif_com_static() {
	echo "#### close dif_com: START!"

	echo "## Step1: close dif_com clock"
	echo "#(Setting DIF_COM_SUB_CG_PARA0_ADDR(0x90443040), value 0)"
	DIF_COM_SUB_CG_PARA0_ADDR=0x90443040
	orig_value=$(devmem $DIF_COM_SUB_CG_PARA0_ADDR)
	new_value=0
	echo "# update $DIF_COM_SUB_CG_PARA0_ADDR: $orig_value ->  $new_value"
	devmem $DIF_COM_SUB_CG_PARA0_ADDR 32 $new_value

	echo "## Step2: close dif_com ram"
	echo "#(Setting DIF_COM_SUB_RST_PARA0_ADDR(0x90443818), value 0x010101)"
	DIF_COM_SUB_RST_PARA0_ADDR=0x90443818
	orig_value=$(devmem $DIF_COM_SUB_RST_PARA0_ADDR)
	new_value=0x010101
	echo "# update $DIF_COM_SUB_RST_PARA0_ADDR: $orig_value ->  $new_value"
	devmem $DIF_COM_SUB_RST_PARA0_ADDR 32 $new_value

	echo "#### close dif_com: END!"
}

if [ -z "$1" ]; then
    echo "please input one arg: soc_ip_static/serdes_static/2g_static/2g_dyn/5g_static/5g_dyn"
	echo "dpd_static/smp_static/intfndet_static/radar_static/bist_fft_static/dif_com_static"
    exit 1
fi

case "$1" in
    serdes_static)
        fwd_serder_static
        ;;
    soc_ip_static)
        soc_ip_static
        ;;
    2g_static)
        radio_2g_static
        ;;
    2g_dyn)
        radio_2g_dyn
        ;;
    5g_static)
        radio_5g_static
        ;;
    5g_dyn)
        radio_5g_dyn
        ;;
    dpd_static)
        dpd_static
        ;;
    smp_static)
        smp_static
        ;;
    intfndet_static)
        intfndet_static
        ;;
    radar_static)
        radar_static
        ;;
    bist_fft_static)
        bist_fft_static
        ;;
    dif_com_static)
        dif_com_static
        ;;
    *)
        echo "invalid cmd: soc_ip_static/serdes_static/2g_static/2g_dyn/5g_static/5g_dyn"
		echo "dpd_static/smp_static/intfndet_static/radar_static/bist_fft_static/dif_com_static"
        exit 1
        ;;
esac

