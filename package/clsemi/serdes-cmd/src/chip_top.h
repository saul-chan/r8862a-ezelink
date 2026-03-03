if (!REG_R(FWD_TOP_PLL_PARA3, PLL_PARA_0)) {
	printf("ERR: FWD PLL NOT LOCK!!!!!");
	return;
}

writel(1, 0x9044020c);
writel(7, 0x90442844);
#if 0
writel(1, 	0x90440a0c);
writel(0x1, 0x9044120c);
writel(0x3, 0x90441a0c);
writel(0xf, 0x9045020c);
writel(0x3, 0x9041620c);
writel(0x7f, 0x90442800);
writel(0x1ff, 0x90442800);
writel(0x1, 0x90443018);
#endif
//CG
writel(0xf,0x9044304c);
writel(0x0, FWD_SUB_RST_PARA);
writel(0xA, FWD_SUB_RST_PARA);

sleep(1);

if (g_debug) {
	writel(0x3, WIFI_2G_SUB_RST_PARA);//WIFI40_SUB_RST_PARA
	writel(0x3, WIFI_5G_SUB_RST_PARA);//WIFI160_SUB_RST_PARA
	writel(0x3F, DIF_2G_SUB_RST_PARA);//DIF40_SUB_RST_PARA
	writel(0x3F, DIF_5G_SUB_RST_PARA);///DIF160_SUB_RST_PARA
	writel(0x1f, DPD_SUB_RST_PARA);//AFE_SRC_SUB_RST_PARA
	writel(0x3f, DIF_COM_SUB_RST_PARA );//AFE_SRC_SUB_RST_PARA
	writel(0x7f, AFE_SRC_SUB_RST_PARA);//AFE_SRC_SUB_RST_PARA
	writel(0x1, WIFI_2G_SUB_CG_PARA0);//WIFI_40_SUB_CG_PARA
	writel(0x1, WIFI_5G_SUB_CG_PARA0);//WIFI_160_SUB_CG_PARA
	writel(0xFFFFFF, DIF_2G_SUB_CG_PARA0);//DIF_40_SUB_CG_PARA
	writel(0xFFFFFF, DIF_5G_SUB_CG_PARA0);//DIF_160_SUB_CG_PARA
}
//PCIE_PLL_Config(2, 125, 0, 3, 1);

//ETH_PLL_Config(1, 65, 1747627, 5, 2);
if (g_debug) {
	uint32_t val = readl(FWD_SUB_CG_PARA);
	printf("FWD_SUB_CG_PARA val [%#x]\n", val);
	sleep(1);
}

writel(0xF, FWD_SUB_RST_PARA);//FWD_SUB_RST_PARA

if (g_debug) {
	uint32_t  addr = WIFI_2G_SUB_RST_PARA;
	for (; addr <= FWD_SUB_RST_PARA; addr += 4 )//FWD_SUB_RST_PARA
	{
		uint32_t val = readl(addr);
		printf("[%#x] val [%#x]\n",addr, val);
	}
}
