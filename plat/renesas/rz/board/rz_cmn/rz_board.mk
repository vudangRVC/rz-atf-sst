#
# Copyright (c) 2023, Renesas Electronics Corporation. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

DDR_SOURCES +=  plat/renesas/rz/soc/${PLAT}/drivers/ddr/ddr_cmn.c \
				plat/renesas/rz/soc/${PLAT}/drivers/ddr/param_mc_cmn.c \
				plat/renesas/rz/common/drivers/ddr/param_swizzle_cmn.c

DDR_PLL4    := 1600
$(eval $(call add_define,DDR_PLL4))

# TODO: Testing purpose only, removed it later
DTB_FILE_NAME		?=	r9a07g054l2-smarc
