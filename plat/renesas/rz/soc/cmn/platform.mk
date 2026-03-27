#
# Copyright (c) 2020-2021, Renesas Electronics Corporation. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include plat/renesas/rz/common/rz_common.mk
include plat/renesas/rz/board/${BOARD}/rz_board.mk
include lib/libfdt/libfdt.mk

PLAT_INCLUDES	+=	-Iplat/renesas/rz/soc/cmn/include

PLAT_BL_COMMON_SOURCES	+=	plat/renesas/rz/soc/cmn/drivers/pwrc/pwrc.c	\
							plat/renesas/rz/soc/cmn/drivers/pwrc/pwrc_stack.S	\

BL2_SOURCES				+=	lib/fconf/fconf_dyn_cfg_getter.c 	\
							plat/renesas/rz/common/rz_dt.c		\
							plat/renesas/rz/common/rz_fconf.c

DDR_SOURCES				+=	plat/renesas/rz/soc/${PLAT}/drivers/ddr/lpddr.c				\
							plat/renesas/rz/soc/${PLAT}/drivers/ddr/ddr_misc.c			\
							plat/renesas/rz/soc/${PLAT}/drivers/ddr/plat_ddr_setup.c

DTB_LIST := rzg2l-sbc.dtb r9a07g044l2-smarc.dtb r9a07g054l2-smarc.dtb rzv2h-evk-ver1.dtb rzv2h-sbc-ver1.dtb
DTB_OUT := $(BUILD_PLAT)/fdts

dtbs: $(DTB_LIST:%=$(DTB_OUT)/%)

$(DTB_OUT)/%.dtb: fdts/%.dts
	@echo "  DTB     $@"
	@mkdir -p $(DTB_OUT)
	$(DTC) -I dts -O dtb -o $@ $<
