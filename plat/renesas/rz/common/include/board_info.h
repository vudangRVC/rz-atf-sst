#ifndef BOARD_INFO_H
#define BOARD_INFO_H

#include <stdint.h>
#include <stddef.h>

/* Offset range of board information within the QSPI flash region */
#define BOARD_INFO_QSPI_OFFSET U(0x1C700)
#define BOARD_INFO_QSPI_END    U(0x1CF0F)

/* Offset range of board information within the eMMC device
 * BOARD_INFO_EMMC_SECTOR_START is an LBA (512B sectors), not bytes.
 */
#define BOARD_INFO_EMMC_SECTOR_START  U(227)   // LBA
#define BOARD_INFO_EMMC_SECTOR_COUNT  U(5)     // as you had
#define BOARD_INFO_EMMC_SECTOR_SIZE  U(512)

#define MAX_STRING_LEN           256

/* Offsets INSIDE the board-info blob (in BYTES) */
#define OFFSET_MODEL_ID          0x00
#define OFFSET_REVISION          0x04
#define OFFSET_MODEL_STRING      0x08
#define OFFSET_MFG_NAME          0x08 + MAX_STRING_LEN

/* eMMC block size (typically 512 bytes) */
#define EMMC_BLOCK_SIZE 512

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
 * @board_info_offset: Offset to start of board info structure (in bytes).
 * @field_offset:      Offset to a specific field inside the structure (in bytes).
 *
 * Example:
 *   uint32_t model = get_board_info_u32_emmc(&emmc_dev, BOARD_INFO_QSPI_OFFSET, OFFSET_MODEL_ID);
 */
uint32_t get_board_info_u32_emmc(size_t board_info_offset, size_t field_offset);

/**
 * get_board_info_string - Read a string field from the board info region
 *
 * @flash_base:           Base address of the memory-mapped QSPI flash.
 * @board_info_offset:    Offset to the start of the board info structure.
 * @field_offset:         Offset to a specific field within the structure.
 * @buf:                  Buffer to store the string.
 * @len:                  Length of the buffer.
 *
 * Reads a string from flash_base + board_info_offset + field_offset into buf.
 * Ensures the string is null-terminated.
 *
 * Example:
 *   char model_string[MAX_STRING_LEN];
 *   get_board_info_string(RZG2L_SPIROM_BASE, BOARD_INFO_QSPI_OFFSET, OFFSET_MODEL_STRING, model_string, sizeof(model_string));
 */
void get_board_info_string(uintptr_t flash_base, size_t board_info_offset, size_t field_offset, char *buf, size_t len);
void dump_part_cfg(void);
void quick_probe(void);

#endif /* BOARD_INFO_H */
