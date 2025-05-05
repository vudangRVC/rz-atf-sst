#
# Copyright (c) 2021, Renesas Electronics Corporation. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include plat/renesas/rz/common/rz_common.mk
include plat/renesas/rz/board/${BOARD}/rz_board.mk

PLAT_INCLUDES	+=	-Iplat/renesas/rz/soc/v2l/include

DDR_SOURCES += plat/renesas/rz/soc/v2l/drivers/ddr/ddr_v2l.c

include lib/libfdt/libfdt.mk

BL2_SOURCES		+=	lib/fconf/fconf_dyn_cfg_getter.c 	\
					plat/renesas/rz/common/rz_dt.c		\
					plat/renesas/rz/common/rz_fconf.c
FDT_SOURCES		:=	$(addprefix ${BUILD_PLAT}/fdts/, $(patsubst %.dtb,%.dts,$(DTB_FILE_NAME).dtb))

# Create DTB file for BL2
${BUILD_PLAT}/fdts/${DTB_FILE_NAME}.dts: fdts/${DTB_FILE_NAME}.dts| ${BUILD_PLAT} fdt_dirs
	cp $< $@

${BUILD_PLAT}/fdts/${DTB_FILE_NAME}.dtb: fdts/${DTB_FILE_NAME}.dts | ${BUILD_PLAT} fdt_dirs

# Define paths for the BL2 binary and DTB
BOARD_NAME := $(shell echo $(BOARD) | awk -F'_' '{print $$1}')

# BL2_ELF: TF-A places the ELF at <build>/<plat>/<mode>/bl2/bl2.elf.
# Always use the ELF as source to extract a clean raw binary, avoiding
# the case where bl2.bin already contains a merged DTB from a prior run.
BL2_ELF    := ${BUILD_PLAT}/bl2/bl2.elf
BL2_DTB    := ${BUILD_PLAT}/fdts/${DTB_FILE_NAME}.dtb
BL2_OUTPUT := ${BUILD_PLAT}/bl2_with_dtb-${BOARD_NAME}.bin

ifeq (${TRUSTED_BOARD_BOOT}, 0)
BL2_BASE := 0x12000
else
BL2_BASE := 0x13000
endif

SRAM_LIMIT := $(shell printf "%d" 0x1D000)
BL2_BIN_LIMIT := $(shell grep 'RZG2L_BINARY_LIMIT_SIZE' plat/renesas/rz/common/include/rzg2l_def.h | sed -E 's/.*\((0x[0-9A-Fa-f]+)\).*/\1/')
BL2_BIN_LIMIT_DEC := $(shell printf "%d" $(BL2_BIN_LIMIT))

# Rule for creating the merged BL2+DTB binary.
# Extracts a fresh raw binary from bl2.elf each time so that calling this
# rule multiple times (e.g. different boards in one build session) is safe.
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
	cat /tmp/bl2_raw_$$$$.bin > /tmp/bl2_padded_$$$$.bin; \
	dd if=/dev/zero bs=1 count=$$PADDING >> /tmp/bl2_padded_$$$$.bin 2>/dev/null; \
	cat /tmp/bl2_padded_$$$$.bin ${BL2_DTB} > ${BL2_OUTPUT}; \
	rm -f /tmp/bl2_raw_$$$$.bin /tmp/bl2_padded_$$$$.bin; \
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
