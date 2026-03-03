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

CLSAPI_NET_CFLAGS	= $(CLSAPI_CFLAGS) -I.. -I$(DEST_INC_DIR)
CLSAPI_NET_LIB		= libclsapi_net.so
CLSAPI_NET_LIB_OBJ	= network.o ../clsapi_util.o ../clsapi_common.o
CLSAPI_NET_LIB_LDFLAGS	= $(LDFLAGS) -shared -fPIC -lubus -lubox -luci -L$(DEST_LIB_DIR) -lclsapi_base
CLSAPI_NET_H	= clsapi_net.h

CLSAPI_NET_UBUS_PLUGIN		= clsapi_ubus_net.so
CLSAPI_NET_UBUS_PLUGIN_OBJ	= ubus_net.o
CLSAPI_NET_UBUS_PLUGIN_LDFLAGS	= $(LDFLAGS) -shared -fPIC -L. -lclsapi_net

CLSAPI_CLI_NET_PLUGIN		= clsapi_cli_net.so
CLSAPI_CLI_NET_PLUGIN_OBJ	= cli_net.o ../clsapi_cli.o
CLSAPI_CLI_NET_PLUGIN_LDFLAGS	= $(LDFLAGS) -shared -fPIC -L. -lclsapi_net -luci -lubus -lubox


all: build install

build: $(CLSAPI_NET_LIB) $(CLSAPI_NET_UBUS_PLUGIN) $(CLSAPI_CLI_NET_PLUGIN)

auto_gen_ubus:
	python3 ../autogen_clsapi_ubus.py clsapi_net.h autogened_ubus_net.h

auto_gen_cli:
	rm -f autogened_cli_net.h
	python ../autogen_clsapi_cli.py clsapi_net.h autogened_cli_net.h

%.o: %.c
	$(CC) $(CLSAPI_NET_CFLAGS) -c -o $@ $<


$(CLSAPI_NET_LIB): $(CLSAPI_NET_LIB_OBJ)
	$(CC) $(CLSAPI_NET_LIB_LDFLAGS) -o $(CLSAPI_NET_LIB) $(CLSAPI_NET_LIB_OBJ)

$(CLSAPI_NET_UBUS_PLUGIN): auto_gen_ubus $(CLSAPI_NET_UBUS_PLUGIN_OBJ)
	$(CC) $(CLSAPI_NET_UBUS_PLUGIN_LDFLAGS) -o $(CLSAPI_NET_UBUS_PLUGIN) $(CLSAPI_NET_UBUS_PLUGIN_OBJ)

$(CLSAPI_CLI_NET_PLUGIN): auto_gen_cli $(CLSAPI_CLI_NET_PLUGIN_OBJ)
	$(CC) $(CLSAPI_CLI_NET_PLUGIN_LDFLAGS) -o $(CLSAPI_CLI_NET_PLUGIN) $(CLSAPI_CLI_NET_PLUGIN_OBJ)

install:
	mkdir -p $(DEST_INC_DIR) $(DEST_LIB_DIR) $(DEST_PLUGIN_DIR) $(DEST_CLSAPI_CLI_DIR)
	cp $(CLSAPI_NET_H) $(DEST_INC_DIR)
	cp $(CLSAPI_NET_LIB) $(DEST_LIB_DIR)
	if [ -f $(CLSAPI_CLI_NET_PLUGIN) ]; then cp $(CLSAPI_CLI_NET_PLUGIN) $(DEST_CLSAPI_CLI_DIR); fi
	if [ -f $(CLSAPI_NET_UBUS_PLUGIN) ]; then cp $(CLSAPI_NET_UBUS_PLUGIN) $(DEST_PLUGIN_DIR); fi

clean:
	rm -rf *.o *.so
