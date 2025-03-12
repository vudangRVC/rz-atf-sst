/*
 * Copyright (c) 2020, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/desc_image_load.h>
#include <plat/common/platform.h>
#include <rzg2l_def.h>

#if (RZG2L_BL33_EXECUTION_EL == 0)
#define BL33_MODE MODE_EL1
#else
#define BL33_MODE MODE_EL2
#endif

static bl_mem_params_node_t bl2_mem_params_descs[] = {
	{
		.image_id = BL31_IMAGE_ID,

		SET_STATIC_PARAM_HEAD(ep_info, PARAM_EP, VERSION_2,
			entry_point_info_t, SECURE | EXECUTABLE | EP_FIRST_EXE),
		.ep_info.spsr = SPSR_64(MODE_EL3,
			MODE_SP_ELX, DISABLE_ALL_EXCEPTIONS),
		.ep_info.pc = BL31_BASE,
		.ep_info.args.arg0 = (uintptr_t)PARAMS_BASE,

		SET_STATIC_PARAM_HEAD(image_info, PARAM_EP, VERSION_2,
			image_info_t, IMAGE_ATTRIB_PLAT_SETUP),
		.image_info.image_max_size = BL31_LIMIT - BL31_BASE,
		.image_info.image_base = BL31_BASE,

# ifdef BL32_BASE
		.next_handoff_image_id = BL32_IMAGE_ID,
# else
		.next_handoff_image_id = FW_CONFIG_ID,
# endif /* BL32_BASE */
	},
# ifdef BL32_BASE
	{
		.image_id = BL32_IMAGE_ID,

		SET_STATIC_PARAM_HEAD(ep_info, PARAM_EP, VERSION_2,
			entry_point_info_t, SECURE | EXECUTABLE),
		.ep_info.pc = BL32_BASE,
		.ep_info.spsr = 0,

		SET_STATIC_PARAM_HEAD(image_info, PARAM_EP, VERSION_2,
			image_info_t, 0),
		.image_info.image_max_size = BL32_LIMIT - BL32_BASE,
		.image_info.image_base = BL32_BASE,

		.next_handoff_image_id = BL33_IMAGE_ID,
	},
# endif /* BL32_BASE */
	{
		.image_id = FW_CONFIG_ID,

		SET_STATIC_PARAM_HEAD(ep_info, PARAM_EP, VERSION_2,
				entry_point_info_t, NON_SECURE | NON_EXECUTABLE),
		.ep_info.pc = FW_CONFIG_BASE,
		.ep_info.spsr = 0,

		SET_STATIC_PARAM_HEAD(image_info, PARAM_EP, VERSION_2,
				image_info_t, 0),
		.image_info.image_max_size = FW_CONFIG_LIMIT - FW_CONFIG_BASE,
		.image_info.image_base = FW_CONFIG_BASE,

		.next_handoff_image_id = HW_CONFIG_ID,
	}, /* Finish load for rzv2l_cm33_rpmsg_demo_secure_code.bin */
	{
		.image_id = HW_CONFIG_ID,

		SET_STATIC_PARAM_HEAD(ep_info, PARAM_EP, VERSION_2,
				entry_point_info_t, NON_SECURE | NON_EXECUTABLE),
		.ep_info.pc = HW_CONFIG_BASE,
		.ep_info.spsr = 0,

		SET_STATIC_PARAM_HEAD(image_info, PARAM_EP, VERSION_2,
				image_info_t, 0),
		.image_info.image_max_size = HW_CONFIG_LIMIT - HW_CONFIG_BASE,
		.image_info.image_base = HW_CONFIG_BASE,

		.next_handoff_image_id = SOC_FW_CONFIG_ID,
	}, /* Finish load for rzv2l_cm33_rpmsg_demo_non_secure_vector.bin */
	{
		.image_id = SOC_FW_CONFIG_ID,

		SET_STATIC_PARAM_HEAD(ep_info, PARAM_EP, VERSION_2,
				entry_point_info_t, NON_SECURE | NON_EXECUTABLE),
		.ep_info.pc = SOC_FW_CONFIG_BASE,
		.ep_info.spsr = 0,

		SET_STATIC_PARAM_HEAD(image_info, PARAM_EP, VERSION_2,
				image_info_t, 0),
		.image_info.image_max_size = SOC_FW_CONFIG_LIMIT - SOC_FW_CONFIG_BASE,
		.image_info.image_base = SOC_FW_CONFIG_BASE,

		.next_handoff_image_id = RMM_IMAGE_ID,
	}, /* Finish load for rzv2l_cm33_rpmsg_demo_secure_vector.bin */
	{
		.image_id = RMM_IMAGE_ID,

		SET_STATIC_PARAM_HEAD(ep_info, PARAM_EP, VERSION_2,
				entry_point_info_t, NON_SECURE | EXECUTABLE),
		.ep_info.pc = RMM_FW_BASE,
		.ep_info.spsr = 0,

		SET_STATIC_PARAM_HEAD(image_info, PARAM_EP, VERSION_2,
				image_info_t, 0),
		.image_info.image_max_size = RMM_FW_LIMIT - RMM_FW_BASE,
		.image_info.image_base = RMM_FW_BASE,

		.next_handoff_image_id = BL331_IMAGE_ID,
	}, /* Finish load for rzv2l_cm33_rpmsg_demo_non_secure_code.bin */
	{
		.image_id = BL331_IMAGE_ID,

		SET_STATIC_PARAM_HEAD(ep_info, PARAM_EP, VERSION_2,
				entry_point_info_t, NON_SECURE | NON_EXECUTABLE),
		.ep_info.pc = BL331_BASE,
		.ep_info.spsr = 0,

		SET_STATIC_PARAM_HEAD(image_info, PARAM_EP, VERSION_2,
				image_info_t, 0),
		.image_info.image_max_size = BL331_LIMIT - BL331_BASE,
		.image_info.image_base = BL331_BASE,

		.next_handoff_image_id = BL332_IMAGE_ID,
	},
	{
		.image_id = BL332_IMAGE_ID,

		SET_STATIC_PARAM_HEAD(ep_info, PARAM_EP, VERSION_2,
			entry_point_info_t, NON_SECURE | EXECUTABLE),
		.ep_info.spsr = SPSR_64(BL33_MODE, MODE_SP_ELX,
			DISABLE_ALL_EXCEPTIONS),
		.ep_info.pc = BL332_BASE,
#ifdef RZ_BL332_ARG0
		.ep_info.args.arg0 = RZ_BL332_ARG0,
#endif
		SET_STATIC_PARAM_HEAD(image_info, PARAM_EP, VERSION_2,
			image_info_t, 0),
		.image_info.image_max_size =
				(uint32_t) (BL332_LIMIT - BL332_BASE),
		.image_info.image_base = BL332_BASE,

		.next_handoff_image_id = INVALID_IMAGE_ID,
	}
};

REGISTER_BL_IMAGE_DESCS(bl2_mem_params_descs)
