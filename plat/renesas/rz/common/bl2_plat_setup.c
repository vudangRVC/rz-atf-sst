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
#include <scifa.h>
#include <ddr.h>
#include <sys_regs.h>
#include <plat_tzc_def.h>
#include <rzcmn_def.h>
#include <rzv2h_def.h>
#include <rz_private.h>
#include <drivers/delay_timer.h>
#include <lib/fconf/fconf.h>
#include <rz_dt.h>
#include <rz_fconf.h>
#include <board_info.h>

static const mmap_region_t rzcmn_mmap[] = {
#if TRUSTED_BOARD_BOOT
	MAP_REGION_FLAT(RZCMN_BOOT_ROM_BASE, RZCMN_BOOT_ROM_SIZE,
			MT_MEMORY | MT_RO | MT_SECURE),
#endif
	MAP_REGION_FLAT(RZCMN_SRAM_BASE, RZCMN_SRAM_SIZE,
			MT_MEMORY | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(PARAMS_BASE, PARAMS_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(RZCMN_DEVICE_BASE, RZCMN_DEVICE_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(RZCMN_SPIROM_BASE, RZCMN_SPIROM_SIZE,
			MT_MEMORY | MT_RO | MT_SECURE),
	MAP_REGION_FLAT(RZCMN_DDR1_BASE, RZCMN_DDR1_SIZE,
			MT_MEMORY | MT_RW | MT_SECURE),
	{0}
};

/* Global variable where runtime BL2 base is stored */
u_register_t bootrom_param0;
static console_t rzcmn_bl31_console;

int bl2_plat_handle_pre_image_load(unsigned int image_id)
{
	return 0;
}

int bl2_plat_handle_post_image_load(unsigned int image_id)
{
	static bl2_to_bl31_params_mem_t *params;
	bl_mem_params_node_t *bl_mem_params;

	if (!params) {
		params = (bl2_to_bl31_params_mem_t *) PARAMS_BASE;
		memset((void *)PARAMS_BASE, 0, sizeof(*params));
	}

	bl_mem_params = get_bl_mem_params_node(image_id);

	switch (image_id) {
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
	u_register_t dtb_base = 0U;

	bootrom_param0 = arg1;

	if (bootrom_param0 == RZCMN_BL2_BASE) {
		dtb_base = RZCMN_DTB_BASE;
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
}

void bl2_el3_plat_arch_setup(void)
{
	const mmap_region_t bl2_regions[] = {
		MAP_REGION_FLAT(RZCMN_BL2_BASE, BL2_END - RZCMN_BL2_BASE,
			MT_MEMORY | MT_RW | MT_SECURE),
		MAP_REGION_FLAT(BL_CODE_BASE, BL_CODE_END - BL_CODE_BASE,
			MT_CODE | MT_SECURE),
		MAP_REGION_FLAT(BL_RO_DATA_BASE, BL_RO_DATA_END - BL_RO_DATA_BASE,
			MT_RO_DATA | MT_SECURE),
		{0}
	};

	setup_page_tables(bl2_regions, rzcmn_mmap);
	enable_mmu_el3(0);
}

void bl2_platform_setup(void)
{
	/* Setup TZC-400, Access Control */
	plat_security_setup();

#if !DEBUG_RZCMN_FPGA
	/* initialize DDR */
	ddr_setup();
#endif /* DEBUG_FPGA */
	rz_io_setup();
}
