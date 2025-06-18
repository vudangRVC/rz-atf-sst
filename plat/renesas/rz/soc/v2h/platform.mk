#
# Copyright (c) 2023, Renesas Electronics Corporation. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#Set the platform and SOC specific header files 1st
PLAT_INCLUDES	:=	-Iplat/renesas/rz/soc/v2h/include
FIP_ALIGN		:=	16

include plat/renesas/rz/common/v2h_common.mk
include plat/renesas/rz/board/${BOARD}/rz_board.mk

DDR_SOURCES	+=				plat/renesas/rz/soc/v2h/drivers/ddr/rzv2h_ddr.c	\
							plat/renesas/rz/soc/v2h/drivers/ddr/ddr_misc.c	\
							plat/renesas/rz/soc/v2h/plat_ddr_setup.c

PLAT_BL_COMMON_SOURCES	+=	plat/renesas/rz/soc/v2h/plat_security.c		\
							plat/renesas/rz/soc/v2h/drivers/riic.c		\
							plat/renesas/rz/soc/v2h/drivers/rzv2h_cpg.c		\
							plat/renesas/rz/soc/v2h/drivers/pwrc/pwrc.c	\
							plat/renesas/rz/soc/v2h/drivers/pwrc/pwrc_stack.S	\
							plat/renesas/rz/common/rz_dt.c	\
							plat/renesas/rz/common/board_info.c	\
							${DDR_SOURCES}

BL2_SOURCES				+=	plat/renesas/rz/soc/v2h/bl2_plat_setup.c		\
							plat/renesas/rz/soc/v2h/plat_storage.c			\
							plat/renesas/rz/soc/v2h/drivers/sys.c			\
							plat/renesas/rz/soc/v2h/drivers/rzv2h_syc.c			\
							plat/renesas/rz/soc/v2h/drivers/rzv2h_pfc.c

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

# Define addr in SRAM for BL2 and DTB
BL2_LOAD_ADDR_HEX := $(shell grep 'BL2_BASE' plat/renesas/rz/soc/v2h/include/platform_def.h | sed -E 's/.*\((0x[0-9A-Fa-f]+)\).*/\1/')
DTB_LOAD_ADDR_HEX := $(shell grep 'V2H_DTB_LOAD_ADDR' plat/renesas/rz/soc/v2h/include/platform_def.h | sed -E 's/.*\((0x[0-9A-Fa-f]+)\).*/\1/')

# Define file name
BL2_IMAGE  := ${BUILD_PLAT}/bl2.bin
BL2_DTB    := ${BUILD_PLAT}/fdts/${DTB_FILE_NAME}.dtb
BL2_OUTPUT := ${BUILD_PLAT}/bl2_with_dtb.bin
BL2_FINAL  := ${BUILD_PLAT}/bl2.bin

# Rule for creating the merged BL2 with DTB file
bl2_with_dtb: ${BL2_IMAGE} ${BL2_DTB}
	@echo "Merging BL2 and DTB with alignment and padding..."
	@BL2_SIZE=$$(stat -c %s ${BL2_IMAGE}); \
	DTB_SIZE=$$(stat -c %s ${BL2_DTB}); \
	BL2_LOAD_ADDR=$$(printf "%d" ${BL2_LOAD_ADDR_HEX}); \
	DTB_LOAD_ADDR=$$(printf "%d" ${DTB_LOAD_ADDR_HEX}); \
	PADDING_SIZE=$$(( $$DTB_LOAD_ADDR - $$BL2_LOAD_ADDR - $$BL2_SIZE )); \
	if [ $$PADDING_SIZE -lt 0 ]; then \
		echo "Error: BL2 overlaps DTB region!"; \
		echo "BL2_SIZE: $$BL2_SIZE, Padding would be: $$PADDING_SIZE"; \
		exit 1; \
	fi; \
	echo "  BL2 size       : $$BL2_SIZE bytes"; \
	echo "  BL2_LOAD_ADDR  : $$BL2_LOAD_ADDR "; \
	echo "  DTB size       : $$DTB_SIZE bytes"; \
	echo "  DTB_LOAD_ADDR  : $$DTB_LOAD_ADDR "; \
	echo "  Padding needed : $$PADDING_SIZE bytes"; \
	cat ${BL2_IMAGE} > bl2_padded.bin; \
	dd if=/dev/zero bs=1 count=$$PADDING_SIZE >> bl2_padded.bin; \
	cat bl2_padded.bin ${BL2_DTB} > ${BL2_OUTPUT}; \
	rm -f bl2_padded.bin; \
	MERGED_SIZE=$$(stat -c %s ${BL2_OUTPUT}); \
	echo "  Final merged image size: $$MERGED_SIZE bytes"; \
	echo "  Output written to: ${BL2_OUTPUT}"
	xxd ${BL2_OUTPUT} > bl2_with_dtb.hex
	mv -f ${BL2_OUTPUT} ${BL2_FINAL}
