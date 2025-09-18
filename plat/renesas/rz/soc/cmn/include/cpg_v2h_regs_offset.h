/*
 * Copyright (c) 2025, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __CPG_V2H_REGS_OFFSET_H__
#define __CPG_V2H_REGS_OFFSET_H__

#define CPG_V2H_PLLCM33_MON                 (0x010) /* PLLCM33 monitor register */
#define CPG_V2H_PLLCLN_STBY                 (0x020) /* PLLCLN standby control register */
#define CPG_V2H_PLLCLN_MON                  (0x030) /* PLLCLN monitor register */
#define CPG_V2H_PLLDTY_STBY                 (0x040) /* PLLDTY standby control register */
#define CPG_V2H_PLLDTY_MON                  (0x050) /* PLLDTY monitor register */
#define CPG_V2H_PLLCA55_STBY                (0x060) /* PLLCA55 standby control register */
#define CPG_V2H_PLLCA55_CLK1                (0x064) /* PLLCA55 output clock setting register 1 */
#define CPG_V2H_PLLCA55_CLK2                (0x068) /* PLLCA55 output clock setting register 2 */
#define CPG_V2H_PLLCA55_MON                 (0x070) /* PLLCA55 monitor register */
#define CPG_V2H_PLLVDO_STBY                 (0x080) /* PLLVDO standby control register */
#define CPG_V2H_PLLVDO_MON                  (0x090) /* PLLVDO monitor register */
#define CPG_V2H_PLLETH_STBY                 (0x0A0) /* PLLETH standby control register */
#define CPG_V2H_PLLETH_MON                  (0x0B0) /* PLLETH monitor register */
#define CPG_V2H_PLLDSI_STBY                 (0x0C0) /* PLLDSI standby control register */
#define CPG_V2H_PLLDSI_CLK1                 (0x0C4) /* PLLDSI output clock setting register 1 */
#define CPG_V2H_PLLDSI_CLK2                 (0x0C8) /* PLLDSI output clock setting register 2 */
#define CPG_V2H_PLLDSI_MON                  (0x0D0) /* PLLDSI monitor register */
#define CPG_V2H_PLLDDR0_STBY                (0x0E0) /* PLLDDR0 standby control register */
#define CPG_V2H_PLLDDR0_MON                 (0x0F0) /* PLLDDR0 monitor register*/
#define CPG_V2H_PLLDDR1_STBY                (0x100) /* PLLDDR1standby control register*/
#define CPG_V2H_PLLDDR1_MON                 (0x110) /* PLLDDR1monitor register */
#define CPG_V2H_PLLGPU_STBY                 (0x120) /* PLLGPU standby control register */
#define CPG_V2H_PLLGPU_CLK1                 (0x124) /* PLLGPU output clock setting register 1 */
#define CPG_V2H_PLLGPU_CLK2                 (0x128) /* PLLGPU output clock setting register 2 */
#define CPG_V2H_PLLGPU_MON                  (0x130) /* PLLGPU monitor register */
#define CPG_V2H_PLLDRP_STBY                 (0x140) /* PLLDRP standby control register */
#define CPG_V2H_PLLDRP_CLK1                 (0x144) /* PLLDRP output clock setting register 1 */
#define CPG_V2H_PLLDRP_CLK2                 (0x148) /* PLLDRP output clock setting register 2 */
#define CPG_V2H_PLLDRP_MON                  (0x150) /* PLLDRP monitor register */
#define CCLMA0_CTL                          (0x200) /* CLMA0 control register */
#define CCLMA1_CTL                          (0x204) /* CLMA1 control register */
#define CCLMA2_CTL                          (0x208) /* CLMA2 control register */
#define CCLMA3_CTL                          (0x20C) /* CLMA3 control register */
#define CCLMA4_CTL                          (0x210) /* CLMA4 control register */
#define CCLMA5_CTL                          (0x214) /* CLMA5 control register */
#define CCLMA6_CTL                          (0x218) /* CLMA6 control register */
#define CCLMA7_CTL                          (0x21C) /* CLMA7 control register */
#define CCLMA8_CTL                          (0x220) /* CLMA8 control register */
#define CCLMA9_CTL                          (0x224) /* CLMA9 control register */
#define CCLMA10_CTL                         (0x228) /* CLMA10 control register */
#define CCLMA11_CTL                         (0x22C) /* CLMA11 control register */
#define CCLMA12_CTL                         (0x230) /* CLMA12 control register */
#define CCLMA13_CTL                         (0x234) /* CLMA13 control register */
#define CCLMA14_CTL                         (0x238) /* CLMA14 control register */
#define CCLMA_MON                           (0x280) /* CLMA monitor register */
#define CPG_V2H_SSEL0                       (0x300) /* Static Mux control */
#define CPG_V2H_SSEL1                       (0x304) /* Static Mux control */
#define CPG_V2H_SSEL2                       (0x308) /* Static Mux control */
#define CPG_V2H_CDDIV0                      (0x400) /* Dynamic Gear control(Counter type) */
#define CPG_V2H_CDDIV1                      (0x404) /* Dynamic Gear control(Counter type) */
#define CPG_V2H_CDDIV2                      (0x408) /* Dynamic Gear control(Counter type) */
#define CPG_V2H_CDDIV3                      (0x40c) /* Dynamic Gear control(Counter type) */
#define CPG_V2H_CDDIV4                      (0x410) /* Dynamic Gear control(Counter type) */
#define CPG_V2H_CSDIV0                      (0x500) /* Static Gear control(Sparse type) */
#define CPG_V2H_CSDIV1                      (0x504) /* Static Gear control(Sparse type) */
#define CPG_V2H_CLKON_0                     (0x600) /* CGC control */
#define CPG_V2H_CLKON_1                     (0x604) /* CGC control */
#define CPG_V2H_CLKON_2                     (0x608) /* CGC control */
#define CPG_V2H_CLKON_3                     (0x60c) /* CGC control */
#define CPG_V2H_CLKON_4                     (0x610) /* CGC control */
#define CPG_V2H_CLKON_5                     (0x614) /* CGC control */
#define CPG_V2H_CLKON_6                     (0x618) /* CGC control */
#define CPG_V2H_CLKON_7                     (0x61c) /* CGC control */
#define CPG_V2H_CLKON_8                     (0x620) /* CGC control */
#define CPG_V2H_CLKON_9                     (0x624) /* CGC control */
#define CPG_V2H_CLKON_10                    (0x628) /* CGC control */
#define CPG_V2H_CLKON_11                    (0x62c) /* CGC control */
#define CPG_V2H_CLKON_12                    (0x630) /* CGC control */
#define CPG_V2H_CLKON_13                    (0x634) /* CGC control */
#define CPG_V2H_CLKON_14                    (0x638) /* CGC control */
#define CPG_V2H_CLKON_15                    (0x63c) /* CGC control */
#define CPG_V2H_CLKON_16                    (0x640) /* CGC control */
#define CPG_V2H_CLKON_17                    (0x644) /* CGC control */
#define CPG_V2H_CLKON_18                    (0x648) /* CGC control */
#define CPG_V2H_CLKON_19                    (0x64c) /* CGC control */
#define CPG_V2H_CLKON_20                    (0x650) /* CGC control */
#define CPG_V2H_CLKON_21                    (0x654) /* CGC control */
#define CPG_V2H_CLKON_22                    (0x658) /* CGC control */
#define CPG_V2H_CLKON_23                    (0x65C) /* CGC control */
#define CPG_V2H_CLKON_24                    (0x660) /* CGC control */
#define CPG_V2H_CLKSTATUS0                  (0x700) /* Dynamic gear/mux status monitor */
#define CPG_V2H_CLKMON_0                    (0x800) /* CGC monitor */
#define CPG_V2H_CLKMON_1                    (0x804) /* CGC monitor */
#define CPG_V2H_CLKMON_2                    (0x808) /* CGC monitor */
#define CPG_V2H_CLKMON_3                    (0x80c) /* CGC monitor */
#define CPG_V2H_CLKMON_4                    (0x810) /* CGC monitor */
#define CPG_V2H_CLKMON_5                    (0x814) /* CGC monitor */
#define CPG_V2H_CLKMON_6                    (0x818) /* CGC monitor */
#define CPG_V2H_CLKMON_7                    (0x81c) /* CGC monitor */
#define CPG_V2H_CLKMON_8                    (0x820) /* CGC monitor */
#define CPG_V2H_CLKMON_9                    (0x824) /* CGC monitor */
#define CPG_V2H_CLKMON_10                   (0x828) /* CGC monitor */
#define CPG_V2H_RST_0                       (0x900) /* RESET ON/OFF control */
#define CPG_V2H_RST_1                       (0x904) /* RESET ON/OFF control */
#define CPG_V2H_RST_2                       (0x908) /* RESET ON/OFF control */
#define CPG_V2H_RST_3                       (0x90C) /* RESET ON/OFF control */
#define CPG_V2H_RST_4                       (0x910) /* RESET ON/OFF control */
#define CPG_V2H_RST_5                       (0x914) /* RESET ON/OFF control */
#define CPG_V2H_RST_6                       (0x918) /* RESET ON/OFF control */
#define CPG_V2H_RST_7                       (0x91C) /* RESET ON/OFF control */
#define CPG_V2H_RST_8                       (0x920) /* RESET ON/OFF control */
#define CPG_V2H_RST_9                       (0x924) /* RESET ON/OFF control */
#define CPG_V2H_RST_10                      (0x928) /* RESET ON/OFF control */
#define CPG_V2H_RST_11                      (0x92C) /* RESET ON/OFF control */
#define CPG_V2H_RST_12                      (0x930) /* RESET ON/OFF control */
#define CPG_V2H_RST_13                      (0x934) /* RESET ON/OFF control */
#define CPG_V2H_RST_14                      (0x938) /* RESET ON/OFF control */
#define CPG_V2H_RST_15                      (0x93C) /* RESET ON/OFF control */
#define CPG_V2H_RST_16                      (0x940) /* RESET ON/OFF control */
#define CPG_V2H_RST_17                      (0x944) /* RESET ON/OFF control */
#define CPG_V2H_RSTMON_0                    (0xA00) /* RESET monitor */
#define CPG_V2H_RSTMON_1                    (0xA04) /* RESET monitor */
#define CPG_V2H_RSTMON_2                    (0xA08) /* RESET monitor */
#define CPG_V2H_RSTMON_3                    (0xA0C) /* RESET monitor */
#define CPG_V2H_RSTMON_4                    (0xA10) /* RESET monitor */
#define CPG_V2H_RSTMON_5                    (0xA14) /* RESET monitor */
#define CPG_V2H_RSTMON_6                    (0xA18) /* RESET monitor */
#define CPG_V2H_RSTMON_7                    (0xA1C) /* RESET monitor */
#define CPG_V2H_RSTMON_8                    (0xA20) /* RESET monitor */
#define CPG_V2H_ERRORRST_SEL1               (0xB00) /* Error reset selection register */
#define CPG_V2H_ERRORRST_SEL2               (0xB04) /* Error reset selection register */
#define CPG_V2H_ERRORRST_SEL3               (0xB08) /* Error reset selection register */
#define CPG_V2H_ERRORRST_SEL4               (0xB0C) /* Error reset selection register */
#define CPG_V2H_ERRORRST_SEL5               (0xB10) /* Error reset selection register */
#define CPG_V2H_ERRORRST_SEL6               (0xB14) /* Error reset selection register */
#define CPG_V2H_ERRORRST_SEL7               (0xB18) /* Error reset selection register */
#define CPG_V2H_ERRORRST_SEL8               (0xB1C) /* Error reset selection register */
#define CPG_V2H_ERROR_RST2                  (0xB40) /* Error reset register */
#define CPG_V2H_ERROR_RST3                  (0xB44) /* Error reset register */
#define CPG_V2H_ERROR_RST4                  (0xB48) /* Error reset register */
#define CPG_V2H_ERROR_RST5                  (0xB4C) /* Error reset register */
#define CPG_V2H_ERROR_RST6                  (0xB50) /* Error reset register */
#define CPG_V2H_ERROR_RST7                  (0xB54) /* Error reset register */
#define CPG_V2H_ERROR_RST8                  (0xB58) /* Error reset register */
#define CPG_V2H_LP_CTL1                     (0xC00) /* Lowpower Sequence Control Register 1 */
#define CPG_V2H_LP_CTL2                     (0xC04) /* Lowpower Sequence Control Register 2 */
#define CPG_V2H_LP_GPU_CTL                  (0xC08) /* GPU Lowpower Sequence Control Register */
#define CPG_V2H_CM33_CTL                    (0xC0C) /* CM33 Control register */
#define CPG_V2H_CR8_CORESTATUS              (0xC10) /* CR8 core status register */
#define CPG_V2H_CR8_CONFIG1                 (0xC14) /* CR8 core configuration register1 */
#define CPG_V2H_LP_CM33CTL0                 (0xC18) /* Lowpower Sequence Cortex-M33 Control Register 0 */
#define CPG_V2H_LP_CM33CTL1                 (0xC1C) /* Lowpower Sequence Cortex-M33 Control Register 1 */
#define CPG_V2H_LP_CA55_CTL1                (0xC20) /* Cortex-A55 Clock Control Register 1 */
#define CPG_V2H_LP_CA55_CTL2                (0xC24) /* Cortex-A55 Clock Control Register 2 */
#define CPG_V2H_LP_CA55_CTL3                (0xC28) /* Cortex-A55 Clock Control Register 3 */
#define CPG_V2H_LP_CA55_CTL4                (0xC2C) /* Cortex-A55 Clock Control Register 4 */
#define CPG_V2H_LP_CA55_CTL5                (0xC30) /* Cortex-A55 Clock Control Register 5 */
#define CPG_V2H_LP_CA55_CTL6                (0xC34) /* Cortex-A55 Clock Control Register 6 */
#define CPG_V2H_LP_CA55_CTL7                (0xC38) /* Cortex-A55 Clock Control Register 7 */
#define CPG_V2H_LP_CR8_CTL1                 (0xC3C) /* CR8SS control register 1 */
#define CPG_V2H_LP_CR8_CTL3                 (0xC44) /* CR8SS control register 3 */
#define CPG_V2H_LP_CR8_CTL4                 (0xC48) /* CR8SS control register 4 */
#define CPG_V2H_LP_PMU_CTL1                 (0xC4C) /* Lowpower Sequence Control Register @ */
#define CPG_V2H_LP_SRAM_STBY_CTL1           (0xC50) /* DRP SRAM standby control */
#define CPG_V2H_LP_SRAM_STBY_CTL2           (0xC54) /* Shared SRAM standby control 0 */
#define CPG_V2H_LP_SRAM_STBY_CTL3           (0xC58) /* Shared SRAM standby control 8 */
#define CPG_V2H_LP_GIC_CTL1                 (0xC5C) /* GIC control */
#define CPG_V2H_LP_DDR_CTL1                 (0xC60) /* DDR retention control */
#define CPG_V2H_LP_OTP_CTL1                 (0xC64) /* OTP control register 1 */
#define CPG_V2H_LP_CST_CTL1                 (0xC6C) /* CST control register 1 */
#define CPG_V2H_LP_CST_CTL2                 (0xC70) /* CST control register 2 */
#define CPG_V2H_LP_CST_CTL3                 (0xC74) /* CST control register 3 */
#define CPG_V2H_LP_PWC_CTL1                 (0xC78) /* PWC control register 1 */
#define CPG_V2H_LP_PWC_CTL2                 (0xC7C) /* PWC control register 2 */
#define CPG_V2H_OSTMTCKE                    (0xC80) /* OSTM enable control */
#define CPG_V2H_DBGRST                      (0xC84) /* Reset control register when CA55 is debugged */
#define OTP_HANDSHAKE_MON                   (0xC88) /* OTP handshake monitor register */
#define CPG_V2H_OTHERS_INI                  (0xC8C) /* Others area reset control */
#define CPG_V2H_BUS_1_MSTOP                 (0xD00) /* MSTOP register 1 */
#define CPG_V2H_BUS_2_MSTOP                 (0xD04) /* MSTOP register 2 */
#define CPG_V2H_BUS_3_MSTOP                 (0xD08) /* MSTOP register 3 */
#define CPG_V2H_BUS_4_MSTOP                 (0xD0C) /* MSTOP register 4 */
#define CPG_V2H_BUS_5_MSTOP                 (0xD10) /* MSTOP register 5 */
#define CPG_V2H_BUS_6_MSTOP                 (0xD14) /* MSTOP register 6 */
#define CPG_V2H_BUS_7_MSTOP                 (0xD18) /* MSTOP register 7 */
#define CPG_V2H_BUS_8_MSTOP                 (0xD1C) /* MSTOP register 8 */
#define CPG_V2H_BUS_9_MSTOP                 (0xD20) /* MSTOP register 9 */
#define CPG_V2H_BUS_10_MSTOP                (0xD24) /* MSTOP register 10*/
#define CPG_V2H_BUS_11_MSTOP                (0xD28) /* MSTOP register 11 */
#define CPG_V2H_BUS_12_MSTOP                (0xD2C) /* MSTOP register 12 */
#define CPG_V2H_RSV1                        (0xE00) /* Booking register 1 */
#define CPG_V2H_RSV2                        (0xE04) /* Booking register 2 */
#define CPG_V2H_RSV3                        (0xE08) /* Booking register 3 */
#define CPG_V2H_RSV4                        (0xE0C) /* Booking register 4 */
#define CPG_V2H_RSV5                        (0xE10) /* Booking register 5 */
#define CPG_V2H_RSV6                        (0xE14) /* Booking register 6 */

#define CPG_ERRORRST_SELx_ERRRSTSEL0        (1UL << 0)
#define CPG_ERRORRST_SELx_ERRRSTSEL1        (1UL << 1)
#define CPG_ERRORRST_SELx_ERRRSTSEL2        (1UL << 2)
#define CPG_ERRORRST_SELx_ERRRSTSEL3        (1UL << 3)

#define CPG_BUS_1_MSTOP_WDT1                (1UL << 0)
#define CPG_BUS_1_MSTOP_RIIC0               (1UL << 1)
#define CPG_BUS_1_MSTOP_RIIC1               (1UL << 2)
#define CPG_BUS_1_MSTOP_RIIC2               (1UL << 3)
#define CPG_BUS_1_MSTOP_RIIC3               (1UL << 4)
#define CPG_BUS_1_MSTOP_RIIC4               (1UL << 5)
#define CPG_BUS_1_MSTOP_RIIC5               (1UL << 6)
#define CPG_BUS_1_MSTOP_RIIC6               (1UL << 7)
#define CPG_BUS_1_MSTOP_RIIC7               (1UL << 8)
#define CPG_BUS_1_MSTOP_SPDIF0              (1UL << 9)
#define CPG_BUS_1_MSTOP_SPDIF1              (1UL << 10)
#define CPG_BUS_1_MSTOP_SPDIF2              (1UL << 11)
#define CPG_BUS_1_MSTOP_TMZ400_SRAM2        (1UL << 12)
#define CPG_BUS_1_MSTOP_TZC400_PCIE0        (1UL << 13)
#define CPG_BUS_1_MSTOP_TZC400_ACPU_RCPU    (1UL << 14)
#define CPG_BUS_1_MSTOP_TZC400_PCIE1        (1UL << 15)

#define CPG_BUS_2_MSTOP_SCU                 (1UL << 0)
#define CPG_BUS_2_MSTOP_SCU_DMAC            (1UL << 1)
#define CPG_BUS_2_MSTOP_ADG                 (1UL << 2)
#define CPG_BUS_2_MSTOP_SSIU                (1UL << 3)
#define CPG_BUS_2_MSTOP_SSIU_DMAC           (1UL << 4)
#define CPG_BUS_2_MSTOP_ADMAC               (1UL << 5)
#define CPG_BUS_2_MSTOP_TZC400_DDR00        (1UL << 6)
#define CPG_BUS_2_MSTOP_TZC400_DDR01        (1UL << 7)
#define CPG_BUS_2_MSTOP_TZC400_DDR10        (1UL << 8)
#define CPG_BUS_2_MSTOP_TZC400_DDR11        (1UL << 9)
#define CPG_BUS_2_MSTOP_TZC400_AXI_RCPU     (1UL << 10)
#define CPG_BUS_2_MSTOP_TZC400_SRAMA        (1UL << 11)
#define CPG_BUS_2_MSTOP_TZC400_PCIE         (1UL << 12)
#define CPG_BUS_2_MSTOP_GTM2                (1UL << 13)
#define CPG_BUS_2_MSTOP_GTM3                (1UL << 14)
#define CPG_BUS_2_MSTOP_TSU1                (1UL << 15)

#define CPG_BUS_3_MSTOP_SYC                 (1UL << 0)
#define CPG_BUS_3_MSTOP_SRAM2_REG           (1UL << 1)
#define CPG_BUS_3_MSTOP_DMAC1               (1UL << 2)
#define CPG_BUS_3_MSTOP_DMAC2               (1UL << 3)
#define CPG_BUS_3_MSTOP_GE3D                (1UL << 4)
#define CPG_BUS_3_MSTOP_GIC                 (1UL << 5)
#define CPG_BUS_3_MSTOP_GPV_ACPU            (1UL << 6)
#define CPG_BUS_3_MSTOP_GPV_ACPU_REG0       (1UL << 7)
#define CPG_BUS_3_MSTOP_GPV_ACPU_REG1       (1UL << 8)
#define CPG_BUS_3_MSTOP_ADC                 (1UL << 9)
#define CPG_BUS_3_MSTOP_WDT0                (1UL << 10)
#define CPG_BUS_3_MSTOP_RTC_P0              (1UL << 11)
#define CPG_BUS_3_MSTOP_RTC_P1              (1UL << 12)
#define CPG_BUS_3_MSTOP_RIIC8               (1UL << 13)
#define CPG_BUS_3_MSTOP_SCIF                (1UL << 14)
#define CPG_BUS_3_MSTOP_CMTW0               (1UL << 15)

#define CPG_BUS_4_MSTOP_CMTW1               (1UL << 0)
#define CPG_BUS_4_MSTOP_CMTW2               (1UL << 1)
#define CPG_BUS_4_MSTOP_CMTW3               (1UL << 2)
#define CPG_BUS_4_MSTOP_SRAM0               (1UL << 3)
#define CPG_BUS_4_MSTOP_SRAM1               (1UL << 4)
#define CPG_BUS_4_MSTOP_XSPI                (1UL << 5)

/* Bit6 is reserved */
#define CPG_BUS_4_MSTOP_PFC                 (1UL << 7)

/* Bit8 is reserved */
/* Bit9 is reserved */
#define CPG_BUS_4_MSTOP_SECURE_IP_P0        (1UL << 10)
#define CPG_BUS_4_MSTOP_SECURE_IP_P1        (1UL << 11)
#define CPG_BUS_4_MSTOP_TZC400_SRAMM        (1UL << 12)
#define CPG_BUS_4_MSTOP_TZC400_XSPI         (1UL << 13)
#define CPG_BUS_4_MSTOP_MHU                 (1UL << 14)

/* Bit15 is reserved */

/* Bit0 is reserved */
#define CPG_BUS_5_MSTOP_CST                 (1UL << 1)
#define CPG_BUS_5_MSTOP_TSU0                (1UL << 2)
#define CPG_BUS_5_MSTOP_SRAM0_REG           (1UL << 3)
#define CPG_BUS_5_MSTOP_SRAM1_REG           (1UL << 4)
#define CPG_BUS_5_MSTOP_XSPI_REG            (1UL << 5)
#define CPG_BUS_5_MSTOP_PDM0                (1UL << 6)
#define CPG_BUS_5_MSTOP_PDM1                (1UL << 7)
#define CPG_BUS_5_MSTOP_GPV_MCPU            (1UL << 8)
#define CPG_BUS_5_MSTOP_DMAC0               (1UL << 9)
#define CPG_BUS_5_MSTOP_GTM0                (1UL << 10)
#define CPG_BUS_5_MSTOP_GTM1                (1UL << 11)
#define CPG_BUS_5_MSTOP_WDT2                (1UL << 12)
#define CPG_BUS_5_MSTOP_WDT3                (1UL << 13)
#define CPG_BUS_5_MSTOP_CRC                 (1UL << 14)
#define CPG_BUS_5_MSTOP_CMTW4               (1UL << 15)

#define CPG_BUS_6_MSTOP_CMTW5               (1UL << 0)
#define CPG_BUS_6_MSTOP_CMTW6               (1UL << 1)
#define CPG_BUS_6_MSTOP_CMTW7               (1UL << 2)
#define CPG_BUS_6_MSTOP_POEG0A              (1UL << 3)
#define CPG_BUS_6_MSTOP_POEG0B              (1UL << 4)
#define CPG_BUS_6_MSTOP_POEG0C              (1UL << 5)
#define CPG_BUS_6_MSTOP_POEG0D              (1UL << 6)
#define CPG_BUS_6_MSTOP_POEG1A              (1UL << 7)
#define CPG_BUS_6_MSTOP_POEG1B              (1UL << 8)
#define CPG_BUS_6_MSTOP_POEG1C              (1UL << 9)
#define CPG_BUS_6_MSTOP_POEG1D              (1UL << 10)
#define CPG_BUS_6_MSTOP_GPT0                (1UL << 11)
#define CPG_BUS_6_MSTOP_GPT1                (1UL << 12)
#define CPG_BUS_6_MSTOP_DDR0_P0             (1UL << 13)
#define CPG_BUS_6_MSTOP_DDR0_P1             (1UL << 14)
#define CPG_BUS_6_MSTOP_DDR0_P2             (1UL << 15)

#define CPG_BUS_7_MSTOP_DDR_0_P3            (1UL << 0)
#define CPG_BUS_7_MSTOP_DDR_0_P4            (1UL << 1)
#define CPG_BUS_7_MSTOP_DDR_1_P0            (1UL << 2)
#define CPG_BUS_7_MSTOP_DDR_1_P1            (1UL << 3)
#define CPG_BUS_7_MSTOP_DDR_1_P2            (1UL << 4)
#define CPG_BUS_7_MSTOP_DDR_1_P3            (1UL << 5)
#define CPG_BUS_7_MSTOP_DDR_1_P4            (1UL << 6)
#define CPG_BUS_7_MSTOP_USB20_HOST          (1UL << 7)
#define CPG_BUS_7_MSTOP_USB21_HOST          (1UL << 8)
#define CPG_BUS_7_MSTOP_USB2_FUNC           (1UL << 9)
#define CPG_BUS_7_MSTOP_USB20_PHY           (1UL << 10)
#define CPG_BUS_7_MSTOP_USB21_PHY           (1UL << 11)
#define CPG_BUS_7_MSTOP_USB30_HOST          (1UL << 12)
#define CPG_BUS_7_MSTOP_USB31_HOST          (1UL << 13)
#define CPG_BUS_7_MSTOP_USB30_PHY           (1UL << 14)
#define CPG_BUS_7_MSTOP_USB31_PHY           (1UL << 15)

#define CPG_BUS_8_MSTOP_PCIE_PHY            (1UL << 0)
#define CPG_BUS_8_MSTOP_GPV_COM_SUB         (1UL << 1)
#define CPG_BUS_8_MSTOP_SD0                 (1UL << 2)
#define CPG_BUS_8_MSTOP_SD1                 (1UL << 3)
#define CPG_BUS_8_MSTOP_SD2                 (1UL << 4)
#define CPG_BUS_8_MSTOP_GBETH0              (1UL << 5)
#define CPG_BUS_8_MSTOP_GBETH1              (1UL << 6)
#define CPG_BUS_8_MSTOP_GPV_COM             (1UL << 7)
#define CPG_BUS_8_MSTOP_DRP_AI_MAC          (1UL << 8)
#define CPG_BUS_8_MSTOP_DRP_AP_DRP0         (1UL << 9)
#define CPG_BUS_8_MSTOP_DRP1                (1UL << 10)
#define CPG_BUS_8_MSTOP_GPV_DRP             (1UL << 11)
#define CPG_BUS_8_MSTOP_SRAM4_REG           (1UL << 12)
#define CPG_BUS_8_MSTOP_SRAM5_REG           (1UL << 13)
#define CPG_BUS_8_MSTOP_SRAM6_REG           (1UL << 14)
#define CPG_BUS_8_MSTOP_SRAM7_REG           (1UL << 15)

#define CPG_BUS_9_MSTOP_SRAM8_REG           (1UL << 0)
#define CPG_BUS_9_MSTOP_SRAM9_REG           (1UL << 1)
#define CPG_BUS_9_MSTOP_SRAM10_REG          (1UL << 2)
#define CPG_BUS_9_MSTOP_SRAM11_REG          (1UL << 3)
#define CPG_BUS_9_MSTOP_CRU0                (1UL << 4)
#define CPG_BUS_9_MSTOP_CRU1                (1UL << 5)
#define CPG_BUS_9_MSTOP_CRU2                (1UL << 6)
#define CPG_BUS_9_MSTOP_CRU3                (1UL << 7)
#define CPG_BUS_9_MSTOP_ISP_APB             (1UL << 8)
#define CPG_BUS_9_MSTOP_ISP_AXI             (1UL << 9)
#define CPG_BUS_9_MSTOP_GPV_VIDEO0          (1UL << 10)
#define CPG_BUS_9_MSTOP_VCD_P0              (1UL << 11)
#define CPG_BUS_9_MSTOP_VCD_P1              (1UL << 12)
#define CPG_BUS_9_MSTOP_VCD_P2              (1UL << 13)
#define CPG_BUS_9_MSTOP_DSI_LINK            (1UL << 14)
#define CPG_BUS_9_MSTOP_DSI_PHY             (1UL << 15)

#define CPG_BUS_10_MSTOP_ISU                (1UL << 0)
#define CPG_BUS_10_MSTOP_LCDC_DU            (1UL << 1)
#define CPG_BUS_10_MSTOP_LCDC_FCPVD         (1UL << 2)
#define CPG_BUS_10_MSTOP_LCDC_VSPD          (1UL << 3)
#define CPG_BUS_10_MSTOP_GPV_VIDEO1         (1UL << 4)
#define CPG_BUS_10_MSTOP_DDR0_PHY           (1UL << 5)
#define CPG_BUS_10_MSTOP_DDR1_PHY           (1UL << 6)
#define CPG_BUS_10_MSTOP_DDR0_CTRL          (1UL << 7)
#define CPG_BUS_10_MSTOP_DDR1_CTRL          (1UL << 8)
#define CPG_BUS_10_MSTOP_SRAM3              (1UL << 9)
#define CPG_BUS_10_MSTOP_CR8_TCM            (1UL << 10)
#define CPG_BUS_10_MSTOP_DMAC3              (1UL << 11)
#define CPG_BUS_10_MSTOP_DMAC4              (1UL << 12)
#define CPG_BUS_10_MSTOP_SRAM3_REG          (1UL << 13)
#define CPG_BUS_10_MSTOP_CANFD              (1UL << 14)
#define CPG_BUS_10_MSTOP_I3C0               (1UL << 15)

#define CPG_BUS_11_MSTOP_RSPI0              (1UL << 0)
#define CPG_BUS_11_MSTOP_RSPI1              (1UL << 1)
#define CPG_BUS_11_MSTOP_RSPI2              (1UL << 2)
#define CPG_BUS_11_MSTOP_RSCI0              (1UL << 3)
#define CPG_BUS_11_MSTOP_RSCI1              (1UL << 4)
#define CPG_BUS_11_MSTOP_RSCI2              (1UL << 5)
#define CPG_BUS_11_MSTOP_RSCI3              (1UL << 6)
#define CPG_BUS_11_MSTOP_RSCI4              (1UL << 7)
#define CPG_BUS_11_MSTOP_RSCI5              (1UL << 8)
#define CPG_BUS_11_MSTOP_RSCI6              (1UL << 9)
#define CPG_BUS_11_MSTOP_RSCI7              (1UL << 10)
#define CPG_BUS_11_MSTOP_RSCI8              (1UL << 11)
#define CPG_BUS_11_MSTOP_RSCI9              (1UL << 12)
#define CPG_BUS_11_MSTOP_GTM4               (1UL << 13)
#define CPG_BUS_11_MSTOP_GTM5               (1UL << 14)
#define CPG_BUS_11_MSTOP_GTM6               (1UL << 15)

#define CPG_BUS_12_MSTOP_GTM7               (1UL << 0)
#define CPG_BUS_12_MSTOP_SRAM4              (1UL << 1)
#define CPG_BUS_12_MSTOP_SRAM5              (1UL << 2)
#define CPG_BUS_12_MSTOP_SRAM6              (1UL << 3)
#define CPG_BUS_12_MSTOP_SRAM7              (1UL << 4)
#define CPG_BUS_12_MSTOP_SRAM8              (1UL << 5)
#define CPG_BUS_12_MSTOP_SRAM9              (1UL << 6)
#define CPG_BUS_12_MSTOP_SRAM10             (1UL << 7)
#define CPG_BUS_12_MSTOP_SRAM11             (1UL << 8)
#define CPG_BUS_12_MSTOP_MCPU_TO_ACPU       (1UL << 9)
#define CPG_BUS_12_MSTOP_ACPU_TO_MCPU       (1UL << 10)

#define CPG_LP_PWC_CTL1_ALL_OFF_TRG			(1UL << 3)

#define CPG_LP_CTL1_STBY					(0x00000001UL)
#define CPG_LP_CTL1_STBY_MSK				(0x00000001UL)
#define CPG_LP_CTL1_CA55SLEEP_REQ			(0x00000100UL)
#define CPG_LP_CTL1_CA55SLEEP_REQ_MSK		(0x00000F00UL)
#define CPG_LP_CTL1_STBY_CA55ST				(0x00010000UL)
#define CPG_LP_CTL1_STBY_CA55ST_MSK			(0x00010000UL)
#define CPG_LP_CTL1_CA55SLEEP_ACK			(0x01000000UL)
#define CPG_LP_CTL1_CA55SLEEP_ACK_MSK		(0x0F000000UL)

#define CPG_LP_CA55_CTL2_COREPREQ0			(1UL << 0)
#define CPG_LP_CA55_CTL2_COREPREQ1			(1UL << 16)
#define CPG_LP_CA55_CTL3_COREPREQ2			(1UL << 0)
#define CPG_LP_CA55_CTL3_COREPREQ3			(1UL << 16)

#define CPG_LP_CA55_CTL2_CORESTATE0			(1UL << 1)
#define CPG_LP_CA55_CTL2_CORESTATE1			(1UL << 17)
#define CPG_LP_CA55_CTL3_CORESTATE2			(1UL << 1)
#define CPG_LP_CA55_CTL3_CORESTATE3			(1UL << 17)

#define CPG_LP_CA55_CTL2_CORESTATE0_ON_MASK	(1UL << 4)
#define CPG_LP_CA55_CTL2_CORESTATE1_ON_MASK	(1UL << 20)
#define CPG_LP_CA55_CTL3_CORESTATE2_ON_MASK	(1UL << 4)
#define CPG_LP_CA55_CTL3_CORESTATE3_ON_MASK	(1UL << 20)

#define CPG_LP_CA55_CTL2_COREACCEPT0		(1UL << 8)
#define CPG_LP_CA55_CTL2_COREACCEPT1		(1UL << 24)
#define CPG_LP_CA55_CTL3_COREACCEPT2		(1UL << 8)
#define CPG_LP_CA55_CTL3_COREACCEPT3		(1UL << 24)

#endif                                 /* __CPG_V2H_REGS_OFFSET_H__ */
