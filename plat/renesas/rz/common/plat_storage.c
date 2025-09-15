/*
 * Copyright (c) 2020-2022, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <common/tbbr/tbbr_img_def.h>
#include <drivers/io/io_driver.h>
#include <drivers/io/io_storage.h>
#include <drivers/io/io_fip.h>
#include <drivers/io/io_memmap.h>
#include <io_common.h>
#include <io_emmcdrv.h>
#include <io_sddrv.h>
#include <lib/mmio.h>
#include <tools_share/firmware_image_package.h>
#include <spi_multi.h>
#include <xspi.h>
#include <rzg2l_def.h>
#include <esdif.h>
#include <io_sddrv.h>
#include <platform_def.h>
#include <sys.h>
#include <emmc_def.h>
#include <sys_regs_offset.h>
#include <rz_fconf.h>
#include <lib/fconf/fconf.h>
#include <board_info.h>

static uintptr_t memdrv_dev_handle;
static uintptr_t fip_dev_handle;
static uintptr_t emmcdrv_dev_handle;
static uintptr_t sddrv_dev_handle;


static uintptr_t boot_io_drv_id;

#define SPI_MULTI_IF	(0)
#define XSPI_IF			(1)

static io_block_spec_t spirom_block_spec = {
	.offset = RZG2L_SPIROM_FIP_BASE,
	.length = RZG2L_SPIROM_FIP_SIZE,
};

static io_drv_spec_t emmc_block_spec = {
	.offset = RZG2L_EMMC_FIP_BASE,
	.length = RZG2L_EMMC_FIP_SIZE,
};

static io_drv_spec_t sd_block_spec = {
	.offset = RZG2L_SD_FIP_BASE,
	.length = RZG2L_SD_FIP_SIZE,
};

static const io_uuid_spec_t bl31_file_spec = {
	.uuid = UUID_EL3_RUNTIME_FIRMWARE_BL31,
};

static const io_uuid_spec_t bl32_file_spec = {
	.uuid = UUID_SECURE_PAYLOAD_BL32,
};

#ifdef REMOVE_UBOOT
static const io_uuid_spec_t fw_config_file_spec = {
	.uuid = UUID_FW_CONFIG,
};

static const io_uuid_spec_t hw_config_file_spec = {
	.uuid = UUID_HW_CONFIG,
};

static const io_uuid_spec_t soc_fw_config_file_spec = {
	.uuid = UUID_SOC_FW_CONFIG,
};

static const io_uuid_spec_t rmm_fw_file_spec = {
	.uuid = UUID_REALM_MONITOR_MGMT_FIRMWARE,
};

static const io_uuid_spec_t bl331_file_spec = {
	.uuid = UUID_NT_FW_CONFIG,
};

static const io_uuid_spec_t bl332_file_spec = {
#else
static const io_uuid_spec_t bl33_file_spec = {
#endif /* REMOVE_UBOOT */
	.uuid = UUID_NON_TRUSTED_FIRMWARE_BL33,
};

#if TRUSTED_BOARD_BOOT
static const io_uuid_spec_t soc_fw_key_cert_file_spec = {
	.uuid = UUID_SOC_FW_KEY_CERT,
};

static const io_uuid_spec_t soc_fw_content_cert_file_spec = {
	.uuid = UUID_SOC_FW_CONTENT_CERT,
};

static const io_uuid_spec_t tos_fw_key_cert_file_spec = {
	.uuid = UUID_TRUSTED_OS_FW_KEY_CERT,
};

static const io_uuid_spec_t tos_fw_content_cert_file_spec = {
	.uuid = UUID_TRUSTED_OS_FW_CONTENT_CERT,
};

static const io_uuid_spec_t nt_fw_key_cert_file_spec = {
	.uuid = UUID_NON_TRUSTED_FW_KEY_CERT,
};

static const io_uuid_spec_t nt_fw_content_cert_file_spec = {
	.uuid = UUID_NON_TRUSTED_FW_CONTENT_CERT,
};
#endif

static int32_t open_emmcdrv(const uintptr_t spec);
static int32_t open_memmap(const uintptr_t spec);
static int32_t open_fipdrv(const uintptr_t spec);
static int32_t open_sddrv(const uintptr_t spec);


struct plat_io_policy {
	uintptr_t *dev_handle;
	uintptr_t image_spec;
	int32_t (*check)(const uintptr_t spec);
};

#if PLAT_SOC_RZV2H
static const struct plat_io_policy sd_fip_policy = {
	&sddrv_dev_handle,
	(uintptr_t) &sd_block_spec,
	&open_sddrv
};

static const struct plat_io_policy emmc_fip_policy = {
	&emmcdrv_dev_handle,
	(uintptr_t) &emmc_block_spec,
	&open_emmcdrv
};

static const struct plat_io_policy spirom_fip_policy = {
	&memdrv_dev_handle,
	(uintptr_t) &spirom_block_spec,
	&open_memmap
};
#endif


static struct plat_io_policy policies[] = {
	[BL31_IMAGE_ID] = {
				&fip_dev_handle,
				(uintptr_t) &bl31_file_spec,
				&open_fipdrv},
	[BL32_IMAGE_ID] = {
				&fip_dev_handle,
				(uintptr_t) &bl32_file_spec,
				&open_fipdrv},
#ifdef REMOVE_UBOOT
	[FW_CONFIG_ID] = {
				&fip_dev_handle,
				(uintptr_t) &fw_config_file_spec,
				&open_fipdrv},
	[HW_CONFIG_ID] = {
				&fip_dev_handle,
				(uintptr_t) &hw_config_file_spec,
				&open_fipdrv},
	[SOC_FW_CONFIG_ID] = {
				&fip_dev_handle,
				(uintptr_t) &soc_fw_config_file_spec,
				&open_fipdrv},
	[RMM_IMAGE_ID] = {
				&fip_dev_handle,
				(uintptr_t) &rmm_fw_file_spec,
				&open_fipdrv},
	[BL331_IMAGE_ID] = {
#else
	[BL33_IMAGE_ID] = {
#endif /* REMOVE_UBOOT */
				&fip_dev_handle,
#ifdef REMOVE_UBOOT
				(uintptr_t) &bl331_file_spec,
				&open_fipdrv},
	[BL332_IMAGE_ID] = {
				&fip_dev_handle,
				(uintptr_t) &bl332_file_spec,
#else
				(uintptr_t) &bl33_file_spec,
#endif /* REMOVE_UBOOT */
				&open_fipdrv},
#if TRUSTED_BOARD_BOOT
	[SOC_FW_KEY_CERT_ID] = {
				&fip_dev_handle,
				(uintptr_t) &soc_fw_key_cert_file_spec,
				&open_fipdrv},
	[SOC_FW_CONTENT_CERT_ID] = {
				&fip_dev_handle,
				(uintptr_t) &soc_fw_content_cert_file_spec,
				&open_fipdrv},
	[TRUSTED_OS_FW_KEY_CERT_ID] = {
				&fip_dev_handle,
				(uintptr_t) &tos_fw_key_cert_file_spec,
				&open_fipdrv},
	[TRUSTED_OS_FW_CONTENT_CERT_ID] = {
				&fip_dev_handle,
				(uintptr_t) &tos_fw_content_cert_file_spec,
				&open_fipdrv},
	[NON_TRUSTED_FW_KEY_CERT_ID] = {
				&fip_dev_handle,
				(uintptr_t) &nt_fw_key_cert_file_spec,
				&open_fipdrv},
	[NON_TRUSTED_FW_CONTENT_CERT_ID] = {
				&fip_dev_handle,
				(uintptr_t) &nt_fw_content_cert_file_spec,
				&open_fipdrv},
#endif
	{ 0, 0, 0}
};

static int32_t open_fipdrv(const uintptr_t spec)
{
	int32_t result;

	result = io_dev_init(fip_dev_handle, boot_io_drv_id);
	if (result != 0)
		return result;

	return result;
}

static int32_t open_memmap(const uintptr_t spec)
{
	uintptr_t handle;
	int32_t result;

	result = io_dev_init(memdrv_dev_handle, 0);
	if (result != 0)
		return result;

	result = io_open(memdrv_dev_handle, spec, &handle);
	if (result == 0)
		io_close(handle);

	return result;
}

static int32_t open_emmcdrv(const uintptr_t spec)
{
	return io_dev_init(emmcdrv_dev_handle, 0);
}

static int32_t open_sddrv(const uintptr_t spec)
{
	return io_dev_init(sddrv_dev_handle, 0);
}

void rz_io_setup(void)
{
	const io_dev_connector_t *memmap;
	const io_dev_connector_t *emmc;
	const io_dev_connector_t *rzcmn;
	const io_dev_connector_t *sd;
	
	uint32_t stat_md_boot;
	const struct common_config_t * common_fconf_cfg = common_config_getter();

	boot_io_drv_id = FIP_IMAGE_ID;

	register_io_dev_fip(&rzcmn);

	io_dev_open(rzcmn, 0, &fip_dev_handle);

	/* Boot Mode eSD */
	stat_md_boot = sys_get_boot_mode();
	if (stat_md_boot == BOOT_MODE_ESD){
		panic();
		if (esd_main() != SD_OK) {
			NOTICE("BL2: Failed to eSD driver initialize.\n");
			panic();
		}
		register_io_dev_sddrv(&sd);
		io_dev_open(sd, 0, &sddrv_dev_handle);

		sd_block_spec.offset = common_fconf_cfg->sd_fip_base;
		sd_block_spec.length = common_fconf_cfg->sd_fip_size;

		struct plat_io_policy sd_fip_policy = {
				&sddrv_dev_handle,
				(uintptr_t) &sd_block_spec,
				&open_sddrv};
		policies[FIP_IMAGE_ID] =  sd_fip_policy;
	}
	else if (stat_md_boot == BOOT_MODE_SPI_1_8 ||
		stat_md_boot == BOOT_MODE_SPI_3_3) {
		uint32_t spi_type = FCONF_GET_PROPERTY(hw_config, spi_config, spi_type);

		if (spi_type == SPI_MULTI_IF) {
			spi_multi_setup();
		} else if (spi_type == XSPI_IF) {
			xspi_setup();
		}
		register_io_dev_memmap(&memmap);
		io_dev_open(memmap, 0, &memdrv_dev_handle);

		spirom_block_spec.offset = common_fconf_cfg->spirom_fip_base;
		spirom_block_spec.length = common_fconf_cfg->spirom_fip_size;

		struct plat_io_policy spirom_fip_policy = {
				&memdrv_dev_handle,
				(uintptr_t) &spirom_block_spec,
				&open_memmap};
		policies[FIP_IMAGE_ID] = spirom_fip_policy;
	}
	else if (stat_md_boot == BOOT_MODE_EMMC_1_8 ||
	stat_md_boot == BOOT_MODE_EMMC_3_3) {
		if (emmc_init() != EMMC_SUCCESS) {
			NOTICE("BL2: Failed to eMMC driver initialize.\n");
			panic();
		}
		emmc_memcard_power(EMMC_POWER_ON);
		if (emmc_mount() != EMMC_SUCCESS) {
			NOTICE("BL2: Failed to eMMC mount operation.\n");
			panic();
		}

		register_io_dev_emmcdrv(&emmc);
		io_dev_open(emmc, 0, &emmcdrv_dev_handle);

		emmc_block_spec.offset = common_fconf_cfg->emmc_fip_base;
		emmc_block_spec.length = common_fconf_cfg->emmc_fip_size;

		struct plat_io_policy emmc_fip_policy = {
				&emmcdrv_dev_handle,
				(uintptr_t) &emmc_block_spec,
				&open_emmcdrv};
		policies[FIP_IMAGE_ID] = emmc_fip_policy;
	} else {
		panic();
	}
}

int plat_get_image_source(unsigned int image_id, uintptr_t *dev_handle,
				uintptr_t *image_spec)
{
	const struct plat_io_policy *policy;
	int result;

	policy = &policies[image_id];

	result = policy->check(policy->image_spec);
	if (result != 0)
		return result;

	*image_spec = policy->image_spec;
	*dev_handle = *(policy->dev_handle);

	return 0;
}
