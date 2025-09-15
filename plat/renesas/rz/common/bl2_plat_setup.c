/*
 * Copyright (c) 2020, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>
#include <arch.h>
#include <arch_helpers.h>
#include <assert.h>
#include <common/bl_common.h>
#include <common/desc_image_load.h>
#include <drivers/generic_delay_timer.h>
#include <lib/xlat_tables/xlat_tables_compat.h>
#include <plat/common/common_def.h>
#include <lib/mmio.h>
#include <pfc.h>
#include <cpg_regs.h>
#include <cpg.h>
#include <syc.h>
#include <sys.h>
#include <scifa.h>
#include <ddr.h>
#include <pwrc.h>
#include <sys_regs_offset.h>
#include <plat_tzc_def.h>
#include <rzg2l_def.h>
#include <rzv2h_def.h>
#include <rz_private.h>
#include <drivers/delay_timer.h>
#include <lib/fconf/fconf.h>
#include <rz_dt.h>
#include <rz_fconf.h>
#include <board_info.h>

/* BL1 to BL2 parameters:
 *   - arg1: BL2 start address 
 *   - arg2: Boot ROM info size (not available on all SoCs)
 *   - arg3: Magic string (not available on all SoCs)
 *   - arg4: BL2 end address (not available on all SoCs)
 * 
 *   BL2 arg1 should be used for early board identification.
 */
u_register_t bootrom_param0;
u_register_t dtb_base = 0U;

extern void bl2_enter_bl31(const struct entry_point_info *bl_ep_info);
extern uintptr_t bl31_params_base;
static console_t rzcmn_bl31_console;

static uint32_t bl2_plat_get_boot_mode(void)
{
	if (sys_is_resume_reboot())
		return RZ_WARM_BOOT;

	return RZ_COLD_BOOT;
}

int bl2_plat_handle_pre_image_load(unsigned int image_id)
{
	uint32_t enable_cold_boot = FCONF_GET_PROPERTY(hw_config, common_config, enable_cold_boot);

	if (image_id == BL31_IMAGE_ID && enable_cold_boot) {
		uintptr_t bl2_limit = FCONF_GET_PROPERTY(hw_config, common_config, bl2_limit);
		bl2_to_bl31_params_mem_t *params = (bl2_to_bl31_params_mem_t *)bl2_limit;

		params->boot_kind = bl2_plat_get_boot_mode();

		/* If a warm start is in progress then skip rest of initialisation and jump directly to BL31 */
		if (params->boot_kind == RZ_WARM_BOOT) {
			bl_mem_params_node_t *bl_mem_params = get_bl_mem_params_node(image_id);

			bl_mem_params->image_info.h.attr |= IMAGE_ATTRIB_SKIP_LOADING;
			flush_dcache_range((uintptr_t)bl2_limit, sizeof(bl2_to_bl31_params_mem_t));
			bl2_enter_bl31(&bl_mem_params->ep_info);
		}
	}

	return 0;
}

int bl2_plat_handle_post_image_load(unsigned int image_id)
{
	static bl2_to_bl31_params_mem_t *params;
	bl_mem_params_node_t *bl_mem_params;
	uint32_t soc_id = FCONF_GET_PROPERTY(hw_config, common_config, soc_id);
	uint32_t enable_cold_boot = FCONF_GET_PROPERTY(hw_config, common_config, enable_cold_boot);
	uintptr_t bl2_limit = FCONF_GET_PROPERTY(hw_config, common_config, bl2_limit);

	if (!params) {
		params = (bl2_to_bl31_params_mem_t *) bl2_limit;
		memset((void *)bl2_limit, 0, sizeof(*params));
	}

	bl_mem_params = get_bl_mem_params_node(image_id);

	switch (image_id) {
	case BL31_IMAGE_ID:
		/* This function is only entered for BL31 image if it is the cold boot */
		if(enable_cold_boot) {
			params->boot_kind = RZ_COLD_BOOT;
		}

		bl_mem_params->ep_info.args.arg0 = bl2_limit;
		params->soc_id = soc_id;
		break;

	case BL32_IMAGE_ID:
		memcpy(&params->bl32_ep_info, &bl_mem_params->ep_info,
			sizeof(entry_point_info_t));
		break;
	case BL33_IMAGE_ID:
		memcpy(&params->bl33_ep_info, &bl_mem_params->ep_info,
			sizeof(entry_point_info_t));
		break;
	default:
		/* Do nothing in default case */
		break;
	}

	return 0;
}


void bl2_el3_early_platform_setup(u_register_t arg1, u_register_t arg2,
								u_register_t arg3, u_register_t arg4)
{
	int ret;

	bootrom_param0 = arg1;

	if ((bootrom_param0 == RZG2L_BL2_BASE)) {
		dtb_base = RZG2L_DTB_BASE;
	} else if (bootrom_param0 == RZV2H_BL2_BASE) {
		dtb_base = RZV2H_DTB_BASE;
	}

	/* Validate DTB is valid */
	if (dt_validation(dtb_base) < 0) {
		panic();
	}

	/* Populate HW_CONFIG device tree with the mapped address */
	fconf_populate("HW_CONFIG", dtb_base);

	/* early setup Clock and Reset */
	cpg_early_setup();

	/* initialize SYC */
	uint32_t syc_inck_hz = FCONF_GET_PROPERTY(hw_config, syc_config, syc_inck_hz);
	syc_init(syc_inck_hz);

	/* initialize Timer */
	generic_delay_timer_init();

	/* setup PFC */
	pfc_setup();

	/* setup Clock and Reset */
	cpg_setup();

	/* initialize console driver */
	const struct scif_config_t * g_scif_fconf_cfg = scif_config_getter();
	ret = console_rzcmn_register(
							g_scif_fconf_cfg->scif_base,
							g_scif_fconf_cfg->uart_inck_hz,
							g_scif_fconf_cfg->uart_baudrate,
							&rzcmn_bl31_console);
	if (!ret)
		panic();

	console_set_scope(&rzcmn_bl31_console,
			CONSOLE_FLAG_BOOT | CONSOLE_FLAG_CRASH);

	uint32_t enable_pwrc_setup = FCONF_GET_PROPERTY(hw_config, sysc_config, enable_pwrc_setup);
	if (enable_pwrc_setup) {
		pwrc_setup();
	}
}

void bl2_el3_plat_arch_setup(void)
{
	uint32_t soc_id = FCONF_GET_PROPERTY(hw_config, common_config, soc_id);

	if ((soc_id == RZ_SOC_RZG2L) ||
		(soc_id == RZ_SOC_RZV2L))
	{
		const mmap_region_t bl2_regions[] = {
			MAP_REGION_FLAT(RZG2L_BL2_BASE, BL2_END - RZG2L_BL2_BASE,
				MT_MEMORY | MT_RW | MT_SECURE),
			MAP_REGION_FLAT(BL_CODE_BASE, BL_CODE_END - BL_CODE_BASE,
				MT_CODE | MT_SECURE),
			MAP_REGION_FLAT(BL_RO_DATA_BASE, BL_RO_DATA_END - BL_RO_DATA_BASE,
				MT_RO_DATA | MT_SECURE),
			{0}
		};

		const mmap_region_t rzcmn_mmap[] = {
			MAP_REGION_FLAT(RZG2L_SRAM_BASE, RZG2L_SRAM_SIZE,
					MT_MEMORY | MT_RW | MT_SECURE),
			MAP_REGION_FLAT(PARAMS_BASE, PARAMS_SIZE,
					MT_DEVICE | MT_RW | MT_SECURE),
			MAP_REGION_FLAT(RZG2L_DEVICE_BASE, RZG2L_DEVICE_SIZE,
					MT_DEVICE | MT_RW | MT_SECURE),
			MAP_REGION_FLAT(RZG2L_SPIROM_BASE, RZG2L_SPIROM_SIZE,
					MT_MEMORY | MT_RO | MT_SECURE),
			MAP_REGION_FLAT(RZG2L_DDR1_BASE, RZG2L_DDR1_SIZE,
					MT_MEMORY | MT_RW | MT_SECURE),
			{0}
		};

		setup_page_tables(bl2_regions, rzcmn_mmap);
		enable_mmu_el3(0);
	} 
	// (soc_id == RZ_SOC_RZV2H)
	else
	{
		const mmap_region_t bl2_v2h_regions[] = {
			MAP_REGION_FLAT(RZV2H_BL2_BASE, BL2_END - RZV2H_BL2_BASE,
					MT_MEMORY | MT_RW | MT_SECURE),
			MAP_REGION_FLAT(BL_CODE_BASE, BL_CODE_END - BL_CODE_BASE,
					MT_CODE | MT_SECURE),
			MAP_REGION_FLAT(RZV2H_BOOTINFO_BASE, RZV2H_BOOTINFO_SIZE,
					MT_MEMORY | MT_RO | MT_SECURE),
			MAP_REGION_FLAT(RZV2H_PARAMS_BASE, PARAMS_SIZE,
					MT_MEMORY | MT_RW | MT_SECURE),
			MAP_REGION_FLAT(RZV2H_DTB_BASE, RZV2H_DTB_LIMIT - RZV2H_DTB_BASE,
					MT_MEMORY | MT_RW | MT_SECURE),
			{0}
		};

		const mmap_region_t rzv2h_mmap[] = {
			MAP_REGION_FLAT(RZV2H_DEVICE_BASE, RZV2H_DEVICE_SIZE,
					MT_DEVICE | MT_RW | MT_SECURE),
			MAP_REGION_FLAT(RZV2H_XSPI_MEMORY_MAP_BASE, RZV2H_XSPI_SIZE,
					MT_MEMORY | MT_RO | MT_SECURE),
			MAP_REGION_FLAT(RZV2H_DDR0_BASE, RZV2H_DDR0_SIZE,
					MT_MEMORY | MT_RW | MT_SECURE),
			{0}
		};

		setup_page_tables(bl2_v2h_regions, rzv2h_mmap);
		enable_mmu_el3(0);
	}
}

void bl2_platform_setup(void)
{
	/* Setup TZC-400, Access Control */
	plat_security_setup();

#if !DEBUG_RZG2L_FPGA
	/* initialize DDR */
	ddr_setup();
#endif /* DEBUG_FPGA */
	rz_io_setup();
}
