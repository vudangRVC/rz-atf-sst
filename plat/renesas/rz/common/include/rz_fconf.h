/*
 * Copyright (c) 2025, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __RZ_CONFIG_H__
#define __RZ_CONFIG_H__

#define hw_config__common_config_getter(prop) common_config.prop
#define hw_config__cpg_config_getter(prop) cpg_config.prop
#define hw_config__syc_config_getter(prop) syc_config.prop
#define hw_config__sysc_config_getter(prop) sysc_config.prop
#define hw_config__pfc_config_getter(prop) pfc_config.prop
#define hw_config__scif_config_getter(prop) scif_config.prop
#define hw_config__ddr_config_getter(prop) ddr_config.prop
#define hw_config__spi_config_getter(prop) spi_config.prop

struct common_config_t {
	uint32_t soc_id;
	uint32_t bl2_limit;
	uint32_t enable_cold_boot;
	uint32_t spirom_fip_base;
	uint32_t spirom_fip_size;
	uint32_t emmc_fip_base;
	uint32_t emmc_fip_size;
	uint32_t sd_fip_base;
	uint32_t sd_fip_size;
};

struct cpg_config_t {
	uint32_t cpg_node;
	uint32_t cpg_base;
	uint32_t cpg_type;
	uint32_t cpg_selector_on_off;
	uint32_t cpg_early_setup;
	uint32_t cpg_mstop_setup;
};

struct sysc_config_t {
	uint32_t sysc_base;
	uint32_t enable_pwrc_setup;
	uint32_t sysc_lsi_mode_mask;
	uint32_t sysc_lsi_mode_reg_offset;
	uint32_t sysc_boot_mode_esd;
	uint32_t sysc_boot_mode_emmc_1_8;
	uint32_t sysc_boot_mode_emmc_3_3;
	uint32_t sysc_boot_mode_spi_1_8;
	uint32_t sysc_boot_mode_spi_3_3;

	uint32_t tzc_msram_setup;
	uint32_t tzc_m33_base;
	uint32_t tzc_spi_setup;
	uint32_t tzc_spi_base;
	uint32_t tzc_ddr_setup;
	uint32_t tzc_ddr00_base;
	uint32_t tzc_ddr01_base;
	uint32_t tzc_ddr10_base;
	uint32_t tzc_ddr11_base;
	uint32_t tzc_pci_setup;
	uint32_t tzc_pci_base;
	uint32_t tzc_r8_setup;
	uint32_t tzc_r8_base;

	uint32_t sys_acctl[77];
};

struct syc_config_t {
	uint32_t syc_base;
	uint32_t syc_inck_hz;
};

struct pfc_config_t {
	uint32_t pfc_node;
	uint32_t pfc_base;
	uint32_t pfc_type;
	uint32_t pfc_mux_setup;
	uint32_t pfc_qspi_setup;
	uint32_t pfc_sd_setup;
	uint32_t pfc_scif_setup;
	uint32_t pfc_drive_setup;
	uint32_t pfc_riic_pmic_setup;
};


struct scif_config_t {
	uint32_t scif_base;
	uint32_t uart_inck_hz;
	uint32_t uart_baudrate;
};

struct ddr_config_t {
	uint32_t ddr_type;
	uint32_t lp_cmd_offset;
	uint32_t ddrmc_r000;
	uint32_t ddrmc_r001;
	uint32_t ddrmc_r002;
	uint32_t ddrmc_r003;
	uint32_t ddrmc_r004;
	uint32_t ddrmc_r005;
	uint32_t ddrmc_r006;
	uint32_t ddrmc_r007;
	uint32_t ddrmc_r008;
	uint32_t ddrmc_r009;
	uint32_t ddrmc_r010;
	uint32_t ddrmc_r011;
	uint32_t ddrmc_r012;
	uint32_t ddrmc_r013;
	uint32_t ddrmc_r014;
	uint32_t ddrmc_r015;
	uint32_t ddrmc_r016;
	uint32_t ddrmc_r017;
	uint32_t ddrmc_r018;
	uint32_t ddrmc_r019;
	uint32_t ddrmc_r020;
	uint32_t ddrmc_r021;
	uint32_t ddrmc_r022;
	uint32_t ddrmc_r023;
	uint32_t ddrmc_r024;
	uint32_t ddrmc_r025;
	uint32_t ddrmc_r026;
	uint32_t ddrmc_r027;
	uint32_t ddrmc_r028;
	uint32_t ddrmc_r029;
	uint32_t ddrmc_r039;
	uint32_t ddrmc_r040;
	uint32_t ddrmc_r043;
	uint32_t ddrmc_r044;
	
	uint32_t mc_init_num;
	uint32_t swizzle_mc_tbl[18];
	uint32_t swizzle_phy_tbl[32];
	uint32_t mc_odt_pins_tbl[4];
	uint32_t mc_mr1_tbl[2];
	uint32_t mc_mr2_tbl[2];
	uint32_t mc_mr5_tbl[2];
	uint32_t mc_mr6_tbl[2];
	uint32_t mc_init_tbl[1014];
	uint32_t mc_phy_settings_tbl[8];
};

struct spi_config_t {
	uint32_t spi_base;
	uint32_t spi_type;

    uint32_t phycnt;
    uint32_t phyoffset1;
    uint32_t phyoffset2;
    uint32_t cmncr;
    uint32_t ssldr;
    uint32_t drcr;
    uint32_t drcmr;
    uint32_t drear;
    uint32_t drenr;
    uint32_t drdmcr;
    uint32_t drdrenr;
};

extern struct common_config_t common_config;
extern struct cpg_config_t cpg_config;
extern struct syc_config_t syc_config;
extern struct sysc_config_t sysc_config;
extern struct pfc_config_t pfc_config;
extern struct scif_config_t scif_config;
extern struct ddr_config_t ddr_config;
extern struct spi_config_t spi_config;

const struct common_config_t *common_config_getter(void);
const struct cpg_config_t *cpg_config_getter(void);
const struct pfc_config_t *pfc_config_getter(void);
const struct syc_config_t *syc_config_getter(void);
const struct sysc_config_t *sysc_config_getter(void);
const struct scif_config_t *scif_config_getter(void);
const struct ddr_config_t *ddr_config_getter(void);
const struct spi_config_t *spi_config_getter(void);

#endif	/* __RZ_CONFIG_H__ */
