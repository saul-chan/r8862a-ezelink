# Broadband Forum IEEE 1905.1/1a stack
# 
# Copyright (c) 2017, Broadband Forum
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
# 
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 
# Subject to the terms and conditions of this license, each copyright
# holder and contributor hereby grants to those receiving rights under
# this license a perpetual, worldwide, non-exclusive, no-charge,
# royalty-free, irrevocable (except for failure to satisfy the
# conditions of this license) patent license to make, have made, use,
# offer to sell, sell, import, and otherwise transfer this software,
# where such license applies only to those patent claims, already
# acquired or hereafter acquired, licensable by such copyright holder or
# contributor that are necessarily infringed by:
# 
# (a) their Contribution(s) (the licensed copyrights of copyright holders
#     and non-copyrightable additions of contributors, in source or binary
#     form) alone; or
# 
# (b) combination of their Contribution(s) with the work of authorship to
#     which such Contribution(s) was added by such copyright holder or
#     contributor, if, at the time the Contribution is added, such addition
#     causes such combination to be necessarily infringed. The patent
#     license shall not apply to any other combinations which include the
#     Contribution.
# 
# Except as expressly stated above, no rights or licenses from any
# copyright holder or contributor is granted under this license, whether
# expressly, by implication, estoppel or otherwise.
# 
# DISCLAIMER
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
# IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
# TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
# PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
# OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
# TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
# USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
# DAMAGE.

################################################################################
# Configuration section. Tune the parameters below according to your needs
################################################################################

# Platform dependent CONFIGs
#
# You can either "export PLATFORM=..." (and "FLAVOUR", if needed) before
# calling this Makefile or let "PLATFORM" undefined to use the default value
# (which is "linux")

ifndef PLATFORM

PLATFORM := linux
FLAVOUR  := openwrt
#FLAVOUR  := arm_wrt1900acx
#FLAVOUR  := x86_windows_mingw

endif

ifeq ($(PLATFORM),linux)
    ifeq ($(FLAVOUR), openwrt)
        CC        := gcc

        CCFLAGS   += $(CFLAGS)
        CCFLAGS   += -D_FLAVOUR_X86_GENERIC_ -DUSE_UCI
        CCFLAGS   += -I$(STAGING_DIR)/usr/include/libnl-tiny

        LDFLAGS       += -lrt -lpthread   # For threads
        LDFLAGS       += -lcrypto         # For WPS crypto
        LDFLAGS       += -lubus -lubox
        LDFLAGS       += -lm -lnl-tiny
        LDFLAGS       += -luci

        AL_SUPPORTED  := yes
        HLE_SUPPORTED := no
    else ifeq ($(FLAVOUR), generic)
        CC        := gcc

        CCFLAGS   += $(CFLAGS)
        CCFLAGS   += -D_FLAVOUR_X86_GENERIC_
        CCFLAGS   += -I$(STAGING_DIR)/usr/include/libnl3 -I./

        LDFLAGS       += -lrt -lpthread   # For threads
        LDFLAGS       += -lcrypto         # For WPS crypto
        LDFLAGS       += -lubus -lubox
        LDFLAGS       += -lm -lnl-genl-3 -lnl-3

        AL_SUPPORTED  := yes
        HLE_SUPPORTED := no
    else
        $(error "Linux FLAVOUR unknown")
    endif

    CCFLAGS       += -g -O0 -Wall -Werror #-Wextra
    CCFLAGS       += -D_HOST_IS_LITTLE_ENDIAN_=1 -DMAX_NETWORK_SEGMENT_SIZE=1500
    CCFLAGS       += -D_GNU_SOURCE
else
    $(error "'PLATFORM' variable was not set to a valid value")
endif

# Platform independent CONFIG
# set openssl to 1.1.1
CCFLAGS += -DOPENSSL_API_COMPAT=0x10101000L
#CCFLAGS += -DDO_NOT_ACCEPT_UNAUTHENTICATED_COMMANDS
CCFLAGS += -DSEND_EMPTY_TLVS
CCFLAGS += -DFIX_BROKEN_TLVS
CCFLAGS += -DSPEED_UP_DISCOVERY
#
# These are special flags that change the way the implementation behaves.
# The README file contains more information regarding them.
# Version flag to identify the binaries

#CCFLAGS += -DREGISTER_EXTENSION_BBF
#
# These are special flags to enable Protocol extensions

################################################################################
# End of configuration section. Do not touch anything from this point on
################################################################################


# Shortcuts for later
#
SRC_FOLDER    := $(shell pwd)/src
OUTPUT_FOLDER := $(shell pwd)/output

# "Common" library. Includes platform specific code that is used by both the
# AL and HLE entities (example: "PLATFORM_MALLOC()", "PLATFORM_PRINTF()", ...)
# 
UTILS_LIB    := $(OUTPUT_FOLDER)/libutils.a
UTILS_INC    := $(SRC_FOLDER)/utils/include
UTILS_INC    += $(SRC_FOLDER)/utils/linux/
UTILS_INC    += $(SRC_FOLDER)/
UTILS_INC    += $(SRC_FOLDER)/extension/

# "Factory" library. Includes "pure" 1905 related functions to parse/build 1905
# packets. This library is also used by both AL and HLE but most (if not all) of
# it is platform independent.
#
FACTORY_LIB   := $(OUTPUT_FOLDER)/libfactory.a
FACTORY_INC   := $(SRC_FOLDER)/factory/include
FACTORY_INC   += $(sort $(dir $(wildcard $(SRC_FOLDER)/factory/include/extensions/*/)))

# The actual binaries that we are going to build: one for the AL and another one
# for the HLE entity.
#
AL_EXE        := $(OUTPUT_FOLDER)/clmeshd
HLE_EXE       := $(OUTPUT_FOLDER)/hle_entity

# Compilation will need to create new directories during the process
#
MKDIR         := mkdir -p

# Export all these variable to child Makefiles
#
export


################################################################################
# Targets
################################################################################

.PHONY: all
all:
ifeq ($(AL_SUPPORTED),yes)
all: $(AL_EXE)
endif
ifeq ($(HLE_SUPPORTED),yes)
all: $(HLE_EXE)
endif

.PHONY: $(UTILS_LIB)
$(UTILS_LIB):
	$(MAKE) -C src/utils

.PHONY: $(FACTORY_LIB)
$(FACTORY_LIB):
	$(MAKE) -C src/factory 

$(AL_EXE): $(UTILS_LIB) $(FACTORY_LIB)
	$(MAKE) -C src

$(HLE_EXE): $(UTILS_LIB) $(FACTORY_LIB)
	$(MAKE) -C src/hle


.PHONY: unit_tests
unit_tests: all
	$(MAKE) -C src/factory unit_tests


.PHONY: clean
clean:
	$(MAKE) -C src/        clean


