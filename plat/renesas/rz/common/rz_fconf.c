#include <common/debug.h>
#include <lib/fconf/fconf.h>
#include <lib/libfdt/libfdt.h>
#include <rz_fconf.h>

struct cpg_config_t cpg_config;
struct sysc_config_t sysc_config;
struct pfc_config_t pfc_config;
struct ddr_config_t ddr_config;
struct spi_config_t spi_config;

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
 * CPG FCONF function
 **********************************************************************/
int fconf_populate_cpg_config(uintptr_t config)
{
	const void *fdt = (const void *)config;

	int soc_node = fdt_path_offset(fdt, "/soc");
	int cpg_node = fdt_subnode_offset(fdt, soc_node, "clock-controller@11010000");

	const char *cpg_props[] = {
		"divpl1_set",
		"divpl1_set_wen",
		"cpg_pll4_clk1",
		"cpg_pll4_clk2",
		"cpg_pll4_stby",
		"cpg_pll6_clk1",
		"cpg_pll6_clk2",
		"cpg_pll6_stby"
	};

	uint32_t *cpg_targets[] = {
		&cpg_config.divpl1_set,
		&cpg_config.divpl1_set_wen,
		&cpg_config.pll4_clk1,
		&cpg_config.pll4_clk2,
		&cpg_config.pll4_stby,
		&cpg_config.pll6_clk1,
		&cpg_config.pll6_clk2,
		&cpg_config.pll6_stby
	};

	fconf_read_u32_props(fdt, cpg_node, cpg_props, (uint32_t **)cpg_targets, ARRAY_SIZE(cpg_props));

	return 0;
}

const struct cpg_config_t *cpg_config_getter(void)
{
	return &cpg_config;
}

/**********************************************************************
 * SYSC FCONF function
 **********************************************************************/
int fconf_populate_sysc_config(uintptr_t config)
{
	const void *fdt = (const void *)config;

	int soc_node = fdt_path_offset(fdt, "/soc");
	int sysc_node = fdt_subnode_offset(fdt, soc_node, "system-controller@11020000");

	const char *sysc_props[] = { "syc_inck_hz" };
	uint32_t *targets[] = { &sysc_config.syc_inck_hz };

	fconf_read_u32_props(fdt, sysc_node, sysc_props, (uint32_t **)targets, ARRAY_SIZE(sysc_props));

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
	const void *fdt = (const void *)config;

	int soc_node = fdt_path_offset(fdt, "/soc");
	int pfc_node = fdt_subnode_offset(fdt, soc_node, "pinctrl@11030000");

	const char *pfc_props[] = {
		"pfc_qspi0_iolh0a",
		"pfc_qspi0_pupd0a",
		"pfc_qspi0_sr0a",
		"pfc_qspi1_iolh0b",
		"pfc_qspi1_pupd0b",
		"pfc_qspi1_sr0b",
		"pfc_qspin_iolh0c",
		"pfc_qspin_pupd0c",
		"pfc_qspin_sr0c",
	};

	uint32_t *targets[] = { 
		&pfc_config.qspi0_iolh0a,
		&pfc_config.qspi0_pupd0a,
		&pfc_config.qspi0_sr0a,
		&pfc_config.qspi1_iolh0b,
		&pfc_config.qspi1_pupd0b,
		&pfc_config.qspi1_sr0b,
		&pfc_config.qspin_iolh0c,
		&pfc_config.qspin_pupd0c,
		&pfc_config.qspin_sr0c,
	};

	fconf_read_u32_props(fdt, pfc_node, pfc_props, (uint32_t **)targets, ARRAY_SIZE(pfc_props));

	return 0;
}

const struct pfc_config_t *pfc_config_getter(void)
{
	return &pfc_config;
}

/**********************************************************************
 * DDR FCONF function
 **********************************************************************/
int fconf_populate_ddr_config(uintptr_t config)
{
	const void *fdt = (const void *)config;

	int soc_node = fdt_path_offset(fdt, "/soc");
	int ddr_node = fdt_subnode_offset(fdt, soc_node, "memory@40000000");

	const char *ddr_props[] = {
		"ddrmc_r000",
		"ddrmc_r001",
		"ddrmc_r007",
		"ddrmc_r020",
		"ddrmc_r023",
		"ddrmc_r027",
		"ddrmc_r029",
	};

	uint32_t *targets[] = {
		&ddr_config.mc_r000,
		&ddr_config.mc_r001,
		&ddr_config.mc_r007,
		&ddr_config.mc_r020,
		&ddr_config.mc_r023,
		&ddr_config.mc_r027,
		&ddr_config.mc_r029,
	};

	fconf_read_u32_props(fdt, ddr_node, ddr_props, (uint32_t **)targets, ARRAY_SIZE(ddr_props));

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
	const void *fdt = (const void *)config;

	int soc_node = fdt_path_offset(fdt, "/soc");
	int spi_node = fdt_subnode_offset(fdt, soc_node, "spi@10060000");

	const char *spi_props[] = {
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
FCONF_REGISTER_POPULATOR(HW_CONFIG, cpg_config, fconf_populate_cpg_config);
FCONF_REGISTER_POPULATOR(HW_CONFIG, sysc_config, fconf_populate_sysc_config);
FCONF_REGISTER_POPULATOR(HW_CONFIG, pfc_config, fconf_populate_pfc_config);
FCONF_REGISTER_POPULATOR(HW_CONFIG, ddr_config, fconf_populate_ddr_config);
FCONF_REGISTER_POPULATOR(HW_CONFIG, spi_config, fconf_populate_spi_config);
