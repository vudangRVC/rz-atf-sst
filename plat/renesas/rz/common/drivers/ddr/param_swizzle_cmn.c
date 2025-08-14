/*
 * Copyright (c) 2020-2022, Renesas Electronics Corporation. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 * This code was generated with RZ/G2L, G2UL, Five, A3UL DDR config generation tool v3.0.1
 */

#include <stdint.h>
#include <ddr_internal.h>

const uint32_t mc_odt_pins_tbl[4] = {
	0x00000001	,
	0x00000000	,
	0x00000000	,
	0x00000000
};

const uint32_t mc_mr1_tbl[2] = {
	0x00000706	,
	0x00000100
};

const uint32_t mc_mr2_tbl[2] = {
	0x00000E00	,
	0x00000000
};

const uint32_t mc_mr5_tbl[2] = {
	0x000001C0	,
	0x000001C0
};

const uint32_t mc_mr6_tbl[2] = {
	0x0000007F	,
	0x0000000F
};

const uint32_t mc_phy_settings_tbl[MC_PHYSET_NUM][2] = {
	{DDRMC_R040,	0x57630BB8	},
	{DDRMC_R041,	0x00002828	},
	{DDRMC_R042,	0x00003C22	},
	{DDRMC_R043,	0x00102611	}
};

const uint32_t swizzle_mc_tbl[SWIZZLE_MC_NUM][2] = {
	{DDRMC_R030},
	{DDRMC_R031},
	{DDRMC_R032},
	{DDRMC_R033},
	{DDRMC_R034},
	{DDRMC_R035},
	{DDRMC_R036},
	{DDRMC_R037},
	{DDRMC_R038}
};

const uint32_t swizzle_phy_tbl[SIZZLE_PHY_NUM][2] = {
	{DDRPHY_R29},
	{DDRPHY_R11},
	{DDRPHY_R29},
	{DDRPHY_R11},
	{DDRPHY_R29},
	{DDRPHY_R11},
	{DDRPHY_R29},
	{DDRPHY_R11},
	{DDRPHY_R29},
	{DDRPHY_R11},
	{DDRPHY_R29},
	{DDRPHY_R11},
	{DDRPHY_R29},
	{DDRPHY_R11},
	{DDRPHY_R29},
	{DDRPHY_R11}
};
