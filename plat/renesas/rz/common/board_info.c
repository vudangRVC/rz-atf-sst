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
#include <common/debug.h>
#include <lib/utils.h>
#include <drivers/io/io_driver.h>
#include <string.h>
#include <emmc_def.h>

/* Hardcode the in-sector base for BID right now.
 * When you reflash so the BID starts at sector boundary, set this to 0.
 */
#ifndef BOARD_INFO_EMMC_INSECTOR_BASE
#define BOARD_INFO_EMMC_INSECTOR_BASE 0x0U   /* current BID starts at +0x100 */
#endif

#ifndef BOARD_INFO_EMMC_SECTOR_SIZE
#define BOARD_INFO_EMMC_SECTOR_SIZE   512U
#endif

/* Optional: compile-time sanity */
typedef char _boardinfo_sector_size_is_512[(BOARD_INFO_EMMC_SECTOR_SIZE==512)?1:-1];

/* If your driver defines a cache granule, prefer that here */
#ifndef CACHE_WRITEBACK_GRANULE
#define CACHE_WRITEBACK_GRANULE 32U
#endif

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

uint32_t get_board_info_u32_emmc(size_t board_info_sector_start,
                                 size_t field_offset)
{
    enum { WINDOW_SECTORS = (int)BOARD_INFO_EMMC_SECTOR_COUNT };
    enum { WINDOW_BYTES   = WINDOW_SECTORS * (int)BOARD_INFO_EMMC_SECTOR_SIZE };
    enum { WINDOW_WORDS   = WINDOW_BYTES / (int)sizeof(uint32_t) };

    /* emmc_read_sector() requires a uint32_t* buffer; keep DMA-aligned. */
    static uint32_t buffer[WINDOW_WORDS] __aligned(CACHE_WRITEBACK_GRANULE);

    EMMC_ERROR_CODE ret;
    uint32_t val = 0;

    NOTICE("BL31: eMMC board-info read LBA=%zu count=%u base_off=0x%X field_off=0x%zx\n",
           board_info_sector_start, (unsigned)WINDOW_SECTORS,
           (unsigned)BOARD_INFO_EMMC_INSECTOR_BASE, field_offset);

    /* Read the window starting at the given LBA. */
    ret = emmc_read_sector(buffer,
                           (uint32_t)board_info_sector_start,
                           (uint32_t)WINDOW_SECTORS,
                           0 /* flags */);
    if (ret != EMMC_SUCCESS) {
        ERROR("BL31: emmc_read_sector failed: %u (LBA=%zu cnt=%u)\n",
              ret, board_info_sector_start, (unsigned)WINDOW_SECTORS);
        panic();
    }

    /* Effective offset = in-sector base (e.g., 0x100) + field offset. */
    size_t eff = (size_t)BOARD_INFO_EMMC_INSECTOR_BASE + field_offset;

    if (eff + sizeof(uint32_t) > (size_t)WINDOW_BYTES) {
        ERROR("BL31: board-info OOB: eff_off=0x%zx (window=%u bytes)\n",
              eff, (unsigned)WINDOW_BYTES);
        panic();
    }

    /* Safe byte copy (handles any alignment); data is little-endian. */
    memcpy(&val, ((uint8_t *)buffer) + eff, sizeof(val));

    /* Optional quick peek to help during bring-up */
    {
        uint8_t *b = (uint8_t *)buffer;
        NOTICE("BL31: buf[0..15]: %02x %02x %02x %02x  %02x %02x %02x %02x  "
               "%02x %02x %02x %02x  %02x %02x %02x %02x\n",
               b[0], b[1], b[2], b[3], b[4], b[5], b[6], b[7],
               b[8], b[9], b[10], b[11], b[12], b[13], b[14], b[15]);

        const size_t s = BOARD_INFO_EMMC_INSECTOR_BASE;
        NOTICE("BL31: buf[0x%03zx..0x%03zx]: %02x %02x %02x %02x  %02x %02x %02x %02x  "
               "%02x %02x %02x %02x  %02x %02x %02x %02x\n",
               s, s+15,
               b[s+0], b[s+1], b[s+2], b[s+3], b[s+4], b[s+5], b[s+6], b[s+7],
               b[s+8], b[s+9], b[s+10], b[s+11], b[s+12], b[s+13], b[s+14], b[s+15]);
    }

    NOTICE("BL31: board-info[+0x%zx (eff 0x%zx)] = 0x%08x\n", field_offset, eff, val);
    return val;
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

static void probe_lba(uint32_t lba, size_t off)
{
    enum { SECTORS = 1 };
    enum { BYTES = SECTORS * 512 };
    static uint32_t buf[BYTES/4] __aligned(32);

    EMMC_ERROR_CODE r = emmc_read_sector(buf, lba, SECTORS, 0);
    if (r != EMMC_SUCCESS) {
        NOTICE("PROBE: read LBA=%u failed %u\n", lba, r);
        return;
    }
    uint8_t *b = (uint8_t*)buf;
    size_t o = off;
    NOTICE("PROBE: LBA=%u off=0x%zx  16B: %02x %02x %02x %02x  %02x %02x %02x %02x  "
           "%02x %02x %02x %02x  %02x %02x %02x %02x\n",
           lba, off,
           b[o+0], b[o+1], b[o+2], b[o+3], b[o+4], b[o+5], b[o+6], b[o+7],
           b[o+8], b[o+9], b[o+10], b[o+11], b[o+12], b[o+13], b[o+14], b[o+15]);
}

void quick_probe(void)
{
    /* BOOT#1 */
    NOTICE("=== Probe BOOT#1 ===\n");
    probe_lba(226, 0x000); probe_lba(226, 0x100);
    probe_lba(227, 0x000); probe_lba(227, 0x100);
    probe_lba(228, 0x000); probe_lba(228, 0x100);

    /* USER AREA */
    emmc_select_partition(PARTITION_ID_USER);
    NOTICE("=== Probe USER ===\n");
    probe_lba(226, 0x000); probe_lba(226, 0x100);
    probe_lba(227, 0x000); probe_lba(227, 0x100);
    probe_lba(228, 0x000); probe_lba(228, 0x100);

	probe_lba(551, 0x000);
	probe_lba(551, 0x100);
	probe_lba(552, 0x000);
	probe_lba(552, 0x100);

	/* decimal 227 case */
probe_lba(227, 0x000);
probe_lba(227, 0x100);
probe_lba(228, 0x000);
probe_lba(228, 0x100);

/* hex 0x227 interpreted → decimal 551 */
probe_lba(551, 0x000);
probe_lba(551, 0x100);
probe_lba(552, 0x000);
probe_lba(552, 0x100);

/* alternative guesses seen in BL2 logs */
probe_lba(250, 0x000);
probe_lba(250, 0x100);
probe_lba(251, 0x000);
probe_lba(251, 0x100);

probe_lba(328, 0x000);
probe_lba(328, 0x100);
probe_lba(329, 0x000);
probe_lba(329, 0x100);

    /* restore BOOT#1 for normal reads */
    emmc_select_partition(PARTITION_ID_BOOT_1);
}
