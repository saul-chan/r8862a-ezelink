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

CLSAPI_WIFI_CFLAGS	= $(CLSAPI_CFLAGS) -I.. -I$(DEST_INC_DIR) -D_GNU_SOURCE -I$(STAGING_DIR)/usr/include/libnl-tiny/

CLSAPI_WIFI_LIB		= libclsapi_wifi.so
CLSAPI_WIFI_LIB_OBJ	= wifi_common.o wifi.o ../clsapi_util.o ../clsapi_common.o nl80211_adapter.o wpa_adapter.o cls_hostapd_config.o
CLSAPI_WIFI_LIB_LDFLAGS	= $(LDFLAGS) -shared -fPIC -lnl-tiny -L$(DEST_LIB_DIR) -lclsapi_base
CLSAPI_WIFI_H	= clsapi_wifi.h

CLSAPI_WIFI_UBUS_PLUGIN		= clsapi_ubus_wifi.so
CLSAPI_WIFI_UBUS_PLUGIN_OBJ	= ubus_wifi.o
CLSAPI_WIFI_UBUS_PLUGIN_LDFLAGS	= $(LDFLAGS) -shared -fPIC -L. -lclsapi_wifi

CLSAPI_CLI_WIFI_PLUGIN		= clsapi_cli_wifi.so
CLSAPI_CLI_WIFI_PLUGIN_OBJ	= cli_wifi.o ../clsapi_cli.o
CLSAPI_CLI_WIFI_PLUGIN_LDFLAGS	= $(LDFLAGS) -shared -fPIC -L. -lclsapi_wifi


all: build install

build: $(CLSAPI_WIFI_LIB) $(CLSAPI_WIFI_UBUS_PLUGIN) $(CLSAPI_CLI_WIFI_PLUGIN)

auto_gen_ubus:
	if [ -f ../autogen_clsapi_ubus.py ]; then python3 ../autogen_clsapi_ubus.py clsapi_wifi.h autogened_ubus_wifi.h; fi

auto_gen_cli:
	rm -f autogened_cli_wifi.h
	python ../autogen_clsapi_cli.py clsapi_wifi.h autogened_cli_wifi.h

%.o: %.c
	$(CC) $(CLSAPI_WIFI_CFLAGS) -c -o $@ $<


$(CLSAPI_WIFI_LIB): $(CLSAPI_WIFI_LIB_OBJ)
	$(CC) $(CLSAPI_WIFI_LIB_LDFLAGS) -lm -o $(CLSAPI_WIFI_LIB) $(CLSAPI_WIFI_LIB_OBJ)

$(CLSAPI_WIFI_UBUS_PLUGIN): auto_gen_ubus $(CLSAPI_WIFI_UBUS_PLUGIN_OBJ)
	$(CC) $(CLSAPI_WIFI_UBUS_PLUGIN_LDFLAGS) -o $(CLSAPI_WIFI_UBUS_PLUGIN) $(CLSAPI_WIFI_UBUS_PLUGIN_OBJ)

$(CLSAPI_CLI_WIFI_PLUGIN): auto_gen_cli $(CLSAPI_CLI_WIFI_PLUGIN_OBJ)
	$(CC) $(CLSAPI_CLI_WIFI_PLUGIN_LDFLAGS) -o $(CLSAPI_CLI_WIFI_PLUGIN) $(CLSAPI_CLI_WIFI_PLUGIN_OBJ)

install:
	mkdir -p $(DEST_INC_DIR) $(DEST_LIB_DIR) $(DEST_PLUGIN_DIR) $(DEST_CLSAPI_CLI_DIR)
	cp $(CLSAPI_WIFI_H) $(DEST_INC_DIR)
	cp $(CLSAPI_WIFI_LIB) $(DEST_LIB_DIR)
	if [ -f $(CLSAPI_CLI_WIFI_PLUGIN) ]; then cp $(CLSAPI_CLI_WIFI_PLUGIN) $(DEST_CLSAPI_CLI_DIR); fi
	if [ -f $(CLSAPI_WIFI_UBUS_PLUGIN) ]; then cp $(CLSAPI_WIFI_UBUS_PLUGIN) $(DEST_PLUGIN_DIR); fi

clean:
	rm -rf *.o *.so
