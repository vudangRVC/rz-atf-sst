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

// Node -> Sub Node                -> Child Node       -> Grand Child Node -> Property -> 5 Values
// /soc -> cpg-clk-config@10420000 -> clocks-on-config -> cr8_part1        -> config   -> 0x0600 0x0000e000 0x0800 0x0000e000 0
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

// Node -> Sub Node                -> Child Node       -> Grand Child Node -> Property -> 5 Values
// /soc -> cpg-clk-config@10420000 -> clocks-on-config -> cr8_part1        -> config   -> 0x0600 0x0000e000 0x0800 0x0000e000 0
uint8_t read_prop_from_child_node(void *fdt, const char *node, const char *sub_node, const char *child_node, const char *prop_names, 
	uint32_t *value, uint8_t *num)
{
	// Get parent node offset
	int node_offset = fdt_path_offset(fdt, node);
	NOTICE("node_offset = %d\n", node_offset);

	// Get sub node offset
	int sub_node_offset = fdt_subnode_offset(fdt, node_offset, sub_node);
	NOTICE("sub_node_offset = %d\n", sub_node_offset);

	// Get child node offset
	int child_node_offset = fdt_subnode_offset(fdt, sub_node_offset, child_node);
	NOTICE("child_node_offset = %d\n", child_node_offset);
	
	uint32_t target[5];
	int32_t len = 0;

	NOTICE("prop_names = %s\n", prop_names);
	const fdt32_t *val = fdt_getprop(fdt, child_node_offset, prop_names, &len);
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

uint8_t fdt_read_node_level_4(
	const void *fdt,
	const char *node_name_L1,
	const char *node_name_L2,
	const char *node_name_L3,
	const char *node_name_L4,
	const char *prop_name,
	uint32_t *value, uint8_t *num)
{
	int node_L1 = fdt_path_offset(fdt, node_name_L1);
	NOTICE("node_L1 = %d\n", node_L1);

	int node_L2 = fdt_subnode_offset(fdt, node_L1, node_name_L2);
	NOTICE("node_L2 = %d\n", node_L2);

	int node_L3 = fdt_subnode_offset(fdt, node_L2, node_name_L3);
	NOTICE("node_L3 = %d\n", node_L3);

	int node_L4 = fdt_subnode_offset(fdt, node_L3, node_name_L4);
	NOTICE("node_L4 = %d\n", node_L4);

	uint32_t targets[5];
	int len;
	const fdt32_t *val = fdt_getprop(fdt, node_L4, prop_name, &len);
	if (!val) {
		NOTICE("Missing or invalid property: %s\n", prop_name);
		return 1;
	}
	*num = len / sizeof(uint32_t);
	for (size_t i = 0; i < len/sizeof(uint32_t); ++i) {
		NOTICE("Parsed:\n");
		targets[i] = fdt32_to_cpu(val[i]);
		NOTICE("Parsed %s = 0x%08x\n", prop_name, targets[i]);
		value[i] = targets[i];
	}
	return 0;
}

