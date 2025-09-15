#include <common/debug.h>
#include <lib/fconf/fconf.h>
#include <lib/libfdt/libfdt.h>
#include <platform_def.h>
#include <rz_fconf.h>
#include <rz_dt.h>

struct common_config_t common_config;
struct cpg_config_t cpg_config;
struct syc_config_t syc_config;
struct sysc_config_t sysc_config;
struct pfc_config_t pfc_config;
struct scif_config_t scif_config;
struct ddr_config_t ddr_config;
struct spi_config_t spi_config;

extern u_register_t bootrom_param0;

/**********************************************************************
 * Common helper function
 **********************************************************************/ 
void fconf_read_u32_props(const void *fdt, int node_offset,
						const char **prop_names,
						uint32_t **target_ptrs,
						size_t count)
{
	for (size_t i = 0; i < count; ++i) {
		int len;
		const fdt32_t *val = fdt_getprop(fdt, node_offset, prop_names[i], &len);
		if (!val || len < 4) {
			WARN("Missing or invalid property: %s\n", prop_names[i]);
			continue;
		}

		*target_ptrs[i] = fdt32_to_cpu(val[0]);
		INFO("Parsed %s = 0x%08x\n", prop_names[i], *target_ptrs[i]);
	}
}

void fconf_set_array_props(const void *fdt, int node_offset,
						const char * prop_names,
						uint32_t * target_ptrs)
{
	int len;
	const fdt32_t *val = fdt_getprop(fdt, node_offset, prop_names, &len);

	for (size_t i = 0; i < (size_t)(len / sizeof(uint32_t)); i += 2) {
		uint32_t reg = fdt32_to_cpu(val[i]);
		uint32_t val32 = fdt32_to_cpu(val[i+1]);
		target_ptrs[i]   = reg;
		target_ptrs[i+1] = val32;
	}
}

/**********************************************************************
 * COMMON FCONF function
 **********************************************************************/
int fconf_populate_common_config(uintptr_t config)
{
	void *fdt = (void *)config;

	int soc_node = fdt_path_offset(fdt, "/soc");

	const char *common_props[] = {
		"soc_id",
		"bl2_limit",
		"enable_cold_boot",
		"spirom_fip_base",
		"spirom_fip_size",
		"emmc_fip_base",
		"emmc_fip_size",
		"sd_fip_base",
		"sd_fip_size",
	};

	uint32_t *common_targets[] = {
		&common_config.soc_id,
		&common_config.bl2_limit,
		&common_config.enable_cold_boot,
		&common_config.spirom_fip_base,
		&common_config.spirom_fip_size,
		&common_config.emmc_fip_base,
		&common_config.emmc_fip_size,
		&common_config.sd_fip_base,
		&common_config.sd_fip_size,
	};
	
	fconf_read_u32_props(fdt, soc_node, common_props, (uint32_t **)common_targets, ARRAY_SIZE(common_props));

	return 0;
}

const struct common_config_t *common_config_getter(void)
{
	return &common_config;
}

/**********************************************************************
 * CPG FCONF function
 **********************************************************************/
int fconf_populate_cpg_config(uintptr_t config)
{
	void *fdt = (void *)config;

	int soc_node = fdt_path_offset(fdt, "/soc");
	const char *cpg_path = NULL;

	if (bootrom_param0 == RZG2L_BL2_BASE) {
		cpg_path = "clock-controller@11010000";
	} else if (bootrom_param0 == RZV2H_BL2_BASE) {
		cpg_path = "clock-controller@10420000";
	}

	int cpg_node = fdt_subnode_offset(fdt, soc_node, cpg_path);

	/* Preserve the CPG node for further SoC-specific initialization. */
	cpg_config.cpg_node = cpg_node;

	const char *cpg_props[] = {
		"cpg_type",
		"cpg_selector_on_off",
		"cpg_early_setup",
		"cpg_mstop_setup",
	};

	uint32_t *cpg_targets[] = {
		&cpg_config.cpg_type,
		&cpg_config.cpg_selector_on_off,
		&cpg_config.cpg_early_setup,
		&cpg_config.cpg_mstop_setup,
	};

	read_prop_from_subnode(fdt, "/soc", cpg_path, "reg", 1, &cpg_config.cpg_base);
	fconf_read_u32_props(fdt, cpg_node, cpg_props, (uint32_t **)cpg_targets, ARRAY_SIZE(cpg_props));

	return 0;
}

const struct cpg_config_t *cpg_config_getter(void)
{
	return &cpg_config;
}

/**********************************************************************
 * SYC FCONF function
 **********************************************************************/
int fconf_populate_syc_config(uintptr_t config)
{
	void *fdt = (void *)config;

	int soc_node = fdt_path_offset(fdt, "/soc");
	const char *syc_path = NULL;

	if (bootrom_param0 == RZG2L_BL2_BASE) {
		syc_path = "system-counter@11000000";
	} else if (bootrom_param0 == RZV2H_BL2_BASE) {
		syc_path = "system-counter@14010000";
	}

	int syc_node = fdt_subnode_offset(fdt, soc_node, syc_path);

	const char *syc_props[] = { "syc_inck_hz" };
	uint32_t *targets[] = { &syc_config.syc_inck_hz };

	read_prop_from_subnode(fdt, "/soc", syc_path, "reg", 1, &syc_config.syc_base);
	fconf_read_u32_props(fdt, syc_node, syc_props, (uint32_t **)targets, ARRAY_SIZE(syc_props));

	return 0;
}

const struct syc_config_t *syc_config_getter(void)
{
	return &syc_config;
}

/**********************************************************************
 * SYSC FCONF function
 **********************************************************************/
int fconf_populate_sysc_config(uintptr_t config)
{
	void *fdt = (void *)config;

	int soc_node = fdt_path_offset(fdt, "/soc");
	const char *sysc_path = NULL;

	if (bootrom_param0 == RZG2L_BL2_BASE) {
		sysc_path = "system-controller@11020000";
	} else if (bootrom_param0 == RZV2H_BL2_BASE) {
		sysc_path = "system-controller@10430000";
	}

	int sysc_node = fdt_subnode_offset(fdt, soc_node, sysc_path);

	const char *sysc_props[] = {
		"enable_pwrc_setup",
		"sysc_lsi_mode_mask",
		"sysc_lsi_mode_reg_offset",
		"sysc_boot_mode_esd",
		"sysc_boot_mode_emmc_1_8",
		"sysc_boot_mode_emmc_3_3",
		"sysc_boot_mode_spi_1_8",
		"sysc_boot_mode_spi_3_3",

		"tzc_msram_setup",
		"tzc_m33_base",
		"tzc_spi_setup",
		"tzc_spi_base",
		"tzc_ddr_setup",
		"tzc_ddr00_base",
		"tzc_ddr01_base",
		"tzc_ddr10_base",
		"tzc_ddr11_base",
		"tzc_pci_setup",
		"tzc_pci_base",
		"tzc_r8_setup",
		"tzc_r8_base",
	};

	uint32_t *targets[] = {
		&sysc_config.enable_pwrc_setup,
		&sysc_config.sysc_lsi_mode_mask,
		&sysc_config.sysc_lsi_mode_reg_offset,
		&sysc_config.sysc_boot_mode_esd,
		&sysc_config.sysc_boot_mode_emmc_1_8,
		&sysc_config.sysc_boot_mode_emmc_3_3,
		&sysc_config.sysc_boot_mode_spi_1_8,
		&sysc_config.sysc_boot_mode_spi_3_3,

		&sysc_config.tzc_msram_setup,
		&sysc_config.tzc_m33_base,
		&sysc_config.tzc_spi_setup,
		&sysc_config.tzc_spi_base,
		&sysc_config.tzc_ddr_setup,
		&sysc_config.tzc_ddr00_base,
		&sysc_config.tzc_ddr01_base,
		&sysc_config.tzc_ddr10_base,
		&sysc_config.tzc_ddr11_base,
		&sysc_config.tzc_pci_setup,
		&sysc_config.tzc_pci_base,
		&sysc_config.tzc_r8_setup,
		&sysc_config.tzc_r8_base,
	};

	read_prop_from_subnode(fdt, "/soc", sysc_path, "reg", 1, &sysc_config.sysc_base);
	fconf_read_u32_props(fdt, sysc_node, sysc_props, (uint32_t **)targets, ARRAY_SIZE(sysc_props));
	fconf_set_array_props(fdt, sysc_node, "sys_acctl", &sysc_config.sys_acctl[0]);

	return 0;
}

const struct sysc_config_t *sysc_config_getter(void)
{
	return &sysc_config;
}

/**********************************************************************
 * PFC FCONF function
 **********************************************************************/
int fconf_populate_pfc_config(uintptr_t config)
{
	void *fdt = (void *)config;

	int soc_node = fdt_path_offset(fdt, "/soc");
	const char *pfc_path = NULL;

	if (bootrom_param0 == RZG2L_BL2_BASE) {
		pfc_path = "pinctrl@11030000";
	} else if (bootrom_param0 == RZV2H_BL2_BASE) {
		pfc_path = "pinctrl@10410000";
	}

	int pfc_node = fdt_subnode_offset(fdt, soc_node, pfc_path);

	/* Preserve the PFC node for further SoC-specific initialization. */
	pfc_config.pfc_node = pfc_node;

	const char *pfc_props[] = {
		"pfc_type",
		"pfc_mux_setup",
		"pfc_qspi_setup",
		"pfc_sd_setup",
		"pfc_scif_setup",
		"pfc_drive_setup",
		"pfc_riic_pmic_setup",
	};

	uint32_t *targets[] = {
		&pfc_config.pfc_type,
		&pfc_config.pfc_mux_setup,
		&pfc_config.pfc_qspi_setup,
		&pfc_config.pfc_sd_setup,
		&pfc_config.pfc_scif_setup,
		&pfc_config.pfc_drive_setup,
		&pfc_config.pfc_riic_pmic_setup,
	};

	read_prop_from_subnode(fdt, "/soc", pfc_path, "reg", 1, &pfc_config.pfc_base);
	fconf_read_u32_props(fdt, pfc_node, pfc_props, (uint32_t **)targets, ARRAY_SIZE(pfc_props));

	return 0;
}

const struct pfc_config_t *pfc_config_getter(void)
{
	return &pfc_config;
}

/**********************************************************************
 * SCIF FCONF function
 **********************************************************************/
int fconf_populate_scif_config(uintptr_t config)
{
	void *fdt = (void *)config;

	int soc_node = fdt_path_offset(fdt, "/soc");
	const char *scif_path = NULL;

	if (bootrom_param0 == RZG2L_BL2_BASE) {
		scif_path = "serial@1004b800";
	} else if (bootrom_param0 == RZV2H_BL2_BASE) {
		scif_path = "serial@11c01400";
	}

	int scif_node = fdt_subnode_offset(fdt, soc_node, scif_path);

	const char *scif_props[] = {
		"uart_inck_hz",
		"uart_baudrate",
	};

	uint32_t *scif_targets[] = {
		&scif_config.uart_inck_hz,
		&scif_config.uart_baudrate,
	};

	read_prop_from_subnode(fdt, "/soc", scif_path, "reg", 1, &scif_config.scif_base);
	fconf_read_u32_props(fdt, scif_node, scif_props, (uint32_t **)scif_targets, ARRAY_SIZE(scif_props));

	return 0;
}

const struct scif_config_t *scif_config_getter(void)
{
	return &scif_config;
}

/**********************************************************************
 * DDR FCONF function
 **********************************************************************/
int fconf_populate_ddr_config(uintptr_t config)
{
	void *fdt = (void *)config;

	int soc_node = fdt_path_offset(fdt, "/soc");
	int ddr_node = fdt_subnode_offset(fdt, soc_node, "memory@40000000");

	const char *ddr_props[] = {
		"ddr_type",
	};

	uint32_t *ddr_targets[] = {
		&ddr_config.ddr_type,
	};

	fconf_read_u32_props(fdt, ddr_node, ddr_props, (uint32_t **) ddr_targets, ARRAY_SIZE(ddr_targets));

	/* Specific initilize for DDR4 */
	if (ddr_config.ddr_type == 0) {
		const char *ddr4_props[] = {
				"lp_cmd_offset",
				"ddrmc_r000",
				"ddrmc_r001",
				"ddrmc_r002",
				"ddrmc_r003",
				"ddrmc_r004",
				"ddrmc_r005",
				"ddrmc_r006",
				"ddrmc_r007",
				"ddrmc_r008",
				"ddrmc_r009",
				"ddrmc_r010",
				"ddrmc_r011",
				"ddrmc_r012",
				"ddrmc_r013",
				"ddrmc_r014",
				"ddrmc_r015",
				"ddrmc_r016",
				"ddrmc_r017",
				"ddrmc_r018",
				"ddrmc_r019",
				"ddrmc_r020",
				"ddrmc_r021",
				"ddrmc_r022",
				"ddrmc_r023",
				"ddrmc_r024",
				"ddrmc_r025",
				"ddrmc_r026",
				"ddrmc_r027",
				"ddrmc_r028",
				"ddrmc_r029",
				"ddrmc_r039",
				"ddrmc_r040",
				"ddrmc_r043",
				"ddrmc_r044",
				"mc_init_num",
			};

		uint32_t *ddr4_targets[] = {
			&ddr_config.lp_cmd_offset,
			&ddr_config.ddrmc_r000,
			&ddr_config.ddrmc_r001,
			&ddr_config.ddrmc_r002,
			&ddr_config.ddrmc_r003,
			&ddr_config.ddrmc_r004,
			&ddr_config.ddrmc_r005,
			&ddr_config.ddrmc_r006,
			&ddr_config.ddrmc_r007,
			&ddr_config.ddrmc_r008,
			&ddr_config.ddrmc_r009,
			&ddr_config.ddrmc_r010,
			&ddr_config.ddrmc_r011,
			&ddr_config.ddrmc_r012,
			&ddr_config.ddrmc_r013,
			&ddr_config.ddrmc_r014,
			&ddr_config.ddrmc_r015,
			&ddr_config.ddrmc_r016,
			&ddr_config.ddrmc_r017,
			&ddr_config.ddrmc_r018,
			&ddr_config.ddrmc_r019,
			&ddr_config.ddrmc_r020,
			&ddr_config.ddrmc_r021,
			&ddr_config.ddrmc_r022,
			&ddr_config.ddrmc_r023,
			&ddr_config.ddrmc_r024,
			&ddr_config.ddrmc_r025,
			&ddr_config.ddrmc_r026,
			&ddr_config.ddrmc_r027,
			&ddr_config.ddrmc_r028,
			&ddr_config.ddrmc_r029,
			&ddr_config.ddrmc_r039,
			&ddr_config.ddrmc_r040,
			&ddr_config.ddrmc_r043,
			&ddr_config.ddrmc_r044,
			&ddr_config.mc_init_num,
		};
		
		fconf_read_u32_props(fdt, ddr_node, ddr4_props, (uint32_t **) ddr4_targets, ARRAY_SIZE(ddr4_targets));
		
		fconf_set_array_props(fdt, ddr_node, "swizzle_mc_tbl", &ddr_config.swizzle_mc_tbl[0]);
		fconf_set_array_props(fdt, ddr_node, "swizzle_phy_tbl", &ddr_config.swizzle_phy_tbl[0]);
		fconf_set_array_props(fdt, ddr_node, "mc_odt_pins_tbl", &ddr_config.mc_odt_pins_tbl[0]);
		fconf_set_array_props(fdt, ddr_node, "mc_mr1_tbl", &ddr_config.mc_mr1_tbl[0]);
		fconf_set_array_props(fdt, ddr_node, "mc_mr2_tbl", &ddr_config.mc_mr2_tbl[0]);
		fconf_set_array_props(fdt, ddr_node, "mc_mr5_tbl", &ddr_config.mc_mr5_tbl[0]);
		fconf_set_array_props(fdt, ddr_node, "mc_mr6_tbl", &ddr_config.mc_mr6_tbl[0]);
		fconf_set_array_props(fdt, ddr_node, "mc_init_tbl", &ddr_config.mc_init_tbl[0]);
		fconf_set_array_props(fdt, ddr_node, "mc_phy_settings_tbl", &ddr_config.mc_phy_settings_tbl[0]);
	}

	return 0;
}

const struct ddr_config_t *ddr_config_getter(void)
{
	return &ddr_config;
}

/**********************************************************************
 * SPI FCONF function
 **********************************************************************/
int fconf_populate_spi_config(uintptr_t config)
{
	void *fdt = (void *)config;

	int soc_node = fdt_path_offset(fdt, "/soc");
	const char *spi_path = NULL;
	
	if (bootrom_param0 == RZG2L_BL2_BASE) {
		spi_path = "spi@10060000";
	} else if (bootrom_param0 == RZV2H_BL2_BASE) {
		spi_path = "spi@11030000";
	}
	int spi_node = fdt_subnode_offset(fdt, soc_node, spi_path);

	const char *spi_props[] = {
		"spi_type",
		"spim_phycnt",
		"spim_phyoffset1",
		"spim_phyoffset2",
		"spim_cmncr",
		"spim_ssldr",
		"spim_drcr",
		"spim_drcmr",
		"spim_drear",
		"spim_drenr",
		"spim_drdmcr",
		"spim_drdrenr",
	};

	uint32_t *targets[] = {
		&spi_config.spi_type,
		&spi_config.phycnt,
		&spi_config.phyoffset1,
		&spi_config.phyoffset2,
		&spi_config.cmncr,
		&spi_config.ssldr,
		&spi_config.drcr,
		&spi_config.drcmr,
		&spi_config.drear,
		&spi_config.drenr,
		&spi_config.drdmcr,
		&spi_config.drdrenr,
	};

	read_prop_from_subnode(fdt, "/soc", spi_path, "reg", 1, &spi_config.spi_base);
	fconf_read_u32_props(fdt, spi_node, spi_props, (uint32_t **)targets, ARRAY_SIZE(spi_props));

	return 0;
}

const struct spi_config_t *spi_config_getter(void)
{
	return &spi_config;
}

/**********************************************************************
 * FCONF registration
 **********************************************************************/
FCONF_REGISTER_POPULATOR(HW_CONFIG, common_config, fconf_populate_common_config);
FCONF_REGISTER_POPULATOR(HW_CONFIG, cpg_config, fconf_populate_cpg_config);
FCONF_REGISTER_POPULATOR(HW_CONFIG, syc_config, fconf_populate_syc_config);
FCONF_REGISTER_POPULATOR(HW_CONFIG, sysc_config, fconf_populate_sysc_config);
FCONF_REGISTER_POPULATOR(HW_CONFIG, pfc_config, fconf_populate_pfc_config);
FCONF_REGISTER_POPULATOR(HW_CONFIG, scif_config, fconf_populate_scif_config);
FCONF_REGISTER_POPULATOR(HW_CONFIG, ddr_config, fconf_populate_ddr_config);
FCONF_REGISTER_POPULATOR(HW_CONFIG, spi_config, fconf_populate_spi_config);
