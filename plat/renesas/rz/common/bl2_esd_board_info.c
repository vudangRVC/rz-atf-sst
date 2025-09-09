#include <string.h>
#include <common/debug.h>
#include <drivers/io/io_storage.h>
#include "board_mailbox.h"

extern void flush_dcache_range(uintptr_t addr, size_t size);

int bl2_esd_load_boardinfo(uintptr_t sd_handle)
{
    platform_desc_t tmp;
    size_t bytes_read = 0;

    const io_block_spec_t spec = {
        .offset = BOARDINFO_OFFSET,
        .length = sizeof(tmp),
    };

    uintptr_t h = 0;
    int rc = io_open(sd_handle, (uintptr_t)&spec, &h);
    if (rc) {
        WARN("BL2: boardinfo open rc=%d\n", rc);
        return rc;
    }

    rc = io_read(h, (uintptr_t)&tmp, sizeof(tmp), &bytes_read);
    io_close(h);

    if (rc || bytes_read != sizeof(tmp)) {
        WARN("BL2: boardinfo read rc=%d bytes=%zu\n", rc, bytes_read);
        return -1;
    }

    /* Minimal safety checks on strings */
    if (!memchr(tmp.model_string, '\0', sizeof(tmp.model_string))) {
        WARN("BL2: model_string not NUL-terminated\n");
        return -1;
    }
    if (!memchr(tmp.mfg_name, '\0', sizeof(tmp.mfg_name))) {
        WARN("BL2: mfg_name not NUL-terminated\n");
        return -1;
    }

    /* Publish to SRAM mailbox */
    struct board_mb *mb = (struct board_mb *)BOARD_MB_ADDR;
    mb->desc  = tmp;
    mb->size  = sizeof(tmp);
    mb->magic = BOARD_MB_MAGIC;

    flush_dcache_range((uintptr_t)mb, sizeof(*mb));

    NOTICE("BL2: boardinfo model=0x%x rev=%u.%u \"%s\"\n",
           mb->desc.model_id,
           mb->desc.revision_major, mb->desc.revision_minor,
           mb->desc.model_string);

    return 0;
}