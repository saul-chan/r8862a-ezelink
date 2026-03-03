
SIGMA_DIR = ../wfa/Sigma_Sample_DUT_Code-Linux_v8.1.1/
# include dirs
CFLAGS += -I./
CFLAGS += -I$(SIGMA_DIR)/inc

//CFLAGS += -D__LINUX

LIBS = -lpthread -lubus -lubox

CA_OBJ_LIST += \
	ap/cls_ap.o   \
	sta/cls_sta.o   \
	dev/cls_dev.o	\
	cls_ca_maps.o \
	cls_ca.o      \
	common/cls_cmd_parser.o \
	common/cls_ca_common.o

DUT_OBJ_LIST += \
	cls_dut.o             \
	cls_dut_ap_handler.o  \
	cls_dut_dev_handler.o	\
	cls_dut_request.o     \
	common/cls_defconf.o  \
	common/cls_cmd_parser.o \
	common/cls_dut_common.o \
	common/clsapi.o		\
	common/cls_timer.o

all: cls_ca cls_dut

wfa:
	$(MAKE) -C $(SIGMA_DIR)

cls_ca: $(CA_OBJ_LIST) $(SIGMA_DIR)/ca/wfa_ca.o $(SIGMA_DIR)lib/libwfa_ca.a
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS) $(LIBS)

cls_dut: $(DUT_OBJ_LIST) $(SIGMA_DIR)/dut/wfa_dut.o $(SIGMA_DIR)/dut/wfa_dut_init.o $(SIGMA_DIR)/lib/libwfa_dut.a
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS) $(LIBS)

%.o: %.c wfa
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f cls_ca cls_dut $(CA_OBJ_LIST) $(DUT_OBJ_LIST)
	$(MAKE) -C $(SIGMA_DIR) clean

