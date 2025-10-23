/*
 * Copyright (c) 2015-2017, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * @file  emmc_def.h
 * @brief eMMC boot is expecting this header file
 *
 */

#ifndef EMMC_DEF_H
#define EMMC_DEF_H

#include "emmc_std.h"

/* ************************ HEADER (INCLUDE) SECTION *********************** */

/* ***************** MACROS, CONSTANTS, COMPILATION FLAGS ****************** */
#define EMMC_POWER_ON		(1U)

/* ********************** STRUCTURES, TYPE DEFINITIONS ********************* */

/* ********************** DECLARATION OF EXTERNAL DATA ********************* */
extern st_mmc_base mmc_drv_obj;

/* ************************** FUNCTION PROTOTYPES ************************** */

/** @brief for assembler program
 */
uint32_t _rom_emmc_finalize(void);

/** @brief eMMC driver API
 */
EMMC_ERROR_CODE emmc_main(void);
EMMC_ERROR_CODE emmc_select_partition(EMMC_PARTITION_ID id);
EMMC_ERROR_CODE emmc_read_sector(uint32_t *buff_address_virtual,
				 uint32_t sector_number, uint32_t count,
				 uint32_t feature_flags);

/* ********************************* CODE ********************************** */

#endif /* EMMC_DEF_H */
/* ******************************** END ************************************ */
