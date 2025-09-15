#
# Copyright (c) 2025, Renesas Electronics Corporation. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

DDR_SOURCES +=  plat/renesas/rz/soc/${PLAT}/drivers/ddr/ddr_cmn.c \
				plat/renesas/rz/soc/${PLAT}/drivers/ddr/ddr_setup_lpddr4.c

DDR_PLL4    := 1600
$(eval $(call add_define,DDR_PLL4))

BL_COMMON_SOURCES	+=	plat/renesas/rz/board/rz_cmn/pwrc_board.c
