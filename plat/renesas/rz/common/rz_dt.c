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
	return fdt_check_header((void *)dt_addr);
}

int8_t read_prop_from_subnode(void *fdt, const char *node, const char *sub_node, const char *prop_name,
	uint8_t index, uint32_t *value)
{
	// Get node offset
	int node_offset = fdt_path_offset(fdt, node);

	// Get sub node offset
	int sub_node_offset = fdt_subnode_offset(fdt, node_offset, sub_node);

	uint32_t target = 0;
	int32_t len = 0;
	const fdt32_t *val = fdt_getprop(fdt, sub_node_offset, prop_name, &len);
	if (!val || len < 4) {
		NOTICE("Missing or invalid property: %s\n", prop_name);
		return -1;
	}
	target = fdt32_to_cpu(val[index]);
	*value = target;
	return 0;
}

int8_t read_prop_64_from_subnode(void *fdt, const char *node, const char *sub_node, const char *prop_name,
	uint8_t index, uint64_t *value)
{
	// Get node offset
	int node_offset = fdt_path_offset(fdt, node);

	// Get sub node offset
	int sub_node_offset = fdt_subnode_offset(fdt, node_offset, sub_node);

	uint64_t target = 0;
	int32_t len = 0;
	const fdt32_t *val = fdt_getprop(fdt, sub_node_offset, prop_name, &len);

	// Check the property exists and length
	if (!val || len < (index * 8 + 8)) {
		return -1;
	}

	// Read the 64-bit value from the property
	uint32_t high_part = fdt32_to_cpu(val[index * 2]);
	uint32_t low_part = fdt32_to_cpu(val[index * 2 + 1]);

	// Combine the high and low parts into a single 64-bit value
	target = ((uint64_t)high_part << 32) | low_part;

	*value = target;
	return 0;
}
