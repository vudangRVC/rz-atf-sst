/*
 * Copyright (c) 2025, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <stddef.h>
#include "ddr_regs.h"
#include "ddr_private.h"
#include <common/debug.h>
#include <lib/fconf/fconf.h>
#include <lib/libfdt/libfdt.h>

static int8_t dwc_ddrphy_cdd_int(uint8_t val);
static int8_t dwc_ddrphy_cdd_abs(uint8_t val);

extern u_register_t dtb_base;

void setup_mc(void)
{
	int i;
	const void *fdt = (const void *)(uintptr_t)dtb_base;
	int len;

	int soc_node = fdt_path_offset(fdt, "/soc");
	int ddr_node = fdt_subnode_offset(fdt, soc_node, "memory@40000000");
	const fdt32_t *prop = fdt_getprop(fdt, ddr_node, "param_setup_mc", &len);

	/* Iterate over pairs */
	INFO("DDR: setup_mc from DTB\n");

	for (i = 0; i < (len / sizeof(uint32_t)); i += 2) {
		uint32_t addr = fdt32_to_cpu(prop[i]);
		uint32_t val  = fdt32_to_cpu(prop[i + 1]);

		ddrtop_mc_apb_wr(addr, val);
	}
}

void update_mc(void)
{
	ddrtop_mc_param_wr(INT_MASK_MASTER_ADDR, INT_MASK_MASTER_OFFSET+31, 1, 0);
	uint32_t val = ddrtop_mc_param_rd(PCPCS_PD_EN_ADDR, PCPCS_PD_EN_OFFSET, PCPCS_PD_EN_WIDTH);

	if (val == 1) {
		ddrtop_mc_param_wr(LPI_WAKEUP_EN_ADDR, LPI_WAKEUP_EN_OFFSET, LPI_WAKEUP_EN_WIDTH, 0x1E);
	} else {
		ddrtop_mc_param_wr(LPI_WAKEUP_EN_ADDR, LPI_WAKEUP_EN_OFFSET, LPI_WAKEUP_EN_WIDTH, 0x1F);
	}
}

void phyinit_configuration(void)
{
	int i;
	const void *fdt = (const void *)(uintptr_t)dtb_base;
	int len;

	int soc_node = fdt_path_offset(fdt, "/soc");
	int ddr_node = fdt_subnode_offset(fdt, soc_node, "memory@40000000");
	const fdt32_t *prop = fdt_getprop(fdt, ddr_node, "param_phyinit_c", &len);

	/* Iterate over pairs */
	INFO("DDR: param_phyinit_c from DTB\n");

	for (i = 0; i < (len / sizeof(uint32_t)); i += 2) {
		uint32_t addr = fdt32_to_cpu(prop[i]);
		uint32_t val  = fdt32_to_cpu(prop[i + 1]);

		dwc_ddrphy_apb_wr(addr, val);
	}
}

void phyinit_pin_swizzling(void)
{
	int i;
	const void *fdt = (const void *)(uintptr_t)dtb_base;
	int len;

	int soc_node = fdt_path_offset(fdt, "/soc");
	int ddr_node = fdt_subnode_offset(fdt, soc_node, "memory@40000000");
	const fdt32_t *prop = fdt_getprop(fdt, ddr_node, "param_phyinit_swizzle", &len);

	/* Iterate over pairs */
	INFO("DDR: phyinit_pin_swizzling from DTB\n");

	for (i = 0; i < (len / sizeof(uint32_t)); i += 2) {
		uint32_t addr = fdt32_to_cpu(prop[i]);
		uint32_t val  = fdt32_to_cpu(prop[i + 1]);

		dwc_ddrphy_apb_wr(addr, val);
	}
}

void phyinit_load_1d_image(void)
{
	int i;
	const void *fdt = (const void *)(uintptr_t)dtb_base;
	int len;

	int soc_node = fdt_path_offset(fdt, "/soc");
	int ddr_node = fdt_subnode_offset(fdt, soc_node, "memory@40000000");
	const fdt32_t *prop = fdt_getprop(fdt, ddr_node, "phyinit_1d", &len);

	/* Iterate over pairs */
	INFO("DDR: phyinit_load_1d_image from DTB\n");

	dwc_ddrphy_apb_wr(0x020060, 0x2);

	dwc_ddrphy_apb_wr(0x0d0000, 0x0);

	for (i = 0x0; i <= 0x3fff; i++)	{
		uint32_t val = fdt32_to_cpu(prop[i]);

		dwc_ddrphy_apb_wr((uint32_t)(0x50000+i), (uint32_t)(val));
	}

	dwc_ddrphy_apb_wr(0x0d0000, 0x1);

	prop = fdt_getprop(fdt, ddr_node, "param_phyinit_f_1d_0", &len);
	for (i = 0; i < (len / sizeof(uint32_t)); i += 2) {
		uint32_t addr = fdt32_to_cpu(prop[i]);
		uint32_t val  = fdt32_to_cpu(prop[i + 1]);

		dwc_ddrphy_apb_wr(addr, val);
	}
}

void phyinit_exec_1d_image(void)
{
	uint32_t val, sel_train, num_rank;
	int8_t val0, val1, cdd_rr, cdd_rw_abs, cdd_ww, cdd_ww_abs;
	uint32_t r2r_adr, r2r_ofs, r2r_wid;
	uint32_t r2w_adr, r2w_ofs, r2w_wid;
	uint32_t w2r_adr, w2r_ofs, w2r_wid;
	uint32_t w2w_adr, w2w_ofs, w2w_wid;

	dwc_ddrphy_apb_wr(0x0d0000, 0x0);
	val = dwc_ddrphy_apb_rd(0x01005f);
	sel_train = ((val & 0x700) == 0x100) ? 0 : 2;
	dwc_ddrphy_apb_wr(0x0d0000, 0x1);

	dwc_ddrphy_apb_wr(0x0d0000, 0x1);
	dwc_ddrphy_apb_wr(0x0d0099, 0x9);
	dwc_ddrphy_apb_wr(0x0d0099, 0x1);
	dwc_ddrphy_apb_wr(0x0d0099, 0x0);
	dwc_ddrphy_phyinit_userCustom_G_waitDone(sel_train);
	dwc_ddrphy_apb_wr(0x0d0099, 0x1);

	dwc_ddrphy_apb_wr(0x0d0000, 0x0);

	val = ddrtop_mc_param_rd(CS_MAP_ADDR, CS_MAP_OFFSET, CS_MAP_WIDTH);
	num_rank = (val == 3) ? 2 : 1;

	if (num_rank > 1) {
		val = dwc_ddrphy_apb_rd(0x54013);
		val0 = dwc_ddrphy_cdd_int((val>>0)&0xff);
		val1 = dwc_ddrphy_cdd_int((val>>8)&0xff);
		cdd_rr = val0;
		cdd_rr = (val1 > cdd_rr) ? val1 : cdd_rr;

		val = dwc_ddrphy_apb_rd(0x54014);
		val0 = dwc_ddrphy_cdd_abs((val>>0)&0xff);
		val1 = dwc_ddrphy_cdd_abs((val>>8)&0xff);
		cdd_rw_abs = val0;
		cdd_rw_abs = (val1 > cdd_rw_abs) ? val1 : cdd_rw_abs;

		val = dwc_ddrphy_apb_rd(0x54015);
		val0 = dwc_ddrphy_cdd_abs((val>>0)&0xff);
		val1 = dwc_ddrphy_cdd_abs((val>>8)&0xff);
		cdd_rw_abs = (val0 > cdd_rw_abs) ? val0 : cdd_rw_abs;
		cdd_rw_abs = (val1 > cdd_rw_abs) ? val1 : cdd_rw_abs;

		val = dwc_ddrphy_apb_rd(0x54018);
		val0 = dwc_ddrphy_cdd_int((val>>0)&0xff);
		val1 = dwc_ddrphy_cdd_int((val>>8)&0xff);
		cdd_ww = val0;
		cdd_ww = (val1 > cdd_ww) ? val1 : cdd_ww;
		val0 = dwc_ddrphy_cdd_abs((val>>0)&0xff);
		val1 = dwc_ddrphy_cdd_abs((val>>8)&0xff);
		cdd_ww_abs = val0;
		cdd_ww_abs = (val1 > cdd_ww_abs) ? val1 : cdd_ww_abs;

		val = dwc_ddrphy_apb_rd(0x5402c);
		val1 = dwc_ddrphy_cdd_int((val>>8)&0xff);
		cdd_rr = (val1 > cdd_rr) ? val1 : cdd_rr;

		val = dwc_ddrphy_apb_rd(0x5402d);
		val0 = dwc_ddrphy_cdd_int((val>>0)&0xff);
		cdd_rr = (val0 > cdd_rr) ? val0 : cdd_rr;
		val1 = dwc_ddrphy_cdd_abs((val>>8)&0xff);
		cdd_rw_abs = (val1 > cdd_rw_abs) ? val1 : cdd_rw_abs;

		val = dwc_ddrphy_apb_rd(0x5402e);
		val0 = dwc_ddrphy_cdd_abs((val>>0)&0xff);
		val1 = dwc_ddrphy_cdd_abs((val>>8)&0xff);
		cdd_rw_abs = (val0 > cdd_rw_abs) ? val0 : cdd_rw_abs;
		cdd_rw_abs = (val1 > cdd_rw_abs) ? val1 : cdd_rw_abs;

		val = dwc_ddrphy_apb_rd(0x5402f);
		val0 = dwc_ddrphy_cdd_abs((val>>0)&0xff);
		cdd_rw_abs = (val0 > cdd_rw_abs) ? val0 : cdd_rw_abs;

		val = dwc_ddrphy_apb_rd(0x54031);
		val1 = dwc_ddrphy_cdd_int((val>>8)&0xff);
		cdd_ww = (val1 > cdd_ww) ? val1 : cdd_ww;
		val1 = dwc_ddrphy_cdd_abs((val>>8)&0xff);
		cdd_ww_abs = (val1 > cdd_ww_abs) ? val1 : cdd_ww_abs;

		val = dwc_ddrphy_apb_rd(0x54032);
		val0 = dwc_ddrphy_cdd_int((val>>0)&0xff);
		cdd_ww = (val0 > cdd_ww) ? val0 : cdd_ww;
		val0 = dwc_ddrphy_cdd_abs((val>>0)&0xff);
		cdd_ww_abs = (val0 > cdd_ww_abs) ? val0 : cdd_ww_abs;

		r2r_adr = R2R_DIFFCS_DLY_F0_ADDR; r2r_ofs = R2R_DIFFCS_DLY_F0_OFFSET; r2r_wid = R2R_DIFFCS_DLY_F0_WIDTH;
		r2w_adr = R2W_DIFFCS_DLY_F0_ADDR; r2w_ofs = R2W_DIFFCS_DLY_F0_OFFSET; r2w_wid = R2W_DIFFCS_DLY_F0_WIDTH;
		w2r_adr = W2R_DIFFCS_DLY_F0_ADDR; w2r_ofs = W2R_DIFFCS_DLY_F0_OFFSET; w2r_wid = W2R_DIFFCS_DLY_F0_WIDTH;
		w2w_adr = W2W_DIFFCS_DLY_F0_ADDR; w2w_ofs = W2W_DIFFCS_DLY_F0_OFFSET; w2w_wid = W2W_DIFFCS_DLY_F0_WIDTH;

		if (cdd_rr > 0) {
			val = ddrtop_mc_param_rd(r2r_adr, r2r_ofs, r2r_wid);
			val += cdd_rr;
			ddrtop_mc_param_wr(r2r_adr, r2r_ofs, r2r_wid, val);
		}

		if (cdd_ww > 0) {
			val = ddrtop_mc_param_rd(w2w_adr, w2w_ofs, w2w_wid);
			val += cdd_ww;
			ddrtop_mc_param_wr(w2w_adr, w2w_ofs, w2w_wid, val);
		}

		val = ddrtop_mc_param_rd(r2w_adr, r2w_ofs, r2w_wid);
		val += cdd_rw_abs;
		ddrtop_mc_param_wr(r2w_adr, r2w_ofs, r2w_wid, val);

		val = ddrtop_mc_param_rd(w2r_adr, w2r_ofs, w2r_wid);
		val += cdd_ww_abs;
		ddrtop_mc_param_wr(w2r_adr, w2r_ofs, w2r_wid, val);
	}

	dwc_ddrphy_apb_wr(0x0d0000, 0x1);
}

void phyinit_load_2d_image(void)
{
	int i;
	const void *fdt = (const void *)(uintptr_t)dtb_base;
	int len;

	int soc_node = fdt_path_offset(fdt, "/soc");
	int ddr_node = fdt_subnode_offset(fdt, soc_node, "memory@40000000");
	const fdt32_t *prop = fdt_getprop(fdt, ddr_node, "phyinit_2d", &len);
	
	/* Iterate over pairs */
	INFO("DDR: phyinit_load_2d_image from DTB\n");

	dwc_ddrphy_apb_wr(0x0d0000, 0x0);

	for (i = 0x0; i <= 0x3fff; i++)	{
		uint32_t val = fdt32_to_cpu(prop[i]);

		dwc_ddrphy_apb_wr((uint32_t)(0x50000+i), (uint32_t)(val));
	}

	dwc_ddrphy_apb_wr(0x0d0000, 0x1);

	prop = fdt_getprop(fdt, ddr_node, "param_phyinit_f_2d_0", &len);
	for (i = 0; i < (len / sizeof(uint32_t)); i += 2) {
		uint32_t addr = fdt32_to_cpu(prop[i]);
		uint32_t val  = fdt32_to_cpu(prop[i + 1]);

		dwc_ddrphy_apb_wr(addr, val);
	}
}

void phyinit_exec_2d_image(void)
{
	uint32_t val, sel_train;

	dwc_ddrphy_apb_wr(0x0d0000, 0x0);
	val = dwc_ddrphy_apb_rd(0x01005f);
	sel_train = ((val & 0x700) == 0x100) ? 1 : 3;
	dwc_ddrphy_apb_wr(0x0d0000, 0x1);

	dwc_ddrphy_apb_wr(0x0d0000, 0x1);
	dwc_ddrphy_apb_wr(0x0d0099, 0x9);
	dwc_ddrphy_apb_wr(0x0d0099, 0x1);
	dwc_ddrphy_apb_wr(0x0d0099, 0x0);
	dwc_ddrphy_phyinit_userCustom_G_waitDone(sel_train);
	dwc_ddrphy_apb_wr(0x0d0099, 0x1);

	dwc_ddrphy_apb_wr(0x0d0000, 0x0);
	dwc_ddrphy_apb_wr(0x0d0000, 0x1);
}

void phyinit_load_eng_image(void)
{
	int i;
	const void *fdt = (const void *)(uintptr_t)dtb_base;
	int len;

	int soc_node = fdt_path_offset(fdt, "/soc");
	int ddr_node = fdt_subnode_offset(fdt, soc_node, "memory@40000000");
	const fdt32_t *prop = fdt_getprop(fdt, ddr_node, "param_phyinit_i", &len);

	/* Iterate over pairs */
	INFO("DDR: phyinit_load_eng_image from DTB\n");

	for (i = 0; i < (len / sizeof(uint32_t)); i += 2) {
		uint32_t addr = fdt32_to_cpu(prop[i]);
		uint32_t val  = fdt32_to_cpu(prop[i + 1]);

		dwc_ddrphy_apb_wr(addr, val);
	}
}

static int8_t dwc_ddrphy_cdd_int(uint8_t val)
{
	return (int8_t)((((val>>7)&0x1) == 1) ? -((0x7f^((val>>0)&0x7f)) + 1) : ((val>>0)&0x7f));
}

static int8_t dwc_ddrphy_cdd_abs(uint8_t val)
{
	return (uint8_t)((((val>>7)&0x1) == 1) ? ((0x7f^((val>>0)&0x7f)) + 1) : ((val>>0)&0x7f));
}
