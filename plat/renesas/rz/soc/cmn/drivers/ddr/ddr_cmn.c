/*
 * Copyright (c) 2021, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <ddr_internal.h>
#include <sys_regs.h>
#include <board_info.h>

void ddr_ctrl_reten_en_n(uint8_t val, uint8_t board_id)
{
	val &= 1;

	switch (board_id)
	{
		case BOARD_ID_RZG2L_EVK:
		case BOARD_ID_RZG2L_SBC:
			if ((mmio_read_32(SYS_LSI_DEVID) >> 28) + 1 > 1)
			{
				write_phy_reg(DDRPHY_R79, (val << 1));
			}
			else
			{
				rmw_phy_reg(DDRPHY_R78, 0xFFFEFFFF, (val << 16));
			}
			break;

		case BOARD_ID_RZV2L_EVK:
			val &= 1;
			write_phy_reg(DDRPHY_R79, (val << 1));
			break;

		case BOARD_ID_RZV2H_EVK:
			// TODO: Implement for RZV2H
			break;
		default:
			break;
	}

}
