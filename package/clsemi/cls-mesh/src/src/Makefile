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

# When calling this Makefile, the following environment variables must be set:
#
#   CC ------------> Path to the compiler
#   CCFLAGS -------> Extra flags to use while compiling
#   LDGLAGS -------> Extra flags to use while linking
#
#   PLATFORM      -> Possible values: "linux"
#   OUTPUT_FOLDER -> Absolute path to the folder where binaries will be built
#
#   UTILS_LIB ----> Absolute path to the "utils library" *.a file
#   UTILS_INC ----> Absolute path to the folder containing utils (to all
#                    sub-projects) header files
#
#   FACTORY_LIB   -> Absolute path to the "factory library" *.a file 
#   FACTORY_INC   -> Absolute path to the folder containing the "factory
#                    library" header files
#
#   AL_EXE --------> Absolute path to the final AL entity binary
#
#   MKDIR ---------> Tool to create a directory
#

EXE                        := $(AL_EXE)

AL_DIRECTORY               := al

SRC_AL                     := $(wildcard *.c)
SRC_PLATFORM               := $(wildcard $(PLATFORM)/*.c)
SRC_EXTENSION              := $(shell find ./extension -name \*.c)

OBJ_AL                     := $(patsubst %.c,$(OUTPUT_FOLDER)/tmp/$(AL_DIRECTORY)/%.o,$(SRC_AL))
OBJ_PLATFORM               := $(patsubst %.c,$(OUTPUT_FOLDER)/tmp/$(AL_DIRECTORY)/%.o,$(SRC_PLATFORM))
OBJ_EXTENSION                 := $(patsubst %.c,$(OUTPUT_FOLDER)/tmp/$(AL_DIRECTORY)/%.o,$(SRC_EXTENSION))

INTERNAL_INC_AL            := include
INTERNAL_INC_PLATFORM      := $(PLATFORM) interface
INTERNAL_INC_EXTENSION     := extension

EXTERNAL_INC               := $(UTILS_INC) $(FACTORY_INC)

HDR_AL                     := $(wildcard *.h)
HDR_PLATFORM               := $(shell find $(INTERNAL_INC_PLATFORM)    $(EXTERNAL_INC) -name *.h)
HDR_EXTENSION                 := $(shell find $(INTERNAL_INC_EXTENSION)    $(EXTERNAL_INC) -name *.h)

################################################################################
# Targets
################################################################################

.PHONY: all
all: $(EXE)


$(EXE):  $(OBJ_AL) $(OBJ_PLATFORM) $(OBJ_EXTENSION) $(FACTORY_LIB) $(UTILS_LIB)
	$(CC) $^ $(LDFLAGS) -o $@

$(OBJ_AL) : $(OUTPUT_FOLDER)/tmp/$(AL_DIRECTORY)/%.o : %.c $(HDR_AL)
	$(foreach directory, $(sort $(dir $(wildcard $(SRC_AL)))), $(MKDIR) $(OUTPUT_FOLDER)/tmp/$(AL_DIRECTORY)/$(directory);)
	$(CC) $(CCFLAGS) -c $(addprefix -I,$(INTERNAL_INC_AL) $(EXTERNAL_INC)) $< -o $@

$(OBJ_PLATFORM)    : $(OUTPUT_FOLDER)/tmp/$(AL_DIRECTORY)/%.o : %.c $(HDR_PLATFORM)
	$(foreach directory, $(sort $(dir $(wildcard $(SRC_PLATFORM)))), $(MKDIR) $(OUTPUT_FOLDER)/tmp/$(AL_DIRECTORY)/$(directory);)
	$(CC) $(CCFLAGS) -c $(addprefix -I,$(INTERNAL_INC_PLATFORM) $(INTERNAL_INC_AL) $(EXTERNAL_INC)) $< -o $@

$(OBJ_EXTENSION)    : $(OUTPUT_FOLDER)/tmp/$(AL_DIRECTORY)/%.o : %.c $(HDR_EXTENSION)
	$(foreach directory, $(sort $(dir $(wildcard $(SRC_EXTENSION)))), $(MKDIR) $(OUTPUT_FOLDER)/tmp/$(AL_DIRECTORY)/$(directory);)
	$(CC) $(CCFLAGS) -c $(addprefix -I,$(INTERNAL_INC_EXTENSION) $(INTERNAL_INC_AL) $(EXTERNAL_INC)) $< -o $@

.PHONY: clean
clean:
	rm -rf $(EXE)
	rm -rf $(OUTPUT_FOLDER)/tmp/$(AL_DIRECTORY)
