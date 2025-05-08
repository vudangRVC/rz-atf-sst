#
# Copyright (c) 2023, Renesas Electronics Corporation. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#Set the platform and SOC specific header files 1st
PLAT_INCLUDES	:=	-Iplat/renesas/rz/soc/v2h/include
FIP_ALIGN		:=	16

include plat/renesas/rz/common/v2h_common.mk
include plat/renesas/rz/board/${PLAT}_${BOARD}/rz_board.mk

DDR_SOURCES	+=				plat/renesas/rz/soc/v2h/drivers/ddr/ddr.c	\
							plat/renesas/rz/soc/v2h/drivers/ddr/ddr_misc.c	\
							plat/renesas/rz/soc/v2h/plat_ddr_setup.c

PLAT_BL_COMMON_SOURCES	+=	plat/renesas/rz/soc/v2h/plat_security.c		\
							plat/renesas/rz/soc/v2h/drivers/riic.c		\
							plat/renesas/rz/soc/v2h/drivers/cpg.c		\
							plat/renesas/rz/soc/v2h/drivers/pwrc/pwrc.c	\
							plat/renesas/rz/soc/v2h/drivers/pwrc/pwrc_stack.S	\
							${DDR_SOURCES}

BL2_SOURCES				+=	plat/renesas/rz/soc/v2h/bl2_plat_setup.c		\
							plat/renesas/rz/soc/v2h/plat_storage.c			\
							plat/renesas/rz/soc/v2h/drivers/sys.c			\
							plat/renesas/rz/soc/v2h/drivers/pfc.c

BL31_SOURCES			+=	plat/renesas/rz/soc/v2h/bl31_plat_setup.c		\
							plat/renesas/rz/soc/v2h/plat_pm.c				\
							plat/renesas/rz/soc/v2h/rz_plat_sip_handler.c

.PHONY: bptool_make bptool_clean

bptool: bptool_make
distclean realclean clean: bptool_clean

BPTOOLPATH		?=	tools/renesas/rz_boot_param

bptool_make:
	${Q}${MAKE} --no-print-directory -C ${BPTOOLPATH}

bptool_clean:
	${Q}${MAKE} --no-print-directory -C ${BPTOOLPATH} clean

pkg:
	./tools/renesas/bptool build/v2h/${BUILD_TYPE}/bl2.bin build/v2h/${BUILD_TYPE}/bp_spi.bin 0x08103000 spi
	cat build/v2h/${BUILD_TYPE}/bp_spi.bin build/v2h/${BUILD_TYPE}/bl2.bin > build/v2h/${BUILD_TYPE}/bl2_bp_spi.bin
	objcopy -I binary -O srec --adjust-vma=0x8101E00 --srec-forceS3 build/v2h/${BUILD_TYPE}/bl2_bp_spi.bin  build/v2h/${BUILD_TYPE}/bl2_bp_spi.srec
	if [ ${BOARD} != "evk_1" ]; then \
	./tools/renesas/bptool build/v2h/${BUILD_TYPE}/bl2.bin build/v2h/${BUILD_TYPE}/bp_mmc.bin 0x08103000 mmc;\
	cat build/v2h/${BUILD_TYPE}/bp_mmc.bin build/v2h/${BUILD_TYPE}/bl2.bin > build/v2h/${BUILD_TYPE}/bl2_bp_mmc.bin;\
	objcopy -I binary -O srec --adjust-vma=0x8101E00 --srec-forceS3 build/v2h/${BUILD_TYPE}/bl2_bp_mmc.bin  build/v2h/${BUILD_TYPE}/bl2_bp_mmc.srec;\
	fi
	./tools/renesas/bptool build/v2h/${BUILD_TYPE}/bl2.bin build/v2h/${BUILD_TYPE}/bp_esd.bin 0x08103000 esd
	cat build/v2h/${BUILD_TYPE}/bp_esd.bin build/v2h/${BUILD_TYPE}/bl2.bin > build/v2h/${BUILD_TYPE}/bl2_bp_esd.bin
	objcopy -I binary -O srec --adjust-vma=0x8101E00 --srec-forceS3 build/v2h/${BUILD_TYPE}/bl2_bp_esd.bin  build/v2h/${BUILD_TYPE}/bl2_bp_esd.srec
	#Generate FIP S-Record if FIP binary is present
	if [ -f build/v2h/${BUILD_TYPE}/fip.bin ]; then  objcopy -I binary -O srec --adjust-vma=0x8101E00 --srec-forceS3 build/v2h/${BUILD_TYPE}/fip.bin build/v2h/${BUILD_TYPE}/fip.srec ; fi ;
	tar zcvf ../tf-a.tar.gz $(shell pwd)

ifneq (${DEBUG}, 0)
TF_CFLAGS += -O0 -fstack-usage
ASFLAGS += -O0 -fstack-usage
endif

include lib/libfdt/libfdt.mk

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

ifeq (${TRUSTED_BOARD_BOOT}, 0)
BL2_BASE := 0x12000
else
BL2_BASE := 0x13000
endif

SRAM_LIMIT := $(shell printf "%d" 0x1D000)
BL2_BIN_LIMIT_DEC := $(shell printf "%d" $(BL2_BIN_LIMIT))

# Rule for creating the merged BL2 with DTB file
bl2_with_dtb: ${BL2_IMAGE} ${BL2_DTB} 
	@echo "Embedding DTB into BL2 with dynamic padding..."
	@BL2_SIZE=$$(wc -c < ${BL2_IMAGE} | awk '{print $$1}'); \
	PADDING=$$(($(BL2_BIN_LIMIT_DEC) - $$BL2_SIZE)); 
