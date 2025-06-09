/*
 * Copyright (c) 2023, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <lib/mmio.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <plat/common/platform.h>
#include <rzv2h_cpg.h>
#include <rzv2h_ddr.h>
#include <pwrc.h>
#include <pwrc_board.h>
#include <sys_regs.h>
#include <sys_regs_offset.h>
#include <rz_dt.h>

extern void pwrc_func_call_with_pmustack(uintptr_t jump, void *arg);

void __attribute__ ((section(".sram")))
pwrc_go_suspend_to_ram(void)
{
	cpg_prepare_suspend();
	ddr_retention_entry();
	pwrc_board_suspend_on();

	while (1)
		wfi();
}

void __dead2 pwrc_suspend_to_ram(void)
{
	/* flash all caches */
	dcsw_op_all(DCCISW);

	/* disable MMU */
	disable_mmu_el3();

	/* switch to stack */
	pwrc_func_call_with_pmustack((uintptr_t)pwrc_go_suspend_to_ram, NULL);

	panic();
}

void pwrc_setup(void)
{
	const uint32_t rval[PLATFORM_CORE_COUNT][2] = {
		{ SYS_ACPU_CFG_RVAL0, SYS_ACPU_CFG_RVAH0 },
		{ SYS_ACPU_CFG_RVAL1, SYS_ACPU_CFG_RVAH1 },
		{ SYS_ACPU_CFG_RVAL2, SYS_ACPU_CFG_RVAH2 },
		{ SYS_ACPU_CFG_RVAL3, SYS_ACPU_CFG_RVAH3 }
	};

	unsigned int i;
	uint32_t rvah0 = (uint32_t)(((uintptr_t)&plat_secondary_reset >> 32) & 0xFF);
	uint32_t rval0 = (uint32_t)((uintptr_t)&plat_secondary_reset & 0xFFFFFFFC);

	for (i = 0; i < PLATFORM_CORE_COUNT; i++) {
		mmio_write_32(rval[i][1], rvah0);
		mmio_write_32(rval[i][0], rval0);
	}
}

void rzv2h_pwrc_setup(void *fdt)
{
	const char *node = "/soc";
	const char *sub_node = "system-controller@10430000";
	const char *prop_name = "reg";

	// Get clock-controller base address
	uint32_t sysc_base = 0;
	if(read_prop_from_subnode(fdt, node, sub_node, prop_name, 1, &sysc_base) != 0) {
		ERROR("BL2: Failed to get SYSC base address\n");
		return;
	}

	// Set reset regiters values
	uint32_t rval[PLATFORM_CORE_COUNT][2] = {
		{ SYS_ACPU_CFG_RVAL0_OFFSET, SYS_ACPU_CFG_RVAH0_OFFSET },
		{ SYS_ACPU_CFG_RVAL1_OFFSET, SYS_ACPU_CFG_RVAH1_OFFSET },
		{ SYS_ACPU_CFG_RVAL2_OFFSET, SYS_ACPU_CFG_RVAH2_OFFSET },
		{ SYS_ACPU_CFG_RVAL3_OFFSET, SYS_ACPU_CFG_RVAH3_OFFSET }
	};

	unsigned int i;
	for(i = 0; i < PLATFORM_CORE_COUNT; i++) {
		rval[i][0] += sysc_base;
		rval[i][1] += sysc_base;
	}

	uint32_t rvah0 = (uint32_t)(((uintptr_t)&plat_secondary_reset >> 32) & 0xFF);
	uint32_t rval0 = (uint32_t)((uintptr_t)&plat_secondary_reset & 0xFFFFFFFC);

	for (i = 0; i < PLATFORM_CORE_COUNT; i++) {
		mmio_write_32(rval[i][1], rvah0);
		mmio_write_32(rval[i][0], rval0);
	}
}
