#ifndef BOARD_INFO_H
#define BOARD_INFO_H

#include <stdint.h>
#include <stddef.h>
#include <sys.h>
#include <rzg2l_def.h>

/* Board information magic in SRAM  */
#define BOARD_MB_MAGIC         0x424D4249u
#define BOARD_MB_ADDR_OFFSET   (0x800)

/* Offset range of board information within the QSPI flash region */
#define BOARD_INFO_QSPI_OFFSET U(0x1C700)
#define BOARD_INFO_QSPI_END    U(0x1CF0F)

/* Offset range of board information within the eMMC flash region */
#define BOARD_INFO_EMMC_SECTOR_COUNT U(5)
#define BOARD_INFO_EMMC_SECTOR_SIZE  U(512)

#define MAX_STRING_LEN           256

/* Offset range of board information within the eSD flash region */
#define BOARD_INFO_ESD_SECTOR_COUNT U(5)
#define BOARD_INFO_ESD_SECTOR_SIZE  U(512)

/* Offsets for various fields inside the board info region */
#define OFFSET_MODEL_ID          0x00
#define OFFSET_REVISION          0x04
#define OFFSET_MODEL_STRING      0x08
#define OFFSET_MFG_NAME          0x08 + MAX_STRING_LEN

/*
 * RZ Board SoC Identifiers
 * These macros define unique IDs for supported SoC variants.
 * Use them for conditional compilation or SoC-specific configurations.
 */
#define RZ_SOC_RZG2L            0x01
#define RZ_SOC_RZV2L            0x02
#define RZ_SOC_RZV2H            0x03

/* 
 * Platform descriptor structure.
 * - Packed to ensure no padding is added by the compiler,
 *   so the memory layout is consistent when exchanged between components.
 */
typedef struct __attribute__((packed)) platform_desc {
    uint32_t model_id;
} platform_desc_t;

/* mailbox placed in SRAM */
struct board_mb {
    uint32_t magic;
    uint32_t size;
    platform_desc_t desc;
};

/**
 * get_board_info_u32 - Read a 32-bit field from the board info region
 *
 * @flash_map_base:             Base start address of the memory-mapped QSPI/xSPI flash.
 * @flash_size:                 Size of QSPI/xSPI flash.
 * @board_info_offset:    Offset to the start of the board info structure.
 * @field_offset:         Offset to a specific field within the structure.
 *
 * Example:
 *   uint32_t model_id = get_board_info_u32(RZV2H_XSPI_MEMORY_MAP_BASE, RZV2H_XSPI_SIZE, BOARD_INFO_QSPI_OFFSET, OFFSET_MODEL_ID);
 */
uint32_t get_board_info_u32(uintptr_t flash_map_base, uintptr_t flash_size, size_t board_info_offset, size_t field_offset);

/**
 * get_board_info_u32_emmc - Read a 32-bitalue from eMMC
 *
 * @board_info_offset:  LBA (sector) index where the board info structure starts.
 * @field_offset:       Index (in 32-bit words) of the desired field within the structure.
 *
 * Example:
 *   uint32_t model = get_board_info_u32_emmc(BOARD_INFO_QSPI_OFFSET, OFFSET_MODEL_ID);
 */
uint32_t get_board_info_u32_emmc(size_t board_info_offset, size_t field_offset);

#if defined(PLAT_BOOT_DEVICE_EMMC) || !defined(PLAT_STORAGE_FIXED_BOOT)
/**
 * bl2_emmc_load_boardinfo() - Load board identification data from eMMC
 *
 * This routine reads a small board-info structure
 * from a fixed sector window in the eMMC and publishes it
 * into the SRAM mailbox shared with later stages (BL31/BL33).
 *
 * Return: 0 on success, <0 on failure (partition select, open, or read).
 *
 * Notes:
 * - The mailbox is used by BL31 to determine the board variant.
 */
int bl2_emmc_load_boardinfo(uintptr_t sd_handle);
#endif /* PLAT_BOOT_DEVICE_EMMC */

#if defined(PLAT_BOOT_DEVICE_ESD) || !defined(PLAT_STORAGE_FIXED_BOOT)
/**
 * bl2_esd_load_boardinfo() - Load board identification data from eSD
 *
 * This routine reads a small board-info structure
 * from a fixed sector window in the eSD and publishes it
 * into the SRAM mailbox shared with later stages (BL31/BL33).
 *
 * Return: 0 on success, <0 on failure (partition select, open, or read).
 *
 * Notes:
 * - The mailbox is used by BL31 to determine the board variant.
 */
int bl2_esd_load_boardinfo(uintptr_t sd_handle);
#endif /* PLAT_BOOT_DEVICE_ESD */

#endif /* BOARD_INFO_H */
