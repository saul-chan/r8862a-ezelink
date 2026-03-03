CC		 = $(CROSS_COMPILE)gcc
STRIP	 = $(CROSS_COMPILE)strip
APP_NAME = usbcmd
OBJ_DIR  = build
SRC += usbcmd.o

OBJ = $(addprefix $(OBJ_DIR)/, $(SRC:.c=.o))
CFLAGS +=  -I. -I./inc/
#LDFLAGS += -static

%.o:  ../%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

all: $(OBJ)
	$(CC) $^ $(LDFLAGS) -o bin/${APP_NAME}
	$(STRIP) bin/$(APP_NAME)
	cd bin && ln -sf $(APP_NAME) USB_LOOPBACK_EN
	cd bin && ln -sf $(APP_NAME) USB_RD_Register
	cd bin && ln -sf $(APP_NAME) USB_HW_Config
	cd bin && ln -sf $(APP_NAME) USB_TX_PRBS_Config
	cd bin && ln -sf $(APP_NAME) USB_TX_err_Insert
	cd bin && ln -sf $(APP_NAME) USB_RX_PRBS_Config
	cd bin && ln -sf $(APP_NAME) USB_RX_err_count

clean:
	@rm bin/* -rf
	@rm build/* -rf

