#include <common/debug.h>
#include <lib/fconf/fconf.h>
#include <lib/libfdt/libfdt.h>
#include <platform_def.h>
#include <rz_fconf.h>
#include <rz_dt.h>

struct common_config_t common_config;
struct cpg_config_t cpg_config;
struct syc_config_t syc_config;
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


/**********************************************************************
 * COMMON FCONF function
 **********************************************************************/
int fconf_populate_common_config(uintptr_t config)
{
	void *fdt = (void *)config;
	const char *sysc_path = NULL;

	if (bootrom_param0 == RZCMN_BL2_BASE) {
		sysc_path = "system-controller@11020000";
	} else if (bootrom_param0 == RZV2H_BL2_BASE) {
		sysc_path = "system-controller@10430000";
	}

	int soc_node = fdt_path_offset(fdt, "/soc");

	const char *common_props[] = {
		"board_id",
	};
	
	uint32_t *common_targets[] = {
		&common_config.board_id,
	};
	
	read_prop_from_subnode(fdt, "/soc", sysc_path, "reg", 1, &common_config.sysc_base);
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

	if (bootrom_param0 == RZCMN_BL2_BASE) {
		cpg_path = "clock-controller@11010000";
	} else if (bootrom_param0 == RZV2H_BL2_BASE) {
		cpg_path = "clock-controller@10420000";
	}

	int cpg_node = fdt_subnode_offset(fdt, soc_node, cpg_path);

	const char *cpg_props[] = {
		"cpg_early_setup",
		"cpg_mstop_setup",
	};

	uint32_t *cpg_targets[] = {
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

	if (bootrom_param0 == RZCMN_BL2_BASE) {
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
 * PFC FCONF function
 **********************************************************************/
int fconf_populate_pfc_config(uintptr_t config)
{
	void *fdt = (void *)config;

	int soc_node = fdt_path_offset(fdt, "/soc");
	const char *pfc_path = NULL;

	if (bootrom_param0 == RZCMN_BL2_BASE) {
		pfc_path = "pinctrl@11030000";
	} else if (bootrom_param0 == RZV2H_BL2_BASE) {
		pfc_path = "pinctrl@10410000";
	}

	int pfc_node = fdt_subnode_offset(fdt, soc_node, pfc_path);

	const char *pfc_props[] = {
		"pfc_mux_setup",
		"pfc_qspi_setup",
		"pfc_sd_setup",
		"pfc_scif_setup",
		"pfc_drive_setup",
		"pfc_riic_pmic_setup",
	};

	uint32_t *targets[] = {
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

	if (bootrom_param0 == RZCMN_BL2_BASE) {
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

	const char *ddr_mc_props[] = {
		"ddrmc_r030",
		"ddrmc_r031",
		"ddrmc_r032",
		"ddrmc_r033",
		"ddrmc_r034",
		"ddrmc_r035",
		"ddrmc_r036",
		"ddrmc_r037",
		"ddrmc_r038",
	};

	const char *ddrphy_props[] = {
		"ddrphy_setup_step1",
		"ddrphy_setup_step2",
		"ddrphy_setup_step3",
		"ddrphy_setup_step4",
		"ddrphy_setup_step5",
		"ddrphy_setup_step6",
		"ddrphy_setup_step7",
		"ddrphy_setup_step8",
		"ddrphy_setup_step9",
		"ddrphy_setup_step10",
		"ddrphy_setup_step11",
		"ddrphy_setup_step12",
		"ddrphy_setup_step13",
		"ddrphy_setup_step14",
		"ddrphy_setup_step15",
		"ddrphy_setup_step16",
	};

	const char *ddr_denali_props[] = {
		"ddr_denali_ctl_30",
		"ddr_denali_ctl_34",
		"ddr_denali_ctl_35",
		"ddr_denali_ctl_122",
		"ddr_denali_ctl_123",
		"ddr_denali_ctl_124",
		"ddr_denali_ctl_125",
	};


	uint32_t *ddrmc_targets[] = {
		&ddr_config.ddrmc[0],
		&ddr_config.ddrmc[1],
		&ddr_config.ddrmc[2],
		&ddr_config.ddrmc[3],
		&ddr_config.ddrmc[4],
		&ddr_config.ddrmc[5],
		&ddr_config.ddrmc[6],
		&ddr_config.ddrmc[7],
		&ddr_config.ddrmc[8],
	};

	uint32_t *ddrphy_targets[] = {
		&ddr_config.ddrphy[0],
		&ddr_config.ddrphy[1],
		&ddr_config.ddrphy[2],
		&ddr_config.ddrphy[3],
		&ddr_config.ddrphy[4],
		&ddr_config.ddrphy[5],
		&ddr_config.ddrphy[6],
		&ddr_config.ddrphy[7],
		&ddr_config.ddrphy[8],
		&ddr_config.ddrphy[9],
		&ddr_config.ddrphy[10],
		&ddr_config.ddrphy[11],
		&ddr_config.ddrphy[12],
		&ddr_config.ddrphy[13],
		&ddr_config.ddrphy[14],
		&ddr_config.ddrphy[15],
	};

	uint32_t *ddrdenali_targets[] = { 
		&ddr_config.ddrdenali_30,
		&ddr_config.ddrdenali_34,
		&ddr_config.ddrdenali_35,
		&ddr_config.ddrdenali_122,
		&ddr_config.ddrdenali_123,
		&ddr_config.ddrdenali_124,
		&ddr_config.ddrdenali_125,
	};

	fconf_read_u32_props(fdt, ddr_node, ddr_mc_props, (uint32_t **) ddrmc_targets, ARRAY_SIZE(ddr_mc_props));
	fconf_read_u32_props(fdt, ddr_node, ddrphy_props, (uint32_t **) ddrphy_targets, ARRAY_SIZE(ddrphy_props));
	fconf_read_u32_props(fdt, ddr_node, ddr_denali_props, (uint32_t **) ddrdenali_targets, ARRAY_SIZE(ddr_denali_props));

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
	
	if (bootrom_param0 == RZCMN_BL2_BASE) {
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
FCONF_REGISTER_POPULATOR(HW_CONFIG, pfc_config, fconf_populate_pfc_config);
FCONF_REGISTER_POPULATOR(HW_CONFIG, scif_config, fconf_populate_scif_config);
FCONF_REGISTER_POPULATOR(HW_CONFIG, ddr_config, fconf_populate_ddr_config);
FCONF_REGISTER_POPULATOR(HW_CONFIG, spi_config, fconf_populate_spi_config);
