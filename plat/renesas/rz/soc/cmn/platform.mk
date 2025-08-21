#
# Copyright (c) 2020-2021, Renesas Electronics Corporation. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include plat/renesas/rz/common/rz_common.mk
include plat/renesas/rz/board/${BOARD}/rz_board.mk

PLAT_INCLUDES	+=	-Iplat/renesas/rz/soc/cmn/include

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

# Define the input file and target for the merged binary
BL2_IMAGE  := ${BUILD_PLAT}/bl2.bin
BL2_DTB    := ${BUILD_PLAT}/fdts/${DTB_FILE_NAME}.dtb
BL2_OUTPUT := ${BUILD_PLAT}/bl2_with_dtb-${BOARD_NAME}.bin
BL2_FINAL  := ${BUILD_PLAT}/bl2.bin

ifeq (${TRUSTED_BOARD_BOOT}, 0)
RZCMN_BL2_BASE := 0x12000
else
RZCMN_BL2_BASE := 0x13000
endif

SRAM_LIMIT := $(shell printf "%d" 0x1D000)
BL2_BIN_LIMIT := $(shell grep 'RZCMN_DTB_LIMIT' plat/renesas/rz/common/include/rzcmn_def.h | sed -E 's/.*\((0x[0-9A-Fa-f]+)\).*/\1/')
BL2_BIN_LIMIT_DEC := $(shell printf "%d" $(BL2_BIN_LIMIT))

# Rule for creating the merged BL2 with DTB file
bl2_with_dtb: ${BL2_IMAGE} ${BL2_DTB} 
	@echo "Embedding DTB into BL2 with dynamic padding..."
	@BL2_SIZE=$$(wc -c < ${BL2_IMAGE} | awk '{print $$1}'); \
	PADDING=$$(($(BL2_BIN_LIMIT_DEC) - $$BL2_SIZE)); \
	if [ $$PADDING -lt 0 ]; then \
		echo "Error: BL2 size exceeds available space before BL2_BIN_LIMIT ($(BL2_BIN_LIMIT_DEC))"; \
		echo "INFO: BL2 size : $$BL2_SIZE bytes"; \
		exit 1; \
	fi; \
	DTB_SIZE=$$(wc -c < ${BL2_DTB} | awk '{print $$1}'); \
	cat ${BL2_IMAGE} > bl2_padded.bin; \
	dd if=/dev/zero bs=1 count=$$PADDING >> bl2_padded.bin; \
	cat bl2_padded.bin ${BL2_DTB} > ${BL2_OUTPUT}; \
	rm -f bl2_padded.bin; \
	MERGED_SIZE=$$(wc -c < ${BL2_OUTPUT} | awk '{print $$1}'); \
	if [ $$MERGED_SIZE -gt $(SRAM_LIMIT) ]; then \
		echo "Error: Total size of BL2 + padding + DTB ($$MERGED_SIZE bytes) exceeds SRAM limit ($(SRAM_LIMIT) bytes)"; \
		exit 1; \
	fi; \
	BL2_BINARY_LIMIT_SIZE=$$(($$BL2_SIZE + $$PADDING)); \
	DTB_BASE=$$(( $(RZCMN_BL2_BASE) + $$BL2_BINARY_LIMIT_SIZE )); \
	echo "INFO: Created merged binary: ${BL2_OUTPUT}"; \
	echo "INFO: BL2 size           : $$BL2_SIZE bytes"; \
	echo "INFO: DTB size           : $$DTB_SIZE bytes"; \
	echo "INFO: Total merged size  : $$MERGED_SIZE bytes"; \
	echo "INFO: BL2 limit size     : 0x$$(printf '%X' $$BL2_BINARY_LIMIT_SIZE)"; \
	echo "INFO: DTB base address   : 0x$$(printf '%X' $$DTB_BASE)"
	mv -f ${BL2_OUTPUT} ${BL2_FINAL}


# RZV2H

# # Define addr in SRAM for BL2 and DTB
# BL2_LOAD_ADDR_HEX := $(shell grep 'BL2_BASE' plat/renesas/rz/soc/v2h/include/platform_def.h | sed -E 's/.*\((0x[0-9A-Fa-f]+)\).*/\1/')
# DTB_LOAD_ADDR_HEX := $(shell grep 'V2H_DTB_LOAD_ADDR' plat/renesas/rz/soc/v2h/include/platform_def.h | sed -E 's/.*\((0x[0-9A-Fa-f]+)\).*/\1/')

# # Define file name
# BL2_IMAGE  := ${BUILD_PLAT}/bl2.bin
# BL2_DTB    := ${BUILD_PLAT}/fdts/${DTB_FILE_NAME}.dtb
# BL2_OUTPUT := ${BUILD_PLAT}/bl2_with_dtb.bin
# BL2_FINAL  := ${BUILD_PLAT}/bl2.bin

# # Rule for creating the merged BL2 with DTB file
# bl2_with_dtb: ${BL2_IMAGE} ${BL2_DTB}
# 	@echo "Merging BL2 and DTB with alignment and padding..."
# 	@BL2_SIZE=$$(stat -c %s ${BL2_IMAGE}); \
# 	DTB_SIZE=$$(stat -c %s ${BL2_DTB}); \
# 	BL2_LOAD_ADDR=$$(printf "%d" ${BL2_LOAD_ADDR_HEX}); \
# 	DTB_LOAD_ADDR=$$(printf "%d" ${DTB_LOAD_ADDR_HEX}); \
# 	PADDING_SIZE=$$(( $$DTB_LOAD_ADDR - $$BL2_LOAD_ADDR - $$BL2_SIZE )); \
# 	if [ $$PADDING_SIZE -lt 0 ]; then \
# 		echo "Error: BL2 overlaps DTB region!"; \
# 		echo "BL2_SIZE: $$BL2_SIZE, Padding would be: $$PADDING_SIZE"; \
# 		exit 1; \
# 	fi; \
# 	echo "  BL2 size       : $$BL2_SIZE bytes"; \
# 	echo "  BL2_LOAD_ADDR  : $$BL2_LOAD_ADDR "; \
# 	echo "  DTB size       : $$DTB_SIZE bytes"; \
# 	echo "  DTB_LOAD_ADDR  : $$DTB_LOAD_ADDR "; \
# 	echo "  Padding needed : $$PADDING_SIZE bytes"; \
# 	cat ${BL2_IMAGE} > bl2_padded.bin; \
# 	dd if=/dev/zero bs=1 count=$$PADDING_SIZE >> bl2_padded.bin; \
# 	cat bl2_padded.bin ${BL2_DTB} > ${BL2_OUTPUT}; \
# 	rm -f bl2_padded.bin; \
# 	MERGED_SIZE=$$(stat -c %s ${BL2_OUTPUT}); \
# 	echo "  Final merged image size: $$MERGED_SIZE bytes"; \
# 	echo "  Output written to: ${BL2_OUTPUT}"
# 	xxd ${BL2_OUTPUT} > bl2_with_dtb.hex
# 	mv -f ${BL2_OUTPUT} ${BL2_FINAL}
