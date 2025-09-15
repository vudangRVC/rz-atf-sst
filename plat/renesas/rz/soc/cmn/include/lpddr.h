/*
 * Copyright (c) 2025, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __LPDDR_H__
#define __LPDDR_H__

#define RET_CSR_SIZE		(0x400)
extern uint32_t ddr_csr_table[RET_CSR_SIZE];

void lpddr4_setup(void);
void ddr_retention_entry(void);
void ddr_retention_exit(uint8_t base);

#endif	/* __RZV2H_DDR_H__ */
