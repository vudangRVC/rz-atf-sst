/*
 * Copyright (c) 2020, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <cpg_regs_offset.h>
#include <cpg.h>
#include <lib/mmio.h>
#include <drivers/delay_timer.h>
#include <cpg_opt.h>
#include <rz_fconf.h>
#include <rz_dt.h>
#include <lib/fconf/fconf.h>
#include <libfdt.h>
#include <board_info.h>

#define	CPG_OFF			(0)
#define	CPG_ON			(1)

#define CPG_T_CLK		(0)
#define CPG_T_RST		(1)

#define CPG_TYPE_1		(0)
#define CPG_TYPE_2		(1)

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

extern u_register_t dtb_base;

const struct cpg_config_t * g_cpg_fconf_cfg;
#define CPG_REG_ADDR(offset)  		((uintptr_t)(g_cpg_fconf_cfg->cpg_base + (offset)))
#define CPG_REG_WRITE(reg, value)	mmio_write_32(CPG_REG_ADDR(reg), value)
#define CPG_REG_READ(reg)			mmio_read_32(CPG_REG_ADDR(reg))

/*
 * Write given MSTOP register to remove module stops of bits in given 'val'. Corrosponding MSTOP bit
 * enable in top word needs to be set and just zero to MSTOP bits, therefore **no** Read-Modify-Write is required.
 */
#define REMOVE_MSTOPS_W(reg, val)		CPG_REG_WRITE((reg), ((val) << 16U))

#define	CPG_PLL2_INDEX					(0)
#define	CPG_PLL3_INDEX					(1)
#define	CPG_PLL5_INDEX					(2)

static const CPG_SETUP_DATA early_setup_tbl[] = {
	{
		{(uintptr_t)CPG_CLKON_SYC, 0x00010001,},
		{(uintptr_t)CPG_CLKMON_SYC},
		CPG_T_CLK
	},
	{
		{(uintptr_t)CPG_RST_SYC, 0x00010001,},
		{(uintptr_t)CPG_RSTMON_SYC},
		CPG_T_RST
	}
};

#define CPG_SEL_PLL1_ON_OFF					(0)
#define CPG_SEL_PLL2_1_ON_OFF				(1)
#define CPG_SEL_PLL2_2_ON_OFF				(2)
#define CPG_SEL_PLL3_1_ON_OFF				(3)
#define CPG_SEL_PLL3_2_ON_OFF				(4)
#define CPG_SEL_PLL3_3_ON_OFF				(5)
#define CPG_SEL_PLL5_1_ON_OFF				(6)
#define CPG_SEL_PLL5_3_ON_OFF				(7)
#define CPG_SEL_PLL5_4_ON_OFF				(8)
#define CPG_SEL_PLL6_1_ON_OFF				(9)
#define CPG_SEL_GPU1_1_ON_OFF				(10)
#define CPG_SEL_GPU1_2_ON_OFF				(11)
#define CPG_SEL_GPU2_ON_OFF					(12)

static CPG_REG_SETTING cpg_sel_pll1_on_off[] = {
	{(uintptr_t)CPG_CLKON_CA55, 0x00010001 }
};

static CPG_REG_SETTING cpg_sel_pll2_1_on_off[] = {
	{(uintptr_t)CPG_CLKON_ADC, 0x00010001 },
	{(uintptr_t)CPG_CLKON_TSU, 0x00010001 },
	{(uintptr_t)CPG_CLKON_SDHI, 0x00770077 }
};

static CPG_REG_SETTING cpg_sel_pll2_2_on_off[] = {
	{(uintptr_t)CPG_CLKON_SDHI, 0x00770077 },
#if !RZG2UL
	{(uintptr_t)CPG_CLKON_MIPI_DSI, 0x00200020 },
#endif
};

static CPG_REG_SETTING cpg_sel_pll3_1_on_off[] = {
	{(uintptr_t)CPG_CLKON_AXI_ACPU_BUS, 0x000F000F },
	{(uintptr_t)CPG_CLKON_AXI_COM_BUS, 0x00030003 },
	{(uintptr_t)CPG_CLKON_AXI_DEFAULT_SLV, 0x00010001 },
	{(uintptr_t)CPG_CLKON_AXI_MCPU_BUS, 0x01930193 },
	{(uintptr_t)CPG_CLKON_AXI_TZCDDR, 0x001F001F },
	{(uintptr_t)CPG_CLKON_AXI_VIDEO_BUS, 0x00030003 },
	{(uintptr_t)CPG_CLKON_CA55, 0x001E001E },
	{(uintptr_t)CPG_CLKON_CM33, 0x00010001 },
	{(uintptr_t)CPG_CLKON_CRU, 0x000C000C },
	{(uintptr_t)CPG_CLKON_CST, 0x07FD07FD },
	{(uintptr_t)CPG_CLKON_DAMC_REG, 0x00030003 },
	{(uintptr_t)CPG_CLKON_DDR, 0x00030003 },
	{(uintptr_t)CPG_CLKON_ETH, 0x00030003 },
	{(uintptr_t)CPG_CLKON_GIC600, 0x00010001 },
#if !RZG2UL
	{(uintptr_t)CPG_CLKON_GPU, 0x00070007 },
	{(uintptr_t)CPG_CLKON_H264, 0x00010001 },
#endif
	{(uintptr_t)CPG_CLKON_IA55, 0x00030003 },
	{(uintptr_t)CPG_CLKON_IM33, 0x00030003 },
	{(uintptr_t)CPG_CLKON_ISU, 0x00030003 },
	{(uintptr_t)CPG_CLKON_JAUTH, 0x00010001 },
	{(uintptr_t)CPG_CLKON_LCDC, 0x00010001 },
#if !RZG2UL
	{(uintptr_t)CPG_CLKON_MIPI_DSI, 0x000C000C },
#endif
	{(uintptr_t)CPG_CLKON_OTP, 0x00020002 },
	{(uintptr_t)CPG_CLKON_PERI_COM, 0x00030003 },
	{(uintptr_t)CPG_CLKON_PERI_CPU, 0x000D000D },
	{(uintptr_t)CPG_CLKON_PERI_DDR, 0x00010001 },
	{(uintptr_t)CPG_CLKON_PERI_VIDEO, 0x00070007 },
	{(uintptr_t)CPG_CLKON_REG0_BUS, 0x00010001 },
	{(uintptr_t)CPG_CLKON_REG1_BUS, 0x00030003 },
	{(uintptr_t)CPG_CLKON_ROM, 0x00010001 },
	{(uintptr_t)CPG_CLKON_SDHI, 0x00880088 },
	{(uintptr_t)CPG_CLKON_SRAM_ACPU, 0x00010001 },
	{(uintptr_t)CPG_CLKON_SRAM_MCPU, 0x00010001 },
	{(uintptr_t)CPG_CLKON_SYSC, 0x00020002 },
	{(uintptr_t)CPG_CLKON_TSIPG, 0x00030003 },
	{(uintptr_t)CPG_CLKON_USB, 0x000F000F }
};

static CPG_REG_SETTING cpg_sel_pll3_2_on_off[] = {
	{(uintptr_t)CPG_CLKON_CRU, 0x00030003 },
#if !RZG2UL
	{(uintptr_t)CPG_CLKON_MIPI_DSI, 0x00020002 },
	{(uintptr_t)CPG_CLKON_GPU, 0x00010001 },
#endif
	{(uintptr_t)CPG_CLKON_SPI_MULTI, 0x00030003 },
	{(uintptr_t)CPG_CLKON_AXI_MCPU_BUS, 0x02080208 },
};

static CPG_REG_SETTING cpg_sel_pll3_3_on_off[] = {
	{(uintptr_t)CPG_CLKON_SPI_MULTI, 0x00030003 },
	{(uintptr_t)CPG_CLKON_AXI_MCPU_BUS, 0x02080208 },
};

static CPG_REG_SETTING cpg_sel_pll5_1_on_off[] = {
#if !RZG2UL
	{(uintptr_t)CPG_CLKON_MIPI_DSI, 0x00010001 },
#endif
	{(uintptr_t)CPG_CLKON_CRU, 0x00100010 },
#if !RZG2UL
	{(uintptr_t)CPG_CLKON_MIPI_DSI, 0x00100010 },
#endif
	{(uintptr_t)CPG_CLKON_LCDC, 0x00020002 }
};

static CPG_REG_SETTING cpg_sel_pll5_3_on_off[] = {
#if !RZG2UL
	{(uintptr_t)CPG_CLKON_MIPI_DSI, 0x00100010 },
#endif
	{(uintptr_t)CPG_CLKON_LCDC, 0x00020002 }
};

static CPG_REG_SETTING cpg_sel_pll5_4_on_off[] = {
#if !RZG2UL
	{(uintptr_t)CPG_CLKON_MIPI_DSI, 0x00100010 },
#endif
	{(uintptr_t)CPG_CLKON_LCDC, 0x00020002 }
};

static CPG_REG_SETTING cpg_sel_pll6_1_on_off[] = {
#if !RZG2UL
	{(uintptr_t)CPG_CLKON_GPU, 0x00010001 }
#endif
};

static CPG_REG_SETTING cpg_sel_gpu1_1_on_off[] = {
#if !RZG2UL
	{(uintptr_t)CPG_CLKON_GPU, 0x00010001 }
#endif
};

static CPG_REG_SETTING cpg_sel_gpu1_2_on_off[] = {
#if !RZG2UL
	{(uintptr_t)CPG_CLKON_GPU, 0x00010001 }
#endif
};

static CPG_REG_SETTING cpg_sel_gpu2_on_off[] = {
#if !RZG2UL
	{(uintptr_t)CPG_CLKON_GPU, 0x00010001 }
#endif
};

static CPG_SETUP_DATA cpg_clk_sr2_tbl[] = {
	{	/* I2C8 */
		{ (uintptr_t)CPG_V2H_CLKON_9, 0x00000008 },
		{ (uintptr_t)CPG_V2H_CLKMON_4, 0x00080000 },
		CPG_T_CLK
	},

	{	/* I2C8 */
		{ (uintptr_t)CPG_V2H_RST_10, 0x00000001 },
		{ (uintptr_t)CPG_V2H_RSTMON_4, 0x00020000 },
		CPG_T_RST
	}
};

static void cpg_ctrl_clkrst(CPG_SETUP_DATA const *array, uint32_t num)
{
	int i;
	uint32_t mask = 0;
	uint32_t cmp = 0;
	uint32_t val = 0;

	for (i = 0; i < num; i++, array++) {
		if (array->reg.addr == (uintptr_t)NULL) {
			continue;
		}

		if (g_cpg_fconf_cfg->cpg_type == CPG_TYPE_1) {
			val = array->reg.val;
		} else if (g_cpg_fconf_cfg->cpg_type == CPG_TYPE_2) {
			/*
			* Upper 16bits are enables for lower 16bits so write the upper 16bits with same value as lower value
			*/
			val = (array->reg.val & 0xFFFF) | ((array->reg.val & 0xFFFF) << 16);
		}

		CPG_REG_WRITE(array->reg.addr, val);

		/*
		 * This generic function needs to handle case where Montitor for clock
		 * is looking for a HIGH as clock active whereas Montitoring a reset
		 * it is looking for a LOW to indicate reset release.
		 */
		if (g_cpg_fconf_cfg->cpg_type == CPG_TYPE_1) {
			mask = (array->reg.val >> 16) & 0xFFFF;
			cmp = array->reg.val & 0xFFFF;
		} else if (g_cpg_fconf_cfg->cpg_type == CPG_TYPE_2) {
			mask = array->mon.val;
			cmp  = mask;
		}
		if (array->type == CPG_T_RST)
			cmp = ~cmp;

		while ((CPG_REG_READ(array->mon.addr) & mask) != (cmp & mask))
			;
	}
}

static void cpg_selector_on_off(uint32_t sel, uint8_t flag)
{
	uint32_t cnt;
	uint32_t tbl_num;
	CPG_REG_SETTING *ptr;

	switch (sel) {
	case CPG_SEL_PLL1_ON_OFF:
		tbl_num = ARRAY_SIZE(cpg_sel_pll1_on_off);
		ptr = &cpg_sel_pll1_on_off[0];
		break;
	case CPG_SEL_PLL2_1_ON_OFF:
		tbl_num = ARRAY_SIZE(cpg_sel_pll2_1_on_off);
		ptr = &cpg_sel_pll2_1_on_off[0];
		break;
	case CPG_SEL_PLL2_2_ON_OFF:
		tbl_num = ARRAY_SIZE(cpg_sel_pll2_2_on_off);
		ptr = &cpg_sel_pll2_2_on_off[0];
		break;
	case CPG_SEL_PLL3_1_ON_OFF:
		tbl_num = ARRAY_SIZE(cpg_sel_pll3_1_on_off);
		ptr = &cpg_sel_pll3_1_on_off[0];
		break;
	case CPG_SEL_PLL3_2_ON_OFF:
		tbl_num = ARRAY_SIZE(cpg_sel_pll3_2_on_off);
		ptr = &cpg_sel_pll3_2_on_off[0];
		break;
	case CPG_SEL_PLL3_3_ON_OFF:
		tbl_num = ARRAY_SIZE(cpg_sel_pll3_3_on_off);
		ptr = &cpg_sel_pll3_3_on_off[0];
		break;
	case CPG_SEL_PLL5_1_ON_OFF:
		tbl_num = ARRAY_SIZE(cpg_sel_pll5_1_on_off);
		ptr = &cpg_sel_pll5_1_on_off[0];
		break;
	case CPG_SEL_PLL5_3_ON_OFF:
		tbl_num = ARRAY_SIZE(cpg_sel_pll5_3_on_off);
		ptr = &cpg_sel_pll5_3_on_off[0];
		break;
	case CPG_SEL_PLL5_4_ON_OFF:
		tbl_num = ARRAY_SIZE(cpg_sel_pll5_4_on_off);
		ptr = &cpg_sel_pll5_4_on_off[0];
		break;
	case CPG_SEL_PLL6_1_ON_OFF:
		tbl_num = ARRAY_SIZE(cpg_sel_pll6_1_on_off);
		ptr = &cpg_sel_pll6_1_on_off[0];
		break;
	case CPG_SEL_GPU1_1_ON_OFF:
		tbl_num = ARRAY_SIZE(cpg_sel_gpu1_1_on_off);
		ptr = &cpg_sel_gpu1_1_on_off[0];
		break;
	case CPG_SEL_GPU1_2_ON_OFF:
		tbl_num = ARRAY_SIZE(cpg_sel_gpu1_2_on_off);
		ptr = &cpg_sel_gpu1_2_on_off[0];
		break;
	case CPG_SEL_GPU2_ON_OFF:
		tbl_num = ARRAY_SIZE(cpg_sel_gpu2_on_off);
		ptr = &cpg_sel_gpu2_on_off[0];
		break;
	default:
		break;
	}

	for (cnt = 0; cnt < tbl_num; cnt++) {
		if (flag == CPG_ON) {
			CPG_REG_WRITE(ptr[cnt].addr, (CPG_REG_READ(ptr[cnt].addr) | ptr[cnt].val));
		} else {
			CPG_REG_WRITE(ptr[cnt].addr, (CPG_REG_READ(ptr[cnt].addr) | (ptr[cnt].val & 0xFFFF0000)));
		}
	}

}

static void cpg_pll_start(CPG_PLL_SETTINGS *pdata, uint32_t size)
{
	int cnt;
	uint32_t val;

	for (cnt = 0; cnt < size; cnt++, pdata++) {
		if (pdata->stby.addr != (uintptr_t)NULL) {
			CPG_REG_WRITE(pdata->stby.addr, pdata->stby.val);
		}
		if (pdata->clk1.addr != (uintptr_t)NULL) {
			CPG_REG_WRITE(pdata->clk1.addr, pdata->clk1.val);
		}
		if (pdata->clk2.addr != (uintptr_t)NULL) {
			CPG_REG_WRITE(pdata->clk2.addr, pdata->clk2.val);
		}

		do {
			val = CPG_REG_READ(pdata->mon.addr);
		} while (((val & pdata->mon.val) != pdata->mon.val) && (pdata->mon.addr != (uintptr_t)NULL));
	}
}

/* It is assumed that the PLL has stopped by the time this function is executed. */
static void cpg_pll_setup(void)
{
	static CPG_PLL_SETTINGS cpg_pll_tbl[3];
	int i, j = 0;
	const void *fdt = (const void *)(uintptr_t)dtb_base;
	int len;

	const fdt32_t *	prop = fdt_getprop(fdt, g_cpg_fconf_cfg->cpg_node, "cpg_pll_tbl", &len);
	for (i = 0; i < (len / sizeof(uint32_t)); i += 8, j++) {
		cpg_pll_tbl[j].stby.addr	= fdt32_to_cpu(prop[i]);
		cpg_pll_tbl[j].stby.val		= fdt32_to_cpu(prop[i + 1]);

		cpg_pll_tbl[j].clk1.addr	= fdt32_to_cpu(prop[i + 2]);
		cpg_pll_tbl[j].clk1.val		= fdt32_to_cpu(prop[i + 3]);

		cpg_pll_tbl[j].clk2.addr	= fdt32_to_cpu(prop[i + 4]);
		cpg_pll_tbl[j].clk2.val		= fdt32_to_cpu(prop[i + 5]);

		cpg_pll_tbl[j].mon.addr		= fdt32_to_cpu(prop[i + 6]);
		cpg_pll_tbl[j].mon.val		= fdt32_to_cpu(prop[i + 7]);
	}

	cpg_pll_start(&cpg_pll_tbl[0], ARRAY_SIZE(cpg_pll_tbl));
}

static void cpg_div_sel_setup(CPG_REG_SETTING *tbl, uint32_t size)
{
	int cnt;

	for (cnt = 0; cnt < size; cnt++, tbl++) {
		CPG_REG_WRITE(tbl->addr, tbl->val);
	}

#if !DEBUG_FPGA
	/* Wait for completion of settings */
	while (CPG_REG_READ(CPG_CLKSTATUS) != 0)
		;
#endif
}

static void cpg_div_sel_static_setup(void)
{
	static CPG_REG_SETTING cpg_static_select_tbl[2];
	int i, j = 0;
	const void *fdt = (const void *)(uintptr_t)dtb_base;
	int len;

	const fdt32_t *	prop = fdt_getprop(fdt, g_cpg_fconf_cfg->cpg_node, "cpg_static_select_tbl", &len);
	for (i = 0; i < (len / sizeof(uint32_t)); i += 2, j++) {
		cpg_static_select_tbl[j].addr	= fdt32_to_cpu(prop[i]);
		cpg_static_select_tbl[j].val	= fdt32_to_cpu(prop[i + 1]);
	}

	cpg_div_sel_setup(&cpg_static_select_tbl[0], ARRAY_SIZE(cpg_static_select_tbl));
}

static void cpg_div_sel_dynamic_setup(void)
{
	static CPG_REG_SETTING cpg_dynamic_select_tbl[5];
	int i, j = 0;
	const void *fdt = (const void *)(uintptr_t)dtb_base;
	int len;

	const fdt32_t *	prop = fdt_getprop(fdt, g_cpg_fconf_cfg->cpg_node, "cpg_dynamic_select_tbl", &len);
	for (i = 0; i < (len / sizeof(uint32_t)); i += 2, j++) {
		cpg_dynamic_select_tbl[j].addr	= fdt32_to_cpu(prop[i]);
		cpg_dynamic_select_tbl[j].val	= fdt32_to_cpu(prop[i + 1]);
	}

	cpg_div_sel_setup(&cpg_dynamic_select_tbl[0], ARRAY_SIZE(cpg_dynamic_select_tbl));
}

void cpg_prepare_suspend(void)
{
	cpg_ctrl_clkrst(&cpg_clk_sr2_tbl[0], ARRAY_SIZE(cpg_clk_sr2_tbl));
}

static void cpg_clk_on_setup(void)
{
	static CPG_SETUP_DATA cpg_clk_on_tbl[50];
	int i, j = 0;
	const void *fdt = (const void *)(uintptr_t)dtb_base;
	int len;

	const fdt32_t *	prop = fdt_getprop(fdt, g_cpg_fconf_cfg->cpg_node, "cpg_clk_on_tbl", &len);
	for (i = 0; i < (len / sizeof(uint32_t)); i += 4, j++) {
		cpg_clk_on_tbl[j].reg.addr 	= fdt32_to_cpu(prop[i]);
		cpg_clk_on_tbl[j].reg.val	= fdt32_to_cpu(prop[i + 1]);
		cpg_clk_on_tbl[j].mon.addr 	= fdt32_to_cpu(prop[i + 2]);
		cpg_clk_on_tbl[j].type 		= fdt32_to_cpu(prop[i + 3]);
	}

	cpg_ctrl_clkrst(&cpg_clk_on_tbl[0], ARRAY_SIZE(cpg_clk_on_tbl));
}

static void cpg_reset_setup(void)
{
	static CPG_SETUP_DATA cpg_reset_tbl[51];
	int i, j = 0;
	const void *fdt = (const void *)(uintptr_t)dtb_base;
	int len;

	const fdt32_t *prop = fdt_getprop(fdt, g_cpg_fconf_cfg->cpg_node, "cpg_reset_tbl", &len);
	for (i = 0; i < (len / sizeof(uint32_t)); i += 4, j++) {
		cpg_reset_tbl[j].reg.addr 	= fdt32_to_cpu(prop[i]);
		cpg_reset_tbl[j].reg.val	= fdt32_to_cpu(prop[i + 1]);
		cpg_reset_tbl[j].mon.addr 	= fdt32_to_cpu(prop[i + 2]);
		cpg_reset_tbl[j].type 		= fdt32_to_cpu(prop[i + 3]);
	}

	cpg_ctrl_clkrst(&cpg_reset_tbl[0], ARRAY_SIZE(cpg_reset_tbl));
}

void cpg_active_ddr(void (*disable_phy)(void))
{
	/* Assert the reset of DDRTOP */
	CPG_REG_WRITE(CPG_RST_DDR, 0x005F0000 | (g_cpg_fconf_cfg->cpg_rst_ddr_opt << 16));
	CPG_REG_WRITE(CPG_OTHERFUNC2_REG, 0x00010000);
	while ((CPG_REG_READ(CPG_RSTMON_DDR) & 0x0000005F) != 0x0000005F)
		;

	/* Start the clocks of DDRTOP */
	CPG_REG_WRITE(CPG_CLKON_DDR, 0x00030003);
	while ((CPG_REG_READ(CPG_CLKMON_DDR) & 0x00000003) != 0x00000003)
		;

	udelay(1);

	/* De-assert rst_n */
	CPG_REG_WRITE(CPG_OTHERFUNC2_REG, 0x00010001);

	udelay(1);

	/* De-assert PRESETN */
	CPG_REG_WRITE(CPG_RST_DDR, 0x00020002);
	while ((CPG_REG_READ(CPG_RSTMON_DDR) & 0x00000002) != 0x00000000)
		;

	udelay(1);

	disable_phy();

	/* De-assert axiY_ARESETn, regARESETn, reset_n */
	CPG_REG_WRITE(CPG_RST_DDR, 0x005D005D | (g_cpg_fconf_cfg->cpg_rst_ddr_opt << 16) | g_cpg_fconf_cfg->cpg_rst_ddr_opt);
	while ((CPG_REG_READ(CPG_RSTMON_DDR) & 0x0000005D) != 0x00000000)
		;

	udelay(1);
}

void cpg_reset_ddr_mc(void)
{
	/* Assert rst_n, axiY_ARESETn, regARESETn */
	CPG_REG_WRITE(CPG_RST_DDR, 0x005C0000 | (g_cpg_fconf_cfg->cpg_rst_ddr_opt << 16));
	CPG_REG_WRITE(CPG_OTHERFUNC2_REG, 0x00010000);
	while ((CPG_REG_READ(CPG_RSTMON_DDR) & 0x0000005C) != 0x0000005C)
		;

	udelay(1);

	/* De-assert rst_n */
	CPG_REG_WRITE(CPG_OTHERFUNC2_REG, 0x00010001);

	udelay(1);

	/* De-assert axiY_ARESETn, regARESETn */
	CPG_REG_WRITE(CPG_RST_DDR, 0x005C005C | (g_cpg_fconf_cfg->cpg_rst_ddr_opt << 16) | g_cpg_fconf_cfg->cpg_rst_ddr_opt);
	while ((CPG_REG_READ(CPG_RSTMON_DDR) & 0x0000005C) != 0x00000000)
		;

	udelay(1);
}

static void cpg_mstop_setup(void)
{
	/* Remove all MSTOPS apart from reserved and those already removed at TF-A entry */
	REMOVE_MSTOPS_W(CPG_V2H_BUS_1_MSTOP,      CPG_BUS_1_MSTOP_WDT1
										| CPG_BUS_1_MSTOP_RIIC0
										| CPG_BUS_1_MSTOP_RIIC1
										| CPG_BUS_1_MSTOP_RIIC2
										| CPG_BUS_1_MSTOP_RIIC3
										| CPG_BUS_1_MSTOP_RIIC4
										| CPG_BUS_1_MSTOP_RIIC5
										| CPG_BUS_1_MSTOP_RIIC6
										| CPG_BUS_1_MSTOP_RIIC7
										| CPG_BUS_1_MSTOP_SPDIF0
										| CPG_BUS_1_MSTOP_SPDIF1
										| CPG_BUS_1_MSTOP_SPDIF2
										| CPG_BUS_1_MSTOP_TZC400_PCIE0
										| CPG_BUS_1_MSTOP_TZC400_PCIE1);

	REMOVE_MSTOPS_W(CPG_V2H_BUS_2_MSTOP,      CPG_BUS_2_MSTOP_SCU
										| CPG_BUS_2_MSTOP_SCU_DMAC
										| CPG_BUS_2_MSTOP_ADG
										| CPG_BUS_2_MSTOP_SSIU
										| CPG_BUS_2_MSTOP_SSIU_DMAC
										| CPG_BUS_2_MSTOP_ADMAC
										| CPG_BUS_2_MSTOP_GTM2
										| CPG_BUS_2_MSTOP_GTM3
										| CPG_BUS_2_MSTOP_TSU1);

	REMOVE_MSTOPS_W(CPG_V2H_BUS_3_MSTOP,      CPG_BUS_3_MSTOP_DMAC1
										| CPG_BUS_3_MSTOP_DMAC2
										| CPG_BUS_3_MSTOP_GE3D
										| CPG_BUS_3_MSTOP_ADC
										| CPG_BUS_3_MSTOP_WDT0
										| CPG_BUS_3_MSTOP_RTC_P0
										| CPG_BUS_3_MSTOP_RTC_P1
										| CPG_BUS_3_MSTOP_RIIC8
										| CPG_BUS_3_MSTOP_SCIF
										| CPG_BUS_3_MSTOP_CMTW0);

	REMOVE_MSTOPS_W(CPG_V2H_BUS_4_MSTOP,      CPG_BUS_4_MSTOP_CMTW1
										| CPG_BUS_4_MSTOP_CMTW2
										| CPG_BUS_4_MSTOP_CMTW3
										| CPG_BUS_4_MSTOP_XSPI
										| CPG_BUS_4_MSTOP_SECURE_IP_P0
										| CPG_BUS_4_MSTOP_SECURE_IP_P1
										| CPG_BUS_4_MSTOP_MHU);

	REMOVE_MSTOPS_W(CPG_V2H_BUS_5_MSTOP,      CPG_BUS_5_MSTOP_TSU0
										| CPG_BUS_5_MSTOP_XSPI_REG
										| CPG_BUS_5_MSTOP_PDM0
										| CPG_BUS_5_MSTOP_PDM1
										| CPG_BUS_5_MSTOP_DMAC0
										| CPG_BUS_5_MSTOP_GTM0
										| CPG_BUS_5_MSTOP_GTM1
										| CPG_BUS_5_MSTOP_WDT2
										| CPG_BUS_5_MSTOP_WDT3
										| CPG_BUS_5_MSTOP_CRC
										| CPG_BUS_5_MSTOP_CMTW4);

	REMOVE_MSTOPS_W(CPG_V2H_BUS_6_MSTOP,      CPG_BUS_6_MSTOP_CMTW5
										| CPG_BUS_6_MSTOP_CMTW6
										| CPG_BUS_6_MSTOP_CMTW7
										| CPG_BUS_6_MSTOP_POEG0A
										| CPG_BUS_6_MSTOP_POEG0B
										| CPG_BUS_6_MSTOP_POEG0C
										| CPG_BUS_6_MSTOP_POEG0D
										| CPG_BUS_6_MSTOP_POEG1A
										| CPG_BUS_6_MSTOP_POEG1B
										| CPG_BUS_6_MSTOP_POEG1C
										| CPG_BUS_6_MSTOP_POEG1D
										| CPG_BUS_6_MSTOP_GPT0
										| CPG_BUS_6_MSTOP_GPT1
										| CPG_BUS_6_MSTOP_DDR0_P0
										| CPG_BUS_6_MSTOP_DDR0_P1
										| CPG_BUS_6_MSTOP_DDR0_P2);

	REMOVE_MSTOPS_W(CPG_V2H_BUS_7_MSTOP,      CPG_BUS_7_MSTOP_DDR_0_P3
										| CPG_BUS_7_MSTOP_DDR_0_P4
										| CPG_BUS_7_MSTOP_DDR_1_P0
										| CPG_BUS_7_MSTOP_DDR_1_P1
										| CPG_BUS_7_MSTOP_DDR_1_P2
										| CPG_BUS_7_MSTOP_DDR_1_P3
										| CPG_BUS_7_MSTOP_DDR_1_P4
										| CPG_BUS_7_MSTOP_USB20_HOST
										| CPG_BUS_7_MSTOP_USB21_HOST
										| CPG_BUS_7_MSTOP_USB2_FUNC
										| CPG_BUS_7_MSTOP_USB20_PHY
										| CPG_BUS_7_MSTOP_USB21_PHY
										| CPG_BUS_7_MSTOP_USB30_HOST
										| CPG_BUS_7_MSTOP_USB31_HOST
										| CPG_BUS_7_MSTOP_USB30_PHY
										| CPG_BUS_7_MSTOP_USB31_PHY);

	REMOVE_MSTOPS_W(CPG_V2H_BUS_8_MSTOP,      CPG_BUS_8_MSTOP_PCIE_PHY
										| CPG_BUS_8_MSTOP_SD0
										| CPG_BUS_8_MSTOP_SD1
										| CPG_BUS_8_MSTOP_SD2
										| CPG_BUS_8_MSTOP_GBETH0
										| CPG_BUS_8_MSTOP_GBETH1
										| CPG_BUS_8_MSTOP_DRP_AI_MAC
										| CPG_BUS_8_MSTOP_DRP_AP_DRP0
										| CPG_BUS_8_MSTOP_DRP1);

	REMOVE_MSTOPS_W(CPG_V2H_BUS_9_MSTOP,      CPG_BUS_9_MSTOP_CRU0
										| CPG_BUS_9_MSTOP_CRU1
										| CPG_BUS_9_MSTOP_CRU2
										| CPG_BUS_9_MSTOP_CRU3
										| CPG_BUS_9_MSTOP_ISP_APB
										| CPG_BUS_9_MSTOP_ISP_AXI
										| CPG_BUS_9_MSTOP_VCD_P0
										| CPG_BUS_9_MSTOP_VCD_P1
										| CPG_BUS_9_MSTOP_VCD_P2
										| CPG_BUS_9_MSTOP_DSI_LINK
										| CPG_BUS_9_MSTOP_DSI_PHY);

	REMOVE_MSTOPS_W(CPG_V2H_BUS_10_MSTOP,     CPG_BUS_10_MSTOP_ISU
										| CPG_BUS_10_MSTOP_LCDC_DU
										| CPG_BUS_10_MSTOP_LCDC_FCPVD
										| CPG_BUS_10_MSTOP_LCDC_VSPD
										| CPG_BUS_10_MSTOP_DDR0_PHY
										| CPG_BUS_10_MSTOP_DDR1_PHY
										| CPG_BUS_10_MSTOP_DDR0_CTRL
										| CPG_BUS_10_MSTOP_DDR1_CTRL
										| CPG_BUS_10_MSTOP_CR8_TCM
										| CPG_BUS_10_MSTOP_DMAC3
										| CPG_BUS_10_MSTOP_DMAC4
										| CPG_BUS_10_MSTOP_CANFD
										| CPG_BUS_10_MSTOP_I3C0);

	REMOVE_MSTOPS_W(CPG_V2H_BUS_11_MSTOP,     CPG_BUS_11_MSTOP_RSPI0
										| CPG_BUS_11_MSTOP_RSPI1
										| CPG_BUS_11_MSTOP_RSPI2
										| CPG_BUS_11_MSTOP_RSCI0
										| CPG_BUS_11_MSTOP_RSCI1
										| CPG_BUS_11_MSTOP_RSCI2
										| CPG_BUS_11_MSTOP_RSCI3
										| CPG_BUS_11_MSTOP_RSCI4
										| CPG_BUS_11_MSTOP_RSCI5
										| CPG_BUS_11_MSTOP_RSCI6
										| CPG_BUS_11_MSTOP_RSCI7
										| CPG_BUS_11_MSTOP_RSCI8
										| CPG_BUS_11_MSTOP_RSCI9
										| CPG_BUS_11_MSTOP_GTM4
										| CPG_BUS_11_MSTOP_GTM5
										| CPG_BUS_11_MSTOP_GTM6);

	REMOVE_MSTOPS_W(CPG_V2H_BUS_12_MSTOP,     CPG_BUS_12_MSTOP_GTM7
										| CPG_BUS_12_MSTOP_MCPU_TO_ACPU);
}

static void cpu_cpg_setup(void)
{

	while ((CPG_REG_READ(CPG_CLKSTATUS) & CLKSTATUS_DIVPL1_STS) != 0x00000000)
		;
	CPG_REG_WRITE(CPG_PL1_DDIV, PL1_DDIV_DIVPL1_SET_WEN | PL1_DDIV_DIVPL1_SET_1_1);
	while ((CPG_REG_READ(CPG_CLKSTATUS) & CLKSTATUS_DIVPL1_STS) != 0x00000000)
		;
}

void cpg_early_setup(void)
{
	/* Initialize global CPG config from DTB.  */
	g_cpg_fconf_cfg = cpg_config_getter();

	if (g_cpg_fconf_cfg->cpg_early_setup) {
		cpu_cpg_setup();
		cpg_ctrl_clkrst(early_setup_tbl, ARRAY_SIZE(early_setup_tbl));
	}
}

void cpg_wdtrst_sel_setup(void)
{
	if (g_cpg_fconf_cfg->cpg_type == CPG_TYPE_1)
	{
		uint32_t reg;
		reg = CPG_REG_READ(CPG_WDTRST_SEL);
		reg |=
			WDTRST_SEL_WDTRSTSEL0 | WDTRST_SEL_WDTRSTSEL0_WEN |
			WDTRST_SEL_WDTRSTSEL1 | WDTRST_SEL_WDTRSTSEL1_WEN |
			WDTRST_SEL_WDTRSTSEL2 | WDTRST_SEL_WDTRSTSEL2_WEN;
		CPG_REG_WRITE(CPG_WDTRST_SEL, reg);
	}
	else if (g_cpg_fconf_cfg->cpg_type == CPG_TYPE_2)
	{
		// Get ICU base address
		uint32_t v2h_icu_base = 0;
		void *fdt = (void *)(uintptr_t)dtb_base;

		read_prop_from_subnode(fdt, "/soc", "interrupt-unit@10400000", "reg", 1, &v2h_icu_base);

		uint32_t val	= CPG_ERRORRST_SELx_ERRRSTSEL0
						| CPG_ERRORRST_SELx_ERRRSTSEL1
						| CPG_ERRORRST_SELx_ERRRSTSEL2
						| CPG_ERRORRST_SELx_ERRRSTSEL3;
		uint32_t ca33_w01, ca33_w23, ca55_w01, ca55_w23;

		/* Clear bit 28 interrupt source for both M33 and CA55 */
		mmio_write_32(((uintptr_t)(v2h_icu_base) + 0x0348 + ((0) * 0x004)), 0x10000000);
		mmio_write_32((uintptr_t)(v2h_icu_base) + 0x0348 + ((0) * 0x004), 0x10000000);

		ca33_w01 = mmio_read_32((uintptr_t)(v2h_icu_base) + 0x0304 + ((0) * 0x004));
		ca33_w23 = mmio_read_32((uintptr_t)(v2h_icu_base) + 0x0304 + ((1) * 0x004));
		ca55_w01 = mmio_read_32((uintptr_t)(v2h_icu_base) + 0x0338 + ((0) * 0x004));
		ca55_w23 = mmio_read_32((uintptr_t)(v2h_icu_base) + 0x0338 + ((0) * 0x004));

		/* Checking ICU interrupt WDT CM33 */
		if ((ca33_w01 == 0x40000000) || (ca33_w01 == 0x80000000) ||
				(ca33_w23 == 0x00000001) || (ca33_w23 == 0x00000002)) {
			/* ERINTM33CLR0 bit for clear 28-31 */
			mmio_write_32((uintptr_t)(v2h_icu_base + 0x0314 + ((0) * 0x004)), 0xF0000000);
		}

		/* Checking ICU interrupt WDT CA55 */
		if ((ca55_w01 == 0x40000000) || (ca55_w01 == 0x80000000) ||
				(ca55_w23 == 0x00000001) || (ca55_w23 == 0x00000002)) {
			/* ERINTA55CLR0 bit for clear 28-31 */
			mmio_write_32((uintptr_t)(v2h_icu_base) + 0x0348 + ((0) * 0x004), 0xF0000000);
		}

		/* Add in the WEN bits for the selected bits */
		val = (val & 0xFFFF) | ((val & 0xFFFF) << 16);

		CPG_REG_WRITE(CPG_V2H_ERRORRST_SEL2, val);
	}
}

void cpg_ddr0_part1(void)
{
	CPG_REG_WRITE(CPG_V2H_RST_11, 0x0FF80000);

	CPG_REG_WRITE(CPG_V2H_LP_DDR_CTL1, CPG_REG_READ(CPG_V2H_LP_DDR_CTL1) & ~0x00000001);

	CPG_REG_WRITE(CPG_V2H_PLLDDR0_STBY, 0x00010001);	/* PLLDDR0 clock start */
	while ((CPG_REG_READ(CPG_V2H_PLLDDR0_MON) & 0x00000011) != 0x00000011)
		;

	CPG_REG_WRITE(CPG_V2H_CLKON_12, 0x0FC00FC0);

	udelay(1);

	CPG_REG_WRITE(CPG_V2H_RST_11, 0x00080008);
	CPG_REG_WRITE(CPG_V2H_LP_DDR_CTL1, CPG_REG_READ(CPG_V2H_LP_DDR_CTL1) | 0x00000001);

	udelay(1);

	CPG_REG_WRITE(CPG_V2H_RST_11, 0x03F003F0);

	udelay(1);
}

void cpg_ddr0_part2(void)
{
	CPG_REG_WRITE(CPG_V2H_RST_11, 0x08000800);

	udelay(10);

	CPG_REG_WRITE(CPG_V2H_RST_11, 0x04000400);

	udelay(10);
}

void cpg_ddr1_part1(void)
{
	CPG_REG_WRITE(CPG_V2H_RST_11, 0xF0000000);
	CPG_REG_WRITE(CPG_V2H_RST_12, 0x001F0000);

	CPG_REG_WRITE(CPG_V2H_LP_DDR_CTL1, CPG_REG_READ(CPG_V2H_LP_DDR_CTL1) & ~0x00000002);

	CPG_REG_WRITE(CPG_V2H_PLLDDR1_STBY, 0x00010001);
	while ((CPG_REG_READ(CPG_V2H_PLLDDR1_MON) & 0x00000011) != 0x00000011)
		;

	CPG_REG_WRITE(CPG_V2H_CLKON_12, 0xF000F000);
	CPG_REG_WRITE(CPG_V2H_CLKON_13, 0x00030003);

	udelay(1);

	CPG_REG_WRITE(CPG_V2H_RST_11, 0x10001000);
	CPG_REG_WRITE(CPG_V2H_LP_DDR_CTL1, CPG_REG_READ(CPG_V2H_LP_DDR_CTL1) | 0x00000002);

	udelay(1);

	CPG_REG_WRITE(CPG_V2H_RST_11, 0xE000E000);
	CPG_REG_WRITE(CPG_V2H_RST_12, 0x00070007);

	udelay(1);
}

void cpg_ddr1_part2(void)
{
	CPG_REG_WRITE(CPG_V2H_RST_12, 0x00100010);

	udelay(10);

	CPG_REG_WRITE(CPG_V2H_RST_12, 0x00080008);

	udelay(10);
}

void cpg_ddr_pwrokin_off(uint8_t ddr)
{
	if (!ddr)
		CPG_REG_WRITE(CPG_V2H_LP_DDR_CTL1, CPG_REG_READ(CPG_V2H_LP_DDR_CTL1) & ~0x00000001);	/* DDR0 */
	else
		CPG_REG_WRITE(CPG_V2H_LP_DDR_CTL1, CPG_REG_READ(CPG_V2H_LP_DDR_CTL1) & ~0x00000002);	/* DDR1 */
}

void cpg_setup(void)
{
	if (g_cpg_fconf_cfg->cpg_selector_on_off) { cpg_selector_on_off(CPG_SEL_PLL3_3_ON_OFF, CPG_OFF); }
	cpg_div_sel_static_setup();
	if (g_cpg_fconf_cfg->cpg_selector_on_off) { cpg_selector_on_off(CPG_SEL_PLL3_3_ON_OFF, CPG_ON); }
	cpg_pll_setup();
	cpg_clk_on_setup();
	cpg_reset_setup();
	if (g_cpg_fconf_cfg->cpg_mstop_setup) { cpg_mstop_setup(); }
	cpg_div_sel_dynamic_setup();
	cpg_wdtrst_sel_setup();
}
