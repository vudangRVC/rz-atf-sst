/*
 * Copyright (c) 2023, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __RZ_PRIVATE_H__
#define __RZ_PRIVATE_H__

#include <common/bl_common.h>
#include <platform_def.h>

/* plat_helper.S */
void plat_invalidate_icache(void);

/* plat_gic.c */
void plat_gic_driver_init(void);
void plat_gic_init(void);
void plat_gic_cpuif_enable(void);
void plat_gic_cpuif_disable(void);
void plat_gic_pcpu_init(void);
void plat_gic_save(void);
void plat_gic_resume(void);

/* plat_storage.c */
void rz_io_setup(void);
void rzcmn_io_setup(void);
/* plat_ddr_setup.c  */
void plat_ddr_setup(void);

/* bl31_plat_setup.c */
void plat_copy_code_to_system_ram(void);

typedef enum boot_kind {
	RZ_COLD_BOOT,
	RZ_WARM_BOOT
} boot_kind_t;

typedef struct bl2_to_bl31_params_mem {
	boot_kind_t boot_kind;
	uint32_t soc_id;
	entry_point_info_t bl32_ep_info;
	entry_point_info_t bl33_ep_info;
} bl2_to_bl31_params_mem_t;

typedef struct {
	/* Device specific info. */
	const char *soc_name;
	uint32_t    scif0_base;
	uint32_t    sram_base;
	uint32_t    tzc_msram_base;
	uint32_t    tzc_asram_base;
	uint32_t    syc_timer_base;
	uint32_t    sysc_base;
	uint32_t    cpg_base;
	uint32_t    gicd_base;
	uint32_t    gicr_base;
	uint32_t    platform_core_count;
	uint32_t    otp_base;
	uint32_t    otp_base_chipid;
	uint32_t    device_area_size;
	uint64_t    ddr_area_size;
	uint32_t    board_info_qspi_offset;
	uint32_t    board_info_emmc_offset;
	uint32_t    board_info_esd_offset;

	/* Platform specific function control. */
	bool enable_tzc_setup;
	bool enable_pwrc_setup;
} bl31_board_cfg_t;

#endif	/* __RZ_PRIVATE_H__ */
