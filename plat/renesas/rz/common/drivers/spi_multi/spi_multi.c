/*
 * Copyright (c) 2020, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <lib/utils_def.h>
#include <lib/mmio.h>
#include <arch_helpers.h>
#include <spi_multi_regs_offset.h>
#include <spi_multi.h>
#include <spi_multi_reg_values.h>
#include <common/debug.h>
#include <rz_fconf.h>

const struct spi_config_t *g_spi_fconf_cfg;

#define SPI_REG_ADDR(offset)  		((uintptr_t)(g_spi_fconf_cfg->spi_base + (offset)))
#define SPI_REG_WRITE(reg, value)	mmio_write_32(SPI_REG_ADDR(reg), value)
#define SPI_REG_READ(reg)			mmio_read_32(SPI_REG_ADDR(reg))

void spi_multi_timing_set(void)
{

	/* Timing adjustment register setting */
	SPI_REG_WRITE(SPIM_PHYADJ2, 0xA5390000);
	SPI_REG_WRITE(SPIM_PHYADJ1, 0x80000000);
	SPI_REG_WRITE(SPIM_PHYADJ2, 0x00008080);
	SPI_REG_WRITE(SPIM_PHYADJ1, 0x80000022);
	SPI_REG_WRITE(SPIM_PHYADJ2, 0x00008080);
	SPI_REG_WRITE(SPIM_PHYADJ1, 0x80000024);

	/* SDR mode serial flash settings */
	SPI_REG_WRITE(SPIM_PHYCNT, SPIM_PHYCNT_SDR_TIM_ADJ_SET_VALUE);

	/* Timing adjustment register setting */
	SPI_REG_WRITE(SPIM_PHYADJ2, 0x00000030);
	SPI_REG_WRITE(SPIM_PHYADJ1, 0x80000032);

	dmbsy();
}

uint8_t spi_multi_cmd_read(uint8_t command)
{
	uint32_t val;
	uint8_t r_status;

	/* SDR mode serial flash settings */
	SPI_REG_WRITE(SPIM_PHYCNT, SPIM_PHYCNT_SET_VALUE);
	SPI_REG_WRITE(SPIM_PHYCNT, SPIM_PHYCNT_SDR_TIM_ADJ_SET_VALUE);

	/* Set the QSPIn_SSL setting value & Manual Mode */
	SPI_REG_WRITE(SPIM_CMNCR, SPIM_CMNCR_MANUAL_SET_VALUE);

	val = command << SMCMR_CMD_BIT_SHIFT;
	SPI_REG_WRITE(SPIM_SMCMR, val);

	/* Set the Transfer Data size setting value &  command output enable */
	val = SMENR_CDE | SPI_MANUAL_COMMAND_SIZE_16_BIT;
	SPI_REG_WRITE(SPIM_SMENR, val);

	/* Set the SDR transfer & SPI flash mode setting value */
	SPI_REG_WRITE(SPIM_SMDRENR, SPIM_SMDRENR_SET_VALUE);

	val = SMCR_SPIE | SMCR_SPIRE;
	SPI_REG_WRITE(SPIM_SMCR, val);

	/* Wait until the transfer is complete */
	do {
		val = SPI_REG_READ(SPIM_CMNSR);
	} while ((val & CMNSR_TEND) == 0);

	val=SPI_REG_READ(SPIM_SMRDR0);

	r_status = (uint8_t)val;
	return(r_status);
}

void spi_multi_cmd_write(uint8_t command,uint8_t size,uint32_t data)
{
	uint32_t val;

	/* SDR mode serial flash settings */
	SPI_REG_WRITE(SPIM_PHYCNT, SPIM_PHYCNT_SET_VALUE);
	SPI_REG_WRITE(SPIM_PHYCNT, SPIM_PHYCNT_SDR_TIM_ADJ_SET_VALUE);

	/* Set the QSPIn_SSL setting value & Manual Mode */
	SPI_REG_WRITE(SPIM_CMNCR, SPIM_CMNCR_MANUAL_SET_VALUE);

	/* Set the Manual Mode Command */
	val = command << SMCMR_CMD_BIT_SHIFT;
	SPI_REG_WRITE(SPIM_SMCMR, val);

	/* Set the Transfer Data size setting value &  command output enable */
	val = SMENR_CDE | size;
	SPI_REG_WRITE(SPIM_SMENR, val);

	/* Set the write data in Manual mode */
	SPI_REG_WRITE(SPIM_SMWDR0, data);

	/* Set the SDR transfer & SPI flash mode setting value */
	SPI_REG_WRITE(SPIM_SMDRENR, SPIM_SMDRENR_SET_VALUE);

	/* Set the data transfer enable & data write enable  */
	if (size == SPI_MANUAL_COMMAND_SIZE_0)
	{
		val = SMCR_SPIE;
	} else {
		val = SMCR_SPIE | SMCR_SPIWE;
	}
	SPI_REG_WRITE(SPIM_SMCR, val);

	/* Wait until the transfer is complete */
	do {
		val = SPI_REG_READ(SPIM_CMNSR);
	} while ((val & CMNSR_TEND) == 0);
}

int spi_multi_setup( void )
{
	uint32_t val;

	/* Initialize global SPI config from DTB.  */
	g_spi_fconf_cfg = spi_config_getter();

	/* Wait until the transfer is complete */
	do {
		val = SPI_REG_READ(SPIM_CMNSR);
	} while ((val & CMNSR_TEND) == 0);

	/* Device-specific settings */
	spi_multi_setup_device();
	/* SDR mode serial flash settings */
	INFO("g_spi_fconf_cfg->phycnt: 0x%x\n", g_spi_fconf_cfg->phycnt);
	SPI_REG_WRITE(SPIM_PHYCNT, g_spi_fconf_cfg->phycnt);

	/* Read timing setting */
	INFO("g_spi_fconf_cfg->phyoffset1: 0x%x\n", g_spi_fconf_cfg->phyoffset1);
	SPI_REG_WRITE(SPIM_PHYOFFSET1, g_spi_fconf_cfg->phyoffset1);
	INFO("g_spi_fconf_cfg->phyoffset2: 0x%x\n", g_spi_fconf_cfg->phyoffset2);
	SPI_REG_WRITE(SPIM_PHYOFFSET2, g_spi_fconf_cfg->phyoffset2);

	/* Set the QSPIn_SSL setting value */
	INFO("g_spi_fconf_cfg->cmncr: 0x%x\n", g_spi_fconf_cfg->cmncr);
	SPI_REG_WRITE(SPIM_CMNCR, g_spi_fconf_cfg->cmncr);
	/* Set SSL delay setting value */
	INFO("g_spi_fconf_cfg->ssldr: 0x%x\n", g_spi_fconf_cfg->ssldr);
	SPI_REG_WRITE(SPIM_SSLDR, g_spi_fconf_cfg->ssldr);

	/* Clear the RBE bit */
	INFO("g_spi_fconf_cfg->drcr: 0x%x\n", g_spi_fconf_cfg->drcr);
	SPI_REG_WRITE(SPIM_DRCR, g_spi_fconf_cfg->drcr);
	SPI_REG_READ(SPIM_DRCR);

	/* Set the data read command */
	INFO("g_spi_fconf_cfg->drcmr: 0x%x\n", g_spi_fconf_cfg->drcmr);
	SPI_REG_WRITE(SPIM_DRCMR, g_spi_fconf_cfg->drcmr);

	/* Extended external address setting */
	INFO("g_spi_fconf_cfg->drear: 0x%x\n", g_spi_fconf_cfg->drear);
	SPI_REG_WRITE(SPIM_DREAR, g_spi_fconf_cfg->drear);

	/* Set the bit width of command and address output to 1 bit and	*/
	/* the address size to 4 byte									*/
	INFO("g_spi_fconf_cfg->drenr: 0x%x\n", g_spi_fconf_cfg->drenr);
	SPI_REG_WRITE(SPIM_DRENR, g_spi_fconf_cfg->drenr);

	/* Dummy cycle setting */
	INFO("g_spi_fconf_cfg->drdmcr: 0x%x\n", g_spi_fconf_cfg->drdmcr);
	SPI_REG_WRITE(SPIM_DRDMCR, g_spi_fconf_cfg->drdmcr);

	/* Change to SPI flash mode */
	INFO("g_spi_fconf_cfg->drdrenr: 0x%x\n", g_spi_fconf_cfg->drdrenr);
	SPI_REG_WRITE(SPIM_DRDRENR, g_spi_fconf_cfg->drdrenr);

	/* Timing adjustment register setting */
	spi_multi_timing_set();

	return SPI_MULTI_SUCCESS;
}
