#
# Copyright (c) 2021-2024, Clourney Semiconductor. All rights reserved.
#
# This software and/or documentation is licensed by Clourney Semiconductor under limited terms and conditions.
# Reproduction and redistribution in binary or source form, with or without modification,
# for use solely in conjunction with a Clourney Semiconductor chipset, is permitted in condition which
# must retain the above copyright notice.
#
# By using this software and/or documentation, you agree to the limited terms and conditions.
#

all: build install

build: demo_ubus_uci_wifi_cfg demo_clsapi

demo_ubus_uci_wifi_cfg:
	$(CC) $(CFLAGS) -o $@ demo_ubus_uci_wifi_cfg.c $(LDFLAGS) -lubox -lubus -luci

demo_clsapi:
	$(CC) $(CFLAGS) -Wall -Werror -o $@ demo_clsapi.c $(LDFLAGS) -lubox -lubus -luci -lclsapi_wifi -lclsapi_base -liwinfo -lnl-tiny


install:
	mkdir -p ./bin/
	mv demo_ubus_uci_wifi_cfg ./bin/
	mv demo_clsapi ./bin/

clean:
	rm -f ./bin/
