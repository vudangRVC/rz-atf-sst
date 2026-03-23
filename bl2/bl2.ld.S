/*
 * Copyright (c) 2013-2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/bl_common.ld.h>
#include <lib/xlat_tables/xlat_tables_defs.h>

OUTPUT_FORMAT(PLATFORM_LINKER_FORMAT)
OUTPUT_ARCH(PLATFORM_LINKER_ARCH)
ENTRY(bl2_entrypoint)

MEMORY {
#if ENABLE_PIE
    RAM (rwx): ORIGIN = RZG2L_BL2_BASE, LENGTH = (RZCMN_RAM_MAX_SIZE)
#else
    RAM (rwx): ORIGIN = BL2_BASE, LENGTH = BL2_LIMIT - BL2_BASE
#endif
}

SECTIONS {
    RAM_REGION_START = ORIGIN(RAM);
    RAM_REGION_LENGTH = LENGTH(RAM);
#if ENABLE_PIE
    . = 0x0;  /* Start at zero for PIE */
#else
    . = BL2_BASE;

    ASSERT(. == ALIGN(PAGE_SIZE),
        "BL2_BASE address is not aligned on a page boundary.")
#endif

#if SEPARATE_CODE_AND_RODATA
    .text . : {
        ASSERT(. == ALIGN(PAGE_SIZE),
        ".text address is not aligned on a page boundary.");

        __TEXT_START__ = .;

        *bl2_entrypoint.o(.text*)

        *(SORT_BY_ALIGNMENT(.text*))
        *(.vectors)
        __TEXT_END_UNALIGNED__ = .;

        . = ALIGN(PAGE_SIZE);

        __TEXT_END__ = .;
    } >RAM

    /* .ARM.extab and .ARM.exidx are only added because Clang needs them */
    .ARM.extab . : {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } >RAM

    .ARM.exidx . : {
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    } >RAM

    .rodata . : {
        __RODATA_START__ = .;

        *(SORT_BY_ALIGNMENT(.rodata*))

        RODATA_COMMON

        __RODATA_END_UNALIGNED__ = .;
        . = ALIGN(PAGE_SIZE);

        __RODATA_END__ = .;
    } >RAM
#else /* SEPARATE_CODE_AND_RODATA */
    .ro . : {
        ASSERT(. == ALIGN(PAGE_SIZE),
        ".ro address is not aligned on a page boundary.");

        __RO_START__ = .;

        *bl2_entrypoint.o(.text*)
        *(SORT_BY_ALIGNMENT(.text*))
        *(SORT_BY_ALIGNMENT(.rodata*))

        RODATA_COMMON

        *(.vectors)

        __RO_END_UNALIGNED__ = .;

        /*
         * Memory page(s) mapped to this section will be marked as read-only,
         * executable. No RW data from the next section must creep in. Ensure
         * that the rest of the current memory page is unused.
         */
        . = ALIGN(PAGE_SIZE);

        __RO_END__ = .;
    } >RAM
#endif /* SEPARATE_CODE_AND_RODATA */

#if ENABLE_PIE
    /* PIE fixup table */
    .pie_fixup : {
        __PIE_FIXUP_START__ = .;
        KEEP(*(.pie_fixup))
        __PIE_FIXUP_END__ = .;
    }
#endif

    __RW_START__ = .;

    DATA_SECTION >RAM
    STACK_SECTION >RAM
    BSS_SECTION >RAM
    XLAT_TABLE_SECTION >RAM

#if USE_COHERENT_MEM
    /*
     * The base address of the coherent memory section must be page-aligned to
     * guarantee that the coherent data are stored on their own pages and are
     * not mixed with normal data.  This is required to set up the correct
     * memory attributes for the coherent data page tables.
     */
    .coherent_ram (NOLOAD) : ALIGN(PAGE_SIZE) {
        __COHERENT_RAM_START__ = .;
        *(.tzfw_coherent_mem)
        __COHERENT_RAM_END_UNALIGNED__ = .;

        /*
         * Memory page(s) mapped to this section will be marked as device
         * memory. No other unexpected data must creep in. Ensure the rest of
         * the current memory page is unused.
         */
        . = ALIGN(PAGE_SIZE);

        __COHERENT_RAM_END__ = .;
    } >RAM
#endif /* USE_COHERENT_MEM */

    __RW_END__ = .;
    __BL2_END__ = .;
    RAM_REGION_END = .;

    __BSS_SIZE__ = SIZEOF(.bss);

#if USE_COHERENT_MEM
    __COHERENT_RAM_UNALIGNED_SIZE__ =
        __COHERENT_RAM_END_UNALIGNED__ - __COHERENT_RAM_START__;
#endif /* USE_COHERENT_MEM */

}
