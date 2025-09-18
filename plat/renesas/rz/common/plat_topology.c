/*
 * Copyright (c) 2020, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>

#include <common/debug.h>
#include <lib/psci/psci.h>
#include <rz_private.h>

extern bl31_board_cfg_t bl31_board_cfg[];
extern uint32_t soc_id;

static unsigned char rzcmn_power_domain_tree_desc[3];

const unsigned char *plat_get_power_domain_tree_desc(void)
{
	rzcmn_power_domain_tree_desc[0] = PLATFORM_SYSTEM_COUNT;
	rzcmn_power_domain_tree_desc[1] = PLATFORM_CLUSTER_COUNT;
	rzcmn_power_domain_tree_desc[2] = bl31_board_cfg[soc_id].platform_core_count;
	return rzcmn_power_domain_tree_desc;
}

int plat_core_pos_by_mpidr(u_register_t mpidr)
{
	unsigned int cluster_id, cpu_id;

	cluster_id = MPIDR_AFFLVL2_VAL(mpidr);
	cpu_id = MPIDR_AFFLVL1_VAL(mpidr);

	if ((cluster_id >= PLATFORM_CLUSTER_COUNT) || (cpu_id >= bl31_board_cfg[soc_id].platform_core_count))
		return -1;

	return cpu_id;
}
