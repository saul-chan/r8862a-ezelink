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

CLSAPI_BASE_CFLAGS = $(CLSAPI_CFLAGS) -I..

CLSAPI_BASE_LIB		= libclsapi_base.so
CLSAPI_BASE_LIB_OBJ	= base.o ../clsapi_util.o ../clsapi_common.o
CLSAPI_BASE_LIB_LDFLAGS	= $(LDFLAGS) -shared -fPIC -lubus -lubox -luci
CLSAPI_BASE_H	= clsapi_base.h

CLSAPI_BASE_UBUS_PLUGIN		= clsapi_ubus_base.so
CLSAPI_BASE_UBUS_PLUGIN_OBJ	= ubus_base.o
CLSAPI_BASE_UBUS_PLUGIN_LDFLAGS	= $(LDFLAGS) -shared -fPIC -L. -lclsapi_base

CLSAPI_CLI_BASE_PLUGIN		= clsapi_cli_base.so
CLSAPI_CLI_BASE_PLUGIN_OBJ	= cli_base.o ../clsapi_cli.o
CLSAPI_CLI_BASE_PLUGIN_LDFLAGS	= $(LDFLAGS) -shared -fPIC -L. -lclsapi_base -lubus -lubox -luci


all: build install

build: $(CLSAPI_BASE_LIB) $(CLSAPI_BASE_UBUS_PLUGIN) $(CLSAPI_CLI_BASE_PLUGIN)

auto_gen_ubus:
	python3 ../autogen_clsapi_ubus.py clsapi_base.h autogened_ubus_base.h

auto_gen_cli:
	rm -f autogened_cli_base.h
	python ../autogen_clsapi_cli.py clsapi_base.h autogened_cli_base.h

%.o: %.c
	$(CC) $(CLSAPI_BASE_CFLAGS) -c -o $@ $<


$(CLSAPI_BASE_LIB): $(CLSAPI_BASE_LIB_OBJ)
	$(CC) $(CLSAPI_BASE_LIB_LDFLAGS) -o $(CLSAPI_BASE_LIB) $(CLSAPI_BASE_LIB_OBJ)

$(CLSAPI_BASE_UBUS_PLUGIN): auto_gen_ubus $(CLSAPI_BASE_UBUS_PLUGIN_OBJ)
	$(CC) $(CLSAPI_BASE_UBUS_PLUGIN_LDFLAGS) -o $(CLSAPI_BASE_UBUS_PLUGIN) $(CLSAPI_BASE_UBUS_PLUGIN_OBJ)

$(CLSAPI_CLI_BASE_PLUGIN): auto_gen_cli $(CLSAPI_CLI_BASE_PLUGIN_OBJ)
	$(CC) $(CLSAPI_CLI_BASE_PLUGIN_LDFLAGS) -o $(CLSAPI_CLI_BASE_PLUGIN) $(CLSAPI_CLI_BASE_PLUGIN_OBJ)

install:
	mkdir -p $(DEST_INC_DIR) $(DEST_LIB_DIR) $(DEST_PLUGIN_DIR) $(DEST_CLSAPI_CLI_DIR)
	cp $(CLSAPI_BASE_H) $(DEST_INC_DIR)
	cp $(CLSAPI_BASE_LIB) $(DEST_LIB_DIR)
	if [ -f $(CLSAPI_CLI_BASE_PLUGIN) ]; then cp $(CLSAPI_CLI_BASE_PLUGIN) $(DEST_CLSAPI_CLI_DIR); fi
	if [ -f $(CLSAPI_BASE_UBUS_PLUGIN) ]; then cp $(CLSAPI_BASE_UBUS_PLUGIN) $(DEST_PLUGIN_DIR); fi

clean:
	rm -rf *.o *.so
