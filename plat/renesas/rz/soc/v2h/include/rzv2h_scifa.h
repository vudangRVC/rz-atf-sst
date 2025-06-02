/*
 * Copyright (c) 2020, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __RZV2H_SCIFA_H__
#define __RZV2H_SCIFA_H__

#ifndef __ASSEMBLER__

#include <stdint.h>
#include <drivers/console.h>

int console_rz_register(uintptr_t baseaddr, uint32_t clock,
		uint32_t baud, console_t *console);
#endif /* __ASSEMBLER__ */

#endif	/* __RZV2H_SCIFA_H__ */
