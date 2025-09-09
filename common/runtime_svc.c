/*
 * Copyright (c) 2013-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <string.h>

#include <common/debug.h>
#include <common/runtime_svc.h>

/*******************************************************************************
 * The 'rt_svc_descs' array holds the runtime service descriptors exported by
 * services by placing them in the 'rt_svc_descs' linker section.
 * The 'rt_svc_descs_indices' array holds the index of a descriptor in the
 * 'rt_svc_descs' array. When an SMC arrives, the OEN[29:24] bits and the call
 * type[31] bit in the function id are combined to get an index into the
 * 'rt_svc_descs_indices' array. This gives the index of the descriptor in the
 * 'rt_svc_descs' array which contains the SMC handler.
 ******************************************************************************/
uint8_t rt_svc_descs_indices[MAX_RT_SVCS];

#define RT_SVC_DECS_NUM		((RT_SVC_DESCS_END - RT_SVC_DESCS_START)\
					/ sizeof(rt_svc_desc_t))

/*******************************************************************************
 * Function to invoke the registered `handle` corresponding to the smc_fid in
 * AArch32 mode.
 ******************************************************************************/
uintptr_t handle_runtime_svc(uint32_t smc_fid,
			     void *cookie,
			     void *handle,
			     unsigned int flags)
{
	u_register_t x1, x2, x3, x4;
	unsigned int index;
	unsigned int idx;
	const rt_svc_desc_t *rt_svc_descs;

	assert(handle != NULL);
	idx = get_unique_oen_from_smc_fid(smc_fid);
	assert(idx < MAX_RT_SVCS);

	index = rt_svc_descs_indices[idx];
	if (index >= RT_SVC_DECS_NUM)
		SMC_RET1(handle, SMC_UNK);

	rt_svc_descs = (rt_svc_desc_t *) RT_SVC_DESCS_START;

	get_smc_params_from_ctx(handle, x1, x2, x3, x4);

	return rt_svc_descs[index].handle(smc_fid, x1, x2, x3, x4, cookie,
						handle, flags);
}

/*******************************************************************************
 * Simple routine to sanity check a runtime service descriptor before using it
 ******************************************************************************/
static int32_t validate_rt_svc_desc(const rt_svc_desc_t *desc)
{
	if (desc == NULL)
		return -EINVAL;

	if (desc->start_oen > desc->end_oen)
		return -EINVAL;

	if (desc->end_oen >= OEN_LIMIT)
		return -EINVAL;

	if ((desc->call_type != SMC_TYPE_FAST) &&
	    (desc->call_type != SMC_TYPE_YIELD))
		return -EINVAL;

	/* A runtime service having no init or handle function doesn't make sense */
	if ((desc->init == NULL) && (desc->handle == NULL))
		return -EINVAL;

	return 0;
}

/*******************************************************************************
 * This function calls the initialisation routine in the descriptor exported by
 * a runtime service. Once a descriptor has been validated, its start & end
 * owning entity numbers and the call type are combined to form a unique oen.
 * The unique oen is used as an index into the 'rt_svc_descs_indices' array.
 * The index of the runtime service descriptor is stored at this index.
 ******************************************************************************/
void __init runtime_svc_init(void)
{
	int rc = 0;
	unsigned int index;
	uint8_t start_idx, end_idx;
	rt_svc_desc_t *rt_svc_descs;

	INFO("BL31: runtime_svc_init() enter\n");
	INFO("BL31: RT_SVC_DESCS_START=0x%lx RT_SVC_DESCS_END=0x%lx NUM=%u MAX=%u\n",
	     (unsigned long)RT_SVC_DESCS_START,
	     (unsigned long)RT_SVC_DESCS_END,
	     (unsigned int)RT_SVC_DECS_NUM,
	     (unsigned int)MAX_RT_SVCS);

	/* Sanity check */
	assert((RT_SVC_DESCS_END >= RT_SVC_DESCS_START) &&
	       (RT_SVC_DECS_NUM < MAX_RT_SVCS));

	/* If no runtime services are implemented then simply bail out */
	if (RT_SVC_DECS_NUM == 0U) {
		INFO("BL31: No runtime services found, return\n");
		return;
	}

	/* Initialise internal variables to invalid state */
	(void)memset(rt_svc_descs_indices, -1, sizeof(rt_svc_descs_indices));
	INFO("BL31: rt_svc_descs_indices[] set to -1\n");

	rt_svc_descs = (rt_svc_desc_t *)RT_SVC_DESCS_START;

	for (index = 0U; index < RT_SVC_DECS_NUM; index++) {
		rt_svc_desc_t *service = &rt_svc_descs[index];

		INFO("BL31: validating service[%u] @%p name='%s' start_oen=%u end_oen=%u call_type=%u\n",
		     index, (void *)service,
		     (service->name ? service->name : "<null>"),
		     service->start_oen, service->end_oen,
		     service->call_type);

		/* Validate descriptor */
		rc = validate_rt_svc_desc(service);
		if (rc != 0) {
			ERROR("BL31: invalid runtime service descriptor @%p\n",
			      (void *)service);
			panic();
		}

		/* Init the service */
		if (service->init != NULL) {
			INFO("BL31: calling init() for service '%s'\n", service->name);
			rc = service->init();
			INFO("BL31: service '%s' init() rc=%d\n", service->name, rc);
			if (rc != 0) {
				ERROR("BL31: service '%s' init failed rc=%d (skipping)\n",
				      service->name, rc);
				continue;
			}
		} else {
			INFO("BL31: service '%s' has no init() (skipping)\n",
			     service->name);
		}

		/* Fill OEN index mapping */
		start_idx = (uint8_t)get_unique_oen(service->start_oen,
						    service->call_type);
		end_idx   = (uint8_t)get_unique_oen(service->end_oen,
						    service->call_type);

		assert(start_idx <= end_idx);
		assert(end_idx < MAX_RT_SVCS);

		INFO("BL31: map service '%s' index=%u to oen[%u..%u]\n",
		     service->name, index, start_idx, end_idx);

		for (uint8_t o = start_idx; o <= end_idx; o++) {
			rt_svc_descs_indices[o] = index;
			VERBOSE("BL31:   rt_svc_descs_indices[%u] = %u ('%s')\n",
				o, index, service->name);
		}

		INFO("BL31: service '%s' initialization done\n", service->name);
	}

	INFO("BL31: runtime_svc_init() complete\n");
}

