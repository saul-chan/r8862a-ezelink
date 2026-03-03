TARGET_MODULE:=dts_overlay

$(TARGET_MODULE)-objs := configfs.o
obj-m := $(TARGET_MODULE).o
ccflags-y += -I$(PWD)

all : modules
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules_install INSTALL_MOD_STRIP=1 INSTALL_MOD_PATH=$(KERNELOUT)

clean:
	#$(MAKE) -C $(BUILDSYSTEM_DIR) M=$(PWD) clean

modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) $@

