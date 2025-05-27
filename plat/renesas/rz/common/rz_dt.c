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

//     Node -> Sub Node                -> Property -> Value
// ex: /soc -> cpg-clk-config@10420000 -> reg      -> 0x10420000
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

int8_t read_prop_64_from_subnode(void *fdt, const char *node, const char *sub_node, const char *prop_name,
	uint8_t index, uint64_t *value)
{
	// Get node offset
	int node_offset = fdt_path_offset(fdt, node);
	NOTICE("node_offset = %d\n", node_offset);

	// Get sub node offset
	int sub_node_offset = fdt_subnode_offset(fdt, node_offset, sub_node);
	NOTICE("sub_node_offset = %d\n", sub_node_offset);

	uint64_t target = 0;
	int32_t len = 0;
	const fdt32_t *val = fdt_getprop(fdt, sub_node_offset, prop_name, &len);

	// Check the property exists and length
	if (!val || len < (index * 8 + 8)) {
		NOTICE("Missing or invalid property: %s length for 64-bit value at index %d\n", prop_name, index);
		return -1;
	}

	// Read the 64-bit value from the property
	uint32_t high_part = fdt32_to_cpu(val[index * 2]);
	uint32_t low_part = fdt32_to_cpu(val[index * 2 + 1]);

	// Combine the high and low parts into a single 64-bit value
	target = ((uint64_t)high_part << 32) | low_part;

	NOTICE("target = 0x%lx\n", target);
	*value = target;
	return 0;
}

// Node -> Sub Node                -> Child Node    -> Grand Child Node -> Property -> 5 Values
// /soc -> cpg-clk-config@10420000 -> clocks-config -> cr8_part1        -> config   -> 0x0600 0x0000e000 0x0800 0x0000e000 0
uint8_t read_prop_from_grand_chil_node(void *fdt, 
	const char *node,
	const char *sub_node,
	const char *chil_node,
	const char *grand_chil_node,
	const char *prop_name,
	uint32_t *value)
{
	// Get node offset
	int node_offset = fdt_path_offset(fdt, node);
	NOTICE("node_offset = %d\n", node_offset);

	// Get sub node offset
	int sub_node_offset = fdt_subnode_offset(fdt, node_offset, sub_node);
	NOTICE("sub_node_offset = %d\n", sub_node_offset);

	// Get chil node offset
	int chil_node_offset = fdt_subnode_offset(fdt, sub_node_offset, chil_node);
	NOTICE("chil_node_offset = %d\n", chil_node_offset);

	// Get grand chil node offset
	int grand_chil_node_offset = fdt_subnode_offset(fdt, chil_node_offset, grand_chil_node);
	NOTICE("grand_chil_node_offset = %d\n", grand_chil_node_offset);

	// Get property value
	uint32_t target[5];
	int32_t len = 0;
	const fdt32_t *val = fdt_getprop(fdt, grand_chil_node_offset, prop_name, &len);
	if (!val || len < 5) {
		NOTICE("Missing or invalid property: %s\n", prop_name);
		return 1;
	}

	// Return 5 values from property
	for(int i = 0; i < 5; i++) {
		target[i] = fdt32_to_cpu(val[i]);
		NOTICE("target[%d] = 0x%x\n", i, target[i]);
		value[i] = target[i];
	}

	return 0;
}

// Node -> Sub Node                -> Child Node    -> Grand Child Node -> Property -> 5 Values
// /soc -> cpg-clk-config@10420000 -> clocks-config -> cr8_part1        -> config   -> 0x0600 0x0000e000 0x0800 0x0000e000 0
uint8_t read_all_grand_chil_node(void *fdt,
	const char *node,
	const char *sub_node,
	const char *chil_node)
{
	// Get node offset: /soc
	int node_offset = fdt_path_offset(fdt, node);
	NOTICE("node_offset = %d\n", node_offset);

	// Get sub node offset: cpg-clk-config@10420000
	int sub_node_offset = fdt_subnode_offset(fdt, node_offset, sub_node);
	NOTICE("sub_node_offset = %d\n", sub_node_offset);

	// Get chil node offset: clocks-config
	int chil_node_offset = fdt_subnode_offset(fdt, sub_node_offset, chil_node);
	NOTICE("chil_node_offset = %d\n", chil_node_offset);

	// Get all grand chil node in chil node
	int node1;
	int len, i;
    fdt_for_each_subnode(node1, fdt, chil_node_offset) {
        const char *name = fdt_get_name(fdt, node1, NULL);
        printf("Node: %s\n", name);

        const fdt32_t *prop = fdt_getprop(fdt, node1, "config", &len);
        if (prop && len >= 0) {
            int count = len / sizeof(fdt32_t);
            for (i = 0; i < count; i++) {
                uint32_t val = fdt32_to_cpu(prop[i]);
                printf("  config[%d] = 0x%08x\n", i, val);
            }
        } else {
            printf("  No config property found\n");
        }
    }

    return 0;
}



