CC		= $(CROSS_COMPILE)gcc
STRIP		= $(CROSS_COMPILE)strip
APP_NAME = serdes_cmd
OBJ_DIR = build
SRC += sys_hal.o cmd.o serdes.o dubhe1000_mac_stats.o dubhe1000_mdio.o dubhe1000_switch_stats.o dubhe1000_switch.o 

OBJ = $(addprefix $(OBJ_DIR)/, $(SRC:.c=.o))
CFLAGS +=  -I. -I./inc/ 
#LDFLAGS += -static  

%.o:  ../%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

all: $(OBJ)
	$(CC) $^ $(LDFLAGS) -o bin/${APP_NAME} 
	$(STRIP) bin/$(APP_NAME)
	cd bin && ln -sf $(APP_NAME) rx_afe_overwride
	cd bin && ln -sf $(APP_NAME) dump_serdes 
	cd bin && ln -sf $(APP_NAME) Serdes_init_pre
	cd bin && ln -sf $(APP_NAME) ETH_RX_ADAPT
	cd bin && ln -sf $(APP_NAME) Serdes_init_proc
	cd bin && ln -sf $(APP_NAME) Serdes_init_proc_test
	cd bin && ln -sf $(APP_NAME) ETH_serdes_reg_write
	cd bin && ln -sf $(APP_NAME) ETH_serdes_reg_read
	cd bin && ln -sf $(APP_NAME) TX_FFE_Config
	cd bin && ln -sf $(APP_NAME) PCIE_TX_FFE_Config
	cd bin && ln -sf $(APP_NAME) RX_Adapt_enable
	cd bin && ln -sf $(APP_NAME) TX_PRBS_Config
	cd bin && ln -sf $(APP_NAME) TX_err_Insert
	cd bin && ln -sf $(APP_NAME) RX_PRBS_Config
	cd bin && ln -sf $(APP_NAME) RX_err_count
	cd bin && ln -sf $(APP_NAME) Loopback_config
	cd bin && ln -sf $(APP_NAME) Repeat_CLK_Config
	cd bin && ln -sf $(APP_NAME) VR_RST
	cd bin && ln -sf $(APP_NAME) tx_req
	cd bin && ln -sf $(APP_NAME) rx_req
	cd bin && ln -sf $(APP_NAME) Serdes_dump
	cd bin && ln -sf $(APP_NAME) TX_Swing_Config
	cd bin && ln -sf $(APP_NAME) show_xgmac_stats
	cd bin && ln -sf $(APP_NAME) show_switch_stats
	cd bin && ln -sf $(APP_NAME) switch_init
	cd bin && ln -sf $(APP_NAME) xgmac_set
	cd bin && ln -sf $(APP_NAME) soc_read
	cd bin && ln -sf $(APP_NAME) soc_write
	cd bin && ln -sf $(APP_NAME) ETH_PLL_Config
	cd bin && ln -sf $(APP_NAME) PCIE_PLL_Config
	cd bin && ln -sf $(APP_NAME) xgmac_reset_all
	cd bin && ln -sf $(APP_NAME) switch_read 
	cd bin && ln -sf $(APP_NAME) switch_write 
	cd bin && ln -sf $(APP_NAME) xgmac_read 
	cd bin && ln -sf $(APP_NAME) xgmac_write 
	cd bin && ln -sf $(APP_NAME) xpcs_read 
	cd bin && ln -sf $(APP_NAME) xpcs_write 
	cd bin && ln -sf $(APP_NAME) xpcs_mii_dig_sts 
	cd bin && ln -sf $(APP_NAME) xpcs_mii_ctrl 
	cd bin && ln -sf $(APP_NAME) npe_top_dump 
	cd bin && ln -sf $(APP_NAME) xpcs_an_sts 
	cd bin && ln -sf $(APP_NAME) mdio_cmd 
	cd bin && ln -sf $(APP_NAME) PCIE_serdes_reg_read
	cd bin && ln -sf $(APP_NAME) PCIE_serdes_reg_write
	cd bin && ln -sf $(APP_NAME) PCIE_TX_PRBS_Config
	cd bin && ln -sf $(APP_NAME) PCIE_TX_err_Insert
	cd bin && ln -sf $(APP_NAME) PCIE_RX_PRBS_Config
	cd bin && ln -sf $(APP_NAME) PCIE_RX_err_count
	cd bin && ln -sf $(APP_NAME) PCIE_init 
	cd bin && ln -sf $(APP_NAME) PCIE_init 
	cd bin && ln -sf $(APP_NAME) Log_printf 
	cd bin && ln -sf $(APP_NAME) Log_config 
	cd bin && ln -sf $(APP_NAME) PCIE_RX_ADAPT 
	cd bin && ln -sf $(APP_NAME) XPCS_speed 
	cd bin && ln -sf $(APP_NAME) XPCS_get_link_state 
	cd bin && ln -sf $(APP_NAME) XPCS_clear_an_interrupt 
	cd bin && ln -sf $(APP_NAME) XPCS_an_enable
	cd bin && ln -sf $(APP_NAME) XPCS_mac_auto_sw

clean:
	@rm bin/* -rf
	@rm build/* -rf
