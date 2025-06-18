/*
 * Copyright (c) 2025, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <stddef.h>
#include <platform_def.h>
#include <lib/mmio.h>
#include <assert.h>
#include <board_info.h>

/**
 * get_board_info_field - Read a 32-bit field from the board info region
 *
 * @flash_map_base:           Base address of the memory-mapped QSPI flash.
 * @flash_size:               Size of QSPI/xSPI flash.
 * @board_info_offset:    Offset to the start of the board info structure.
 * @field_offset:         Offset to a specific field within the structure.
 *
 * Returns the 32-bit value read from flash_base + board_info_offset + field_offset.
 */
uint32_t get_board_info_u32(uintptr_t flash_map_base, uintptr_t flash_size, size_t board_info_offset, size_t field_offset)
{
	uintptr_t addr = flash_map_base + board_info_offset + field_offset;

	/* Check invalid access to board info region */
	if (addr < flash_map_base ||
		addr + sizeof(uint32_t) - 1 > flash_map_base + flash_size) {
		ERROR("Board info offset out of bounds: addr=0x%lx (valid: 0x%lx - 0x%lx)\n",
			addr,
			flash_map_base,
			flash_map_base + flash_size);
#if DEBUG
		assert(0);
#else
		panic();
#endif
	}

	return mmio_read_32(addr);
}

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
 */
void get_board_info_string(uintptr_t flash_base, size_t board_info_offset, size_t field_offset, char *buf, size_t len)
{

	uintptr_t addr = flash_base + board_info_offset + field_offset;

	/* Check invalid access to board info region */
	if (addr < flash_base + BOARD_INFO_QSPI_OFFSET ||
		addr + sizeof(uint32_t) - 1 > flash_base + BOARD_INFO_QSPI_END) {
		ERROR("Board info offset out of bounds: addr=0x%lx (valid: 0x%lx - 0x%lx)\n",
				addr,
				flash_base + BOARD_INFO_QSPI_OFFSET,
				flash_base + BOARD_INFO_QSPI_END);
#if DEBUG
		assert(0);
#else
		panic();
#endif
	}

	if (!buf || len == 0)
		return;

	size_t bytes_read = 0;
	size_t total_bytes = len - 1;

	while (bytes_read < total_bytes) {
		uint32_t val = mmio_read_32(addr + bytes_read);

		/* Extract each byte from the 32-bit word in little-endian order and store in buffer */
		buf[bytes_read++] = (val >> 0) & 0xFF;
		if (bytes_read >= total_bytes) break;

		buf[bytes_read++] = (val >> 8) & 0xFF;
		if (bytes_read >= total_bytes) break;

		buf[bytes_read++] = (val >> 16) & 0xFF;
		if (bytes_read >= total_bytes) break;

		buf[bytes_read++] = (val >> 24) & 0xFF;
	}

	buf[bytes_read] = '\0';
}
