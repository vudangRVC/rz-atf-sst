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
#include <drivers/generic_delay_timer.h>
#include <drivers/io/io_storage.h>
#include <drivers/io/io_driver.h>
#include <sys.h>
#include <io_emmcdrv.h>
#include <emmc_def.h>

#include <scifa.h>
#include <plat_tzc_def.h>
#include <rz_private.h>
#include <rzg2l_def.h>

#include <board_info.h>

static EMMC_PARTITION_ID g_last_selected_part = PARTITION_ID_USER;

EMMC_ERROR_CODE rz_emmc_select_partition(EMMC_PARTITION_ID id)
{
    EMMC_ERROR_CODE rc = emmc_select_partition(id);
    if (rc == EMMC_SUCCESS) g_last_selected_part = id;
    return rc;
}

static const char *part_name(EMMC_PARTITION_ID id)
{
    switch (id) {
    case PARTITION_ID_USER: return "USER";
    case PARTITION_ID_BOOT_1:    return "BOOT#1";
    case PARTITION_ID_BOOT_2:    return "BOOT#2";
    case PARTITION_ID_RPMB:      return "RPMB";
    default:                     return "GEN/UNKNOWN";
    }
}

void dump_part_cfg(void)
{
    NOTICE("BL31: selected eMMC partition: %s (%d)\n",
           part_name(g_last_selected_part), (int)g_last_selected_part);
}

static const mmap_region_t rzg2l_mmap[] = {
	MAP_REGION_FLAT(RZG2L_SRAM_BASE, RZG2L_SRAM_SIZE,
			MT_MEMORY | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(RZG2L_DEVICE_BASE, RZG2L_DEVICE_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(RZG2L_DDR1_BASE, RZG2L_DDR1_SIZE,
			MT_MEMORY | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(RZG2L_SPIROM_BASE, RZG2L_SPIROM_SIZE,
			MT_MEMORY | MT_RW | MT_SECURE),
	{0}
};

static console_t rzg2l_bl31_console;
static bl2_to_bl31_params_mem_t from_bl2;
static uintptr_t emmcdrv_dev_handle;

entry_point_info_t *bl31_plat_get_next_image_ep_info(uint32_t type);

static int init_emmc_driver(void)
{
	int ret;
	const io_dev_connector_t *emmc;

	if (emmc_init() != EMMC_SUCCESS) {
		NOTICE("BL2: Failed to eMMC driver initialize.\n");
		panic();
	}
	emmc_memcard_power(EMMC_POWER_ON);
	if (emmc_mount() != EMMC_SUCCESS) {
		NOTICE("BL2: Failed to eMMC mount operation.\n");
		panic();
	}

	/* Open the eMMC driver */
	register_io_dev_emmcdrv(&emmc);
	ret = io_dev_open(emmc, 0, &emmcdrv_dev_handle);
	if (ret != 0) {
		ERROR("Failed to open eMMC device: %d\n", ret);
		return ret;
	}

	/* Initialize the device */
	ret = io_dev_init(emmcdrv_dev_handle, 0);
	if (ret != 0) {
		ERROR("Failed to init eMMC device: %d\n", ret);
		return ret;
	}

	return 0;
}

void bl31_early_platform_setup2(u_register_t arg0,
								u_register_t arg1,
								u_register_t arg2,
								u_register_t arg3)
{
	int ret;

	/* initialize console driver */
	ret = console_rzg2l_register(
							RZG2L_SCIF0_BASE,
							RZG2L_UART_INCK_HZ,
							RZG2L_UART_BARDRATE,
							&rzg2l_bl31_console);
	if (!ret)
		panic();

	console_set_scope(&rzg2l_bl31_console,
			CONSOLE_FLAG_BOOT | CONSOLE_FLAG_RUNTIME | CONSOLE_FLAG_CRASH);

	/* copy bl2_to_bl31_params_mem_t*/
	memcpy(&from_bl2, (void *)arg0, sizeof(from_bl2));
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

	setup_page_tables(bl31_regions, rzg2l_mmap);
	enable_mmu_el3(0);
}

void bl31_platform_setup(void)
{
	/* Setup TZC-400 */
	plat_security_setup();

#if !DEBUG_RZG2L_FPGA
	/* initialize GIC-600 */
	plat_gic_driver_init();
	plat_gic_init();
#endif

	/* initialize Timer */
	generic_delay_timer_init();

	uint32_t model;
	uint32_t revision;
	boot_mode_t boot_mode;
	boot_mode = sys_get_boot_mode();
	NOTICE("BL31: Boot mode: %d\n", boot_mode);

	if (boot_mode == SYS_BOOT_MODE_SPI_1_8 ||
		boot_mode == SYS_BOOT_MODE_SPI_3_3) {
		/* Read model and revision id from QSPI */
		model = get_board_info_u32(RZG2L_SPIROM_BASE, RZG2L_SPIROM_SIZE, BOARD_INFO_QSPI_OFFSET, OFFSET_MODEL_ID);
		revision = get_board_info_u32(RZG2L_SPIROM_BASE, RZG2L_SPIROM_SIZE, BOARD_INFO_QSPI_OFFSET, OFFSET_REVISION);
		
	} else if  (boot_mode == SYS_BOOT_MODE_EMMC_1_8 ||
		boot_mode == SYS_BOOT_MODE_EMMC_3_3) {

		/* Initialize eMMC driver */
		uint32_t ret;
		ret = init_emmc_driver();
		if (ret != 0) {
			ERROR("Failed to initialize eMMC driver: %d\n", ret);
			panic();
		}

		NOTICE("BL31: eMMC driver initialized successfully\n");

		/* after init_emmc_driver() */
		if (rz_emmc_select_partition(PARTITION_ID_BOOT_1) != EMMC_SUCCESS) {
			ERROR("BL31: select BOOT#1 failed\n"); panic();
		}
		dump_part_cfg();  /* prints BOOT#1 */

		/* after init_emmc_driver() and before any read */
		EMMC_ERROR_CODE eret = emmc_select_partition(PARTITION_ID_BOOT_1);
		if (eret != EMMC_SUCCESS) {
			ERROR("BL31: emmc_select_partition(BOOT#1) failed: %u\n", eret);
			panic();
		}
		NOTICE("BL31: eMMC: selected Boot Partition #1\n");

		quick_probe();

		/* You already select BOOT#1 earlier */
		model    = get_board_info_u32_emmc(227, OFFSET_MODEL_ID);
		revision = get_board_info_u32_emmc(227, OFFSET_REVISION);


	} else if (boot_mode == SYS_BOOT_MODE_ESD) {
		/* Placeholder for eSD support, currently it is unsupported. */
		ERROR("Unsupported IO device %d.\n", boot_mode);
		panic();
	} else {
		ERROR("Unsupported IO device %d.\n", boot_mode);
		panic();
	}

	/* Get entry point info for BL33 */
	entry_point_info_t *bl33_ep_info = bl31_plat_get_next_image_ep_info(NON_SECURE);

	INFO("Model ID: %d\n", model);
	INFO("Revision: %d\n", revision);
	if (bl33_ep_info != NULL) {
		bl33_ep_info->args.arg2 = model;
		bl33_ep_info->args.arg3 = revision;
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
