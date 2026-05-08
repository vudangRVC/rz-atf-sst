/*
 * Copyright (c) 2020, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <stddef.h>
#include <pfc_regs_offset.h>
#include <sys_regs_offset.h>
#include <sys.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>
#include <lib/fconf/fconf.h>
#include <lib/libfdt/libfdt.h>
#include <rz_fconf.h>
#include <common/debug.h>

const struct pfc_config_t * g_pfc_fconf_cfg;

extern u_register_t dtb_base;

#define PFC_REG_ADDR(offset)  			((uintptr_t)(g_pfc_fconf_cfg->pfc_base + (offset)))
#define PFC_REG_WRITE_8(reg, value)		mmio_write_8(PFC_REG_ADDR(reg), value)
#define PFC_REG_WRITE_32(reg, value)	mmio_write_32(PFC_REG_ADDR(reg), value)
#define PFC_REG_WRITE_64(reg, value)	mmio_write_64(PFC_REG_ADDR(reg), value)
#define PFC_REG_READ(reg)				mmio_read_32(PFC_REG_ADDR(reg))

static PFC_REGS  pfc_qspi_reg_tbl[PFC_QSPI_TBL_NUM];

static PFC_REGS  pfc_sd_reg_tbl[PFC_SD_TBL_NUM];

#if PLAT_SYSTEM_SUSPEND
/* I2C8 */
static PFC_REGS pfc_i2c_bus8_reg_tbl[] = {
	/* I2C8_SDA (P20.6), I2C8_SCL (P20.7) */
	{
		{ PFC_ON,  (uintptr_t)PFC_PMC20,  0xC0 },					/* PMC */
		{ PFC_ON,  (uintptr_t)PFC_PFC20,  0x11000000 },				/* PFC */
		{ PFC_OFF, (uintptr_t)NULL,       0 },						/* IOLH */
		{ PFC_OFF, (uintptr_t)NULL,       0 },						/* PUPD */
		{ PFC_OFF, (uintptr_t)NULL,       0 },						/* SR */
		{ PFC_OFF, (uintptr_t)NULL,       0 }						/* IEN */
	},

	/* Padding to make same length as other pin tables */
	{
		{0}
	},
};
#endif /* PLAT_SYSTEM_SUSPEND */

static PFC_IO_DRIVE pfc_io_drive[SYS_BOOT_MODE_MAX] = {
	{SYS_V2H_LSI_OTPPOC_EN_SD_DS_MASK,		SYS_V2H_LSI_OTPPOC_SD_E_MASK,		SYS_V2H_LSI_OTPPOC_SD_E},
	{SYS_V2H_LSI_OTPPOC_EN_EMMC18_DS_MASK,	SYS_V2H_LSI_OTPPOC_EMMC18_E_MASK,	SYS_V2H_LSI_OTPPOC_EMMC18_E},
	{SYS_V2H_LSI_OTPPOC_EN_EMMC33_DS_MASK,	SYS_V2H_LSI_OTPPOC_EMMC33_E_MASK,	SYS_V2H_LSI_OTPPOC_EMMC33_E},
	{SYS_V2H_LSI_OTPPOC_EN_SPI18_DS_MASK,	SYS_V2H_LSI_OTPPOC_SPI18_E_MASK,	SYS_V2H_LSI_OTPPOC_SPI18_E},
	{SYS_V2H_LSI_OTPPOC_EN_SPI33_DS_MASK,	SYS_V2H_LSI_OTPPOC_SPI33_E_MASK,	SYS_V2H_LSI_OTPPOC_SPI33_E},
};

static const PFC_REGS * pfc_boot_mode_tbls[SYS_BOOT_MODE_MAX] = {
	pfc_sd_reg_tbl,
	pfc_sd_reg_tbl,
	pfc_sd_reg_tbl,
	pfc_qspi_reg_tbl,
	pfc_qspi_reg_tbl,
};

static void pfc_mux_setup(void)
{
	int      cnt;
	int i, j = 0;
	static PFC_REGS pfc_mux_reg_tbl[4];
	const void *fdt = (const void *)(uintptr_t)dtb_base;
	int len;

	const fdt32_t *prop = fdt_getprop(fdt, g_pfc_fconf_cfg->pfc_node, "pfc_mux_reg_tbl", &len);

	
	for (i = 0; i < (len / sizeof(uint32_t)); i += 15, j++) {
		/* PMC */
		pfc_mux_reg_tbl[j].pmc.flg = fdt32_to_cpu(prop[i]);
		pfc_mux_reg_tbl[j].pmc.reg = fdt32_to_cpu(prop[i + 1]);
		pfc_mux_reg_tbl[j].pmc.val = fdt32_to_cpu(prop[i + 2]);

		/* PFC */
		pfc_mux_reg_tbl[j].pfc.flg = fdt32_to_cpu(prop[i + 3]);
		pfc_mux_reg_tbl[j].pfc.reg = fdt32_to_cpu(prop[i + 4]);
		pfc_mux_reg_tbl[j].pfc.val = fdt32_to_cpu(prop[i + 5]);

		/* IOLH */
		pfc_mux_reg_tbl[j].iolh.flg = fdt32_to_cpu(prop[i + 6]);
		pfc_mux_reg_tbl[j].iolh.reg = fdt32_to_cpu(prop[i + 7]);
		pfc_mux_reg_tbl[j].iolh.val = fdt32_to_cpu(prop[i + 8]);

		/* PUPD */
		pfc_mux_reg_tbl[j].pupd.flg = fdt32_to_cpu(prop[i + 9]);
		pfc_mux_reg_tbl[j].pupd.reg = fdt32_to_cpu(prop[i + 10]);
		pfc_mux_reg_tbl[j].pupd.val = fdt32_to_cpu(prop[i + 11]);

		/* SR */
		pfc_mux_reg_tbl[j].sr.flg = fdt32_to_cpu(prop[i + 12]);
		pfc_mux_reg_tbl[j].sr.reg = fdt32_to_cpu(prop[i + 13]);
		pfc_mux_reg_tbl[j].sr.val = fdt32_to_cpu(prop[i + 14]);
	}

	/* multiplexer terminal switching */
	PFC_REG_WRITE_32(PFC_PWPR, 0x0);
	PFC_REG_WRITE_32(PFC_PWPR, PWPR_PFCWE);

	for (cnt = 0; cnt < PFC_MUX_TBL_NUM; cnt++) {
		/* PMC */
		if (pfc_mux_reg_tbl[cnt].pmc.flg == PFC_ON) {
			PFC_REG_WRITE_8(pfc_mux_reg_tbl[cnt].pmc.reg, pfc_mux_reg_tbl[cnt].pmc.val);
		}
		/* PFC */
		if (pfc_mux_reg_tbl[cnt].pfc.flg == PFC_ON) {
			PFC_REG_WRITE_32(pfc_mux_reg_tbl[cnt].pfc.reg, pfc_mux_reg_tbl[cnt].pfc.val);
		}
		/* IOLH */
		if (pfc_mux_reg_tbl[cnt].iolh.flg == PFC_ON) {
			PFC_REG_WRITE_64(pfc_mux_reg_tbl[cnt].iolh.reg, pfc_mux_reg_tbl[cnt].iolh.val);
		}
		/* PUPD */
		if (pfc_mux_reg_tbl[cnt].pupd.flg == PFC_ON) {
			PFC_REG_WRITE_64(pfc_mux_reg_tbl[cnt].pupd.reg, pfc_mux_reg_tbl[cnt].pupd.val);
		}
		/* SR */
		if (pfc_mux_reg_tbl[cnt].sr.flg == PFC_ON) {
			PFC_REG_WRITE_64(pfc_mux_reg_tbl[cnt].sr.reg, pfc_mux_reg_tbl[cnt].sr.val);
		}
	}

	PFC_REG_WRITE_32(PFC_PWPR, 0x0);
	PFC_REG_WRITE_32(PFC_PWPR, PWPR_B0Wl);
}

static void pfc_qspi_setup(void)
{
	int      cnt;
	int i, j = 0;
	const void *fdt = (const void *)(uintptr_t)dtb_base;
	int len;

	const fdt32_t *prop = fdt_getprop(fdt, g_pfc_fconf_cfg->pfc_node, "pfc_qspi_reg_tbl", &len);
	for (i = 0; i < (len / sizeof(uint32_t)); i += 9, j++) {
		/* IOLH */
		pfc_qspi_reg_tbl[j].iolh.flg = fdt32_to_cpu(prop[i]);
		pfc_qspi_reg_tbl[j].iolh.reg = fdt32_to_cpu(prop[i + 1]);
		pfc_qspi_reg_tbl[j].iolh.val = fdt32_to_cpu(prop[i + 2]);

		/* PUPD */
		pfc_qspi_reg_tbl[j].pupd.flg = fdt32_to_cpu(prop[i + 3]);
		pfc_qspi_reg_tbl[j].pupd.reg = fdt32_to_cpu(prop[i + 4]);
		pfc_qspi_reg_tbl[j].pupd.val = fdt32_to_cpu(prop[i + 5]);

		/* SR */
		pfc_qspi_reg_tbl[j].sr.flg = fdt32_to_cpu(prop[i + 6]);
		pfc_qspi_reg_tbl[j].sr.reg = fdt32_to_cpu(prop[i + 7]);
		pfc_qspi_reg_tbl[j].sr.val = fdt32_to_cpu(prop[i + 8]);
	}

	for (cnt = 0; cnt < ARRAY_SIZE(pfc_qspi_reg_tbl); cnt++) {
		/* IOLH */
		if (pfc_qspi_reg_tbl[cnt].iolh.flg == PFC_ON) {
			PFC_REG_WRITE_64(pfc_qspi_reg_tbl[cnt].iolh.reg, pfc_qspi_reg_tbl[cnt].iolh.val);
		}
		/* PUPD */
		if (pfc_qspi_reg_tbl[cnt].pupd.flg == PFC_ON) {
			PFC_REG_WRITE_64(pfc_qspi_reg_tbl[cnt].pupd.reg, pfc_qspi_reg_tbl[cnt].pupd.val);
		}
		/* SR */
		if (pfc_qspi_reg_tbl[cnt].sr.flg == PFC_ON) {
			PFC_REG_WRITE_64(pfc_qspi_reg_tbl[cnt].sr.reg, pfc_qspi_reg_tbl[cnt].sr.val);
		}
	}
}

static void pfc_sd_setup(void)
{
	int      cnt;
	int i, j = 0;
	uint64_t temp = 0;
	const void *fdt = (const void *)(uintptr_t)dtb_base;
	int len;

	/* Since SDx is 3.3V, the initial value will be set. */
	PFC_REG_WRITE_32(PFC_SD_ch0, 1);
	PFC_REG_WRITE_32(PFC_SD_ch1, 0);

	const fdt32_t *prop = fdt_getprop(fdt, g_pfc_fconf_cfg->pfc_node, "pfc_sd_reg_tbl", &len);
	for (i = 0; i < (len / sizeof(uint32_t)); i += 18, j++) {
		if (j == 1) {
			/* PMC */
			pfc_sd_reg_tbl[j].pmc.flg = fdt32_to_cpu(prop[i]);
			pfc_sd_reg_tbl[j].pmc.reg = fdt32_to_cpu(prop[i + 1]);
			pfc_sd_reg_tbl[j].pmc.val = fdt32_to_cpu(prop[i + 2]);

			/* PFC */
			pfc_sd_reg_tbl[j].pfc.flg = fdt32_to_cpu(prop[i + 3]);
			pfc_sd_reg_tbl[j].pfc.reg = fdt32_to_cpu(prop[i + 4]);
			pfc_sd_reg_tbl[j].pfc.val = fdt32_to_cpu(prop[i + 5]);

			/* IOLH */
			pfc_sd_reg_tbl[j].iolh.flg = fdt32_to_cpu(prop[i + 6]);
			pfc_sd_reg_tbl[j].iolh.reg = fdt32_to_cpu(prop[i + 7]);
			temp = (uint64_t)fdt32_to_cpu(prop[i + 8]);
			pfc_sd_reg_tbl[j].iolh.val = (temp << 32) | temp;

			/* PUPD */
			pfc_sd_reg_tbl[j].pupd.flg = fdt32_to_cpu(prop[i + 9]);
			pfc_sd_reg_tbl[j].pupd.reg = fdt32_to_cpu(prop[i + 10]);
			pfc_sd_reg_tbl[j].pupd.val = fdt32_to_cpu(prop[i + 11]);

			/* SR */
			pfc_sd_reg_tbl[j].sr.flg = fdt32_to_cpu(prop[i + 12]);
			pfc_sd_reg_tbl[j].sr.reg = fdt32_to_cpu(prop[i + 13]);
			temp = (uint64_t)fdt32_to_cpu(prop[i + 14]);
			pfc_sd_reg_tbl[j].sr.val = (temp << 32) | temp;

			/* IEN */
			pfc_sd_reg_tbl[j].ien.flg = fdt32_to_cpu(prop[i + 15]);
			pfc_sd_reg_tbl[j].ien.reg = fdt32_to_cpu(prop[i + 16]);
			temp = (uint64_t)fdt32_to_cpu(prop[i + 17]);
			pfc_sd_reg_tbl[j].ien.val = (temp << 32) | temp;
		} else {
			/* PMC */
			pfc_sd_reg_tbl[j].pmc.flg = fdt32_to_cpu(prop[i]);
			pfc_sd_reg_tbl[j].pmc.reg = fdt32_to_cpu(prop[i + 1]);
			pfc_sd_reg_tbl[j].pmc.val = fdt32_to_cpu(prop[i + 2]);

			/* PFC */
			pfc_sd_reg_tbl[j].pfc.flg = fdt32_to_cpu(prop[i + 3]);
			pfc_sd_reg_tbl[j].pfc.reg = fdt32_to_cpu(prop[i + 4]);
			pfc_sd_reg_tbl[j].pfc.val = fdt32_to_cpu(prop[i + 5]);

			/* IOLH */
			pfc_sd_reg_tbl[j].iolh.flg = fdt32_to_cpu(prop[i + 6]);
			pfc_sd_reg_tbl[j].iolh.reg = fdt32_to_cpu(prop[i + 7]);
			pfc_sd_reg_tbl[j].iolh.val = fdt32_to_cpu(prop[i + 8]);

			/* PUPD */
			pfc_sd_reg_tbl[j].pupd.flg = fdt32_to_cpu(prop[i + 9]);
			pfc_sd_reg_tbl[j].pupd.reg = fdt32_to_cpu(prop[i + 10]);
			pfc_sd_reg_tbl[j].pupd.val = fdt32_to_cpu(prop[i + 11]);

			/* SR */
			pfc_sd_reg_tbl[j].sr.flg = fdt32_to_cpu(prop[i + 12]);
			pfc_sd_reg_tbl[j].sr.reg = fdt32_to_cpu(prop[i + 13]);
			pfc_sd_reg_tbl[j].sr.val = fdt32_to_cpu(prop[i + 14]);

			/* IEN */
			pfc_sd_reg_tbl[j].ien.flg = fdt32_to_cpu(prop[i + 15]);
			pfc_sd_reg_tbl[j].ien.reg = fdt32_to_cpu(prop[i + 16]);
			pfc_sd_reg_tbl[j].ien.val = fdt32_to_cpu(prop[i + 17]);
		}
	}

	for (cnt = 0; cnt < ARRAY_SIZE(pfc_sd_reg_tbl); cnt++) {
		/* PMC */
		if (pfc_sd_reg_tbl[cnt].pmc.flg == PFC_ON) {
			PFC_REG_WRITE_8(pfc_sd_reg_tbl[cnt].pmc.reg, pfc_sd_reg_tbl[cnt].pmc.val);
		}
		/* PFC */
		if (pfc_sd_reg_tbl[cnt].pfc.flg == PFC_ON) {
			PFC_REG_WRITE_32(pfc_sd_reg_tbl[cnt].pfc.reg, pfc_sd_reg_tbl[cnt].pfc.val);
		}
		/* IOLH */
		if (pfc_sd_reg_tbl[cnt].iolh.flg == PFC_ON) {
			PFC_REG_WRITE_64(pfc_sd_reg_tbl[cnt].iolh.reg, pfc_sd_reg_tbl[cnt].iolh.val);
		}
		/* PUPD */
		if (pfc_sd_reg_tbl[cnt].pupd.flg == PFC_ON) {
			PFC_REG_WRITE_64(pfc_sd_reg_tbl[cnt].pupd.reg, pfc_sd_reg_tbl[cnt].pupd.val);
		}
		/* SR */
		if (pfc_sd_reg_tbl[cnt].sr.flg == PFC_ON) {
			PFC_REG_WRITE_64(pfc_sd_reg_tbl[cnt].sr.reg, pfc_sd_reg_tbl[cnt].sr.val);
		}
		/* IEN */
		if (pfc_sd_reg_tbl[cnt].ien.flg == PFC_ON) {
			PFC_REG_WRITE_64(pfc_sd_reg_tbl[cnt].ien.reg, pfc_sd_reg_tbl[cnt].ien.val);
		}
	}
}

static void pfc_drive_setup(void)
{
	static const uint64_t pfc_iolh_drive_tbl[4] = {0x0000000000000000, 0x0101010101010101, 0x0202020202020202, 0x0303030303030303};
	uint32_t sysc_base = FCONF_GET_PROPERTY(hw_config, sysc_config, sysc_base);

	/* Get the boot mode */
	boot_mode_t boot_mode = sys_get_boot_mode();

	if (boot_mode < SYS_BOOT_MODE_MAX) {
		const PFC_REGS * p_pins_tbl = pfc_boot_mode_tbls[boot_mode];
		uint32_t sys_lsi_otppoc = mmio_read_32(SYS_V2H_LSI_OTPPOC + sysc_base);
		uint64_t pfc_iolh_drive = 0;
		int cnt;

		if (0 != (sys_lsi_otppoc & pfc_io_drive[boot_mode].enable_mask)) {
			uint32_t index = ((sys_lsi_otppoc & pfc_io_drive[boot_mode].drive_mask) >> pfc_io_drive[boot_mode].drive_offset);

			pfc_iolh_drive = pfc_iolh_drive_tbl[index];

			for (cnt = 0; cnt < PFC_TBL_LEN; cnt++) {
				if (p_pins_tbl[cnt].iolh.flg == PFC_ON) {
					/* Write IOLH value from pfc_sd_reg_tbl[] masked with value in pin table */
					mmio_write_64(p_pins_tbl[cnt].iolh.reg, (pfc_iolh_drive & p_pins_tbl[cnt].iolh.val));
				}
			}
		}
	}
}

static void pfc_riic_pmic_setup(void)
{
#if PLAT_SYSTEM_SUSPEND
	// Set data pfc_i2c_bus8_reg_tbl to registers
	PFC_REG_WRITE_32(PFC_PWPR, PFC_REG_READ(PFC_PWPR) | PWPR_REGWE_A);
	for (int cnt = 0; cnt < PFC_TBL_LEN; cnt++) {
		/* PFC */
		if (pfc_i2c_bus8_reg_tbl[cnt].pfc.flg == PFC_ON) {
			PFC_REG_WRITE_32(pfc_i2c_bus8_reg_tbl[cnt].pfc.reg, pfc_i2c_bus8_reg_tbl[cnt].pfc.val);
		}
		/* PMC */
		if (pfc_i2c_bus8_reg_tbl[cnt].pmc.flg == PFC_ON) {
			PFC_REG_WRITE_8(pfc_i2c_bus8_reg_tbl[cnt].pmc.reg, pfc_i2c_bus8_reg_tbl[cnt].pmc.val);
		}
	}
	PFC_REG_WRITE_32(PFC_PWPR, PFC_REG_READ(PFC_PWPR) & ~PWPR_REGWE_A);
#endif /* PLAT_SYSTEM_SUSPEND */
}

void pfc_setup(void)
{
	/* Initialize global PFC config from DTB.  */
	g_pfc_fconf_cfg = pfc_config_getter();

	if (g_pfc_fconf_cfg->pfc_mux_setup) { pfc_mux_setup(); }
	if (g_pfc_fconf_cfg->pfc_qspi_setup) { pfc_qspi_setup(); }
	if (g_pfc_fconf_cfg->pfc_sd_setup) { pfc_sd_setup(); }
	if (g_pfc_fconf_cfg->pfc_drive_setup) { pfc_drive_setup(); }
	if (g_pfc_fconf_cfg->pfc_riic_pmic_setup) { pfc_riic_pmic_setup(); }
}
