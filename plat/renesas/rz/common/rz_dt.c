#include <assert.h>
#include <errno.h>

#include <common/debug.h>
#include <common/fdt_wrappers.h>
#include <libfdt.h>

#include <platform_def.h>
#include <rz_dt.h>

/*******************************************************************************
 * This function checks device tree file with its header.
 * Returns 0 on success and a negative FDT error code on failure.
 ******************************************************************************/
int dt_validation(uintptr_t dt_addr)
{
	int ret;

	ret = fdt_check_header((void *)dt_addr);
	if (ret != 0) {
		ERROR("DTB validation failed: %s (%d)\n", fdt_strerror(ret), ret);
		ERROR("DTB location: 0x%lx, magic: 0x%x\n", 
			BL2_LIMIT, 
			  fdt_magic((const void *)BL2_LIMIT));
	}

	return ret;
}

int8_t read_prop_from_subnode(void *fdt, const char *node, const char *sub_node, const char *prop_name,
	uint8_t index, uint32_t *value)
{
	// Get node offset
	int node_offset = fdt_path_offset(fdt, node);
	NOTICE("node_offset = %d\n", node_offset);

	// Get sub node offset
	int sub_node_offset = fdt_subnode_offset(fdt, node_offset, sub_node);
	NOTICE("sub_node_offset = %d\n", sub_node_offset);

	uint32_t target = 0;
	int32_t len = 0;
	const fdt32_t *val = fdt_getprop(fdt, sub_node_offset, prop_name, &len);
	if (!val || len < 4) {
		NOTICE("Missing or invalid property: %s\n", prop_name);
		return -1;
	}
	target = fdt32_to_cpu(val[index]);
	NOTICE("target = 0x%x\n", target);
	*value = target;
	return 0;
}
