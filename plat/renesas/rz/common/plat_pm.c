/*
 * Copyright (c) 2022, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <assert.h>
#include <lib/psci/psci.h>
#include <lib/mmio.h>
#include <common/debug.h>
#include <lib/bakery_lock.h>
#include <plat/common/platform.h>

#include <cpg_regs_offset.h>
#include <sys_regs_offset.h>
#include <rz_private.h>
#include <rzg2l_def.h>
#include <rzv2h_def.h>
#include <common/bl_common.h>

uintptr_t	gp_warm_ep;

extern bl31_board_cfg_t bl31_board_cfg[];
extern uint32_t soc_id;

#define CPG_REG_ADDR(offset)  		((uintptr_t)(bl31_board_cfg[soc_id].cpg_base + (offset)))
#define CPG_REG_WRITE(reg, value)	mmio_write_32(CPG_REG_ADDR(reg), value)
#define CPG_REG_READ(reg)			mmio_read_32(CPG_REG_ADDR(reg))

#define SYSC_REG_ADDR(offset)  		((uintptr_t)(bl31_board_cfg[soc_id].cpg_base + (offset)))
#define SYSC_REG_WRITE(reg, value)	mmio_write_32(SYSC_REG_ADDR(reg), value)
#define SYSC_REG_READ(reg)			mmio_read_32(SYSC_REG_ADDR(reg))

static int rzcmn_pwr_domain_on(u_register_t mpidr)
{
	const uint32_t rval[2][2] = {
		{ SYS_CA55_CFG_RVAL0, SYS_CA55_CFG_RVAH0 },
		{ SYS_CA55_CFG_RVAL1, SYS_CA55_CFG_RVAH1 }
	};
	const uint32_t pch[2][2] = {
		{ CPG_CORE0_PCHCTL, CPG_CORE0_PCHMON },
		{ CPG_CORE1_PCHCTL, CPG_CORE1_PCHMON }
	};
	uint8_t coreid = MPIDR_AFFLVL1_VAL(mpidr);

	if (coreid > 1)
		return PSCI_E_INTERN_FAIL;

	/*  Apply an external reset */
	if ((SYSC_REG_READ(SYS_LP_CTL2) & 0x1) == 0x1) {
		CPG_REG_WRITE(pch[coreid][0], 0x00000001);
		while ((CPG_REG_READ(pch[coreid][1]) & 0x1) != 0x1)
			;
		CPG_REG_WRITE(pch[coreid][0], 0x00000000);
		while ((CPG_REG_READ(pch[coreid][1]) & 0x1) != 0x0)
			;
	}

	/*  Start the core */
	SYSC_REG_WRITE(rval[coreid][0], (uint32_t)(gp_warm_ep & 0xFFFFFFFC));
	SYSC_REG_WRITE(rval[coreid][1], (uint32_t)((gp_warm_ep >> 32) & 0xFF));

	/* Assert PORESET */
	CPG_REG_WRITE(CPG_RST_CA55, (0x00010000 << coreid));
	while ((CPG_REG_READ(CPG_RSTMON_CA55) & (0x1 << coreid)) == 0x0)
		;

	/* Deassert PORESET */
	CPG_REG_WRITE(CPG_RST_CA55, (0x00050005 << coreid));
	while ((CPG_REG_READ(CPG_RSTMON_CA55) & (0x1 << coreid)) != 0x0)
		;

	CPG_REG_WRITE(pch[coreid][0], 0x00080001);
	while ((CPG_REG_READ(pch[coreid][1]) & 0x1) != 0x1)
		;
	CPG_REG_WRITE(pch[coreid][0], 0x00080000);
	while ((CPG_REG_READ(pch[coreid][1]) & 0x1) != 0x0)
		;

	return PSCI_E_SUCCESS;
}

static void rzcmn_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
#if !DEBUG_FPGA
	plat_gic_pcpu_init();
	plat_gic_cpuif_enable();
#endif /* DEBUG_FPGA */
}

static void rzcmn_pwr_domain_off(const psci_power_state_t *state)
{
	unsigned long mpidr = read_mpidr_el1();
	uint8_t coreid = MPIDR_AFFLVL1_VAL(mpidr);

	/* Prevent interrupts from spuriously waking up this cpu */
	plat_gic_cpuif_disable();

	/*  Enable the transition request interrupt to the Cortex-A55 Sleep Mode */
	SYSC_REG_WRITE(SYS_LP_CTL6, (0x00000100 << coreid));

	/* Transition request to Cortex-A55 CoreX Sleep Mode */
	SYSC_REG_WRITE(SYS_LP_CTL1, (0x00000100 << coreid));

	/* Confirm that the processing on the Cortex-M33 side is completed */
	while ((SYSC_REG_READ(SYS_LP_CTL5) & (0x00000100 << coreid)) != (0x00000100 << coreid))
		;
	/* Enter the Cortex-A55 Sleep Mode */
	SYSC_REG_WRITE(SYS_LP_CTL2, 0x00000001);

	/* Issue Barrier instruction */
	isb();
	dsb();

}

static void __dead2 rzcmn_system_off(void)
{
	wfi();
	ERROR("RZG System Off: operation not handled.\n");
	panic();
}

const plat_psci_ops_t rzcmn_plat_psci_ops = {
	.pwr_domain_on						= rzcmn_pwr_domain_on,
	.pwr_domain_on_finish				= rzcmn_pwr_domain_on_finish,
	.pwr_domain_off						= rzcmn_pwr_domain_off,
	.system_off							= rzcmn_system_off,
};

int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const plat_psci_ops_t **psci_ops)
{
	gp_warm_ep = sec_entrypoint;
	*psci_ops = &rzcmn_plat_psci_ops;

	return 0;
}
