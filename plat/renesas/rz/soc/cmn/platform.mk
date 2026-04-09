#
# Copyright (c) 2020-2021, Renesas Electronics Corporation. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include plat/renesas/rz/common/rz_common.mk
include plat/renesas/rz/board/${BOARD}/rz_board.mk
include lib/libfdt/libfdt.mk
include lib/fconf/fconf.mk

PLAT_INCLUDES	+=	-Iplat/renesas/rz/soc/cmn/include

PLAT_BL_COMMON_SOURCES	+=	plat/renesas/rz/soc/cmn/drivers/pwrc/pwrc.c	\
							plat/renesas/rz/soc/cmn/drivers/pwrc/pwrc_stack.S	\

BL2_SOURCES				+=	${FCONF_SOURCES}			\
							${FCONF_DYN_SOURCES}			\
							plat/renesas/rz/common/rz_dt.c		\
							plat/renesas/rz/common/rz_fconf.c

DDR_SOURCES				+=	plat/renesas/rz/soc/${PLAT}/drivers/ddr/lpddr.c				\
							plat/renesas/rz/soc/${PLAT}/drivers/ddr/ddr_misc.c			\
							plat/renesas/rz/soc/${PLAT}/drivers/ddr/plat_ddr_setup.c

DTB_LIST := rzg2l-sbc.dtb r9a07g044l2-smarc.dtb r9a07g054l2-smarc.dtb rzv2h-evk-ver1.dtb imdt-v2h-sbc.dtb rzv2h-rdk-ver1.dtb rs-g2l100.dtb
DTB_OUT := $(BUILD_PLAT)/fdts

dtbs: $(DTB_LIST:%=$(DTB_OUT)/%)

$(DTB_OUT)/%.dtb: fdts/%.dts
	@echo "  DTB     $@"
	@mkdir -p $(DTB_OUT)
	$($(ARCH)-dtc) -I dts -O dtb -o $@ $<

# For the cmn (multi-SoC) platform, per-board DTB embedding is handled by
# firmware_compile.py at packaging time, so bl2_with_dtb is a no-op here.
# The raw bl2-esd.bin produced is intentionally WITHOUT a DTB.
.PHONY: bl2_with_dtb
bl2_with_dtb:
	@echo "[cmn] DTB embedding is done per-board by firmware_compile.py; skipping."
