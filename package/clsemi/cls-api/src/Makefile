#
# Copyright (C) 2023 Clourney Semiconductor - All Rights Reserved.
# SPDX-License-Identifier: BSD-3-Clause
# https://spdx.org/licenses
#

DESTDIR ?= ./output/
DEST_INC_DIR ?= $(DESTDIR)/usr/include/
DEST_BIN_DIR ?= $(DESTDIR)/bin/

CLSAPI_DEBUG=1
DBG_LVL_DEFAULT=DBG_LVL_DEBUG
DBG_SYSLOG=1

CLSAPI_CFLAGS	= $(filter-out -Wno-error=unused-but-set-variable -Wno-error=unused-result -Wno-error=unused-variable \
				-Wno-error=implicit-function-declaration, $(CFLAGS)) -fPIC -Werror -Wall

CLSAPI_CFLAGS += $(if $(CLSAPI_PLAT_OPENWRT),-DCLSAPI_PLAT_OPENWRT)

ifdef CLSAPI_DEBUG
CLSAPI_CFLAGS += -DCLSAPI_DEBUG
CLSAPI_CFLAGS += -DDBG_LVL_DEFAULT=DBG_LVL_INFO
ifdef DBG_SYSLOG
CLSAPI_CFLAGS += -DDBG_SYSLOG
endif
endif

CLSAPI_BIN		= clsapid
CLSAPI_BIN_OBJ	= main.o clsapi_util.o
CLSAPI_BIN_LDFLAGS	= $(LDFLAGS) -lubox -lubus -ljson-c
CLSAPI_H	= cls_common.h clsapi_common.h clsapi_util.h
CLSAPI_CLI	= clsapi
CLSAPI_CLI_OBJ	= clsapi_cli.o clsapi_util.o


all: build install

build: $(CLSAPI_BIN) build_sub_dirs build_test $(CLSAPI_CLI)

%.o: %.c
	$(CC) $(CLSAPI_CFLAGS) -c -o $@ $<

$(CLSAPI_BIN): $(CLSAPI_BIN_OBJ)
	$(CC) $(CLSAPI_CFLAGS) $(CLSAPI_BIN_LDFLAGS) -o $(CLSAPI_BIN) $(CLSAPI_BIN_OBJ)

$(CLSAPI_CLI): $(CLSAPI_CLI_OBJ)
	$(CC) $(CLSAPI_CFLAGS) -o $(CLSAPI_CLI) $(CLSAPI_CLI_OBJ)


SUBDIRS := $(shell find . -type d ! -name '.' ! -name '..' ! -name 'test' | sort -n)
build_sub_dirs:
	for d in $(SUBDIRS); do [ -d $$d ] && $(MAKE) CLSAPI_CFLAGS="$(CLSAPI_CFLAGS)" -C $$d; done

build_test:
	$(MAKE) CLSAPI_CFLAGS="$(CLSAPI_CFLAGS)" -C test

install:
	mkdir -p $(DEST_INC_DIR) $(DEST_BIN_DIR)
	cp $(CLSAPI_BIN) $(DEST_BIN_DIR)
	cp $(CLSAPI_CLI) $(DEST_BIN_DIR)
	ln -sf $(CLSAPI_CLI) $(DEST_BIN_DIR)/cls_get
	ln -sf $(CLSAPI_CLI) $(DEST_BIN_DIR)/cls_set
	ln -sf $(CLSAPI_CLI) $(DEST_BIN_DIR)/cls_add
	ln -sf $(CLSAPI_CLI) $(DEST_BIN_DIR)/cls_del
	cp $(CLSAPI_H) $(DEST_INC_DIR)

clean:
	rm -rf *.o $(CLSAPI_BIN)
