OBJ_DIR = npe_dtb
SRC := $(notdir $(wildcard dts/*.dts))
OBJ  = $(SRC:.dts=.dtb)
dtc ?= dtc

%.dtb: dts/%.dts
	@mkdir -p $(OBJ_DIR)
	$(dtc) -I dts -O dtb -@ -o $(OBJ_DIR)/$@ $<

all: $(OBJ)

clean:
	rm $(OBJ_DIR) -rf
