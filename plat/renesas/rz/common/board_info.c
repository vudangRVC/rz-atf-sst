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
#include <drivers/io/io_driver.h>
#include <string.h>
#include <emmc_def.h>

extern void flush_dcache_range(uintptr_t addr, size_t size);

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
int bl2_emmc_load_boardinfo(uintptr_t emmc_handle)
{
	platform_desc_t tmp;
	size_t bytes_read = 0;
	int rc;
	uintptr_t h = 0;

	/* Compute byte window from eMMC defines of board identification */
	const size_t bi_start_lba = BOARD_INFO_EMMC_SECTOR_START;
	const size_t bi_sector_sz = BOARD_INFO_EMMC_SECTOR_SIZE;
	const size_t bi_sector_cnt = BOARD_INFO_EMMC_SECTOR_COUNT;
	const size_t bi_window_size = bi_sector_cnt * bi_sector_sz;
	const size_t bi_offset = bi_start_lba * bi_sector_sz;
	const size_t read_len = (sizeof(tmp) <= bi_window_size) ? sizeof(tmp) : bi_window_size;

	const io_block_spec_t spec = {
		.offset = bi_offset,
		.length = read_len,
	};

	/* Select eMMC partition 1 */
	if (emmc_select_partition(PARTITION_ID_BOOT_1) != EMMC_SUCCESS) {
		ERROR("BL2: select BOOT#1 failed\n");
		panic();
	}

	/* Open the I/O window and read data */
	rc = io_open(emmc_handle, (uintptr_t)&spec, &h);
	if (rc) {
		ERROR("BL2: boardinfo(emmc) open rc=%d\n", rc);
		goto fail_restore_user;
	}

	rc = io_read(h, (uintptr_t)&tmp, read_len, &bytes_read);
	io_close(h);

	if (rc || bytes_read != read_len) {
		ERROR("BL2: boardinfo(emmc) read rc=%d bytes=%zu (exp=%zu)\n",
			rc, bytes_read, read_len);
		rc = (rc) ? rc : -1;
		goto fail_restore_user;
	}

	/* Zero-pad if only a partial read occurred */
	if (read_len < sizeof(tmp)) {
		memset((uint8_t *)&tmp + read_len, 0, sizeof(tmp) - read_len);
	}

	/* Publish to SRAM mailbox */
	struct board_mb *mb = (struct board_mb *)BOARD_MB_ADDR;
	mb->desc  = tmp;
	mb->size  = sizeof(tmp);
	mb->magic = BOARD_MB_MAGIC;
	flush_dcache_range((uintptr_t)mb, sizeof(*mb));

	NOTICE("BL2: boardinfo(emmc) model=0x%x\"\n",
		mb->desc.model_id);

	/* Restore eMMC partition to USER so BL33 sees defaults */
	(void)emmc_select_partition(PARTITION_ID_USER);
	return 0;

fail_restore_user:
	/* Restore USER partition */
	(void)emmc_select_partition(PARTITION_ID_USER);
	return rc ? rc : -1;
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

/*
 * get_chipid - Retrieve the 128-bit Chip ID from OTP registers.
 * 
 * Reads four consecutive 32-bit words from @otp_base and stores them
 * into @chipid[0..3] in order.
 */
void get_chipid(uintptr_t otp_base, uint32_t *chipid)
{
    chipid[0] = mmio_read_32(otp_base + 0x0);
    chipid[1] = mmio_read_32(otp_base + 0x4);
    chipid[2] = mmio_read_32(otp_base + 0x8);
    chipid[3] = mmio_read_32(otp_base + 0xC);
}
