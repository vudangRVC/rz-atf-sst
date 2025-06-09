/*
 * Copyright (c) 2023, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


#include <lib/mmio.h>
#include <sys.h>
#include <sys_regs.h>
#include <common/debug.h>
#include <pwrc_board.h>
#include <rzv2h_syc.h>
#include <rz_dt.h>

uint32_t get_syc_inck_hz(void *fdt)
{
	// Get syc_inck_hz value from FDT
	uint32_t syc_inck_hz = 0;
	if(read_prop_from_subnode(fdt, "/soc", "system-controller@10430000", "syc_inck_hz", 0, &syc_inck_hz) != 0) {
		return 0;
	}
	return syc_inck_hz;
}