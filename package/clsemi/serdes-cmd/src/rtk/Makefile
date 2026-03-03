CC		= $(CROSS_COMPILE)gcc
STRIP		= $(CROSS_COMPILE)strip
APP_NAME = rtk_cmd
OBJ_DIR = build
SRC += dal_phy.o \
	dal_waMon.o \
	hwp.o \
	hwp_init.o \
	identify.o \
	memory.o \
	miim.o \
	miim_common_drv.o \
	miim_debug.o \
	phy.o \
	phy_common.o \
	phy_construct.o \
	phy_driver_hwp.o \
	phy_experimental_rtl8211fs.o \
	phy_hal.o \
	phy_init.o \
	phy_misc.o \
	phy_patch.o \
	phy_probe.o \
	phy_rtl8218b.o \
	phy_rtl8218b_patch.o \
	phy_rtl826xb.o \
	phy_rtl826xb_patch.o \
	phy_macsec.o \
	aes.o \
	phy_wrapped_miim.o \
	rt_bitop.o \
	sem.o \
	thread.o \
	time.o \
	cmd.o 
MDIO_C =  ../dubhe1000_mdio.c ../sys_hal.c

OBJ = $(addprefix $(OBJ_DIR)/, $(SRC:.c=.o))
CFLAGS += -DPHY_ONLY -DCONFIG_SDK_WORDSIZE_64 -DCONFIG_SDK_RTL8218B -DCONFIG_SDK_RTL8214FC -I. -I./inc/ -I../ 
#LDFLAGS += -static 

%.o:  ../%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

all: $(OBJ)
	$(CC)  $^ $(LDFLAGS) $(MDIO_C) $(CFLAGS) -o ../bin/$(APP_NAME)
	$(STRIP) ../bin/$(APP_NAME)
	cd ../bin && ln -sf $(APP_NAME) rtl8261be_cmd
	cd ../bin && ln -sf $(APP_NAME) rtl8214fc_cmd
clean:
	@rm ../bin/rtl8214fc_cmd ../bin/rtl8261be_cmd ../bin/$(APP_NAME) -f
	@rm build/* -rf
