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
#include <pwrc.h>
#include <ddr.h>

#define LO_REG							(0U)
#define HI_REG							(1U)

#define SYSTEM_PWR_STATE(s)				((s)->pwr_domain_state[PLAT_MAX_PWR_LVL])
#define CLUSTER_PWR_STATE(s)			((s)->pwr_domain_state[MPIDR_AFFLVL1])
#define CORE_PWR_STATE(s)				((s)->pwr_domain_state[MPIDR_AFFLVL0])

typedef struct {
	uintptr_t reg;
	uint32_t  preq_mask;
	uint32_t  paccept_mask;
	uint32_t  pstate_on_mask;
} CPG_CORE_PWR;

typedef struct {
	unsigned long value __aligned(CACHE_WRITEBACK_GRANULE);
} mailbox_t;

uintptr_t	gp_warm_ep;

extern bl31_board_cfg_t bl31_board_cfg[];
extern uint32_t soc_id;

#define CPG_REG_ADDR(offset)  		((uintptr_t)(bl31_board_cfg[soc_id].cpg_base + (offset)))
#define CPG_REG_WRITE(reg, value)	mmio_write_32(CPG_REG_ADDR(reg), value)
#define CPG_REG_READ(reg)			mmio_read_32(CPG_REG_ADDR(reg))

#define SYSC_REG_ADDR(offset)  		((uintptr_t)(bl31_board_cfg[soc_id].sysc_base + (offset)))
#define SYSC_REG_WRITE(reg, value)	mmio_write_32(SYSC_REG_ADDR(reg), value)
#define SYSC_REG_READ(reg)			mmio_read_32(SYSC_REG_ADDR(reg))

static void rz_program_trusted_mailbox(u_register_t mpidr, uintptr_t address)
{
	mailbox_t *mailbox = (mailbox_t *) PLAT_TRUSTED_MAILBOX_BASE;
	uint64_t linear_id = plat_core_pos_by_mpidr(mpidr);
	unsigned long range;

	mailbox[linear_id].value = address;
	range = (unsigned long)&mailbox[linear_id];

	flush_dcache_range(range, sizeof(range));
}

static void rzcmn_pwr_cpuoff(unsigned long mpidr)
{
	uint8_t coreid = MPIDR_AFFLVL1_VAL(mpidr);

	if (read_mpidr_el1() != mpidr) {
		ERROR("RZ: fail to power-off.\n");
		panic();
	}

	/* Request transition to Cortex-A55 CoreX Sleep Mode */
	CPG_REG_WRITE(CPG_V2H_LP_CTL1, (CPG_LP_CTL1_CA55SLEEP_REQ << coreid));

	/* Enter the Cortex-A55 Sleep Mode */
	CPG_REG_WRITE(CPG_V2H_LP_CTL1, CPG_REG_READ(CPG_V2H_LP_CTL1) | 0x00000001);

	/* Issue Barrier instruction */
	isb();
	dsb();
	dcsw_op_all(DCCISW);
}

static int rzcmn_pwr_domain_on(u_register_t mpidr)
{
	uint8_t coreid = MPIDR_AFFLVL1_VAL(mpidr);

	if (coreid > bl31_board_cfg[soc_id].platform_core_count)
		return PSCI_E_INTERN_FAIL;

	if (bl31_board_cfg[soc_id].platform_core_count == RZV2H_PLATFORM_CORE_COUNT) {
		const CPG_CORE_PWR pch[RZV2H_PLATFORM_CORE_COUNT] = {
			{ CPG_V2H_LP_CA55_CTL2, CPG_LP_CA55_CTL2_COREPREQ0, CPG_LP_CA55_CTL2_COREACCEPT0, CPG_LP_CA55_CTL2_CORESTATE0_ON_MASK },
			{ CPG_V2H_LP_CA55_CTL2, CPG_LP_CA55_CTL2_COREPREQ1, CPG_LP_CA55_CTL2_COREACCEPT1, CPG_LP_CA55_CTL2_CORESTATE1_ON_MASK },
			{ CPG_V2H_LP_CA55_CTL3, CPG_LP_CA55_CTL3_COREPREQ2, CPG_LP_CA55_CTL3_COREACCEPT2, CPG_LP_CA55_CTL3_CORESTATE2_ON_MASK },
			{ CPG_V2H_LP_CA55_CTL3, CPG_LP_CA55_CTL3_COREPREQ3, CPG_LP_CA55_CTL3_COREACCEPT3, CPG_LP_CA55_CTL3_CORESTATE3_ON_MASK }
		};

		/* Check if in standby */
		if ((CPG_REG_READ(CPG_V2H_LP_CTL1) & 0x1) == 0x1) {
			CPG_REG_WRITE(pch[coreid].reg, pch[coreid].preq_mask);
			while ((CPG_REG_READ(pch[coreid].reg) & pch[coreid].paccept_mask) != pch[coreid].paccept_mask)
				;
			CPG_REG_WRITE(pch[coreid].reg, 0x00000000);
			while ((CPG_REG_READ(pch[coreid].reg) & pch[coreid].paccept_mask) != 0x0)
				;
		}

		rz_program_trusted_mailbox(mpidr, gp_warm_ep);

		/* Assert PORESET */
		CPG_REG_WRITE(CPG_V2H_RST_0, (0x00010000 << coreid));
		while ((CPG_REG_READ(CPG_V2H_RSTMON_0) & (0x1 << coreid)) == 0x0)
			;

		/* Deassert PORESET and RERESET */
		CPG_REG_WRITE(CPG_V2H_RST_0, (0x00110011 << coreid));
		while ((CPG_REG_READ(CPG_V2H_RSTMON_0) & (0x1 << coreid)) != 0x0)
			;

		CPG_REG_WRITE(pch[coreid].reg, (pch[coreid].pstate_on_mask | pch[coreid].preq_mask));
		while ((CPG_REG_READ(pch[coreid].reg) & pch[coreid].paccept_mask) != pch[coreid].paccept_mask)
			;

		CPG_REG_WRITE(pch[coreid].reg, pch[coreid].pstate_on_mask);
		while ((CPG_REG_READ(pch[coreid].reg) & pch[coreid].paccept_mask) != 0x0)
			;
	} else {
		const uint32_t rval[2][2] = {
			{ SYS_CA55_CFG_RVAL0, SYS_CA55_CFG_RVAH0 },
			{ SYS_CA55_CFG_RVAL1, SYS_CA55_CFG_RVAH1 }
		};
		const uint32_t pch[2][2] = {
			{ CPG_CORE0_PCHCTL, CPG_CORE0_PCHMON },
			{ CPG_CORE1_PCHCTL, CPG_CORE1_PCHMON }
		};

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

	}

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

	if (coreid >= bl31_board_cfg[soc_id].platform_core_count)
		return;

	/* Prevent interrupts from spuriously waking up this cpu */
	plat_gic_cpuif_disable();

	if (bl31_board_cfg[soc_id].enable_pwrc_setup) {
		rzcmn_pwr_cpuoff(mpidr);
	} else {
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
}

static void rzcmn_pwr_domain_suspend(const psci_power_state_t *target_state)
{
	unsigned long mpidr = read_mpidr_el1();

	if (CORE_PWR_STATE(target_state) != PLAT_MAX_OFF_STATE)
		return;

	rz_program_trusted_mailbox(mpidr, gp_warm_ep);

	/* Prevent interrupts from spuriously waking up this cpu */
	plat_gic_cpuif_disable();
	plat_gic_save();
}

static void rzcmn_pwr_domain_suspend_finish(const psci_power_state_t *target_state)
{
	plat_gic_driver_init();
	plat_gic_resume();
	plat_gic_cpuif_enable();

	pwrc_setup();
	plat_copy_code_to_system_ram();
}

static void rzcmn_pwr_domain_pwr_down(const psci_power_state_t *target_state)
{
#if PLAT_SYSTEM_SUSPEND
	if (SYSTEM_PWR_STATE(target_state) == PLAT_MAX_OFF_STATE) {
		pwrc_suspend_to_ram();
	}
#endif /* PLAT_SYSTEM_SUSPEND */

	wfi();
	ERROR("RZ System Off: operation not handled.\n");
	panic();
}

#if PLAT_SYSTEM_SUSPEND
static void rzcmn_get_sys_suspend_power_state(psci_power_state_t *req_state)
{
	int i;

	for (i = MPIDR_AFFLVL0; i <= PLAT_MAX_PWR_LVL; i++)
		req_state->pwr_domain_state[i] = PLAT_MAX_OFF_STATE;
}
#endif /* PLAT_SYSTEM_SUSPEND */

static void __dead2 rzcmn_system_off(void)
{
	/* Set the CPG_LP_PWC_CTL1.ALL_OFF_TRG bit to allow desired power-off sequencing */
	CPG_REG_WRITE(CPG_V2H_LP_PWC_CTL1, CPG_LP_PWC_CTL1_ALL_OFF_TRG);

	wfi();
	ERROR("RZ/V2H System Off: operation not handled.\n");
	panic();
}
const plat_psci_ops_t rzcmn_plat_psci_ops = {
	.pwr_domain_on						= rzcmn_pwr_domain_on,
	.pwr_domain_on_finish				= rzcmn_pwr_domain_on_finish,
	.pwr_domain_off						= rzcmn_pwr_domain_off,
	.system_off							= rzcmn_system_off,
	.pwr_domain_suspend					= rzcmn_pwr_domain_suspend,
	.pwr_domain_suspend_finish			= rzcmn_pwr_domain_suspend_finish,
	.pwr_domain_pwr_down				= rzcmn_pwr_domain_pwr_down,
#if PLAT_SYSTEM_SUSPEND
	.get_sys_suspend_power_state		= rzcmn_get_sys_suspend_power_state,
#endif /* PLAT_SYSTEM_SUSPEND */
};

int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const plat_psci_ops_t **psci_ops)
{
	gp_warm_ep = sec_entrypoint;
	*psci_ops = &rzcmn_plat_psci_ops;

	return 0;
}

void arm_gicv3_distif_pre_save(unsigned int rdist_proc_num)
{}

void arm_gicv3_distif_post_restore(unsigned int rdist_proc_num)
{}
