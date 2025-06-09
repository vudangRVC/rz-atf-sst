/*
 * Copyright (c) 2022, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __CPG_REGS_OFFSET_H__
#define __CPG_REGS_OFFSET_H__

#include <rzv2h_soc_def.h>				/* Get the CPG base address */

#define	CPG_PLLCM33_MON_OFFSET				(0x010)	/* PLLCM33 monitor register */
#define	CPG_PLLCLN_STBY_OFFSET				(0x020)	/* PLLCLN standby control register */
#define	CPG_PLLCLN_MON_OFFSET				(0x030)	/* PLLCLN monitor register */
#define	CPG_PLLDTY_STBY_OFFSET				(0x040)	/* PLLDTY standby control register */
#define	CPG_PLLDTY_MON_OFFSET				(0x050)	/* PLLDTY monitor register */
#define	CPG_PLLCA55_STBY_OFFSET				(0x060)	/* PLLCA55 standby control register */
#define	CPG_PLLCA55_CLK1_OFFSET				(0x064)	/* PLLCA55 output clock setting register 1 */
#define	CPG_PLLCA55_CLK2_OFFSET				(0x068)	/* PLLCA55 output clock setting register 2 */
#define	CPG_PLLCA55_MON_OFFSET				(0x070)	/* PLLCA55 monitor register */
#define	CPG_PLLVDO_STBY_OFFSET				(0x080)	/* PLLVDO standby control register */
#define	CPG_PLLVDO_MON_OFFSET				(0x090)	/* PLLVDO monitor register */
#define	CPG_PLLETH_STBY_OFFSET				(0x0A0)	/* PLLETH standby control register */
#define	CPG_PLLETH_MON_OFFSET				(0x0B0)	/* PLLETH monitor register */
#define	CPG_PLLDSI_STBY_OFFSET				(0x0C0)	/* PLLDSI standby control register */
#define	CPG_PLLDSI_CLK1_OFFSET				(0x0C4)	/* PLLDSI output clock setting register 1 */
#define	CPG_PLLDSI_CLK2_OFFSET				(0x0C8)	/* PLLDSI output clock setting register 2 */
#define	CPG_PLLDSI_MON_OFFSET				(0x0D0)	/* PLLDSI monitor register */
#define	CPG_PLLDDR0_STBY_OFFSET				(0x0E0)	/* PLLDDR0 standby control register */
#define	CPG_PLLDDR0_MON_OFFSET				(0x0F0)	/* PLLDDR0 monitor register*/
#define	CPG_PLLDDR1_STBY_OFFSET				(0x100)	/* PLLDDR1standby control register*/
#define	CPG_PLLDDR1_MON_OFFSET				(0x110)	/* PLLDDR1monitor register */
#define	CPG_PLLGPU_STBY_OFFSET				(0x120)	/* PLLGPU standby control register */
#define	CPG_PLLGPU_CLK1_OFFSET				(0x124)	/* PLLGPU output clock setting register 1 */
#define	CPG_PLLGPU_CLK2_OFFSET				(0x128)	/* PLLGPU output clock setting register 2 */
#define	CPG_PLLGPU_MON_OFFSET				(0x130)	/* PLLGPU monitor register */
#define	CPG_PLLDRP_STBY_OFFSET				(0x140)	/* PLLDRP standby control register */
#define	CPG_PLLDRP_CLK1_OFFSET				(0x144)	/* PLLDRP output clock setting register 1 */
#define	CPG_PLLDRP_CLK2_OFFSET				(0x148)	/* PLLDRP output clock setting register 2 */
#define	CPG_PLLDRP_MON_OFFSET				(0x150)	/* PLLDRP monitor register */
#define	CCLMA0_CTL_OFFSET					(0x200)	/* CLMA0 control register */
#define	CCLMA1_CTL_OFFSET					(0x204)	/* CLMA1 control register */
#define	CCLMA2_CTL_OFFSET					(0x208)	/* CLMA2 control register */
#define	CCLMA3_CTL_OFFSET					(0x20C)	/* CLMA3 control register */
#define	CCLMA4_CTL_OFFSET					(0x210)	/* CLMA4 control register */
#define	CCLMA5_CTL_OFFSET					(0x214)	/* CLMA5 control register */
#define	CCLMA6_CTL_OFFSET					(0x218)	/* CLMA6 control register */
#define	CCLMA7_CTL_OFFSET					(0x21C)	/* CLMA7 control register */
#define	CCLMA8_CTL_OFFSET					(0x220)	/* CLMA8 control register */
#define	CCLMA9_CTL_OFFSET					(0x224)	/* CLMA9 control register */
#define	CCLMA10_CTL_OFFSET					(0x228)	/* CLMA10 control register */
#define	CCLMA11_CTL_OFFSET					(0x22C)	/* CLMA11 control register */
#define	CCLMA12_CTL_OFFSET					(0x230)	/* CLMA12 control register */
#define	CCLMA13_CTL_OFFSET					(0x234)	/* CLMA13 control register */
#define	CCLMA14_CTL_OFFSET					(0x238)	/* CLMA14 control register */
#define	CCLMA_MON_OFFSET					(0x280)	/* CLMA monitor register */
#define	CPG_SSEL0_OFFSET					(0x300)	/* Static Mux control */
#define	CPG_SSEL1_OFFSET					(0x304)	/* Static Mux control */
#define	CPG_SSEL2_OFFSET					(0x308)	/* Static Mux control */
#define	CPG_CDDIV0_OFFSET					(0x400)	/* Dynamic Gear control(Counter type) */
#define	CPG_CDDIV1_OFFSET					(0x404)	/* Dynamic Gear control(Counter type) */
#define	CPG_CDDIV2_OFFSET					(0x408)	/* Dynamic Gear control(Counter type) */
#define	CPG_CDDIV3_OFFSET					(0x40c)	/* Dynamic Gear control(Counter type) */
#define	CPG_CDDIV4_OFFSET					(0x410)	/* Dynamic Gear control(Counter type) */
#define	CPG_CSDIV0_OFFSET					(0x500)	/* Static Gear control(Sparse type) */
#define	CPG_CSDIV1_OFFSET					(0x504)	/* Static Gear control(Sparse type) */
#define	CPG_CLKON_0_OFFSET					(0x600)	/* CGC control */
#define	CPG_CLKON_1_OFFSET					(0x604)	/* CGC control */
#define	CPG_CLKON_2_OFFSET					(0x608)	/* CGC control */
#define	CPG_CLKON_3_OFFSET					(0x60c)	/* CGC control */
#define	CPG_CLKON_4_OFFSET					(0x610)	/* CGC control */
#define	CPG_CLKON_5_OFFSET					(0x614)	/* CGC control */
#define	CPG_CLKON_6_OFFSET					(0x618)	/* CGC control */
#define	CPG_CLKON_7_OFFSET					(0x61c)	/* CGC control */
#define	CPG_CLKON_8_OFFSET					(0x620)	/* CGC control */
#define	CPG_CLKON_9_OFFSET					(0x624)	/* CGC control */
#define	CPG_CLKON_10_OFFSET				(0x628)	/* CGC control */
#define	CPG_CLKON_11_OFFSET				(0x62c)	/* CGC control */
#define	CPG_CLKON_12_OFFSET				(0x630)	/* CGC control */
#define	CPG_CLKON_13_OFFSET				(0x634)	/* CGC control */
#define	CPG_CLKON_14_OFFSET				(0x638)	/* CGC control */
#define	CPG_CLKON_15_OFFSET				(0x63c)	/* CGC control */
#define	CPG_CLKON_16_OFFSET				(0x640)	/* CGC control */
#define	CPG_CLKON_17_OFFSET				(0x644)	/* CGC control */
#define	CPG_CLKON_18_OFFSET				(0x648)	/* CGC control */
#define	CPG_CLKON_19_OFFSET				(0x64c)	/* CGC control */
#define	CPG_CLKON_20_OFFSET				(0x650)	/* CGC control */
#define	CPG_CLKON_21_OFFSET				(0x654)	/* CGC control */
#define	CPG_CLKON_22_OFFSET				(0x658)	/* CGC control */
#define	CPG_CLKON_23_OFFSET				(0x65C)	/* CGC control */
#define	CPG_CLKON_24_OFFSET				(0x660)	/* CGC control */
#define	CPG_CLKSTATUS0_OFFSET				(0x700)	/* Dynamic gear/mux status monitor */
#define	CPG_CLKMON_0_OFFSET				(0x800)	/* CGC monitor */
#define	CPG_CLKMON_1_OFFSET				(0x804)	/* CGC monitor */
#define	CPG_CLKMON_2_OFFSET				(0x808)	/* CGC monitor */
#define	CPG_CLKMON_3_OFFSET				(0x80c)	/* CGC monitor */
#define	CPG_CLKMON_4_OFFSET				(0x810)	/* CGC monitor */
#define	CPG_CLKMON_5_OFFSET				(0x814)	/* CGC monitor */
#define	CPG_CLKMON_6_OFFSET				(0x818)	/* CGC monitor */
#define	CPG_CLKMON_7_OFFSET				(0x81c)	/* CGC monitor */
#define	CPG_CLKMON_8_OFFSET				(0x820)	/* CGC monitor */
#define	CPG_CLKMON_9_OFFSET				(0x824)	/* CGC monitor */
#define	CPG_CLKMON_10_OFFSET				(0x828)	/* CGC monitor */
#define	CPG_RST_0_OFFSET					(0x900)	/* RESET ON/OFF control */
#define	CPG_RST_1_OFFSET					(0x904)	/* RESET ON/OFF control */
#define	CPG_RST_2_OFFSET					(0x908)	/* RESET ON/OFF control */
#define	CPG_RST_3_OFFSET					(0x90C)	/* RESET ON/OFF control */
#define	CPG_RST_4_OFFSET					(0x910)	/* RESET ON/OFF control */
#define	CPG_RST_5_OFFSET					(0x914)	/* RESET ON/OFF control */
#define	CPG_RST_6_OFFSET					(0x918)	/* RESET ON/OFF control */
#define	CPG_RST_7_OFFSET					(0x91C)	/* RESET ON/OFF control */
#define	CPG_RST_8_OFFSET					(0x920)	/* RESET ON/OFF control */
#define	CPG_RST_9_OFFSET					(0x924)	/* RESET ON/OFF control */
#define	CPG_RST_10_OFFSET					(0x928)	/* RESET ON/OFF control */
#define	CPG_RST_11_OFFSET					(0x92C)	/* RESET ON/OFF control */
#define	CPG_RST_12_OFFSET					(0x930)	/* RESET ON/OFF control */
#define	CPG_RST_13_OFFSET					(0x934)	/* RESET ON/OFF control */
#define	CPG_RST_14_OFFSET					(0x938)	/* RESET ON/OFF control */
#define	CPG_RST_15_OFFSET					(0x93C)	/* RESET ON/OFF control */
#define	CPG_RST_16_OFFSET					(0x940)	/* RESET ON/OFF control */
#define	CPG_RST_17_OFFSET					(0x944)	/* RESET ON/OFF control */
#define	CPG_RSTMON_0_OFFSET				(0xA00)	/* RESET monitor */
#define	CPG_RSTMON_1_OFFSET				(0xA04)	/* RESET monitor */
#define	CPG_RSTMON_2_OFFSET				(0xA08)	/* RESET monitor */
#define	CPG_RSTMON_3_OFFSET				(0xA0C)	/* RESET monitor */
#define	CPG_RSTMON_4_OFFSET				(0xA10)	/* RESET monitor */
#define	CPG_RSTMON_5_OFFSET				(0xA14)	/* RESET monitor */
#define	CPG_RSTMON_6_OFFSET				(0xA18)	/* RESET monitor */
#define	CPG_RSTMON_7_OFFSET				(0xA1C)	/* RESET monitor */
#define	CPG_RSTMON_8_OFFSET				(0xA20)	/* RESET monitor */
#define	CPG_ERRORRST_SEL1_OFFSET			(0xB00)	/* Error reset selection register */
#define	CPG_ERRORRST_SEL2_OFFSET			(0xB04)	/* Error reset selection register */
#define	CPG_ERRORRST_SEL3_OFFSET			(0xB08)	/* Error reset selection register */
#define	CPG_ERRORRST_SEL4_OFFSET			(0xB0C)	/* Error reset selection register */
#define	CPG_ERRORRST_SEL5_OFFSET			(0xB10)	/* Error reset selection register */
#define	CPG_ERRORRST_SEL6_OFFSET			(0xB14)	/* Error reset selection register */
#define	CPG_ERRORRST_SEL7_OFFSET			(0xB18)	/* Error reset selection register */
#define	CPG_ERRORRST_SEL8_OFFSET			(0xB1C)	/* Error reset selection register */
#define	CPG_ERROR_RST2_OFFSET				(0xB40)	/* Error reset register */
#define	CPG_ERROR_RST3_OFFSET				(0xB44)	/* Error reset register */
#define	CPG_ERROR_RST4_OFFSET				(0xB48)	/* Error reset register */
#define	CPG_ERROR_RST5_OFFSET				(0xB4C)	/* Error reset register */
#define	CPG_ERROR_RST6_OFFSET				(0xB50)	/* Error reset register */
#define	CPG_ERROR_RST7_OFFSET				(0xB54)	/* Error reset register */
#define	CPG_ERROR_RST8_OFFSET				(0xB58)	/* Error reset register */
#define	CPG_LP_CTL1_OFFSET					(0xC00)	/* Lowpower Sequence Control Register 1 */
#define	CPG_LP_CTL2_OFFSET					(0xC04)	/* Lowpower Sequence Control Register 2 */
#define	CPG_LP_GPU_CTL_OFFSET				(0xC08)	/* GPU Lowpower Sequence Control Register */
#define	CPG_CM33_CTL_OFFSET				(0xC0C)	/* CM33 Control register */
#define	CPG_CR8_CORESTATUS_OFFSET			(0xC10)	/* CR8 core status register */
#define	CPG_CR8_CONFIG1_OFFSET				(0xC14)	/* CR8 core configuration register1 */
#define	CPG_LP_CM33CTL0_OFFSET				(0xC18)	/* Lowpower Sequence Cortex-M33 Control Register 0 */
#define	CPG_LP_CM33CTL1_OFFSET				(0xC1C)	/* Lowpower Sequence Cortex-M33 Control Register 1 */
#define	CPG_LP_CA55_CTL1_OFFSET			(0xC20)	/* Cortex-A55 Clock Control Register 1 */
#define	CPG_LP_CA55_CTL2_OFFSET			(0xC24)	/* Cortex-A55 Clock Control Register 2 */
#define	CPG_LP_CA55_CTL3_OFFSET			(0xC28)	/* Cortex-A55 Clock Control Register 3 */
#define	CPG_LP_CA55_CTL4_OFFSET			(0xC2C)	/* Cortex-A55 Clock Control Register 4 */
#define	CPG_LP_CA55_CTL5_OFFSET			(0xC30)	/* Cortex-A55 Clock Control Register 5 */
#define	CPG_LP_CA55_CTL6_OFFSET			(0xC34)	/* Cortex-A55 Clock Control Register 6 */
#define	CPG_LP_CA55_CTL7_OFFSET			(0xC38)	/* Cortex-A55 Clock Control Register 7 */
#define	CPG_LP_CR8_CTL1_OFFSET				(0xC3C)	/* CR8SS control register 1 */
#define	CPG_LP_CR8_CTL3_OFFSET				(0xC44)	/* CR8SS control register 3 */
#define	CPG_LP_CR8_CTL4_OFFSET				(0xC48)	/* CR8SS control register 4 */
#define	CPG_LP_PMU_CTL1_OFFSET				(0xC4C)	/* Lowpower Sequence Control Register @ */
#define	CPG_LP_SRAM_STBY_CTL1_OFFSET		(0xC50)	/* DRP SRAM standby control */
#define	CPG_LP_SRAM_STBY_CTL2_OFFSET		(0xC54)	/* Shared SRAM standby control 0 */
#define	CPG_LP_SRAM_STBY_CTL3_OFFSET		(0xC58)	/* Shared SRAM standby control 8 */
#define	CPG_LP_GIC_CTL1_OFFSET				(0xC5C)	/* GIC control */
#define	CPG_LP_DDR_CTL1_OFFSET				(0xC60)	/* DDR retention control */
#define	CPG_LP_OTP_CTL1_OFFSET				(0xC64)	/* OTP control register 1 */
#define	CPG_LP_CST_CTL1_OFFSET				(0xC6C)	/* CST control register 1 */
#define	CPG_LP_CST_CTL2_OFFSET				(0xC70)	/* CST control register 2 */
#define	CPG_LP_CST_CTL3_OFFSET				(0xC74)	/* CST control register 3 */
#define	CPG_LP_PWC_CTL1_OFFSET				(0xC78)	/* PWC control register 1 */
#define	CPG_LP_PWC_CTL2_OFFSET				(0xC7C)	/* PWC control register 2 */
#define	CPG_OSTMTCKE_OFFSET				(0xC80)	/* OSTM enable control */
#define	CPG_DBGRST_OFFSET					(0xC84)	/* Reset control register when CA55 is debugged */
#define	OTP_HANDSHAKE_MON_OFFSET			(0xC88)	/* OTP handshake monitor register */
#define	CPG_OTHERS_INI_OFFSET				(0xC8C)	/* Others area reset control */
#define	CPG_BUS_1_MSTOP_OFFSET				(0xD00)	/* MSTOP register 1 */
#define	CPG_BUS_2_MSTOP_OFFSET				(0xD04)	/* MSTOP register 2 */
#define	CPG_BUS_3_MSTOP_OFFSET				(0xD08)	/* MSTOP register 3 */
#define	CPG_BUS_4_MSTOP_OFFSET				(0xD0C)	/* MSTOP register 4 */
#define	CPG_BUS_5_MSTOP_OFFSET				(0xD10)	/* MSTOP register 5 */
#define	CPG_BUS_6_MSTOP_OFFSET				(0xD14)	/* MSTOP register 6 */
#define	CPG_BUS_7_MSTOP_OFFSET				(0xD18)	/* MSTOP register 7 */
#define	CPG_BUS_8_MSTOP_OFFSET				(0xD1C)	/* MSTOP register 8 */
#define	CPG_BUS_9_MSTOP_OFFSET				(0xD20)	/* MSTOP register 9 */
#define	CPG_BUS_10_MSTOP_OFFSET			(0xD24)	/* MSTOP register 10*/
#define	CPG_BUS_11_MSTOP_OFFSET			(0xD28)	/* MSTOP register 11 */
#define	CPG_BUS_12_MSTOP_OFFSET			(0xD2C)	/* MSTOP register 12 */
#define	CPG_RSV1_OFFSET					(0xE00)	/* Booking register 1 */
#define	CPG_RSV2_OFFSET					(0xE04)	/* Booking register 2 */
#define	CPG_RSV3_OFFSET					(0xE08)	/* Booking register 3 */
#define	CPG_RSV4_OFFSET					(0xE0C)	/* Booking register 4 */
#define	CPG_RSV5_OFFSET					(0xE10)	/* Booking register 5 */
#define	CPG_RSV6_OFFSET					(0xE14)	/* Booking register 6 */

#endif	/* __CPG_REGS_OFFSET_H__ */
