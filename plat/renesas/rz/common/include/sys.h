/*
 * Copyright (c) 2023, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __SYS_H__
#define __SYS_H__

#include <stdbool.h>

#define MASK_BOOTM_DEVICE		(0x0F)
#define MASK_BOOTM_SECURE		(0x10)

typedef enum {
	SYS_BOOT_MODE_ESD = 0,
	SYS_BOOT_MODE_EMMC_1_8,
	SYS_BOOT_MODE_EMMC_3_3,
	SYS_BOOT_MODE_SPI_1_8,
	SYS_BOOT_MODE_SPI_3_3,
	SYS_BOOT_MODE_SCIF,
	SYS_BOOT_MODE_MAX,
} boot_mode_t;


boot_mode_t sys_get_boot_mode(void);
bool sys_is_resume_reboot(void);

#endif	/* __SYS_H__ */
