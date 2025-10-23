#
# Copyright (c) 2021-2023, Renesas Electronics Corporation. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

PLAT_SOC_CMN					:= 1
BL2_AT_EL3						:= 1
RESET_TO_BL2					:= 1
RESET_TO_BL31					:= 1
COLD_BOOT_SINGLE_CPU			:= 1
PROGRAMMABLE_RESET_ADDRESS		:= 0
WARMBOOT_ENABLE_DCACHE_EARLY	:= 1
GICV3_SUPPORT_GIC600			:= 1
GICV3_OVERRIDE_DISTIF_PWR_OPS	:= 1
HW_ASSISTED_COHERENCY			:= 1
USE_COHERENT_MEM				:= 0
TRUSTED_BOARD_BOOT				:= 0
PROTECTED_CHIPID				:= 1
DEBUG_FPGA						:= 0
PLAT_DDR_ECC					:= 0
PLAT_EMMC_WRITE_ENABLE			:= 0
PLAT_SYSTEM_SUSPEND				:= 0
ENABLE_PIE						:= 1
INIT_UNUSED_NS_EL2				:= 1

$(eval $(call add_define,PLAT_SOC_CMN))
$(eval $(call add_define,PROTECTED_CHIPID))
$(eval $(call add_define,DEBUG_FPGA))
$(eval $(call add_define,ENABLE_PIE))
$(eval $(call add_define,PLAT_EXTRA_LD_SCRIPT))

WA_RZG2L_GIC64BIT				:= 1
$(eval $(call add_define,WA_RZG2L_GIC64BIT))

# This option gets enabled automatically if the TRUSTED_BOARD_BOOT
# is set via root Makefile, but Renesas support Trusted-Boot without
# Crypto module.
override CRYPTO_SUPPORT			:= 0

ifneq (${PLAT_SYSTEM_SUSPEND},0)
override PLAT_SYSTEM_SUSPEND	:= 1
endif

$(eval $(call add_define,PLAT_SYSTEM_SUSPEND))

# DEBUG helper flag to generate obj file
ifneq (${DEBUG},0)
CFLAGS += -save-temps=obj \

include lib/libc/libc.mk
$(BUILD_PLAT)/lib/libc/snprintf.o:
    CFLAGS += -Wno-error=implicit-fallthrough
endif

# Enable workarounds for selected Cortex-A55 erratas.
ERRATA_A55_1530923				:= 1
ERRATA_A55_768277				:= 1
ERRATA_A55_778703 				:= 1
ERRATA_A55_798797 				:= 1
ERRATA_A55_846532 				:= 1
ERRATA_A55_903758 				:= 1
ERRATA_A55_1221012				:= 1

# Support QSPI Flash
ifndef SPI_FLASH
	ifeq (${BOARD}, sbc_1)
		SPI_FLASH := IS25WP256
	else
		SPI_FLASH = MT25QU512ABB
	endif
endif

ifeq (${ENABLE_PIE},1)
	include lib/cpus/cpu-ops.mk

	BL2_CPPFLAGS += -DENABLE_PIE=1
	BL2_CFLAGS   += -fpie -fno-plt
	BL2_LDFLAGS  += -pie --no-dynamic-linker --emit-relocs

	PLAT_BL_COMMON_SOURCES += lib/cpus/aarch64/cpu_helpers.S \
							lib/cpus/errata_report.c
endif

PLAT_INCLUDES			:=	-Iplat/renesas/rz/common/include						\
							-Iplat/renesas/rz/common/drivers/emmc					\
							-Iplat/renesas/rz/common/drivers/io						\
							-Idrivers/renesas/common/io								\
							-Iplat/renesas/rz/common/include/drivers/spi_multi		\
							-Iplat/renesas/rz/common/include/drivers/spi_multi/${SPI_FLASH} \
							-Iplat/renesas/rz/common/drivers/sd

RZ_TIMER_SOURCES		:=	drivers/delay_timer/generic_delay_timer.c				\
							drivers/delay_timer/delay_timer.c

DDR_SOURCES				:=	plat/renesas/rz/common/drivers/ddr/ddr.c

EMMC_SOURCES			:=	plat/renesas/rz/common/drivers/emmc/rz_emmc.c

SPI_MULTI_SOURCE 		:=	plat/renesas/rz/common/drivers/spi_multi/spi_multi.c	\
							plat/renesas/rz/common/drivers/spi_multi/${SPI_FLASH}/spi_multi_device.c

XSPI_SOURCES			:=	plat/renesas/rz/common/drivers/xspi.c	\
							plat/renesas/rz/common/drivers/io/io_xspidrv.c

SD_SOURCES				:=	plat/renesas/rz/common/drivers/sd/esd_main.c

ifdef PLAT_BL2_STORAGE
ifeq (${PLAT_BL2_STORAGE},xspi)
BL2_SOURCES			+=	${SPI_MULTI_SOURCE}		\
						${XSPI_SOURCES}
BL2_CPPFLAGS			+=	-DPLAT_BOOT_DEVICE_XSPI
else ifeq (${PLAT_BL2_STORAGE},emmc)
BL2_SOURCES			+=	${EMMC_SOURCES}
BL2_CPPFLAGS			+=	-DPLAT_BOOT_DEVICE_EMMC
else ifeq (${PLAT_BL2_STORAGE},esd)
BL2_SOURCES			+=	${SD_SOURCES}
BL2_CPPFLAGS			+=	-DPLAT_BOOT_DEVICE_ESD
else
$(error Unsupported PLAT_BL2_STORAGE value: ${PLAT_BL2_STORAGE})
endif
else
BL2_SOURCES			+=	${EMMC_SOURCES}									\
						${SPI_MULTI_SOURCE}								\
						${XSPI_SOURCES}									\
						${SD_SOURCES}
endif

BL_COMMON_SOURCES		+=	lib/cpus/aarch64/cortex_a55.S							\
							drivers/arm/tzc/tzc400.c

include lib/xlat_tables_v2/xlat_tables.mk
PLAT_BL_COMMON_SOURCES	:=	${XLAT_TABLES_LIB_SRCS}									\
							plat/renesas/rz/common/aarch64/plat_helpers_system_suspend.S		\
							plat/renesas/rz/common/drivers/scifa.S					\
							plat/renesas/rz/common/drivers/syc.c					\
							plat/renesas/rz/common/drivers/sys.c					\
							plat/renesas/rz/common/drivers/cpg.c					\
							plat/renesas/rz/common/plat_rz_common.c					\
							plat/renesas/rz/common/plat_security.c

ifneq (${ENABLE_STACK_PROTECTOR},0)
PLAT_BL_COMMON_SOURCES	+=	plat/renesas/rz/common/rz_stack_protector.c
endif

BL2_SOURCES				+=	common/desc_image_load.c								\
							drivers/io/io_storage.c									\
							drivers/io/io_memmap.c									\
							drivers/io/io_fip.c										\
							plat/renesas/rz/common/drivers/io/io_emmcdrv.c			\
							plat/renesas/rz/common/drivers/io/io_sddrv.c			\
							plat/renesas/rz/common/bl2_plat_setup.c					\
							plat/renesas/rz/common/bl2_plat_mem_params_desc.c		\
							plat/renesas/rz/common/plat_image_load.c				\
							plat/renesas/rz/common/plat_storage.c					\
							plat/renesas/rz/common/drivers/pfc.c					\
							plat/renesas/rz/common/board_info.c						\
							${RZ_TIMER_SOURCES}										\
							${DDR_SOURCES}											\
							${FDT_WRAPPERS_SOURCES}									\
							${FCONF_SOURCES}

# Include GICv3 driver files
include drivers/arm/gic/v3/gicv3.mk

BL31_SOURCES			+=	plat/common/plat_gicv3.c								\
							plat/common/plat_psci_common.c							\
							plat/renesas/rz/common/bl31_plat_setup.c				\
							plat/renesas/rz/common/plat_pm.c						\
							plat/renesas/rz/common/plat_topology.c					\
							plat/renesas/rz/common/plat_gic.c						\
							plat/renesas/rz/common/rz_plat_sip_handler.c			\
							plat/renesas/rz/common/rz_sip_svc.c						\
							plat/renesas/rz/common/board_info.c						\
							${GICV3_SOURCES}

ifneq (${TRUSTED_BOARD_BOOT},0)

	# Include common TBB sources
	AUTH_SOURCES		:=	drivers/auth/img_parser_mod.c

	# Include the selected chain of trust sources.
	ifeq (${COT},tbbr)
		AUTH_SOURCES	+=	plat/renesas/rz/common/drivers/auth/tbbr/tbbr_cot.c
	else
		$(error Unknown chain of trust ${COT})
	endif

	# Include RZ TBB sources
	AUTH_SOURCES		+=	plat/renesas/rz/common/drivers/auth/auth_mod.c				\
							plat/renesas/rz/common/drivers/auth/sblib/crypto_sblib.c	\
							plat/renesas/rz/common/drivers/auth/sblib/sblib_parser.c


	BL2_SOURCES			+=	${AUTH_SOURCES}

endif

.PHONY: bl2-xspi bl2-emmc bl2-esd bl2-all

define PLAT_BL2_VARIANT_RULE
bl2-$(1):
	@echo "======================================="
	@echo " Building BL2 for $(1)"
	@echo "======================================="
	rm -rf ${BUILD_PLAT}/bl2
	mkdir -p ${BUILD_PLAT}/bl2
	$(MAKE) PLAT=${PLAT} BOARD=${BOARD} PLAT_BL2_STORAGE=$(1) DEBUG=${DEBUG} bl2
ifeq ($(1),esd)
	$(MAKE) PLAT=${PLAT} BOARD=${BOARD} PLAT_BL2_STORAGE=$(1) DEBUG=${DEBUG} bl2_with_dtb
endif
	rm -f ${BUILD_PLAT}/bl2-$(1).bin
	cp ${BUILD_PLAT}/bl2.bin ${BUILD_PLAT}/bl2-$(1).bin
endef

$(foreach variant,xspi emmc esd,$(eval $(call PLAT_BL2_VARIANT_RULE,$(variant))))

# Build all storage variants of BL2
bl2-all:
	$(MAKE) PLAT=${PLAT} BOARD=${BOARD} DEBUG=${DEBUG} bl2-xspi
	$(MAKE) PLAT=${PLAT} BOARD=${BOARD} DEBUG=${DEBUG} bl2-emmc
	$(MAKE) PLAT=${PLAT} BOARD=${BOARD} DEBUG=${DEBUG} bl2-esd
	@echo "======================================="
	@echo "All BL2 variants built successfully."
	@echo "======================================="
