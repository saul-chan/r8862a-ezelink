#
# Copyright (C) 2023 Clourney Semiconductor - All Rights Reserved.
# SPDX-License-Identifier: BSD-3-Clause
# https://spdx.org/licenses
#

DEST_INC_DIR ?= $(DESTDIR)/usr/include/
DEST_LIB_DIR ?= $(DESTDIR)/usr/lib/
CLSAPI_PLUGIN_DIR ?= /usr/lib/clsapi
DEST_PLUGIN_DIR ?= $(DESTDIR)/$(CLSAPI_PLUGIN_DIR)
DEST_CLSAPI_CLI_DIR ?= $(DESTDIR)/usr/lib/cls/clsapi_cli

CLSAPI_SYS_CFLAGS = $(CLSAPI_CFLAGS) -I.. -I$(DEST_INC_DIR)

CLSAPI_SYS_LIB		= libclsapi_sys.so
CLSAPI_SYS_LIB_OBJ	= sys.o ../clsapi_util.o ../clsapi_common.o
CLSAPI_SYS_LIB_LDFLAGS	= $(LDFLAGS) -shared -fPIC -lubus -lubox -luci -L$(DEST_LIB_DIR) -lclsapi_base
CLSAPI_SYS_H	= clsapi_sys.h

CLSAPI_SYS_UBUS_PLUGIN		= clsapi_ubus_sys.so
CLSAPI_SYS_UBUS_PLUGIN_OBJ	= ubus_sys.o
CLSAPI_SYS_UBUS_PLUGIN_LDFLAGS	= $(LDFLAGS) -shared -fPIC -L. -lclsapi_sys

CLSAPI_CLI_SYS_PLUGIN		= clsapi_cli_sys.so
CLSAPI_CLI_SYS_PLUGIN_OBJ	= cli_sys.o ../clsapi_cli.o
CLSAPI_CLI_SYS_PLUGIN_LDFLAGS	= $(LDFLAGS) -shared -fPIC -L. -lclsapi_sys -lubus -lubox -luci


all: build install

build: $(CLSAPI_SYS_LIB) $(CLSAPI_SYS_UBUS_PLUGIN) $(CLSAPI_CLI_SYS_PLUGIN)

auto_gen_ubus:
	python3 ../autogen_clsapi_ubus.py clsapi_sys.h autogened_ubus_sys.h

auto_gen_cli:
	rm -f autogened_cli_sys.h
	python ../autogen_clsapi_cli.py clsapi_sys.h autogened_cli_sys.h

%.o: %.c
	$(CC) $(CLSAPI_SYS_CFLAGS) -c -o $@ $<


$(CLSAPI_SYS_LIB): $(CLSAPI_SYS_LIB_OBJ)
	$(CC) $(CLSAPI_SYS_LIB_LDFLAGS) -o $(CLSAPI_SYS_LIB) $(CLSAPI_SYS_LIB_OBJ)

$(CLSAPI_SYS_UBUS_PLUGIN): auto_gen_ubus $(CLSAPI_SYS_UBUS_PLUGIN_OBJ)
	$(CC) $(CLSAPI_SYS_UBUS_PLUGIN_LDFLAGS) -o $(CLSAPI_SYS_UBUS_PLUGIN) $(CLSAPI_SYS_UBUS_PLUGIN_OBJ)

$(CLSAPI_CLI_SYS_PLUGIN): auto_gen_cli $(CLSAPI_CLI_SYS_PLUGIN_OBJ)
	$(CC) $(CLSAPI_CLI_SYS_PLUGIN_LDFLAGS) -o $(CLSAPI_CLI_SYS_PLUGIN) $(CLSAPI_CLI_SYS_PLUGIN_OBJ)

install:
	mkdir -p $(DEST_INC_DIR) $(DEST_LIB_DIR) $(DEST_PLUGIN_DIR) $(DEST_CLSAPI_CLI_DIR)
	cp $(CLSAPI_SYS_H) $(DEST_INC_DIR)
	cp $(CLSAPI_SYS_LIB) $(DEST_LIB_DIR)
	if [ -f $(CLSAPI_CLI_SYS_PLUGIN) ]; then cp $(CLSAPI_CLI_SYS_PLUGIN) $(DEST_CLSAPI_CLI_DIR); fi
	if [ -f $(CLSAPI_SYS_UBUS_PLUGIN) ]; then cp $(CLSAPI_SYS_UBUS_PLUGIN) $(DEST_PLUGIN_DIR); fi

clean:
	rm -rf *.o *.so
