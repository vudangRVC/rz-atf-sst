#pragma once
#include <stdint.h>
#include "rzcmn_def.h"

#define BOARD_MB_MAGIC  0x424D4249u
#define BOARD_MB_ADDR   (RZCMN_BOOTINFO_BASE + 0x800)

#define BOARDINFO_LBA    4096ULL
#define BOARDINFO_OFFSET  (BOARDINFO_LBA * 512ULL)

#if !defined(ARRAY_SIZE)
#define ARRAY_SIZE(x) (sizeof(x)/sizeof((x)[0]))
#endif

typedef struct __attribute__((packed)) platform_desc {
    uint32_t model_id;
    uint32_t revision_minor : 16;
    uint32_t revision_major : 16;
    char     model_string[256];
    char     mfg_name[256];

    uint32_t bl2_loc        : 4;
    uint32_t bl2_dtb_loc    : 4;
    uint32_t u_boot_loc     : 4;
    uint32_t u_boot_dtb_loc : 4;
    uint32_t kernel_loc     : 4;
    uint32_t kernel_dtb_loc : 4;
    uint32_t res_loc        : 4;
    uint32_t res1_loc       : 4;

    uint32_t bl2_id         : 4;
    uint32_t bl2_dtb_id     : 4;
    uint32_t u_boot_id      : 4;
    uint32_t u_boot_dtb_id  : 4;
    uint32_t kernel_id      : 4;
    uint32_t kernel_dtb_id  : 4;
    uint32_t res_id         : 4;
    uint32_t res1_id        : 4;

    uint8_t  bl2_desc[256];
    uint8_t  bl2_dtb_desc[256];
    uint8_t  u_boot_desc[256];
    uint8_t  u_boot_dtb_desc[256];
    uint8_t  kernel_desc[256];
    uint8_t  kernel_dtb_desc[256];
} platform_desc_t;

/* mailbox placed in SRAM both stages can access */
struct board_mb {
    uint32_t magic;
    uint32_t size;
    platform_desc_t desc;
};

int bl2_esd_load_boardinfo(uintptr_t sd_handle);
