/*
 * Copyright (c) 2021-2025, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <stdint.h>

#include <drivers/console.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>
#include "scif.h"

#include "rcar_def.h"

/* CPG */
#define CPG_BASE		0xE6150000UL
#define CPG_CPGWPR		(CPG_BASE + 0x0000UL)
#define CPG_CPGWPCR		(CPG_BASE + 0x0004UL)
#define CPG_MSTPCR5		(CPG_BASE + 0x2D14UL)
#define CPG_MSTPSR5		(CPG_BASE + 0x2E14UL)
#define CPG_MSTPSR5_HSCIF0	BIT(14)
#define CPG_MSTPCR7		(CPG_BASE + 0x2D1CUL)
#define CPG_MSTPSR7		(CPG_BASE + 0x2E1CUL)
#define CPG_MSTPSR7_SCIF0	BIT(2)
#define CPG_MSTPSR7_SCIF3	BIT(4)

/* RST */
#define RST_BASE		(0xE6160000UL + (RCAR_DOMAIN * 0x4000UL))
#define RST_MODEMR0		RST_BASE
#define RST_MODEMR1		(RST_BASE + 4UL)
#define RST_MODEMR0_MD31	BIT(31)
#define RST_MODEMR1_MD32	BIT(0)

/* SCIF/HSCIF */
#define SCIF0_BASE		0xE6E60000UL
#define SCIF3_BASE		0xE6C50000UL
#define HSCIF0_BASE		0xE6540000UL

/* SCIF */
#if (RCAR_LSI == RCAR_S4) /* S4 */
#define SCIF_BASE	SCIF3_BASE
#define CPG_MSTPSR7_BIT	CPG_MSTPSR7_SCIF3
#else
#define SCIF_BASE	SCIF0_BASE
#define CPG_MSTPSR7_BIT	CPG_MSTPSR7_SCIF0
#endif
#define SCIF_SCFTDR	(SCIF_BASE + 0x000CU)	/*  8 Transmit FIFO data register */
#define SCIF_SCFSR	(SCIF_BASE + 0x0010U)	/* 16 Serial status register */
#define SCIF_SCSMR	(SCIF_BASE + 0x0000U)	/* 16 Serial mode register */
#define SCIF_SCBRR	(SCIF_BASE + 0x0004U)	/*  8 Bit rate register */
#define SCIF_SCSCR	(SCIF_BASE + 0x0008U)	/* 16 Serial control register */
#define SCIF_SCFCR	(SCIF_BASE + 0x0018U)	/* 16 FIFO control register */
#define SCIF_SCLSR	(SCIF_BASE + 0x0024U)	/* 16 Line status register */

/* HSCIF */
#define HSCIF_BASE	HSCIF0_BASE
#define HSCIF_HSSMR	(HSCIF_BASE + 0x0000U) /* 16 Serial mode register */
#define HSCIF_HSBRR	(HSCIF_BASE + 0x0004U) /*  8 Bit rate register */
#define HSCIF_HSSCR	(HSCIF_BASE + 0x0008U) /* 16 Serial control register */
#define HSCIF_HSFTDR	(HSCIF_BASE + 0x000CU) /*  8 Transmit FIFO data register */
#define HSCIF_HSFSR	(HSCIF_BASE + 0x0010U) /* 16 Serial status register */
#define HSCIF_HSFCR	(HSCIF_BASE + 0x0018U) /* 16 FIFO control register */
#define HSCIF_HSLSR	(HSCIF_BASE + 0x0024U) /* 16 Line status register */
#define HSCIF_HSDR	(HSCIF_BASE + 0x0030U) /* 16 BRG frequency division register */
#define HSCIF_HSCKS	(HSCIF_BASE + 0x0034U) /* 16 BRG clock select register */
#define HSCIF_HSSRR	(HSCIF_BASE + 0x0040U) /* 16 Sampling rate register */

/* Mode */
#define MODEMR_SCIF_DLMODE		0U
#define MODEMR_HSCIF_DLMODE_921600	1U
#define MODEMR_HSCIF_DLMODE_1843200	2U
#define MODEMR_HSCIF_DLMODE_3000000	3U
#define MODEMR_SCIF_DLMODE_115200	115200U

#define SCSMR_INIT_DATA			0x0000U
#define SCBRR_115200_BPS		17U
#define SCSCR_TE_EN			BIT(5)
#define SCSCR_RE_EN			BIT(4)
#define SCFSR_INIT_DATA			0x0000U
#define SCFCR_TFRST_EN			BIT(2)
#define SCFCR_RFRST_EN			BIT(1)
#define SCFCR_INIT_DATA			0x0000U
#define SCDL_INIT_DATA			8U
#define SCCKS_INIT_DATA			0x0000U
#define HSSRR_INIT_DATA			0x0000U

static void rcar_gen4_scif_set_baudrate_115200(void)
{
	uint16_t reg;
	uint32_t timeout = 100U;

	/* Disable transmit/receive before reprogramming the UART. */
	mmio_write_16(SCIF_SCSCR, 0U);

	/* Reset the FIFOs and clear sticky status bits. */
	reg = mmio_read_16(SCIF_SCFCR);
	reg |= SCFCR_TFRST_EN | SCFCR_RFRST_EN;
	mmio_write_16(SCIF_SCFCR, reg);
	mmio_write_16(SCIF_SCFSR, SCFSR_INIT_DATA);
	mmio_write_16(SCIF_SCLSR, 0U);

	/* Program 8N1 async mode with internal clock and 115200 baud. */
	mmio_write_16(SCIF_SCSMR, SCSMR_INIT_DATA);
	mmio_write_8(SCIF_SCBRR, SCBRR_115200_BPS);

	while (timeout-- != 0U) {
		;
	}

	/* Release FIFO reset and enable TX/RX. */
	mmio_write_16(SCIF_SCFCR, SCFCR_INIT_DATA);
	mmio_write_16(SCIF_SCSCR, SCSCR_TE_EN | SCSCR_RE_EN);
}

static void rcar_gen4_hscif_set_baudrate_115200(void)
{
	uint16_t reg;
	uint32_t timeout = 100U;

	/* Disable transmit/receive before reprogramming the UART. */
	mmio_write_16(HSCIF_HSSCR, 0U);

	/* Reset the FIFOs and clear sticky status bits. */
	reg = mmio_read_16(HSCIF_HSFCR);
	reg |= SCFCR_TFRST_EN | SCFCR_RFRST_EN;
	mmio_write_16(HSCIF_HSFCR, reg);
	mmio_write_16(HSCIF_HSFSR, SCFSR_INIT_DATA);
	mmio_write_16(HSCIF_HSLSR, 0U);

	/*
	 * Program 8N1 async mode with internal clock. Use the standard BRG
	 * divisors so the port comes up at 115200 for early debug.
	 */
	mmio_write_16(HSCIF_HSSMR, SCSMR_INIT_DATA);
	mmio_write_16(HSCIF_HSDR, SCDL_INIT_DATA);
	mmio_write_16(HSCIF_HSCKS, SCCKS_INIT_DATA);
	mmio_write_16(HSCIF_HSSRR, HSSRR_INIT_DATA);
	mmio_write_8(HSCIF_HSBRR, SCBRR_115200_BPS);

	while (timeout-- != 0U) {
		;
	}

	/* Release FIFO reset and enable TX/RX. */
	mmio_write_16(HSCIF_HSFCR, SCFCR_INIT_DATA);
	mmio_write_16(HSCIF_HSSCR, SCSCR_TE_EN | SCSCR_RE_EN);
}

uint32_t rcar_gen4_scif_get_mode(void)
{
	return ((mmio_read_32(RST_MODEMR0) & RST_MODEMR0_MD31) >> 31U) |
	       ((mmio_read_32(RST_MODEMR1) & RST_MODEMR1_MD32) << 1U);
}

uint32_t rcar_gen4_scif_get_baudrate(void)
{
	switch (rcar_gen4_scif_get_mode()) {
	case MODEMR_HSCIF_DLMODE_921600:
		return MODEMR_SCIF_DLMODE_115200;
	case MODEMR_HSCIF_DLMODE_1843200:
		return MODEMR_SCIF_DLMODE_115200;
	case MODEMR_HSCIF_DLMODE_3000000:
		return MODEMR_SCIF_DLMODE_115200;
	case MODEMR_SCIF_DLMODE:
	default:
		return MODEMR_SCIF_DLMODE_115200;
	}
}

int console_rcar_init(uintptr_t base_addr, uint32_t uart_clk,
		      uint32_t baud_rate)
{
	uint32_t modemr, mstpcr, mstpsr, mstpbit;

	modemr = rcar_gen4_scif_get_mode();

	if (modemr == MODEMR_HSCIF_DLMODE_3000000 ||
	    modemr == MODEMR_HSCIF_DLMODE_1843200 ||
	    modemr == MODEMR_HSCIF_DLMODE_921600) {
		mstpcr = CPG_MSTPCR5;
		mstpsr = CPG_MSTPSR5;
		mstpbit = CPG_MSTPSR5_HSCIF0;
		scif_console_set_regs(HSCIF_HSFSR, HSCIF_HSFTDR);
	} else {
		mstpcr = CPG_MSTPCR7;
		mstpsr = CPG_MSTPSR7;
		mstpbit = CPG_MSTPSR7_BIT;
		scif_console_set_regs(SCIF_SCFSR, SCIF_SCFTDR);
	}

	/* Turn SCIF/HSCIF clock ON. */
	mmio_clrbits_32(mstpcr, mstpbit);
	while (mmio_read_32(mstpsr) & mstpbit)
		;

	if (modemr == MODEMR_SCIF_DLMODE) {
		rcar_gen4_scif_set_baudrate_115200();
	} else {
		rcar_gen4_hscif_set_baudrate_115200();
	}

	return 1;
}
