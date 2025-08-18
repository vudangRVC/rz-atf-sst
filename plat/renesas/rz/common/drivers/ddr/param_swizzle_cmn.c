/*
 * Copyright (c) 2020-2022, Renesas Electronics Corporation. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 * This code was generated with RZ/G2L, G2UL, Five, A3UL DDR config generation tool v3.0.1
 */

#include <stdint.h>
#include <ddr_internal.h>
#include <ddr_mc_if.h>

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

const uint32_t mc_phy_settings_tbl_g2l[MC_PHYSET_NUM][2] = {
	{G2L_DDRMC_R040,	0x57630BB8	},
	{G2L_DDRMC_R041,	0x00002828	},
	{G2L_DDRMC_R042,	0x00003C22	},
	{G2L_DDRMC_R043,	0x00102611	}
};

const uint32_t swizzle_mc_tbl_g2l[SWIZZLE_MC_NUM][2] = {
	{G2L_DDRMC_R030},
	{G2L_DDRMC_R031},
	{G2L_DDRMC_R032},
	{G2L_DDRMC_R033},
	{G2L_DDRMC_R034},
	{G2L_DDRMC_R035},
	{G2L_DDRMC_R036},
	{G2L_DDRMC_R037},
	{G2L_DDRMC_R038}
};

const uint32_t mc_phy_settings_tbl_v2l[MC_PHYSET_NUM][2] = {
	{V2L_DDRMC_R040,	0x57630BB8	},
	{V2L_DDRMC_R041,	0x00002828	},
	{V2L_DDRMC_R042,	0x00003C22	},
	{V2L_DDRMC_R043,	0x00102611	}
};

const uint32_t swizzle_mc_tbl_v2l[SWIZZLE_MC_NUM][2] = {
	{V2L_DDRMC_R030},
	{V2L_DDRMC_R031},
	{V2L_DDRMC_R032},
	{V2L_DDRMC_R033},
	{V2L_DDRMC_R034},
	{V2L_DDRMC_R035},
	{V2L_DDRMC_R036},
	{V2L_DDRMC_R037},
	{V2L_DDRMC_R038}
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
