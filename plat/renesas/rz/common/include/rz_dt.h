#ifndef RZ_DT_H
#define RZ_DT_H

#include <stdbool.h>
#include <stdint.h>

#define DT_DISABLED		U(0)
#define DT_NON_SECURE		U(1)
#define DT_SECURE		U(2)
#define DT_SHARED		(DT_NON_SECURE | DT_SECURE)

/*******************************************************************************
 * Function and variable prototypes
 ******************************************************************************/
int dt_validation(uintptr_t dt_addr);
uint8_t read_prop_from_sub_node(void *fdt, const char *node, const char *sub_node, const char *prop_names, 
	uint32_t *value, uint8_t *num);
uint8_t read_prop_from_child_node(void *fdt, const char *node, const char *sub_node, const char *child_node, const char *prop_names, 
	uint32_t *value, uint8_t *num);

uint8_t fdt_read_node_level_4(
	const void *fdt,
	const char *node_name_L1,
	const char *node_name_L2,
	const char *node_name_L3,
	const char *node_name_L4,
	const char *prop_name,
	uint32_t *value, uint8_t *num);
#endif /* RZ_DT_H */
