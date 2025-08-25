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
#define hw_config__pfc_config_getter(prop) pfc_config.prop
#define hw_config__scif_config_getter(prop) scif_config.prop
#define hw_config__ddr_config_getter(prop) ddr_config.prop
#define hw_config__spi_config_getter(prop) spi_config.prop

struct common_config_t {
	uint32_t board_id;
	uint32_t sysc_base;
};

struct cpg_config_t {
	uint32_t cpg_base;
	uint32_t cpg_early_setup;
	uint32_t cpg_mstop_setup;
};

struct syc_config_t {
	uint32_t syc_base;
	uint32_t syc_inck_hz;
};

struct pfc_config_t {
	uint32_t pfc_base;
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
	uint32_t ddrmc[9];
	uint32_t ddrphy[16];

	uint32_t ddrdenali_30;
	uint32_t ddrdenali_34;
	uint32_t ddrdenali_35;
	uint32_t ddrdenali_122;
	uint32_t ddrdenali_123;
	uint32_t ddrdenali_124;
	uint32_t ddrdenali_125;
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
extern struct pfc_config_t pfc_config;
extern struct scif_config_t scif_config;
extern struct ddr_config_t ddr_config;
extern struct spi_config_t spi_config;

const struct common_config_t *common_config_getter(void);
const struct cpg_config_t *cpg_config_getter(void);
const struct pfc_config_t *pfc_config_getter(void);
const struct syc_config_t *syc_config_getter(void);
const struct scif_config_t *scif_config_getter(void);
const struct ddr_config_t *ddr_config_getter(void);
const struct spi_config_t *spi_config_getter(void);

#endif	/* __RZ_CONFIG_H__ */
