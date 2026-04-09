/*
 * Copyright (c) 2022, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <drivers/arm/tzc400.h>
#include <string.h>
#include <lib/mmio.h>
#include <common/debug.h>

#include <rzg2l_def.h>
#include <rzv2h_def.h>
#include <rz_private.h>
#include "sys_regs_offset.h"
#include "plat_tzc_def.h"
#if IMAGE_BL2
#include <rz_fconf.h>
#include <lib/fconf/fconf.h>
#endif
#include <board_info.h>

#if IMAGE_BL31
extern bl31_board_cfg_t bl31_board_cfg[];
extern uint32_t soc_id;
#endif

#if IMAGE_BL2
const struct sysc_config_t * g_sysc_fconf_cfg;
#endif

typedef struct arm_tzc_regions_info {
	unsigned long long base;
	unsigned long long end;
	unsigned int sec_attr;
	unsigned int nsaid_permissions;
} arm_tzc_regions_info_t;

#if IMAGE_BL2
typedef struct {
	uint32_t reg;
	uint32_t msk;
	uint32_t val;
} PLAT_REG_SETTING;

void plat_access_control_setup(void)
{
	uint32_t i, j = 0;
	
	const struct sysc_config_t * sysc_fconf_cfg = sysc_config_getter();
	PLAT_REG_SETTING sys_acctl[77];

	for (i = 0; i < ARRAY_SIZE(sysc_fconf_cfg->sys_acctl); i += 3, j++) {
		sys_acctl[j].reg = sysc_fconf_cfg->sys_acctl[i];
		sys_acctl[j].msk = sysc_fconf_cfg->sys_acctl[i + 1];
		sys_acctl[j].val = sysc_fconf_cfg->sys_acctl[i + 2];
	}

	for (i = 0; i < ARRAY_SIZE(sys_acctl); i++) {
		if (sys_acctl[i].reg == 0U) {
			break;
		}
		uint32_t val = mmio_read_32((uintptr_t)sysc_fconf_cfg->sysc_base + sys_acctl[i].reg) & (~sys_acctl[i].msk);

		val |= (sys_acctl[i].val & sys_acctl[i].msk);

		mmio_write_32(((uintptr_t)sysc_fconf_cfg->sysc_base + sys_acctl[i].reg), val);
	}
}
#endif

uint8_t tzc400_get_num_filters(uintptr_t tzc_base)
{
	uint32_t tzc400_build;

	tzc400_build = mmio_read_32(tzc_base + BUILD_CONFIG_OFF);

	return (uint8_t)((tzc400_build >> BUILD_CONFIG_NF_SHIFT) & BUILD_CONFIG_NF_MASK) + 1U;
}

static void plat_tzc400_setup(uintptr_t tzc_base, const arm_tzc_regions_info_t *tzc_regions)
{
	uint8_t num_filters;
	unsigned int region_index = 1U;
	const arm_tzc_regions_info_t *p;
	const arm_tzc_regions_info_t init_tzc_regions[] = {
		{0}
	};

	INFO("Configuring TrustZone Controller\n");

	tzc400_init(tzc_base);

	tzc400_disable_filters();

	tzc400_configure_region0(TZC_REGION_S_RDWR, PLAT_TZC_REGION_ACCESS_NS_UNPRIV);

	if (tzc_regions == NULL)
		p = init_tzc_regions;
	else
		p = tzc_regions;

	num_filters = tzc400_get_num_filters(tzc_base);

	for (; p->base != 0UL; p++) {
		tzc400_configure_region((1 << num_filters) - 1, region_index,
			p->base, p->end, p->sec_attr, p->nsaid_permissions);
		region_index++;
	}

	INFO("Total %u regions set.\n", region_index);

	tzc400_set_action(TZC_ACTION_ERR);

	tzc400_enable_filters();
}

#if IMAGE_BL2
static void plat_tzc_msram_setup(void)
{
	const arm_tzc_regions_info_t msram_tzc_regions[] = {
		{
			/* Default Region 0: Lock down */
			.base = 0,	/* Not Used by Region 0*/
			.end  = 0,	/* Not Used by Region 0*/
			.sec_attr = TZC_REGION_S_RDWR,
			.nsaid_permissions = PLAT_TZC_REGION_ACCESS_S_PRIV
		},
		{}
	};

	plat_tzc400_setup(g_sysc_fconf_cfg->tzc_m33_base, &msram_tzc_regions[0]);
}

static void plat_tzc_ddr_setup(void)
{
	const arm_tzc_regions_info_t ddr0_1_tzc_regions[] = {
#if TRUSTED_BOARD_BOOT
		{
			/* Default Region 0: Lock down */
			.base = 0,	/* Not Used by Region 0*/
			.end  = 0,	/* Not Used by Region 0*/
			.sec_attr = TZC_REGION_S_RDWR,
			.nsaid_permissions = PLAT_TZC_REGION_ACCESS_S_PRIV
		},

		{
			/* Region 1: */
			.base = PLAT_FW_TZC_PROT_DRAM01_BASE,
			.end  = PLAT_FW_TZC_PROT_DRAM01_END,
			.sec_attr = TZC_REGION_S_RDWR,
			.nsaid_permissions = PLAT_TZC_REGION_ACCESS_S_UNPRIV
		},

		{
			/* Region 2: */
			.base = PLAT_TEE_TZC_PROT_DRAM01_BASE,
			.end  = PLAT_TEE_TZC_PROT_DRAM01_END,
			.sec_attr = TZC_REGION_S_RDWR,
			.nsaid_permissions = PLAT_TZC_REGION_ACCESS_S_UNPRIV
		},

		{
			/* Region 3: */
			.base = PLAT_TEE_TZC_PROT_DRAM01_END + 1,
			.end  = UL(0xFFFFFFFFF),
			.sec_attr = TZC_REGION_S_NONE,
			.nsaid_permissions = PLAT_TZC_REGION_ACCESS_NS_UNPRIV
		},
#endif /* TRUSTED_BOARD_BOOT */
		{}
	};

	const arm_tzc_regions_info_t ddr_default_tzc_regions[] = {
		{
			/* Default Region 0: Complete access */
			.base = 0,	/* Not Used by Region 0 */
			.end  = 0,	/* Not Used by Region 0 */
			.sec_attr = TZC_REGION_S_NONE,
			.nsaid_permissions = PLAT_TZC_REGION_ACCESS_NS_UNPRIV
		},

		{}
	};

	if (g_sysc_fconf_cfg->tzc_ddr00_base != 0)
	{
		plat_tzc400_setup(g_sysc_fconf_cfg->tzc_ddr00_base, &ddr_default_tzc_regions[1]);
	}
	if (g_sysc_fconf_cfg->tzc_ddr01_base != 0)
	{
		plat_tzc400_setup(g_sysc_fconf_cfg->tzc_ddr01_base, &ddr0_1_tzc_regions[0]);
	}
	if (g_sysc_fconf_cfg->tzc_ddr10_base != 0)
	{
		plat_tzc400_setup(g_sysc_fconf_cfg->tzc_ddr10_base, &ddr_default_tzc_regions[0]);
	}
	if (g_sysc_fconf_cfg->tzc_ddr11_base != 0)
	{
		plat_tzc400_setup(g_sysc_fconf_cfg->tzc_ddr11_base, &ddr_default_tzc_regions[0]);
	}
}

static void plat_tzc_spi_setup(void)
{
	plat_tzc400_setup(g_sysc_fconf_cfg->tzc_spi_base, NULL);
}

static void plat_tzc_pci_setup(void)
{
	const arm_tzc_regions_info_t pci_tzc_regions[] = {
		{
			/* Default Region 0: Complete access */
			.base = 0,	/* Not Used by Region 0 */
			.end  = 0,	/* Not Used by Region 0 */
			.sec_attr = TZC_REGION_S_NONE,
			.nsaid_permissions = PLAT_TZC_REGION_ACCESS_NS_UNPRIV
		},

		{}
	};

	plat_tzc400_setup(g_sysc_fconf_cfg->tzc_pci_base, &pci_tzc_regions[0]);
}

static void plat_tzc_r8_setup(void)
{
	const arm_tzc_regions_info_t r8_tzc_regions[] = {
		{
			/* Default Region 0: Complete access */
			.base = 0,	/* Not Used by Region 0 */
			.end  = 0,	/* Not Used by Region 0 */
			.sec_attr = TZC_REGION_S_NONE,
			.nsaid_permissions = PLAT_TZC_REGION_ACCESS_NS_UNPRIV
		},

		{}
	};

	plat_tzc400_setup(g_sysc_fconf_cfg->tzc_r8_base, &r8_tzc_regions[0]);
}

static void bl2_security_setup(void)
{
	g_sysc_fconf_cfg = sysc_config_getter();

	/* initialize TZC-400 */
	if (g_sysc_fconf_cfg->tzc_msram_setup) { plat_tzc_msram_setup(); }
	if (g_sysc_fconf_cfg->tzc_spi_setup) { plat_tzc_spi_setup(); }
	if (g_sysc_fconf_cfg->tzc_ddr_setup) { plat_tzc_ddr_setup(); }
	if (g_sysc_fconf_cfg->tzc_pci_setup) { plat_tzc_pci_setup(); }
	if (g_sysc_fconf_cfg->tzc_r8_setup) { plat_tzc_r8_setup(); }

	/* setup Master/Slave Access Control */
	plat_access_control_setup();
}
#endif

#if IMAGE_BL31
static void bl31_security_setup(void)
{
	const arm_tzc_regions_info_t msram_tzc_regions[] = {
#if TRUSTED_BOARD_BOOT
		{ PLAT_AP_TZC_PROT_SRAM1_BASE, PLAT_AP_TZC_PROT_SRAM1_END,
			TZC_REGION_S_RDWR, PLAT_TZC_REGION_ACCESS_S_UNPRIV },
#endif /* TRUSTED_BOARD_BOOT */
		{}
	};

	const arm_tzc_regions_info_t asram_tzc_regions[] = {
#if TRUSTED_BOARD_BOOT
		{ PLAT_AP_TZC_PROT_SRAM2_BASE, PLAT_AP_TZC_PROT_SRAM2_END,
			TZC_REGION_S_RDWR, PLAT_TZC_REGION_ACCESS_S_UNPRIV },
#endif /* TRUSTED_BOARD_BOOT */
		{}
	};

	/* Additional settings for TZC-400 SRAM */
	plat_tzc400_setup(bl31_board_cfg[soc_id].tzc_msram_base, &msram_tzc_regions[0]);
	plat_tzc400_setup(bl31_board_cfg[soc_id].tzc_asram_base, &asram_tzc_regions[0]);
}
#endif

void plat_security_setup(void)
{
#if IMAGE_BL2
	bl2_security_setup();
#endif

#if IMAGE_BL31
	bl31_security_setup();
#endif
}
