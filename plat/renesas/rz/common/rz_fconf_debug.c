#include <common/debug.h>
#include <lib/fconf/fconf.h>
#include <lib/libfdt/libfdt.h>
#include <common/fdt_wrappers.h>
#include "rz_fconf.h"

struct cpg_config_t cpg_config;

/**********************************************************************
 * CPG FCONF function.
 **********************************************************************/

int fconf_populate_cpg_config(uintptr_t config) {
    void *fdt = (void *)config; 

    int soc_node = fdt_path_offset(fdt, "/soc");
    if (soc_node < 0) {
        ERROR("Cannot find /soc node\n");
        return -1;
    }

    INFO("Listing subnodes of /soc:\n");

    int subnode;
    fdt_for_each_subnode(subnode, fdt, soc_node) {
        const char *name = fdt_get_name(fdt, subnode, NULL);
        INFO("  Subnode: %s\n", name);
    }

    int cpg_node = fdt_subnode_offset(fdt, soc_node, "clock-controller@11010000");
    if (cpg_node < 0) {
        ERROR("Cannot find clock-controller@11010000 node\n");
        return -1;
    }

    const char *props[] = {
        "divpl1_set",
        "divpl1_set_wen",
        "cpg_pll4_clk1",
        "cpg_pll4_clk2",
        "cpg_pll4_stby",
        "cpg_pll6_clk1",
        "cpg_pll6_clk2",
        "cpg_pll6_stby"
    };

    uint32_t *targets[] = {
        &cpg_config.divpl1_set,
        &cpg_config.divpl1_set_wen,
        &cpg_config.pll4_clk1,
        &cpg_config.pll4_clk2,
        &cpg_config.pll4_stby,
        &cpg_config.pll6_clk1,
        &cpg_config.pll6_clk2,
        &cpg_config.pll6_stby
    };

    for (int i = 0; i < (int)(sizeof(props) / sizeof(props[0])); i++) {
        int len;
        const fdt32_t *val = fdt_getprop(fdt, cpg_node, props[i], &len);
        if (val == NULL) {
            WARN("Missing property: %s\n", props[i]);
            continue;
        }
    
        int count = len / sizeof(fdt32_t);
        if (count < 1) {
            WARN("Property %s has invalid size: %d\n", props[i], len);
            continue;
        }
    
        *targets[i] = fdt32_to_cpu(val[0]);
    
        for (int j = 0; j < count; j++) {
            INFO("%s = 0x%08x\n", props[i], fdt32_to_cpu(val[j]));
        }
    }

    return 0; 
}

const struct cpg_config_t *cpg_config_getter(void)
{
    return &cpg_config;
}

/**********************************************************************
 * CPG FCONF function.
 **********************************************************************/

FCONF_REGISTER_POPULATOR(HW_CONFIG, cpg_config, fconf_populate_cpg_config);
