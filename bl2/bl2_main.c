/*
 * Copyright (c) 2013-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <arch_helpers.h>
#include <arch_features.h>
#include <bl1/bl1.h>
#include <bl2/bl2.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <drivers/auth/auth_mod.h>
#include <drivers/auth/crypto_mod.h>
#include <drivers/console.h>
#include <drivers/fwu/fwu.h>
#include <lib/extensions/pauth.h>
#include <plat/common/platform.h>

#include <lib/mmio.h>
#include "bl2_private.h"
#include <drivers/delay_timer.h>

#ifdef __aarch64__
#define NEXT_IMAGE	"BL31"
#else
#define NEXT_IMAGE	"BL32"
#endif

#if RESET_TO_BL2
/*******************************************************************************
 * Setup function for BL2 when RESET_TO_BL2=1
 ******************************************************************************/
void bl2_el3_setup(u_register_t arg0, u_register_t arg1, u_register_t arg2,
		   u_register_t arg3)
{
	/* Perform early platform-specific setup */
	bl2_el3_early_platform_setup(arg0, arg1, arg2, arg3);

	/* Perform late platform-specific setup */
	bl2_el3_plat_arch_setup();

#if CTX_INCLUDE_PAUTH_REGS
	/*
	 * Assert that the ARMv8.3-PAuth registers are present or an access
	 * fault will be triggered when they are being saved or restored.
	 */
	assert(is_armv8_3_pauth_present());
#endif /* CTX_INCLUDE_PAUTH_REGS */
}
#else /* RESET_TO_BL2 */

/*******************************************************************************
 * Setup function for BL2 when RESET_TO_BL2=0
 ******************************************************************************/
void bl2_setup(u_register_t arg0, u_register_t arg1, u_register_t arg2,
	       u_register_t arg3)
{
	/* Perform early platform-specific setup */
	bl2_early_platform_setup2(arg0, arg1, arg2, arg3);

	/* Perform late platform-specific setup */
	bl2_plat_arch_setup();

#if CTX_INCLUDE_PAUTH_REGS
	/*
	 * Assert that the ARMv8.3-PAuth registers are present or an access
	 * fault will be triggered when they are being saved or restored.
	 */
	assert(is_armv8_3_pauth_present());
#endif /* CTX_INCLUDE_PAUTH_REGS */
}
#endif /* RESET_TO_BL2 */

/*******************************************************************************
 * DEFINE CM33
 ******************************************************************************/
#define CPG_SIPLL3_MON                                  (0x1101013C)    // PLL3 (SSCG) Monitor Register
#define CPG_CLKON_CM33                                  (0x11010504)    // Clock Control Register Cortex-M33
#define CPG_CLKMON_CM33                                 (0x11010684)    // Clock Monitor Register Cortex-M33
#define CPG_RST_CM33                                    (0x11010804)    // Reset Control Register Cortex-M33
#define CPG_RSTMON_CM33                                 (0x11010984)    // Reset Monitor Register Cortex-M33

#define SYS_CM33_CFG0                                   (0x11020804)    // CM33 Config Register0
#define SYS_CM33_CFG1                                   (0x11020808)    // CM33 Config Register1
#define SYS_CM33_CFG2                                   (0x1102080C)    // CM33 Config Register2
#define SYS_CM33_CFG3                                   (0x11020810)    // CM33 Config Register3
#define SYS_CM33_CTL                                    (0x11020818)    // CM33 Control Register
#define SYS_LSI_MODE                                    (0x11020A00)    // LSI Mode Signal Register
#define SYS_LP_CM33CTL1                                 (0x11020D28)    // Lowpower Sequence CM33 Control Register1

void write_register(uintptr_t addr, uint32_t value)
{
        mmio_write_32(addr, value);
        if(mmio_read_32(addr) != value){
                INFO("BL2: Write register addr = 0x%lx <-- value = 0x%x - failed\n", addr, value);
        }
        else {
                INFO("BL2: Write register addr = 0x%lx <-- value = 0x%x - passed\n", addr, value);
        }
}

void cm33_boot_normal_mode()
{
        // Supply clock to CM33_CLKIN
        write_register(CPG_CLKON_CM33 , 0x00010001);

        // Poll CPG_CLKMON_CM33 to confirm that CM33_CLKIN clock is supplied
        while (mmio_read_32(CPG_CLKMON_CM33) != 0x1)
                mdelay(10);

        // Stop the reset signals (released from the reset state)
        write_register(CPG_RST_CM33 , 0x00070007);

        // Poll CPG_RSTMON_CM33 to confirm that all the reset signals are not applied
        while(mmio_read_32(CPG_RSTMON_CM33) != 0)
                mdelay(10);
}

void cm33_boot_debug_mode()
{
        // Supply clock to CM33_TSCLK and CM33_CLKIN
        write_register(CPG_CLKON_CM33 , 0x00030003);

        // Poll CPG_CLKMON_CM33 to confirm that both CM33_TSCLK and CM33_CLKIN clock are supplied
        while (mmio_read_32(CPG_CLKMON_CM33) != 0x3)
                mdelay(10);

        // Set DEBUGQREQn bit of SYS_LP_CM33CTL1 to 1
        write_register(SYS_LP_CM33CTL1 , 0x00001100);

        // Poll SYS_LP_CM33CTL1 to check if DEBUGQACCEPTn bit becomes 1
        // Fixme. Lacking of SYS_LP_CM33CTL1.DEBUGQACCEPTn info

        // Set FETCHCNT bit of SYS_CM33_CTL register to 1
        write_register(SYS_CM33_CTL , 0x00000001);

        // Stop the reset signals (released from the reset state)
        write_register(CPG_RST_CM33 , 0x00070007);

        // Poll CPG_RSTMON_CM33 to confirm that all the reset signals are not applied
        while (mmio_read_32(CPG_RSTMON_CM33) != 0)
                mdelay(10);

        // Set FETCHCNT bit of SYS_CM33_CTL register to 0
        write_register(SYS_CM33_CTL , 0x00000000);
}

void cm33_start(unsigned char debug, uint32_t s_addr, uint32_t ns_addr)
{
        // Check if the SSCG PLL3 is ON or not
        if ((CPG_SIPLL3_MON & 0x1) == 0x1) {
                write_register(SYS_CM33_CFG0 , 0x00103CE5);
                write_register(SYS_CM33_CFG1 , 0x00103CE5);
        } else {
                write_register(SYS_CM33_CFG0 , 0x00003D08);
                write_register(SYS_CM33_CFG1 , 0x00003D08);
        }

        // Set the secure vector address of Cortex-M33
        write_register(SYS_CM33_CFG2 , s_addr);

        // Set the non secure vector address of Cortex-M33
        write_register(SYS_CM33_CFG3 , ns_addr);

        // Start the CM33 propram in normal/debug mode
		cm33_boot_normal_mode();
}

void kick_cm33() {
    cm33_start(1, 0x1001FF80, 0x00010000);
}

/*******************************************************************************
 * The only thing to do in BL2 is to load further images and pass control to
 * next BL. The memory occupied by BL2 will be reclaimed by BL3x stages. BL2
 * runs entirely in S-EL1.
 ******************************************************************************/
void bl2_main(void)
{
	entry_point_info_t *next_bl_ep_info;

	NOTICE("BL2: %s\n", version_string);
	NOTICE("BL2: %s\n", build_message);

	/* Perform remaining generic architectural setup in S-EL1 */
	bl2_arch_setup();

#if PSA_FWU_SUPPORT
	fwu_init();
#endif /* PSA_FWU_SUPPORT */
	crypto_mod_init();

	/* Initialize authentication module */
	auth_mod_init();

	/* Initialize the Measured Boot backend */
	bl2_plat_mboot_init();

	/* Initialize boot source */
	bl2_plat_preload_setup();

	/* Load the subsequent bootloader images. */
	next_bl_ep_info = bl2_load_images();

	/* Kick CM33 */
	kick_cm33();

	/* Teardown the Measured Boot backend */
	bl2_plat_mboot_finish();

#if !BL2_RUNS_AT_EL3
#ifndef __aarch64__
	/*
	 * For AArch32 state BL1 and BL2 share the MMU setup.
	 * Given that BL2 does not map BL1 regions, MMU needs
	 * to be disabled in order to go back to BL1.
	 */
	disable_mmu_icache_secure();
#endif /* !__aarch64__ */

	console_flush();

#if ENABLE_PAUTH
	/*
	 * Disable pointer authentication before running next boot image
	 */
	pauth_disable_el1();
#endif /* ENABLE_PAUTH */

	/*
	 * Run next BL image via an SMC to BL1. Information on how to pass
	 * control to the BL32 (if present) and BL33 software images will
	 * be passed to next BL image as an argument.
	 */
	smc(BL1_SMC_RUN_IMAGE, (unsigned long)next_bl_ep_info, 0, 0, 0, 0, 0, 0);
#else /* if BL2_RUNS_AT_EL3 */

	NOTICE("BL2: Booting " NEXT_IMAGE "\n");
	print_entry_point_info(next_bl_ep_info);
	console_flush();

#if ENABLE_PAUTH
	/*
	 * Disable pointer authentication before running next boot image
	 */
	pauth_disable_el3();
#endif /* ENABLE_PAUTH */

	bl2_run_next_image(next_bl_ep_info);
#endif /* BL2_RUNS_AT_EL3 */
}
