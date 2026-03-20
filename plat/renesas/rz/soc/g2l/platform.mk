#
# Copyright (c) 2020-2021, Renesas Electronics Corporation. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include plat/renesas/rz/common/rz_common.mk
include plat/renesas/rz/board/${BOARD}/rz_board.mk

# RZ/G2L boots U-Boot at EL1, EL2 is implemented but unused
# Must initialize EL2 registers to prevent U-Boot boot failures
INIT_UNUSED_NS_EL2 := 1

PLAT_INCLUDES	+=	-Iplat/renesas/rz/soc/g2l/include

DDR_SOURCES += plat/renesas/rz/soc/g2l/drivers/ddr/ddr_g2l.c

