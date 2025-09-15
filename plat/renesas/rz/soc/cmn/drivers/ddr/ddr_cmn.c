/*
 * Copyright (c) 2025, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <ddr_internal.h>
#include <sys_regs_offset.h>
#include <board_info.h>
#include <lib/fconf/fconf.h>
#include <rz_fconf.h>

void ddr_ctrl_reten_en_n(uint8_t val, uint8_t soc_id)
{
	uint32_t sysc_base = FCONF_GET_PROPERTY(hw_config, sysc_config, sysc_base);
	val &= 1;

	switch (soc_id)
	{
		case RZ_SOC_RZG2L:
			if ((mmio_read_32(sysc_base + SYS_LSI_DEVID) >> 28) + 1 > 1)
			{
				write_phy_reg(DDRPHY_R79, (val << 1));
			}
			else
			{
				rmw_phy_reg(DDRPHY_R78, 0xFFFEFFFF, (val << 16));
			}
			break;

		case RZ_SOC_RZV2L:
			val &= 1;
			write_phy_reg(DDRPHY_R79, (val << 1));
			break;

		default:
			break;
	}

}
