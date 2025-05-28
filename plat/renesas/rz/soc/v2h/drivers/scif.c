/*
 * Copyright (c) 2023, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <stddef.h>
#include <pfc_regs.h>
#include <pfc_regs_offset.h>
#include <sys_regs.h>
#include <sys_regs_offset.h>
#include <sys.h>
#include <lib/mmio.h>

#include <platform_def.h>
#include <rz_dt.h>
#include <common/debug.h>
#include <scifa.h>

void console_setup(void *fdt, console_t *console){

	/* Get serial base address */
	uint32_t scif_base = 0;
	if(read_prop_from_subnode(fdt, "/soc", "serial@11c01400", "reg", 1, &scif_base) != 0) {
		ERROR("BL2: Failed to get scif base address\n");
		return;
	}

	/* Get Serial inck clock*/
	uint32_t uart_inck_hz = 0;
	if(read_prop_from_subnode(fdt, "/soc", "serial@11c01400", "uart_inck_hz", 0, &uart_inck_hz) != 0) {
		ERROR("BL2: Failed to get scif base address\n");
		return;
	}

	/* Get Serial baudrate */
	uint32_t uart_bardrate = 0;
	if(read_prop_from_subnode(fdt, "/soc", "serial@11c01400", "uart_bardrate", 0, &uart_bardrate) != 0) {
		ERROR("BL2: Failed to get scif base address\n");
		return;
	}

	/* initialize console driver */
	int ret = 0;
	ret = console_rz_register(
							scif_base,
							uart_inck_hz,
							uart_bardrate,
							console);
	if (!ret)
		panic();

	console_set_scope(console,
			CONSOLE_FLAG_BOOT | CONSOLE_FLAG_CRASH);
}
