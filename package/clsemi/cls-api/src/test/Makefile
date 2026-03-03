#
# Copyright (C) 2023 Clourney Semiconductor - All Rights Reserved.
# SPDX-License-Identifier: BSD-3-Clause
# https://spdx.org/licenses
#

DEST_INC_DIR ?= $(DESTDIR)/usr/include/
DEST_LIB_DIR ?= $(DESTDIR)/usr/lib/

TEST_CFLAGS	= $(CLSAPI_CFLAGS) -I.. -I$(DEST_INC_DIR) -I$(STAGING_DIR)/usr/include/libubox

TEST_LD_FLAGS = $(LDFLAGS) -liwinfo -lubus -lubox -luci -lnl-tiny -L$(DEST_LIB_DIR) -lclsapi_wifi \
				-lclsapi_net -lclsapi_sys -lclsapi_base
TEST_BIN = vbss_agent vbss_ac


all: build install

build: $(TEST_BIN)

vbss_agent: vbss_agent.o
	$(CC) $(TEST_CFLAGS) $(TEST_LD_FLAGS) -o vbss_agent vbss_agent.o

vbss_ac: vbss_ac.o vbss_ac_x86
	$(CC) $(TEST_CFLAGS) $(TEST_LD_FLAGS) -o vbss_ac vbss_ac.o

vbss_ac_x86:
	gcc vbss_ac.c -o vbss_ac_x86


%.o: %.c
	$(CC) $(TEST_CFLAGS) -c -o $@ $<


install:
	mkdir -p $(DESTDIR)/bin
	cp $(TEST_BIN) $(DESTDIR)/bin

clean:
	rm -rf *.o $(TEST_BIN)
