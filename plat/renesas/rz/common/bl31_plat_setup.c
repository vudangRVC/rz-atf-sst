/*
 * Copyright (c) 2020, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_helpers.h>
#include <assert.h>
#include <common/bl_common.h>
#include <lib/xlat_tables/xlat_tables_compat.h>
#include <plat/common/common_def.h>

#include <scifa.h>
#include <plat_tzc_def.h>
#include <rz_private.h>
#include <pwrc.h>
#include <rzg2l_def.h>
#include <board_info.h>

static console_t rzcmn_bl31_console;
static bl2_to_bl31_params_mem_t from_bl2;

#ifdef PLAT_EXTRA_LD_SCRIPT
IMPORT_SYM(uintptr_t, __BL31_PMUSRAM_START__, BL31_PMUSRAM_START);
IMPORT_SYM(uintptr_t, __BL31_PMUSRAM_END__, BL31_PMUSRAM_END);
IMPORT_SYM(uintptr_t, __BL31_PMUSRAM_BASE__, BL31_PMUSRAM_BASE);
#endif

void plat_copy_code_to_system_ram(void)
{
#if (PLAT_EXTRA_LD_SCRIPT && PLAT_SYSTEM_SUSPEND)
	int ret __attribute__ ((unused));
	uint32_t attr;
	const uintptr_t pmu_code_load = BL31_PMUSRAM_BASE;
	const uintptr_t pmu_code_image = BL31_PMUSRAM_START;
	size_t pmu_code_size = BL31_PMUSRAM_END - BL31_PMUSRAM_START;

	attr = MT_MEMORY | MT_RW | MT_SECURE | MT_EXECUTE_NEVER;
	ret = xlat_change_mem_attributes(pmu_code_image, pmu_code_size, attr);
	assert(ret == 0);

	memcpy((void *)pmu_code_image, (void *)pmu_code_load, pmu_code_size);
	flush_dcache_range(pmu_code_image, pmu_code_size);

	attr = MT_MEMORY | MT_RO | MT_SECURE | MT_EXECUTE;
	ret = xlat_change_mem_attributes(pmu_code_image, pmu_code_size, attr);
	assert(ret == 0);

	/* Invalidate instruction cache */
	plat_invalidate_icache();
	dsb();
	isb();
#endif /* PLAT_EXTRA_LD_SCRIPT && PLAT_SYSTEM_SUSPEND */
}

entry_point_info_t *bl31_plat_get_next_image_ep_info(uint32_t type);

uint32_t soc_id;

bl31_board_cfg_t bl31_board_cfg[] = {
	[RZ_SOC_RZG2L] = {
		.soc_name              = "rzg2l",
		.scif0_base        = RZG2L_SCIF0_BASE,
		.sram_base         = RZG2L_SRAM_BASE,
		.tzc_msram_base    = RZG2L_TZC_MSRAM_BASE,
		.tzc_asram_base    = RZG2L_TZC_ASRAM_BASE,
		.syc_timer_base    = RZG2L_SYC_BASE,
		.sysc_base         = RZG2L_SYSC_BASE,
		.cpg_base          = RZG2L_CPG_BASE,
		.gicd_base         = RZG2L_GICD_BASE,
		.gicr_base         = RZG2L_GICR_BASE,
		.platform_core_count = PLATFORM_CORE_COUNT,
		.otp_base          = RZG2L_OTP_BASE,
		.otp_base_chipid   = (RZG2L_OTP_BASE + 0x1140),
		.device_area_size  = RZG2L_DEVICE_SIZE,
		.ddr_area_size     = RZG2L_DDR1_SIZE,
		.board_info_qspi_offset = BOARD_INFO_QSPI_OFFSET,

		.enable_tzc_setup  = true,
		.enable_pwrc_setup = false,
	},
	[RZ_SOC_RZV2L] = {
		.soc_name              = "rzv2l",
		.scif0_base        = RZG2L_SCIF0_BASE,
		.sram_base         = RZG2L_SRAM_BASE,
		.tzc_msram_base    = RZG2L_TZC_MSRAM_BASE,
		.tzc_asram_base    = RZG2L_TZC_ASRAM_BASE,
		.syc_timer_base    = RZG2L_SYC_BASE,
		.sysc_base         = RZG2L_SYSC_BASE,
		.cpg_base          = RZG2L_CPG_BASE,
		.gicd_base         = RZG2L_GICD_BASE,
		.gicr_base         = RZG2L_GICR_BASE,
		.platform_core_count = PLATFORM_CORE_COUNT,
		.otp_base          = RZG2L_OTP_BASE,
		.otp_base_chipid   = (RZG2L_OTP_BASE + 0x1140),
		.device_area_size  = RZG2L_DEVICE_SIZE,
		.ddr_area_size     = RZG2L_DDR1_SIZE,
		.board_info_qspi_offset = BOARD_INFO_QSPI_OFFSET,

		.enable_tzc_setup  = true,
		.enable_pwrc_setup = false,
	},
	[RZ_SOC_RZV2H] = {
		.soc_name              = "rzv2h",
		.scif0_base        = RZV2H_SCIF_BASE,
		.sram_base         = RZV2H_SRAM_BASE,
		.tzc_msram_base    = RZV2H_TZC400_M33_BASE,
		.tzc_asram_base    = RZV2H_TZC400_A55_BASE,
		.syc_timer_base    = RZV2H_SYC_BASE,
		.sysc_base         = RZV2H_SYSC_BASE,
		.cpg_base          = RZV2H_CPG_BASE,
		.gicd_base         = RZV2H_GICD_BASE,
		.gicr_base         = RZV2H_GICR_BASE,
		.platform_core_count = RZV2H_PLATFORM_CORE_COUNT,
		.otp_base          = RZV2H_OTP_BASE,
		.otp_base_chipid   = (RZV2H_OTP_BASE + 0x114C),
		.device_area_size  = RZV2H_DEVICE_SIZE,
		.ddr_area_size     = RZV2H_DDR0_SIZE,
		.board_info_qspi_offset = RZV2H_BOARD_INFO_xSPI_OFFSET,

		.enable_tzc_setup  = false,
		.enable_pwrc_setup = true,
	},
};

void bl31_early_platform_setup2(u_register_t arg0,
								u_register_t arg1,
								u_register_t arg2,
								u_register_t arg3)
{
	int ret;

	soc_id = arg1 >> 32;

	/* initialize console driver */
	ret = console_rzcmn_register(
							bl31_board_cfg[soc_id].scif0_base,
							RZG2L_UART_INCK_HZ,
							RZG2L_UART_BARDRATE,
							&rzcmn_bl31_console);
	if (!ret)
		panic();

	console_set_scope(&rzcmn_bl31_console,
			CONSOLE_FLAG_BOOT | CONSOLE_FLAG_RUNTIME | CONSOLE_FLAG_CRASH);

	/* copy bl2_to_bl31_params_mem_t*/
	memcpy(&from_bl2, (void *)arg0, sizeof(from_bl2));

	INFO("BL31: Setup for SoC: %s\n", bl31_board_cfg[soc_id].soc_name);
}

void bl31_plat_arch_setup(void)
{
	const mmap_region_t bl31_regions[] = {
		MAP_REGION_FLAT(BL31_START, BL31_END - BL31_START,
						MT_MEMORY | MT_RW | MT_SECURE),
		MAP_REGION_FLAT(BL_CODE_BASE, BL_CODE_END - BL_CODE_BASE,
						MT_CODE | MT_SECURE),
		MAP_REGION_FLAT(BL_RO_DATA_BASE, BL_RO_DATA_END - BL_RO_DATA_BASE,
						MT_RO_DATA | MT_SECURE),
		{0}
	};

	const mmap_region_t rzcmn_mmap[] = {
		MAP_REGION_FLAT(bl31_board_cfg[soc_id].sram_base, RZG2L_SRAM_SIZE,
				MT_MEMORY | MT_RW | MT_SECURE),
		MAP_REGION_FLAT(RZG2L_DEVICE_BASE, bl31_board_cfg[soc_id].device_area_size,
				MT_DEVICE | MT_RW | MT_SECURE),
		MAP_REGION_FLAT(RZG2L_DDR1_BASE, bl31_board_cfg[soc_id].ddr_area_size,
				MT_MEMORY | MT_RW | MT_SECURE),
		MAP_REGION_FLAT(RZG2L_SPIROM_BASE, RZG2L_SPIROM_SIZE,
				MT_MEMORY | MT_RW | MT_SECURE),
		{0}
	};

	setup_page_tables(bl31_regions, rzcmn_mmap);
	enable_mmu_el3(0);
}

void bl31_platform_setup(void)
{
	/* Setup TZC-400 */
	if (bl31_board_cfg[soc_id].enable_tzc_setup == true) {
		plat_security_setup();
	}
	
#if !DEBUG_RZG2L_FPGA
	/* initialize GIC-600 */
	plat_gic_driver_init();
	plat_gic_init();
#endif
	
	if (bl31_board_cfg[soc_id].enable_pwrc_setup == true) {
		pwrc_setup();
	}

	/* Read model id from QSPI */
	uint32_t model = get_board_info_u32(RZG2L_SPIROM_BASE, RZG2L_SPIROM_SIZE, bl31_board_cfg[soc_id].board_info_qspi_offset, OFFSET_MODEL_ID);

	/* Get entry point info for BL33 */
	entry_point_info_t *bl33_ep_info = bl31_plat_get_next_image_ep_info(NON_SECURE);

	/* Pass model ID and SoC ID to U-Boot via registers */
	if (bl33_ep_info != NULL) {
		bl33_ep_info->args.arg2 = model;
		bl33_ep_info->args.arg3 = soc_id;
	}
}

entry_point_info_t *bl31_plat_get_next_image_ep_info(uint32_t type)
{
	entry_point_info_t *next_image_info = NULL;

	next_image_info = (type == NON_SECURE)
			? &from_bl2.bl33_ep_info : &from_bl2.bl32_ep_info;
	
	if (next_image_info->pc)
		return next_image_info;
	else
		return NULL;
}
