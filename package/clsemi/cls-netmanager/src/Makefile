#
# Copyright (C) 2023 Clourney Semiconductor - All Rights Reserved.
# SPDX-License-Identifier: BSD-3-Clause
# https://spdx.org/licenses
#

OUTPUT_FOLDER = ./output

CLSNETMANAGER_CFLAGS = $(filter-out -Wno-error=unused-but-set-variable -Wno-error=unused-result -Wno-error=unused-variable \
				-Wno-error=implicit-function-declaration, $(CFLAGS)) -fPIC -Werror -Wall

CLSNETMANAGER_CFLAGS += $(CFLAGS) -I.. -D_GNU_SOURCE -I$(STAGING_DIR)/usr/include/libubox -I$(STAGING_DIR)/usr/include/libnl-tiny/
CLSNETMANAGER_LDFLAGS =  $(LDFLAGS) -lpthread -liwinfo -lubus -lubox -luci -lnl-tiny -lclsapi_sys -lclsapi_wifi -lclsapi_net -lclsapi_base

CLSNETMANAGER_SRC := $(wildcard *.c)
CLSNETMANAGER_OBJ := $(patsubst %.c,$(OUTPUT_FOLDER)/%.o,$(CLSNETMANAGER_SRC))
CLSNETMANAGER_BIN = $(OUTPUT_FOLDER)/clsnetmanager

MKDIR := mkdir -p

all: build

build: $(CLSNETMANAGER_BIN)

$(OUTPUT_FOLDER)/%.o: %.c
	$(MKDIR) $(OUTPUT_FOLDER)
	$(CC) $(CLSNETMANAGER_CFLAGS) -c -o $@ $<

$(CLSNETMANAGER_BIN): $(CLSNETMANAGER_OBJ)
	$(CC) $(CLSNETMANAGER_LDFLAGS) -o $(CLSNETMANAGER_BIN) $(CLSNETMANAGER_OBJ)

clean:
	rm -rf *.o $(CLSNETMANAGER_BIN)

