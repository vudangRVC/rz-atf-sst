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
#include <rzv2h_pfc.h>

#define PFC_TBL_LEN						(2)


/* SDHI 0 */
static PFC_REGS pfc_sd_reg_tbl[PFC_TBL_LEN] = {
	/* SD0_CLK (P9.0), SD0_CMD (P9.1), SD0_RSTN (P9.2) */
	{
		{ PFC_OFF, (uintptr_t)NULL,       0 },						/* PMC */
		{ PFC_OFF, (uintptr_t)NULL,       0 },						/* PFC */
		{ PFC_ON,  (uintptr_t)PFC_IOLH09_OFFSET, 0x0000000000030303 },		/* IOLH */
		{ PFC_ON,  (uintptr_t)PFC_PUPD09_OFFSET, 0x0000000000000000 },		/* PUPD */
		{ PFC_ON,  (uintptr_t)PFC_SR09_OFFSET,   0x0000000000000000 },		/* SR */
		{ PFC_ON,  (uintptr_t)PFC_IEN09_OFFSET,  0x0000000000000100 }		/* IEN */
	},

	/* SD0_DATA (PA.0 - PA.7 */
	{
		{ PFC_OFF, (uintptr_t)NULL,       0 },						/* PMC */
		{ PFC_OFF, (uintptr_t)NULL,       0 },						/* PFC */
		{ PFC_ON,  (uintptr_t)PFC_IOLH0A_OFFSET, 0x0303030303030303 },		/* IOLH */
		{ PFC_ON,  (uintptr_t)PFC_PUPD0A_OFFSET, 0x0000000000000000 },		/* PUPD */
		{ PFC_ON,  (uintptr_t)PFC_SR0A_OFFSET,   0x0000000000000000 },		/* SR */
		{ PFC_ON,  (uintptr_t)PFC_IEN0A_OFFSET,  0x0101010101010101 }		/* IEN */
	},
};

static PFC_REGS pfc_qspi_reg_tbl[PFC_TBL_LEN] = {
	/* QSPI0 CLK (P7.0), CS0 (P7.2) */
	{
		{ PFC_OFF, (uintptr_t)NULL,       0 },						/* PMC */
		{ PFC_OFF, (uintptr_t)NULL,       0 },						/* PFC */
		{ PFC_ON,  (uintptr_t)PFC_IOLH07_OFFSET, 0x0000000000030003 },		/* IOLH */
		{ PFC_ON,  (uintptr_t)PFC_PUPD07_OFFSET, 0x0000000000000000 },		/* PUPD */
		{ PFC_ON,  (uintptr_t)PFC_SR07_OFFSET,   0x0000000000000000 },		/* SR */
		{ PFC_OFF, (uintptr_t)NULL,       0 }						/* IEN */
	},

	/* QSPI0 IO0-IO3 (P8.0 - P8.3) */
	{
		{ PFC_OFF, (uintptr_t)NULL,       0 },						/* PMC */
		{ PFC_OFF, (uintptr_t)NULL,       0 },						/* PFC */
		{ PFC_ON,  (uintptr_t)PFC_IOLH08_OFFSET, 0x0000000003030303 },		/* IOLH */
		{ PFC_ON,  (uintptr_t)PFC_PUPD08_OFFSET, 0x0000000000000000 },		/* PUPD */
		{ PFC_ON,  (uintptr_t)PFC_SR08_OFFSET,   0x0000000000000000 },		/* SR */
		{ PFC_OFF, (uintptr_t)NULL,       0 }						/* IEN */
	},
};

/* SCIF */
static PFC_REGS pfc_scif_reg_tbl[PFC_TBL_LEN] = {
	/* SCIF_RXD (P6.0), SCIF_TXD (P6.1) */
	{
		{ PFC_OFF, (uintptr_t)NULL,       0 },						/* PMC */
		{ PFC_OFF, (uintptr_t)NULL,       0 },						/* PFC */
		{ PFC_ON,  (uintptr_t)PFC_IOLH06_OFFSET, 0x0000000000000003 },		/* IOLH */
		{ PFC_ON,  (uintptr_t)PFC_PUPD06_OFFSET, 0x0000000000000000 },		/* PUPD */
		{ PFC_ON,  (uintptr_t)PFC_SR06_OFFSET,   0x0000000000000000 },		/* SR */
		{ PFC_ON,  (uintptr_t)NULL,       0x0000000000000000 }		/* IEN */
	},

	/* Padding to make same length as other pin tables */
	{
		{0}
	},
};

#if PLAT_SYSTEM_SUSPEND
/* I2C8 */
static PFC_REGS pfc_i2c_bus8_reg_tbl[PFC_TBL_LEN] = {
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
	{SYS_LSI_OTPPOC_EN_SD_DS_MASK,		SYS_LSI_OTPPOC_SD_E_MASK,		SYS_LSI_OTPPOC_SD_E_OFFSET},
	{SYS_LSI_OTPPOC_EN_EMMC18_DS_MASK,	SYS_LSI_OTPPOC_EMMC18_E_MASK,	SYS_LSI_OTPPOC_EMMC18_E_OFFSET},
	{SYS_LSI_OTPPOC_EN_EMMC33_DS_MASK,	SYS_LSI_OTPPOC_EMMC33_E_MASK,	SYS_LSI_OTPPOC_EMMC33_E_OFFSET},
	{SYS_LSI_OTPPOC_EN_SPI18_DS_MASK,	SYS_LSI_OTPPOC_SPI18_E_MASK,	SYS_LSI_OTPPOC_SPI18_E_OFFSET},
	{SYS_LSI_OTPPOC_EN_SPI33_DS_MASK,	SYS_LSI_OTPPOC_SPI33_E_MASK,	SYS_LSI_OTPPOC_SPI33_E_OFFSET},
	{SYS_LSI_OTPPOC_EN_SCIF_DS_MASK,	SYS_LSI_OTPPOC_SCIF_E_MASK,		SYS_LSI_OTPPOC_SCIF_E_OFFSET},
};

static const PFC_REGS *pfc_boot_mode_tbls[SYS_BOOT_MODE_MAX] = {
	pfc_sd_reg_tbl,
	pfc_sd_reg_tbl,
	pfc_sd_reg_tbl,
	pfc_qspi_reg_tbl,
	pfc_qspi_reg_tbl,
	pfc_scif_reg_tbl
};

static void pfc_sd_setup(void*fdt, uintptr_t pfc_base)
{
	// Reinit static pfc_sd_reg_tbl struct
	int cnt;
	for (cnt = 0; cnt < PFC_TBL_LEN; cnt++) {
		if(pfc_sd_reg_tbl[cnt].pmc.reg != (uintptr_t)NULL) {
			pfc_sd_reg_tbl[cnt].pmc.reg += (uintptr_t)(pfc_base);
		}
		if(pfc_sd_reg_tbl[cnt].pfc.reg != (uintptr_t)NULL) {
			pfc_sd_reg_tbl[cnt].pfc.reg += (uintptr_t)(pfc_base);
		}
		if(pfc_sd_reg_tbl[cnt].iolh.reg != (uintptr_t)NULL) {
			pfc_sd_reg_tbl[cnt].iolh.reg += (uintptr_t)(pfc_base);
		}
		if(pfc_sd_reg_tbl[cnt].pupd.reg != (uintptr_t)NULL) {
			pfc_sd_reg_tbl[cnt].pupd.reg += (uintptr_t)(pfc_base);
		}
		if(pfc_sd_reg_tbl[cnt].sr.reg != (uintptr_t)NULL) {
			pfc_sd_reg_tbl[cnt].sr.reg += (uintptr_t)(pfc_base);
		}
		if(pfc_sd_reg_tbl[cnt].ien.reg != (uintptr_t)NULL) {
			pfc_sd_reg_tbl[cnt].ien.reg += (uintptr_t)(pfc_base);
		}
	}

	// Set data pfc_sd_reg_tbl to registers
	for (cnt = 0; cnt < PFC_TBL_LEN; cnt++) {
		/* PUPD */
		if (pfc_sd_reg_tbl[cnt].pupd.flg == PFC_ON) {
			mmio_write_64(pfc_sd_reg_tbl[cnt].pupd.reg, pfc_sd_reg_tbl[cnt].pupd.val);
		}
		/* SR */
		if (pfc_sd_reg_tbl[cnt].sr.flg == PFC_ON) {
			mmio_write_64(pfc_sd_reg_tbl[cnt].sr.reg, pfc_sd_reg_tbl[cnt].sr.val);
		}
		/* IEN */
		if (pfc_sd_reg_tbl[cnt].ien.flg == PFC_ON) {
			mmio_write_64(pfc_sd_reg_tbl[cnt].ien.reg, pfc_sd_reg_tbl[cnt].ien.val);
		}
	}
}

static void pfc_qspi_setup(void*fdt, uintptr_t pfc_base)
{
	// Reinit static pfc_qspi_reg_tbl struct
	int cnt;
	for (cnt = 0; cnt < PFC_TBL_LEN; cnt++) {
		if(pfc_qspi_reg_tbl[cnt].pmc.reg != (uintptr_t)NULL) {
			pfc_qspi_reg_tbl[cnt].pmc.reg += (uintptr_t)(pfc_base);
		}
		if(pfc_qspi_reg_tbl[cnt].pfc.reg != (uintptr_t)NULL) {
			pfc_qspi_reg_tbl[cnt].pfc.reg += (uintptr_t)(pfc_base);
		}
		if(pfc_qspi_reg_tbl[cnt].iolh.reg != (uintptr_t)NULL) {
			pfc_qspi_reg_tbl[cnt].iolh.reg += (uintptr_t)(pfc_base);
		}
		if(pfc_qspi_reg_tbl[cnt].pupd.reg != (uintptr_t)NULL) {
			pfc_qspi_reg_tbl[cnt].pupd.reg += (uintptr_t)(pfc_base);
		}
		if(pfc_qspi_reg_tbl[cnt].sr.reg != (uintptr_t)NULL) {
			pfc_qspi_reg_tbl[cnt].sr.reg += (uintptr_t)(pfc_base);
		}
		if(pfc_qspi_reg_tbl[cnt].ien.reg != (uintptr_t)NULL) {
			pfc_qspi_reg_tbl[cnt].ien.reg += (uintptr_t)(pfc_base);
		}
	}

	// Set data pfc_qspi_reg_tbl to registers
	for (cnt = 0; cnt < PFC_TBL_LEN; cnt++) {
		/* PUPD */
		if (pfc_qspi_reg_tbl[cnt].pupd.flg == PFC_ON) {
			mmio_write_64(pfc_qspi_reg_tbl[cnt].pupd.reg, pfc_qspi_reg_tbl[cnt].pupd.val);
		}
		/* SR */
		if (pfc_qspi_reg_tbl[cnt].sr.flg == PFC_ON) {
			mmio_write_64(pfc_qspi_reg_tbl[cnt].sr.reg, pfc_qspi_reg_tbl[cnt].sr.val);
		}
	}
}

static void pfc_scif_setup(void*fdt, uintptr_t pfc_base)
{
	// Reinit static pfc_scif_reg_tbl struct
	int cnt;
	for (cnt = 0; cnt < PFC_TBL_LEN; cnt++) {
		if(pfc_scif_reg_tbl[cnt].pmc.reg != (uintptr_t)NULL) {
			pfc_scif_reg_tbl[cnt].pmc.reg += (uintptr_t)(pfc_base);
		}
		if(pfc_scif_reg_tbl[cnt].pfc.reg != (uintptr_t)NULL) {
			pfc_scif_reg_tbl[cnt].pfc.reg += (uintptr_t)(pfc_base);
		}
		if(pfc_scif_reg_tbl[cnt].iolh.reg != (uintptr_t)NULL) {
			pfc_scif_reg_tbl[cnt].iolh.reg += (uintptr_t)(pfc_base);
		}
		if(pfc_scif_reg_tbl[cnt].pupd.reg != (uintptr_t)NULL) {
			pfc_scif_reg_tbl[cnt].pupd.reg += (uintptr_t)(pfc_base);
		}
		if(pfc_scif_reg_tbl[cnt].sr.reg != (uintptr_t)NULL) {
			pfc_scif_reg_tbl[cnt].sr.reg += (uintptr_t)(pfc_base);
		}
		if(pfc_scif_reg_tbl[cnt].ien.reg != (uintptr_t)NULL) {
			pfc_scif_reg_tbl[cnt].ien.reg += (uintptr_t)(pfc_base);
		}
	}

	// Set data pfc_scif_reg_tbl to registers
	for (cnt = 0; cnt < PFC_TBL_LEN; cnt++) {
		/* PUPD */
		if (pfc_scif_reg_tbl[cnt].pupd.flg == PFC_ON) {
			mmio_write_64(pfc_scif_reg_tbl[cnt].pupd.reg, pfc_scif_reg_tbl[cnt].pupd.val);
		}
		/* SR */
		if (pfc_scif_reg_tbl[cnt].sr.flg == PFC_ON) {
			mmio_write_64(pfc_scif_reg_tbl[cnt].sr.reg, pfc_scif_reg_tbl[cnt].sr.val);
		}
	}
}

static void pfc_drive_setup(void*fdt, uintptr_t pfc_base, uintptr_t sysc_base)
{
	static const uint64_t pfc_iolh_drive_tbl[4] = {0x0000000000000000, 0x0101010101010101, 0x0202020202020202, 0x0303030303030303};
	/* Get the boot mode */
	boot_mode_t boot_mode = sys_get_boot_mode();

	if (boot_mode < SYS_BOOT_MODE_MAX) {
		const PFC_REGS *p_pins_tbl = pfc_boot_mode_tbls[boot_mode];
		uint32_t sys_lsi_otppoc = mmio_read_32(SYS_LSI_OTPPOC_OFFSET + sysc_base);
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

static void pfc_riic_pmic_setup(void *fdt, uintptr_t pfc_base)
{
#if PLAT_SYSTEM_SUSPEND
	// Reinit static pfc_i2c_bus8_reg_tbl struct
	int cnt;
	for (cnt = 0; cnt < PFC_TBL_LEN; cnt++) {
		if(pfc_i2c_bus8_reg_tbl[cnt].pmc.reg != (uintptr_t)NULL) {
			pfc_i2c_bus8_reg_tbl[cnt].pmc.reg += (uintptr_t)(pfc_base);
		}
		if(pfc_i2c_bus8_reg_tbl[cnt].pfc.reg != (uintptr_t)NULL) {
			pfc_i2c_bus8_reg_tbl[cnt].pfc.reg += (uintptr_t)(pfc_base);
		}
		if(pfc_i2c_bus8_reg_tbl[cnt].iolh.reg != (uintptr_t)NULL) {
			pfc_i2c_bus8_reg_tbl[cnt].iolh.reg += (uintptr_t)(pfc_base);
		}
		if(pfc_i2c_bus8_reg_tbl[cnt].pupd.reg != (uintptr_t)NULL) {
			pfc_i2c_bus8_reg_tbl[cnt].pupd.reg += (uintptr_t)(pfc_base);
		}
		if(pfc_i2c_bus8_reg_tbl[cnt].sr.reg != (uintptr_t)NULL) {
			pfc_i2c_bus8_reg_tbl[cnt].sr.reg += (uintptr_t)(pfc_base);
		}
		if(pfc_i2c_bus8_reg_tbl[cnt].ien.reg != (uintptr_t)NULL) {
			pfc_i2c_bus8_reg_tbl[cnt].ien.reg += (uintptr_t)(pfc_base);
		}
	}

	// Set data pfc_i2c_bus8_reg_tbl to registers
	mmio_write_32(PFC_PWPR_OFFSET + pfc_base, mmio_read_32(PFC_PWPR_OFFSET + pfc_base) | PWPR_REGWE_A);
	for (cnt = 0; cnt < PFC_TBL_LEN; cnt++) {
		/* PFC */
		if (pfc_i2c_bus8_reg_tbl[cnt].pfc.flg == PFC_ON) {
			mmio_write_32(pfc_i2c_bus8_reg_tbl[cnt].pfc.reg, pfc_i2c_bus8_reg_tbl[cnt].pfc.val);
		}
		/* PMC */
		if (pfc_i2c_bus8_reg_tbl[cnt].pmc.flg == PFC_ON) {
			mmio_write_8(pfc_i2c_bus8_reg_tbl[cnt].pmc.reg, pfc_i2c_bus8_reg_tbl[cnt].pmc.val);
		}
	}
	mmio_write_32(PFC_PWPR_OFFSET + pfc_base, mmio_read_32(PFC_PWPR_OFFSET + pfc_base) & ~PWPR_REGWE_A);
#endif /* PLAT_SYSTEM_SUSPEND */
}

void rzv2h_pfc_setup(void *fdt)
{
	// Get pin-controller base address
	const char *node = "/soc";
	const char *sub_node = "pinctrl@10410000";
	const char *prop_name = "reg";
	uint32_t v2h_pfc_base = 0;
	if(read_prop_from_subnode(fdt, node, sub_node, prop_name, 1, &v2h_pfc_base) != 0) {
		ERROR("BL2: Failed to get PFC base address\n");
		return;
	}

	// Get sysc base address
	uint32_t v2h_sysc_base = 0;
	const char *sub_node_sysc = "system-controller@10430000";
	if(read_prop_from_subnode(fdt, node, sub_node_sysc, prop_name, 1, &v2h_sysc_base) != 0) {
		ERROR("BL2: Failed to get PFC base address\n");
		return;
	}

	pfc_sd_setup(fdt, v2h_pfc_base);
	pfc_qspi_setup(fdt, v2h_pfc_base);
	pfc_scif_setup(fdt, v2h_pfc_base);
	pfc_drive_setup(fdt, v2h_pfc_base, v2h_sysc_base);
	pfc_riic_pmic_setup(fdt, v2h_pfc_base);
}
