/*
 * Copyright (c) 2023, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __CPG_H__
#define __CPG_H__

typedef struct {
	uintptr_t addr;
	uint32_t  val;
} CPG_REG_SETTING;

typedef struct {
	CPG_REG_SETTING reg;
	CPG_REG_SETTING mon;
	uint32_t  type;
} CPG_SETUP_DATA;

typedef struct {
	CPG_REG_SETTING stby;
	CPG_REG_SETTING clk1;
	CPG_REG_SETTING clk2;
	CPG_REG_SETTING mon;
} CPG_PLL_SETTINGS;

void cpg_early_setup(void);
void cpg_setup(void);
void cpg_ddr0_part1(void);
void cpg_ddr0_part2(void);
void cpg_ddr1_part1(void);
void cpg_ddr1_part2(void);
void cpg_prepare_suspend(void);
void cpg_ddr_pwrokin_off(uint8_t base);

uint8_t cpg_pll_get_sub_sub_node(void *fdt, const char *sub_sub_note, CPG_PLL_SETTINGS *p_pll_sub_sub);
uint8_t cpg_pll_re_setup(void *fdt, const char *sub_sub_note, int num);
#endif /* __CPG_H__ */
