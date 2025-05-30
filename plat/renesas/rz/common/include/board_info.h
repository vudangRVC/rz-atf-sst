#ifndef BOARD_INFO_H
#define BOARD_INFO_H

#include <stdint.h>
#include <stddef.h>

/* Offset range of board information within the QSPI flash region */
#define BOARD_INFO_QSPI_OFFSET U(0x1C700)
#define BOARD_INFO_QSPI_END    U(0x1CF0F)

#define MAX_STRING_LEN           256

/* Offsets for various fields inside the board info region */
#define OFFSET_MODEL_ID          0x00
#define OFFSET_REVISION          0x04
#define OFFSET_MODEL_STRING      0x08
#define OFFSET_MFG_NAME          0x08 + MAX_STRING_LEN

/**
 * get_board_info_u32 - Read a 32-bit field from the board info region
 *
 * @flash_base:           Base address of the memory-mapped QSPI flash.
 * @board_info_offset:    Offset to the start of the board info structure.
 * @field_offset:         Offset to a specific field within the structure.
 *
 * Example:
 *   uint32_t model_id = get_board_info_u32(RZG2L_SPIROM_BASE, BOARD_INFO_QSPI_OFFSET, OFFSET_MODEL_ID);
 */
uint32_t get_board_info_u32(uintptr_t flash_base, size_t board_info_offset, size_t field_offset);

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

#endif /* BOARD_INFO_H */
