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

uint8_t read_prop_from_sub_node(void *fdt, const char *node, const char *sub_node, const char *prop_names, 
	uint32_t *value, uint8_t *num)
{
	// Get parent node offset
	int node_offset = fdt_path_offset(fdt, node);
	NOTICE("node_offset = %d\n", node_offset);

	// Get sub node offset
	int sub_node_offset = fdt_subnode_offset(fdt, node_offset, sub_node);
	NOTICE("sub_node_offset = %d\n", sub_node_offset);

	uint32_t target[5];
	int32_t len = 0;

	NOTICE("prop_names = %s\n", prop_names);
	const fdt32_t *val = fdt_getprop(fdt, sub_node_offset, prop_names, &len);
	if (!val) {
		NOTICE("Missing or invalid property: %s\n", prop_names);
		return 1;
	}
	NOTICE("len = %d\n", len);

	for(int i = 0; i < len / sizeof(uint32_t); i++) {
		target[i] = fdt32_to_cpu(val[i]);
		NOTICE("target[%d] = 0x%x\n", i, target[i]);
		value[i] = target[i];
	}
	return 0;
}
