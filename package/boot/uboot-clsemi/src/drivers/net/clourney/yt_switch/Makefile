#***********************************************************
#
# Makefile
#
#***********************************************************
# set MAKE_PATH according to the compilation environment
export MAKE_PATH := $(shell dirname $(abspath $(lastword $(MAKEFILE_LIST))))/sdk/make
export YT_ROOT := $(shell dirname $(abspath $(lastword $(MAKEFILE_LIST))))
include $(MAKE_PATH)/env.h
include $(MAKE_PATH)/env/make.env
include $(MAKE_PATH)/make.path
include $(MAKE_PATH)/make.rule
include $(MAKE_PATH)/make.cfg

#$(TARGET_SDK)-objs += cmd_api/cmd.o yt_switch_mdio.o
ccflags-y += -Wno-declaration-after-statement
#$(info ===>objs=$(yt_switch-objs))

ccflags-y += $(SDK_INCLUDES) -I${YT_ROOT}/sdk
#ccflags-y += -I${YT_ROOT}/cmd_api

ccflags-y += $(SDK_COMPILATION_SWITCHES)

obj-y := $(SDK_SOURCE_OBJECTS)

#obj-y += cmd_api/cmd.o yt_switch_mdio.o
obj-y += yt_switch_mdio.o
