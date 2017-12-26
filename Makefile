#
# Makefile for n64chain ROMs.
#
# n64chain: A (free) open-source N64 development toolchain.
# Copyright 2014-16 Tyler J. Stachecki <stachecki.tyler@gmail.com>
#
# This file is subject to the terms and conditions defined in
# 'LICENSE', which is part of this source code package.
#

ifdef SystemRoot
FIXPATH = $(subst /,\,$1)
RM = del /Q
else
FIXPATH = $1
RM = rm -f
endif

ROM_NAME = $(notdir $(CURDIR))

AS = $(call FIXPATH,$(CURDIR)/../tools/bin/mips64-elf-as)
AR = $(call FIXPATH,$(CURDIR)/../tools/bin/mips64-elf-gcc-ar)
CC = $(call FIXPATH,$(CURDIR)/../tools/bin/mips64-elf-gcc)
LD = $(call FIXPATH,$(CURDIR)/../tools/bin/mips64-elf-ld)
MAKE = $(call FIXPATH,$(CURDIR)/../tools/bin/make)
OBJCOPY = $(call FIXPATH,$(CURDIR)/../tools/bin/mips64-elf-objcopy)

CHECKSUM = $(call FIXPATH,$(CURDIR)/../tools/bin/checksum)

CFLAGS = -Wall -Wextra -std=gnu11 -Wno-main \
	-I../libn64/include -I../libn64 -I.

OPTFLAGS = -Os -march=vr4300 -mtune=vr4300 -mabi=eabi -mgp32 -mlong32 \
	-flto -ffat-lto-objects -ffunction-sections -fdata-sections \
	-G4 -mno-extern-sdata -mgpopt -mfix4300 -mbranch-likely

CFILES = $(call FIXPATH,\
	src/game.c \
	src/graphics.c \
	src/intro.c \
	src/puzzle.c \
	src/victory.c \
	src/controller.c \
)

RAWFILES = $(call FIXPATH,\
	images/1.raw \
	images/2.raw \
	images/3.raw \
	images/4.raw \
	images/5.raw \
	images/6.raw \
	images/7.raw \
	images/8.raw \
	images/9.raw \
	images/10.raw \
	images/11.raw \
	images/12.raw \
	images/13.raw \
	images/14.raw \
	images/15.raw \
	images/16.raw \
	images/17.raw \
	images/18.raw \
	images/balloons.raw \
	images/doge.raw \
	images/flipped.raw \
	images/introbg.raw \
	images/intromatch.raw \
	images/introshiba.raw \
	images/sidebar.raw \
)

OBJFILES = \
	$(CFILES:.c=.o) \
	$(RAWFILES:.raw=.o) \

DEPFILES = $(OBJFILES:.o=.d)

#
# Primary targets.
#
all: $(ROM_NAME).z64

$(ROM_NAME).z64: $(ROM_NAME).elf
	@echo $(call FIXPATH,"Building: $(ROM_NAME)/$@")
	@$(OBJCOPY) -O binary $< $@
	@$(CHECKSUM) $(call FIXPATH,../libn64/header.bin) $@

$(ROM_NAME).elf: libn64 $(OBJFILES) $(UCODEBINS)
	@echo $(call FIXPATH,"Building: $(ROM_NAME)/$@")
	@$(CC) $(CFLAGS) $(OPTFLAGS) -Wl,-Map=$(ROM_NAME).map -nostdlib \
		-T$(call FIXPATH,../libn64/rom.ld) -o $@ $(OBJFILES) src/si.ld \
		-L$(call FIXPATH,../libn64) -ln64

#
# Generic compilation/assembly targets.
#
%.o: %.c
	@echo $(call FIXPATH,"Compiling: $(ROM_NAME)/$<")
	@$(CC) $(CFLAGS) $(OPTFLAGS) -MMD -c $< -o $@

%.o: %.raw
	$(LD) -r -b binary $^ -o $@

.PHONY: libn64
libn64:
	@$(MAKE) -sC $(call FIXPATH,../libn64)

#
# Clean project target.
#
.PHONY: clean
clean:
	@echo "Cleaning $(ROM_NAME)..."
	@$(RM) $(ROM_NAME).map $(ROM_NAME).elf $(ROM_NAME).z64 \
		$(DEPFILES) $(OBJFILES)

#
# Use computed dependencies.
#
-include $(DEPFILES)

