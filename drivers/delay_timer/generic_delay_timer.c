/*
 * Copyright (c) 2016-2019, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2020, NVIDIA Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <arch_features.h>
#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <drivers/generic_delay_timer.h>
#include <lib/utils_def.h>
#include <plat/common/platform.h>
#include <rz_fconf.h>
#include <rz_dt.h>
#include <lib/mmio.h>

static timer_ops_t ops;

static uint32_t get_timer_value(void)
{
	/*
	 * Generic delay timer implementation expects the timer to be a down
	 * counter. We apply bitwise NOT operator to the tick values returned
	 * by read_cntpct_el0() to simulate the down counter. The value is
	 * clipped from 64 to 32 bits.
	 */
	return (uint32_t)(~read_cntpct_el0());
}

void generic_delay_timer_init_args(uint32_t mult, uint32_t div)
{
	ops.get_timer_value	= get_timer_value;
	ops.clk_mult		= mult;
	ops.clk_div		= div;

	timer_init(&ops);

	VERBOSE("Generic delay timer configured with mult=%u and div=%u\n",
		mult, div);
}

void generic_delay_timer_init(void)
{
	assert(is_armv7_gentimer_present());

	/* Value in ticks */
	uint32_t mult = timer_config.timer_mul;

	/* Value in ticks per second (Hz) */
	unsigned int div  = plat_get_syscnt_freq2();

	/* Reduce multiplier and divider by dividing them repeatedly by 10 */
	while (((mult % 10U) == 0U) && ((div % 10U) == 0U)) {
		mult /= 10U;
		div /= 10U;
	}

	generic_delay_timer_init_args(mult, div);
}

void generic_delay_timer_init_v2h(void *fdt)
{
	assert(is_armv7_gentimer_present());

	// Get timer mul value from FDT
	uint32_t mult = 0;
	if(read_prop_from_subnode(fdt, "/soc", "system-timer@14010000", "syc_timer_mul", 0, &mult) != 0) {
		return;
	}

	// Get timer offset value from FDT
	uint32_t timer_offset = 0;
	if(read_prop_from_subnode(fdt, "/soc", "system-timer@14010000", "syc_timer_offset", 0, &timer_offset) != 0) {
		return;
	}

	// Get timer base value from FDT
	uint32_t timer_base = 0;
	if(read_prop_from_subnode(fdt, "/soc", "system-timer@14010000", "reg", 1, &timer_base) != 0) {
		return;
	}

	/* Value in ticks per second (Hz) */
	unsigned int div = mmio_read_32(timer_base + timer_offset);

	/* Reduce multiplier and divider by dividing them repeatedly by 10 */
	while (((mult % 10U) == 0U) && ((div % 10U) == 0U)) {
		mult /= 10U;
		div /= 10U;
	}

	generic_delay_timer_init_args(mult, div);
}

