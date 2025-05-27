#ifndef RZ_CONFIG_H
#define RZ_CONFIG_H

#define hw_config__cpg_config_getter(prop) cpg_config.prop
#define hw_config__sysc_config_getter(prop) sysc_config.prop
#define hw_config__pfc_config_getter(prop) pfc_config.prop
#define hw_config__ddr_config_getter(prop) ddr_config.prop
#define hw_config__spi_config_getter(prop) spi_config.prop

struct cpg_config_t {
    uint32_t divpl1_set;
    uint32_t divpl1_set_wen;
    uint32_t pll4_clk1;
    uint32_t pll4_clk2;
    uint32_t pll4_stby;
    uint32_t pll6_clk1;
    uint32_t pll6_clk2;
    uint32_t pll6_stby;
};

struct sysc_config_t {
    uint32_t syc_inck_hz;
};

struct pfc_config_t {
    uint32_t qspi0_iolh0a;
    uint32_t qspi0_pupd0a;
    uint32_t qspi0_sr0a;
    uint32_t qspi1_iolh0b;
    uint32_t qspi1_pupd0b;
    uint32_t qspi1_sr0b;
    uint32_t qspin_iolh0c;
    uint32_t qspin_pupd0c;
    uint32_t qspin_sr0c;
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

extern struct cpg_config_t cpg_config;
extern struct sysc_config_t sysc_config;
extern struct pfc_config_t pfc_config;
extern struct ddr_config_t ddr_config;
extern struct spi_config_t spi_config;

const struct cpg_config_t *cpg_config_getter(void);
const struct pfc_config_t *pfc_config_getter(void);
const struct sysc_config_t *sysc_config_getter(void);
const struct ddr_config_t *ddr_config_getter(void);
const struct spi_config_t *spi_config_getter(void);

#endif
