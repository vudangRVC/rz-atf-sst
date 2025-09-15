/*
 * Copyright (c) 2022, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/mmio.h>
#include <rzg2l_def.h>
#include <rz_private.h>
#if IMAGE_BL2
#include <rz_fconf.h>
#include <lib/fconf/fconf.h>
#endif

#define CNTCR		(0x000)
#define CNTFID0		(0x020)

#if IMAGE_BL31
extern bl31_board_cfg_t bl31_board_cfg[];
extern uint32_t soc_id;
#endif

static inline void syc_reg_write(uint32_t offset, uint32_t val)
{
#if IMAGE_BL2
	/* Get SYC base address from FCONF */
	uint32_t syc_base = FCONF_GET_PROPERTY(hw_config, syc_config, syc_base);
#endif
#if IMAGE_BL31
	uint32_t syc_base = bl31_board_cfg[soc_id].syc_timer_base;
#endif

	mmio_write_32(syc_base + offset, val);
}

static inline uint32_t syc_reg_read(uint32_t offset)
{
#if IMAGE_BL2
	/* Get SYC base address from FCONF */
	uint32_t syc_base = FCONF_GET_PROPERTY(hw_config, syc_config, syc_base);
#endif
#if IMAGE_BL31
	uint32_t syc_base = bl31_board_cfg[soc_id].syc_timer_base;
#endif

	return mmio_read_32(syc_base + offset);
}

static void enable_counter(unsigned int enable)
{
	syc_reg_write(CNTCR, enable & CNTCR_EN);
}

void syc_init(unsigned int freq)
{
	syc_reg_write(CNTFID0, freq);
	enable_counter(CNTCR_EN);
}

unsigned int syc_get_freq(void)
{
	return syc_reg_read(CNTFID0);
}
