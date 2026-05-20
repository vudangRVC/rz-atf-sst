/*
 * Copyright (c) 2019-2025, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <common/debug.h>
#include <drivers/console.h>
#include "scif.h"

#include "rcar_private.h"

/* RAS functions common to AArch64 ARM platforms */
void plat_ea_handler(unsigned int ea_reason, uint64_t syndrome, void *cookie,
		     void *handle, uint64_t flags)
{
}

void rcar_console_boot_init(void)
{
	static console_t rcar_boot_console = { 0 };
	uint32_t baudrate;
	uint32_t mode;
	int ret;

	ret = console_rcar_register(0, 0, 0, &rcar_boot_console);
	if (ret == 0)
		panic();

	console_set_scope(&rcar_boot_console, CONSOLE_FLAG_BOOT);

	mode = rcar_gen4_scif_get_mode();
	baudrate = rcar_gen4_scif_get_baudrate();
	if (baudrate != 0U) {
		NOTICE("R-Car Gen4 console: MD32:MD31=0x%x, baudrate=%u\n",
		       mode, baudrate);
	} else {
		NOTICE("R-Car Gen4 console: MD32:MD31=0x%x, SCIF mode\n",
		       mode);
	}
}

void rcar_console_runtime_init(void)
{
	static console_t rcar_runtime_console = { 0 };
	int ret;

	ret = console_rcar_register(1, 0, 0, &rcar_runtime_console);
	if (ret == 0)
		panic();

	console_set_scope(&rcar_runtime_console,
			  CONSOLE_FLAG_BOOT |
			  CONSOLE_FLAG_RUNTIME |
			  CONSOLE_FLAG_CRASH);
}
