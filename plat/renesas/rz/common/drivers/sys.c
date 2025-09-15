/*
 * Copyright (c) 2023, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


#include <lib/mmio.h>
#include <sys.h>
#include <sys_regs_offset.h>
#include <common/debug.h>
#include <lib/fconf/fconf.h>
#include <rz_fconf.h>



bool sys_is_resume_reboot(void)
{
#if PLAT_SYSTEM_SUSPEND
	return pwrc_board_is_resume();
#else
	return false;
#endif
}

boot_mode_t sys_get_boot_mode(void)
{
	/* Initialize global SYSC config from DTB.  */
	const struct sysc_config_t * g_sysc_fconf_cfg = sysc_config_getter();

	uint32_t stat_md_boot = mmio_read_32(g_sysc_fconf_cfg->sysc_base + g_sysc_fconf_cfg->sysc_lsi_mode_reg_offset) & g_sysc_fconf_cfg->sysc_lsi_mode_mask;
	boot_mode_t boot_mode = 0;

	if (stat_md_boot == g_sysc_fconf_cfg->sysc_boot_mode_esd) {
		boot_mode = SYS_BOOT_MODE_ESD;
	} else if (stat_md_boot == g_sysc_fconf_cfg->sysc_boot_mode_emmc_1_8) {
		boot_mode = SYS_BOOT_MODE_EMMC_1_8;
	} else if (stat_md_boot == g_sysc_fconf_cfg->sysc_boot_mode_emmc_3_3) {
		boot_mode = SYS_BOOT_MODE_EMMC_3_3;
	} else if (stat_md_boot == g_sysc_fconf_cfg->sysc_boot_mode_spi_1_8) {
		boot_mode = SYS_BOOT_MODE_SPI_1_8;
	} else if (stat_md_boot == g_sysc_fconf_cfg->sysc_boot_mode_spi_3_3) {
		boot_mode = SYS_BOOT_MODE_SPI_3_3;
	}

	return boot_mode;
}
