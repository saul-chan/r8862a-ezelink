#
# Copyright (C) 2023 Clourney Semiconductor - All Rights Reserved.
# SPDX-License-Identifier: BSD-3-Clause
# https://spdx.org/licenses
#

OUTPUT_FOLDER = output

LED_NOTIFY_CFLAGS = $(filter-out -Wno-error=unused-but-set-variable -Wno-error=unused-result -Wno-error=unused-variable \
				-Wno-error=implicit-function-declaration, $(CFLAGS)) -fPIC -Werror -Wall

LED_NOTIFY_CFLAGS	+= $(CFLAGS) -I$(STAGING_DIR)/usr/include/ -I$(STAGING_DIR)/usr/include/libubox

LED_NOTIFY_LD_FLAGS = $(LDFLAGS) -lubus -lubox

MKDIR := mkdir -p

LED_BIN = led_notify

all: build install

build: $(LED_BIN)

$(LED_BIN):
	$(MKDIR) $(OUTPUT_FOLDER)
	$(CC) $(LED_NOTIFY_CFLAGS) $(LED_NOTIFY_LD_FLAGS) cls_led.c -o led_notify

install:
	cp $(LED_BIN) $(OUTPUT_FOLDER)/

clean:
	rm -rf *.o $(LED_BIN)
