#
# Copyright (c) 2020-2021, Renesas Electronics Corporation. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# G2L-specific overrides - MUST be set BEFORE include rz_common.mk
# rz_common.mk uses ?= so these take precedence
ENABLE_PIE := 0
PROGRAMMABLE_RESET_ADDRESS := 1
RESET_TO_BL31 := 0

include plat/renesas/rz/common/rz_common.mk
include plat/renesas/rz/board/${BOARD}/rz_board.mk

# RZ/G2L boots U-Boot at EL1, EL2 is implemented but unused
# Must initialize EL2 registers to prevent U-Boot boot failures
INIT_UNUSED_NS_EL2 := 1

# Support REMOVE_UBOOT option to load and boot CM33 from ATF
ifdef REMOVE_UBOOT
$(eval $(call add_define,REMOVE_UBOOT))
endif

PLAT_INCLUDES	+=	-Iplat/renesas/rz/soc/g2l/include \
			-Iplat/renesas/rz/soc/cmn/include

DDR_SOURCES += plat/renesas/rz/soc/g2l/drivers/ddr/ddr_g2l.c

PLAT_BL_COMMON_SOURCES += plat/renesas/rz/soc/g2l/plat_stubs.c

include lib/libfdt/libfdt.mk

BL2_SOURCES		+=	lib/fconf/fconf_dyn_cfg_getter.c 	\
					plat/renesas/rz/common/rz_dt.c		\
					plat/renesas/rz/common/rz_fconf.c
FDT_SOURCES		:=	$(addprefix ${BUILD_PLAT}/fdts/, $(patsubst %.dtb,%.dts,$(DTB_FILE_NAME).dtb))

# Create FDT directory
fdt_dirs:
	@mkdir -p ${BUILD_PLAT}/fdts

# Create DTB file for BL2
${BUILD_PLAT}/fdts/${DTB_FILE_NAME}.dts: fdts/${DTB_FILE_NAME}.dts| ${BUILD_PLAT} fdt_dirs
	cp $< $@

${BUILD_PLAT}/fdts/${DTB_FILE_NAME}.dtb: fdts/${DTB_FILE_NAME}.dts | ${BUILD_PLAT} fdt_dirs

# Define paths for the BL2 binary and DTB
BOARD_NAME := $(shell echo $(BOARD) | awk -F'_' '{print $$1}')

# Define paths for the BL2 with DTB merged binary (target file per build guide)
BL2_ELF    := ${BUILD_PLAT}/bl2/bl2.elf
BL2_DTB    := ${BUILD_PLAT}/fdts/${DTB_FILE_NAME}.dtb
BL2_OUTPUT := ${BUILD_PLAT}/bl2_with_dtb-${BOARD_NAME}.bin

ifeq (${TRUSTED_BOARD_BOOT}, 0)
BL2_BASE := 0x12000
else
BL2_BASE := 0x13000
endif

SRAM_LIMIT := $(shell printf "%d" 0x1D000)
RZG2L_DTB_BASE_HEX := $(shell grep 'define RZG2L_DTB_BASE' plat/renesas/rz/common/include/rzg2l_def.h | sed -E 's/.*\((0x[0-9A-Fa-f]+)\).*/\1/')
BL2_BIN_LIMIT := $(shell printf "0x%X" $$(( $(RZG2L_DTB_BASE_HEX) - $(BL2_BASE) )))
BL2_BIN_LIMIT_DEC := $(shell printf "%d" $(BL2_BIN_LIMIT))

# Rule for creating the merged BL2 with DTB file.
# Always extracts a fresh pure binary from bl2.elf to avoid reusing a
# previously merged bl2.bin as input.
bl2_with_dtb: ${BL2_ELF} ${BL2_DTB}
	@echo "Embedding DTB into BL2 with dynamic padding..."
	@$(CROSS_COMPILE)objcopy -O binary ${BL2_ELF} /tmp/bl2_raw_$$$$.bin; \
	BL2_SIZE=$$(wc -c < /tmp/bl2_raw_$$$$.bin | awk '{print $$1}'); \
	PADDING=$$(($(BL2_BIN_LIMIT_DEC) - $$BL2_SIZE)); \
	if [ $$PADDING -lt 0 ]; then \
		echo "Error: BL2 size exceeds available space before BL2_BIN_LIMIT ($(BL2_BIN_LIMIT_DEC))"; \
		echo "INFO: BL2 size : $$BL2_SIZE bytes"; \
		rm -f /tmp/bl2_raw_$$$$.bin; \
		exit 1; \
	fi; \
	DTB_SIZE=$$(wc -c < ${BL2_DTB} | awk '{print $$1}'); \
	cat /tmp/bl2_raw_$$$$.bin > bl2_padded.bin; \
	dd if=/dev/zero bs=1 count=$$PADDING >> bl2_padded.bin 2>/dev/null; \
	cat bl2_padded.bin ${BL2_DTB} > ${BL2_OUTPUT}; \
	rm -f bl2_padded.bin /tmp/bl2_raw_$$$$.bin; \
	MERGED_SIZE=$$(wc -c < ${BL2_OUTPUT} | awk '{print $$1}'); \
	if [ $$MERGED_SIZE -gt $(SRAM_LIMIT) ]; then \
		echo "Error: Total size of BL2 + padding + DTB ($$MERGED_SIZE bytes) exceeds SRAM limit ($(SRAM_LIMIT) bytes)"; \
		exit 1; \
	fi; \
	BL2_BINARY_LIMIT_SIZE=$$(($$BL2_SIZE + $$PADDING)); \
	DTB_BASE=$$(( $(BL2_BASE) + $$BL2_BINARY_LIMIT_SIZE )); \
	echo "INFO: Created merged binary: ${BL2_OUTPUT}"; \
	echo "INFO: BL2 size           : $$BL2_SIZE bytes"; \
	echo "INFO: DTB size           : $$DTB_SIZE bytes"; \
	echo "INFO: Total merged size  : $$MERGED_SIZE bytes"; \
	echo "INFO: BL2 limit size     : 0x$$(printf '%X' $$BL2_BINARY_LIMIT_SIZE)"; \
	echo "INFO: DTB base address   : 0x$$(printf '%X' $$DTB_BASE)"
