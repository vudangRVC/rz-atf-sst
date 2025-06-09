#
# Copyright (c) 2023, Renesas Electronics Corporation. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
PLAT_SOC_RZV2H		:= 1
DDR_SOURCES 		+=	plat/renesas/rz/soc/${PLAT}/drivers/ddr/ddr_setup_lpddr4.c	\
						plat/renesas/rz/soc/${PLAT}/drivers/ddr/ddr_retcsr_lpddr4.c	\
						plat/renesas/rz/soc/${PLAT}/drivers/ddr/ddr_param_def_lpddr4.c

LPDDR4		:= 1
DDR_PLL4	:=1600

$(eval $(call add_define,DDR_PLL4))

#Same power control as used on evk_alpha board so use those files
BL_COMMON_SOURCES	+=	plat/renesas/rz/board/v2h_evk_alpha/pwrc_board.c

# Default Device tree
DTB_FILE_NAME		?=	rzv2h-evk-ver1
