/*
 * Copyright (c) 2025, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**********************************************************************************************************************
 * File Name    : esd_main.c
 * Version      : 1.0
 * Description  : SD Driver main.
 *********************************************************************************************************************/
/**********************************************************************************************************************
 * History : DD.MM.YYYY Version  Description
 *         : 07.22.2022 1.00     First Release
 *********************************************************************************************************************/

/**********************************************************************************************************************
 Includes   <System Includes> , "Project Includes"
 *********************************************************************************************************************/
#include <assert.h>
#include <esd.h>
#include <esdif.h>
#include <drivers/delay_timer.h>
#include <rz_fconf.h>

#include "sdmmc_iodefine.h"

/**********************************************************************************************************************
 Macro definitions
 *********************************************************************************************************************/

/**********************************************************************************************************************
 Local Typedef definitions
 *********************************************************************************************************************/

/**********************************************************************************************************************
 Exported global variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 Private (static) variables and functions
 *********************************************************************************************************************/
static uint32_t sd_drv_rw_buffer[SD_SECTOR_SIZE / sizeof(uint32_t)] __aligned(8);
static uint64_t sd_drv_work_area[ESD_SIZE_OF_INIT / sizeof(uint64_t)] __aligned(8);         /* WorkSpace for eSD driver Library */
st_sdhndl_t *gp_sdhandle[NUM_PORT];
static uint16_t s_stat_buff[NUM_PORT][64 / sizeof(uint16_t)];

/* ==== transfer speed table ==== */
static const uint16_t s_tran_speed[8] = {
	1,      // 100kbit/s
	10,     // 1Mbit/s
	100,    // 10Mbit/s
	1000,   // 100Mbit/s
	1000,   // reserved
	1000,   // reserved
	1000,   // reserved
	1000,   // reserved
};

static const uint8_t s_time_value[16] = {
	0, 10, 12, 13, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 70, 80
};

/* for internal error detail    */
static const int32_t s_resp_err_tbl[] = {
	SD_ERR_OUT_OF_RANGE,                /* b31 : OUT_OF_RANGE                   */
	SD_ERR_ADDRESS_ERROR,               /* b30 : ADDRESS_ERROR                  */
	SD_ERR_BLOCK_LEN_ERROR,             /* b29 : BLOCK_LEN_ERROR                */
	SD_ERR_CARD_ERASE,                  /* b28 : ERASE_SEQ_ERROR                */
	SD_ERR_CARD_ERASE,                  /* b27 : ERASE_PARAM                    */
	SD_ERR_WP,                          /* b26 : WP_VIOLATION                   */
	SD_ERR_CARD_LOCK,                   /* b25 : CARD_IS_LOCKED                 */
	SD_ERR_CARD_UNLOCK,                 /* b24 : LOCK_UNLOCK_FAILED             */
	SD_ERR_HOST_CRC,                    /* b23 : COM_CRC_ERROR                  */
	SD_ERR_ILLEGAL_COMMAND,             /* b22 : ILLEGAL_COMMAND                */
	SD_ERR_CARD_ECC,                    /* b21 : CARD_ECC_FAILED                */
	SD_ERR_CARD_CC,                     /* b20 : CC_ERROR                       */
	SD_ERR_CARD_ERROR,                  /* b19 : ERROR                          */
	SD_ERR_RESERVED_ERROR18,            /* b18 : (reserved)                     */
	SD_ERR_RESERVED_ERROR17,            /* b17 : (reserved)                     */
	SD_ERR_OVERWRITE,                   /* b16 : CSD_OVERWRITE                  */
};

/* ==== SD_INFO2 errors table ==== */
static const int32_t s_info2_err_tbl[] = {
	SD_ERR_ILL_ACCESS,                  /* b15 : Illegal Access Error           */
	SD_OK,                              /* b14 :                                */
	SD_OK,                              /* b13 :                                */
	SD_OK,                              /* b12 :                                */
	SD_OK,                              /* b11 :                                */
	SD_OK,                              /* b10 :                                */
	SD_OK,                              /* b9  :                                */
	SD_OK,                              /* b8  :                                */
	SD_OK,                              /* b7  :                                */
	SD_ERR_RES_TOE,                     /* b6  : Response Timeout               */
	SD_ERR_ILL_READ,                    /* b5  : SD_BUF Illegal Read Access     */
	SD_ERR_ILL_WRITE,                   /* b4  : SD_BUF Illegal Write Access    */
	SD_ERR_CARD_TOE,                    /* b3  : Data Timeout                   */
	SD_ERR_END_BIT,                     /* b2  : END Error                      */
	SD_ERR_CRC,                         /* b1  : CRC Error                      */
	SD_ERR_CMD_ERROR,                   /* b0  : CMD Error                      */
};

static int32_t esd_check_int(int32_t sd_port);
static int32_t esddev_init(int32_t sd_port);
static int32_t esddev_int_wait(int32_t sd_port, int32_t time);
static int32_t esddev_loc_cpu(int32_t sd_port);
static int32_t esddev_power_on(int32_t sd_port);
static int32_t esddev_read_data(int32_t sd_port, uint8_t *buff, uint32_t reg_addr, int32_t num);
static int32_t esddev_wp_layout(int32_t sd_port);
static int32_t esddev_write_data(int32_t sd_port, uint8_t *buff, uint32_t reg_addr, int32_t num);
static uint32_t esddev_get_clockdiv(int32_t sd_port, int32_t clock);
static int32_t _esd_card_query_partitions(st_sdhndl_t *p_hndl, uint32_t opcode, uint8_t *p_rw_buff);
static int32_t _esd_card_select_partition(st_sdhndl_t *p_hndl, uint32_t id);
static int32_t _esd_get_partition_id(st_sdhndl_t *p_hndl, int32_t *id);
static int32_t _sd_bit_search(uint16_t data);
static int32_t _sd_calc_erase_sector(st_sdhndl_t *p_hndl);
static int32_t _sd_card_get_scr(st_sdhndl_t *p_hndl);
static int32_t _sd_card_get_status(st_sdhndl_t *p_hndl);
static int32_t _sd_card_init_get_rca(st_sdhndl_t *p_hndl);
static int32_t _sd_card_init(st_sdhndl_t *p_hndl);
static int32_t _sd_card_send_cmd_arg(st_sdhndl_t *p_hndl, uint16_t cmd, int32_t resp, uint16_t h_arg, uint16_t l_arg);
static int32_t _sd_card_send_ocr(st_sdhndl_t *p_hndl, int32_t type);
static int32_t _sd_check_csd(st_sdhndl_t *p_hndl);
static int32_t _sd_check_info2_err(st_sdhndl_t *p_hndl);
static int32_t _sd_clear_info(st_sdhndl_t *p_hndl, uint64_t clear_info1, uint64_t clear_info2);
static int32_t _sd_clear_int_mask(st_sdhndl_t *p_hndl, uint64_t mask1, uint64_t mask2);
static int32_t _sd_get_int(st_sdhndl_t *p_hndl);
static int32_t _sd_get_resp(st_sdhndl_t *p_hndl, int32_t resp);
static int32_t _sd_get_size(st_sdhndl_t *p_hndl, uint32_t area);
static int32_t _sd_init_error(int32_t sd_port, int32_t ret);
static int32_t _sd_init_hndl(st_sdhndl_t *p_hndl, uint32_t mode, uint32_t voltage);
static int32_t _sd_iswp(st_sdhndl_t *p_hndl);
static int32_t _sd_mem_mount_error(st_sdhndl_t *p_hndl);
static int32_t _sd_mem_mount(st_sdhndl_t *p_hndl);
static int32_t _sd_mount_error(st_sdhndl_t *p_hndl);
static int32_t _sd_read_byte_error(st_sdhndl_t *p_hndl);
static int32_t _sd_read_byte(st_sdhndl_t *p_hndl, uint16_t cmd, uint16_t h_arg, uint16_t l_arg, uint8_t *readbuff, uint16_t byte);
static int32_t _sd_read_sect_error(st_sdhndl_t *p_hndl, int32_t mode);
static int32_t _sd_send_acmd(st_sdhndl_t *p_hndl, uint16_t cmd, uint16_t h_arg, uint16_t l_arg);
static int32_t _sd_send_cmd(st_sdhndl_t *p_hndl, uint16_t cmd);
static int32_t _sd_send_mcmd(st_sdhndl_t *p_hndl, uint16_t cmd, uint32_t startaddr);
static int32_t _sd_set_clock(st_sdhndl_t *p_hndl, int32_t clock, int32_t enable);
static int32_t _sd_set_err(st_sdhndl_t *p_hndl, int32_t error);
static int32_t _sd_set_int_mask(st_sdhndl_t *p_hndl, uint64_t mask1, uint64_t mask2);
static int32_t _sd_set_port(st_sdhndl_t *p_hndl, int32_t port);
static int32_t _sd_single_read_error(st_sdhndl_t *p_hndl, int32_t mode);
static int32_t _sd_single_read(st_sdhndl_t *p_hndl, uint8_t *buff, uint32_t psn, int32_t mode);
static int32_t _sd_single_write_error(st_sdhndl_t *p_hndl, int32_t mode);
static int32_t _sd_single_write(st_sdhndl_t *p_hndl, uint8_t *buff, uint32_t psn, int32_t mode);
static int32_t _sd_software_trans(st_sdhndl_t *p_hndl, uint8_t *buff, int32_t cnt, int32_t dir);
static int32_t _sd_write_sect_error(st_sdhndl_t *p_hndl, int32_t mode);
static int32_t _sd_write_sect(st_sdhndl_t *p_hndl, uint8_t *buff, uint32_t psn, int32_t cnt, int32_t writemode);
static void _sd_get_info2(st_sdhndl_t *p_hndl);
static void _sd_set_arg(st_sdhndl_t *p_hndl, uint16_t h_arg, uint16_t l_arg);

/******************************************************************************
 * Function Name: esd_init
 * Description  : initialize SD Driver (more than 2ports).
 *              : initialize SD Driver work memory started from SDHI register
 *              : base
 *              : address specified by argument (base)
 *              : initialize port specified by argument (cd_port)
 *              : work memory is allocated octlet boundary
 * Arguments    : int32_t sd_port : channel no (0 or 1)
 *              : uint32_t base   : SDHI register base address
 *              : void *workarea  : SD Driver work memory
 *              : int32_t cd_port : card detect port
 *              :   SD_CD_SOCKET  : card detect by CD pin
 *              :   SD_CD_DAT3    : card detect by DAT3 pin
 * Return Value : SD_OK : end of succeed
 *              : SD_ERR: end of error
 *              : SD_ERR_CPU_IF : CPU-IF function error
 *****************************************************************************/
int32_t esd_init(int32_t sd_port, uint32_t base, void *workarea, int32_t cd_port)
{
	int32_t     i;
	uint64_t    info1;
	uint8_t     *p_ptr;
	st_sdhndl_t *p_hndl;
	int32_t     ret;

	if ((0 != sd_port) && (1 != sd_port)) {
		return SD_ERR;
	}

	/* ==== initialize work memory  ==== */
	if ((uintptr_t)workarea == 0) {
		ret = SD_ERR;
		return _sd_init_error(sd_port, ret);
	}

	/* ==== work memory boundary check (octlet unit) ==== */
	if ((uintptr_t)workarea & 0x7u) {
		ret = SD_ERR;
		return _sd_init_error(sd_port, ret);
	}

	/* ==== check card detect port ==== */
	if ((SD_CD_SOCKET != cd_port) && (SD_CD_DAT3 != cd_port)) {
		ret = SD_ERR;
		return _sd_init_error(sd_port, ret);
	}

	/* card detect port is fixed at CD pin */
	cd_port = SD_CD_SOCKET;

	/* ==== initialize peripheral module ==== */
	if (esddev_init(sd_port) != SD_OK) {
		ret = SD_ERR_CPU_IF;
		return _sd_init_error(sd_port, ret);
	}

	/* disable all interrupts */
	esddev_loc_cpu(sd_port);

	/* Cast to an appropriate type */
	p_hndl = (st_sdhndl_t *)workarea;

	gp_sdhandle[sd_port] = p_hndl;

	/* ---- clear work memory zero value --- */
	p_ptr = (uint8_t *)p_hndl;
	for (i = sizeof(st_sdhndl_t); i > 0 ; i--) {
		*p_ptr++ = 0;
	}

	/* ---- set SDHI register address ---- */
	p_hndl->reg_base = base;

	/* Cast to an appropriate type */
	p_hndl->cd_port = (uint8_t)cd_port;

	/* ---- initialize maximum block count ---- */
	p_hndl->trans_sectors = 256;
	p_hndl->trans_blocks  = 32;

	p_hndl->sd_port = sd_port;

	/* return to select port0 */
	p_hndl = SD_GET_HNDLS(sd_port);
	if (0 == p_hndl) {
		return SD_ERR;  /* not initilized */
	}

	/* ==== initialize SDHI ==== */
	SDMMC.SD_INFO1_MASK.LONGLONG = SD_INFO1_MASK_ALL;

	/* Cast to an appropriate type */
	SDMMC.SD_INFO2_MASK.LONGLONG = SD_INFO2_MASK_ALLP;

	/* Cast to an appropriate type */
	SDMMC.SDIO_INFO1_MASK.LONGLONG = SDIO_INFO1_MASK_ALLP;

	/* Cast to an appropriate type */
	SDMMC.SDIO_MODE.LONGLONG = 0x0000;

	/* Cast to an appropriate type */
	info1 = SDMMC.SD_INFO1.LONGLONG;

	/* Cast to an appropriate type */
	SDMMC.SD_INFO1.LONGLONG = (uint64_t)(info1 & ~SD_INFO1_MASK_TRNS_RESP);

	/* Cast to an appropriate type */
	SDMMC.SD_INFO2.LONGLONG = 0x0000;

	/* Cast to an appropriate type */
	SDMMC.SDIO_INFO1.LONGLONG = 0x0000;

	/* Cast to an appropriate type */
	SDMMC.SOFT_RST.LONGLONG = SOFT_RST_SDRST_RESET;

	/* Cast to an appropriate type */
	SDMMC.SOFT_RST.LONGLONG = SOFT_RST_SDRST_RELEASED;

	/* Cast to an appropriate type */
	SDMMC.DM_CM_INFO1_MASK.LONGLONG = DM_CM_INFO1_MASK_ALLP;

	/* Cast to an appropriate type */
	SDMMC.DM_CM_INFO2_MASK.LONGLONG = DM_CM_INFO2_MASK_ALLP;

	/* Cast to an appropriate type */
	SDMMC.DM_CM_INFO1.LONGLONG = (uint64_t)0;

	/* Cast to an appropriate type */
	SDMMC.DM_CM_INFO2.LONGLONG = (uint64_t)0;

	/* Cast to an appropriate type */
	SDMMC.HOST_MODE.LONGLONG = HOST_MODE_64BIT_ACCESS;

	/* Cast to an appropriate type */
	SDMMC.SD_OPTION.LONGLONG = SD_OPTION_INIT;

	return SD_OK;
}
/******************************************************************************
 End of function sd_init
 *****************************************************************************/

/******************************************************************************
 * Function Name: _sd_init_error
 * Description  : initialize SD Driver error.
 * Arguments    : int32_t sd_port : channel no (0 or 1)
 *              : int32_t ret     : return value
 * Return Value : SD_OK : end of succeed
 *              : SD_ERR: end of error
 *              : SD_ERR_CPU_IF : CPU-IF function error
 *****************************************************************************/
static int32_t _sd_init_error(int32_t sd_port, int32_t ret)
{
	gp_sdhandle[sd_port] = 0;  /* relese SD handle */
	return ret;
}
/******************************************************************************
 End of function _sd_init_error
 *****************************************************************************/

/******************************************************************************
 * Function Name: sd_set_buffer
 * Description  : initialize SD driver work buffer.
 *              : this buffer is used for mainly MKB process
 * Arguments    : int32_t sd_port : channel no (0 or 1)
 *              : void *buff      : work buffer address
 *              : uint32_t size   : work buffer size
 * Return Value : SD_OK : end of succeed
 *              : SD_ERR: end of error
 * Remark       : if applied to CPRM, allocating more than 8K bytes
 *****************************************************************************/
int32_t esd_set_buffer(int32_t sd_port, void *buff, uint32_t size)
{
	st_sdhndl_t  *p_hndl;

	/* check buffer boundary (octlet unit) */
	if (0 != ((uintptr_t)buff & 0x00000007u)) {
		return SD_ERR;
	}

	if ((0 != sd_port) && (1 != sd_port)) {
		return SD_ERR;
	}

	p_hndl = SD_GET_HNDLS(sd_port);
	if (0 == p_hndl) {
		return SD_ERR;  /* not initilized */
	}

	/* initialize buffer area */
	p_hndl->p_rw_buff = (uint8_t *)buff;

	/* initialize buffer size */
	p_hndl->buff_size = size;

	return SD_OK;
}
/******************************************************************************
 End of function sd_set_buffer
 *****************************************************************************/

/******************************************************************************
 * Function Name: sd_check_media
 * Description  : check card insertion
 *              : if card is inserted, return SD_OK
 *              : if card is not inserted, return SD_ERR
 *              : if SD handle is not initialized, return SD_ERR
 * Arguments    : int32_t sd_port : channel no (0 or 1)
 * Return Value : SD_OK : card is inserted
 *              : SD_ERR: card is not inserted
 *****************************************************************************/
int32_t esd_check_media(int32_t sd_port)
{
	st_sdhndl_t  *p_hndl;

	if ((0 != sd_port) && (1 != sd_port)) {
		return SD_ERR;
	}

	p_hndl = SD_GET_HNDLS(sd_port);
	if (0 == p_hndl) {
		return SD_ERR;  /* not initilized */
	}

	return SD_OK;
}
/******************************************************************************
 End of function sd_check_media
 *****************************************************************************/

/******************************************************************************
 * Function Name: sd_mount
 * Description  : mount SD card.
 *              : mount SD memory card user area
 *              : can be access user area after this function is finished
 *              : without errors
 *              : turn on power
 *              :
 *              : following is available SD Driver mode
 *              : SD_MODE_POLL     : software polling
 *              : SD_MODE_HWINT    : hardware interrupt
 *              : SD_MODE_SW       : software data transfer (SD_BUF)
 *              : SD_MODE_DMA      : DMA data transfer (SD_BUF)
 *              : SD_MODE_MEM      : only memory cards
 *              : SD_MODE_IO       : memory and io cards
 *              : SD_MODE_COMBO    : memory ,io and combo cards
 *              : SD_MODE_DS       : only default speed
 *              : SD_MODE_VER1X    : ver1.1 host
 *              : SD_MODE_VER2X    : ver2.x host
 * Arguments    : int32_t sd_port  : channel no (0 or 1)
 *              : uint32_t mode    : SD Driver operation mode
 *              : uint32_t voltage : operation voltage
 * Return Value : p_hndl->error    : SD handle error value
 *              : SD_OK : end of succeed
 *              : other : end of error
 * Remark       : user area should be mounted
 *****************************************************************************/
int32_t esd_mount(int32_t sd_port, uint32_t mode, uint32_t voltage)
{
	st_sdhndl_t *p_hndl;
	uint64_t    info1_back;
	uint16_t    sd_spec;
	uint16_t    sd_spec3;

	if ((0 != sd_port) && (1 != sd_port)) {
		return SD_ERR;
	}

	p_hndl = SD_GET_HNDLS(sd_port);
	if (0 == p_hndl) {
		return SD_ERR;  /* not initilized */
	}

	/* ==== check work buffer is allocated ==== */
	if (0 == p_hndl->p_rw_buff) {
		return SD_ERR;  /* not allocated yet */
	}

	/* ==== initialize parameter ==== */
	_sd_init_hndl(p_hndl, mode, voltage);
	p_hndl->error = SD_OK;

	/* ==== power on sequence ==== */
	/* ---- turn on voltage ---- */
	if (esddev_power_on(sd_port) != SD_OK) {
		_sd_set_err(p_hndl, SD_ERR_CPU_IF);
		return _sd_mount_error(p_hndl);
	}

	/* ---- set single port ---- */
	_sd_set_port(p_hndl, SD_PORT_SERIAL);

	/* ---- supply clock (card-identification ratio) ---- */
	if (_sd_set_clock(p_hndl, SD_CLK_400KHZ, SD_CLOCK_ENABLE) != SD_OK) {
		return p_hndl->error;     /* not inserted */
	}

	esddev_int_wait(sd_port, 2); /* add wait function  */

	esddev_loc_cpu(sd_port);

	/* Cast to an appropriate type */
	info1_back = SDMMC.SD_INFO1.LONGLONG;

	/* Cast to an appropriate type */
	info1_back &= (uint64_t)0xfff8;

	/* Cast to an appropriate type */
	SDMMC.SD_INFO1.LONGLONG = (uint64_t)info1_back;

	/* Cast to an appropriate type */
	SDMMC.SD_INFO2.LONGLONG = (uint64_t)0;

	/* Clear DMA Enable because of CPU Transfer */
	SDMMC.CC_EXT_MODE.LONGLONG = (uint64_t)(SDMMC.CC_EXT_MODE.LONGLONG & ~CC_EXT_MODE_DMASDRW); /* disable DMA  */

	/* ==== initialize card and distinguish card type ==== */
	if (_sd_card_init(p_hndl) != SD_OK) {
		return _sd_mount_error(p_hndl);  /* failed card initialize */
	}

	if (p_hndl->media_type & SD_MEDIA_MEM) {	/* with memory part */
		/* ==== check card registers ==== */
		/* ---- check CSD register ---- */
		if (_sd_check_csd(p_hndl) != SD_OK) {
			return _sd_mount_error(p_hndl);
		}

		/* ---- no check other registers (to be create) ---- */

		/* get user area size */
		if (_sd_get_size(p_hndl, SD_USER_AREA) != SD_OK) {
			return _sd_mount_error(p_hndl);
		}

		/* check write protect */
		p_hndl->write_protect |= (uint8_t)_sd_iswp(p_hndl);
	}

	if (p_hndl->media_type & SD_MEDIA_MEM) {	/* with memory part */
		if (_sd_mem_mount(p_hndl) != SD_OK) {
			return _sd_mount_error(p_hndl);
		}
		if (SD_ERR_CARD_LOCK == p_hndl->error) {
			p_hndl->mount = (SD_CARD_LOCKED | SD_MOUNT_LOCKED_CARD);

			/* ---- halt clock ---- */
			_sd_set_clock(p_hndl, 0, SD_CLOCK_DISABLE);
			return SD_OK_LOCKED_CARD;
		}
	}

	/* if SD memory card, get SCR register */
	if (p_hndl->media_type & SD_MEDIA_SD) {
		if (_sd_card_get_scr(p_hndl) != SD_OK) {
			return _sd_mount_error(p_hndl);
		}

		if (SD_SPEC_20 == p_hndl->sd_spec) {
			/* Cast to an appropriate type */
			sd_spec = (uint16_t)(p_hndl->scr[0] & SD_SPEC_REGISTER_MASK);

			/* Cast to an appropriate type */
			sd_spec3 = (uint16_t)(p_hndl->scr[1] & SD_SPEC_30_REGISTER);
			if ((SD_SPEC_20_REGISTER == sd_spec) && (SD_SPEC_30_REGISTER == sd_spec3)) {
				/* ---- more than phys spec ver3.00 ---- */
				p_hndl->sd_spec = SD_SPEC_30;
			} else {	/* ---- phys spec ver2.00 ---- */
				p_hndl->sd_spec = SD_SPEC_20;
			}
		} else {
			/* Cast to an appropriate type */
			sd_spec = (uint16_t)(p_hndl->scr[0] & SD_SPEC_REGISTER_MASK);
			if (SD_SPEC_11_REGISTER == sd_spec) {	/* ---- phys spec ver1.10 ---- */
				p_hndl->sd_spec = SD_SPEC_11;
			} else {	/* ---- phys spec ver1.00 or ver1.01 ---- */
				p_hndl->sd_spec = SD_SPEC_10;
			}
		}

		/* Cast to an appropriate type */
		(void)_sd_calc_erase_sector(p_hndl);
	}

	/* ---- set mount flag ---- */
	p_hndl->mount = SD_MOUNT_UNLOCKED_CARD;

	/* ---- halt clock ---- */
	_sd_set_clock(p_hndl, 0, SD_CLOCK_DISABLE);
	return p_hndl->error;
}
/******************************************************************************
 End of function sd_mount
 *****************************************************************************/

/******************************************************************************
 * Function Name: _sd_mount_error
 * Description  : mount SD card error.
 * Arguments    : st_sdhndl_t *p_hndl : SD handle
 * Return Value : p_hndl->error  : SD handle error value
 *              : SD_OK : end of succeed
 *              : other : end of error
 *****************************************************************************/
static int32_t _sd_mount_error(st_sdhndl_t *p_hndl)
{
	/* ---- halt clock ---- */
	_sd_set_clock(p_hndl, 0, SD_CLOCK_DISABLE);
	return p_hndl->error;
}
/******************************************************************************
 End of function _sd_mount_error
 *****************************************************************************/


/******************************************************************************
* Function Name: sddev_power_on
* Description  : Power-on H/W to use SDHI
* Arguments    : int32_t sd_port : channel no (0 or 1)
* Return Value : success : SD_OK
******************************************************************************/
static int32_t esddev_power_on(int32_t sd_port)
{
	return SD_OK;
}
/*******************************************************************************
 End of function sddev_power_on
 ******************************************************************************/

/******************************************************************************
 * Function Name: _sd_card_init
 * Description  : initialize card.
 *              : initialize card from idle state to stand-by
 *              : distinguish card type (SD, MMC, IO or COMBO)
 *              : get CID, RCA, CSD from the card
 * Arguments    : st_sdhndl_t *p_hndl : SD handle
 * Return Value : SD_OK : end of succeed
 *              : SD_ERR: end of error
 *****************************************************************************/
static int32_t _sd_card_init(st_sdhndl_t *p_hndl)
{
	int32_t  ret;
	int32_t  i;
	uint16_t if_cond_0;
	uint16_t if_cond_1;

	p_hndl->media_type = SD_MEDIA_UNKNOWN;
	if_cond_0 = p_hndl->if_cond[0];
	if_cond_1 = p_hndl->if_cond[1];

	/* ==== transfer idle state (issue CMD0) ==== */
	if (SD_MEDIA_UNKNOWN == p_hndl->media_type) {
		for (i = 0; i < 3; i++) {
			ret = _sd_send_cmd(p_hndl, CMD0);
			if (SD_OK == ret) {
				break;
			}
		}

		if (SD_OK != ret) {
			return SD_ERR;  /* error for CMD0 */
		}

		/* clear error by reissuing CMD0 */
		p_hndl->error = SD_OK;

		p_hndl->media_type |= SD_MEDIA_SD;

		p_hndl->partition_id = 0;

		if (SD_MODE_VER2X == p_hndl->sup_ver) {
			ret = _sd_card_send_cmd_arg(p_hndl, CMD8, SD_RSP_R7, if_cond_0, if_cond_1);
			if (SD_OK == ret) {
				/* check R7 response */
				if (p_hndl->if_cond[0] & 0xf000) {
					p_hndl->error = SD_ERR_IFCOND_VER;
					return SD_ERR;
				}
				if ((p_hndl->if_cond[1] & 0x00ff) != 0x00aa) {
					p_hndl->error = SD_ERR_IFCOND_ECHO;
					return SD_ERR;
				}
				p_hndl->sd_spec = SD_SPEC_20;         /* cmd8 have response.              */

				/* because of (phys spec ver2.00)   */
			} else {
				/* ==== clear illegal command error for CMD8 ==== */
				for (i = 0; i < 3; i++) {
					ret = _sd_send_cmd(p_hndl, CMD0);
					if (SD_OK == ret) {
						break;
					}
				}
				p_hndl->error = SD_OK;
				p_hndl->sd_spec = SD_SPEC_10;         /* cmd8 have no response.                   */

				/* because of (phys spec ver1.01 or 1.10)   */
			}
		} else {
			p_hndl->sd_spec = SD_SPEC_10;             /* cmd8 have response.                      */

			/* because of (phys spec ver1.01 or 1.10)   */
		}
	}

	/* set OCR (issue ACMD41) */
	ret = _sd_card_send_ocr(p_hndl, (int32_t)p_hndl->media_type);

	/* clear error due to card distinction */
	p_hndl->error = SD_OK;

	if (SD_OK != ret) {
		/* softreset for error clear (issue CMD0) */
		for (i = 0; i < 3; i++) {
			ret = _sd_send_cmd(p_hndl, CMD0);
			if (SD_OK == ret) {
				break;
			}
		}
		if (SD_OK != ret) {
			return SD_ERR;  /* error for CMD0 */
		}

		/* clear error by reissuing CMD0 */
		p_hndl->error = SD_OK;

		/* ---- get OCR (issue CMD1) ---- */
		ret = _sd_card_send_ocr(p_hndl, SD_MEDIA_MMC);
		if (SD_OK == ret) {
			/* MMC */
			p_hndl->media_type = SD_MEDIA_MMC;
			p_hndl->error = SD_OK;
		} else {
			/* unknown card */
			p_hndl->media_type = SD_MEDIA_UNKNOWN;
			_sd_set_err(p_hndl, SD_ERR_CARD_TYPE);
			return SD_ERR;
		}
	}

	/* ---- get CID (issue CMD2) ---- */
	if (_sd_card_send_cmd_arg(p_hndl, CMD2, SD_RSP_R2_CID, 0, 0) != SD_OK) {
		return SD_ERR;
	}
	return _sd_card_init_get_rca(p_hndl);
}
/******************************************************************************
 End of function _sd_card_init
 *****************************************************************************/

/******************************************************************************
 * Function Name: _sd_card_init_get_rca
 * Description  : initialize card.
 * Arguments    : st_sdhndl_t *p_hndl : SD handle
 * Return Value : SD_OK : end of succeed
 *              : SD_ERR: end of error
 *****************************************************************************/
static int32_t _sd_card_init_get_rca(st_sdhndl_t *p_hndl)
{
	int32_t  i;

	/* ---- get RCA (issue CMD3) ---- */
	if (p_hndl->media_type & SD_MEDIA_COMBO) {	/* IO or SD */
		for (i = 0; i < 3; i++) {
			if (_sd_card_send_cmd_arg(p_hndl, CMD3, SD_RSP_R6, 0, 0) != SD_OK) {
				return SD_ERR;
			}
			if (0x00 != p_hndl->rca[0]) {
				break;
			}
		}

		/* illegal RCA */
		if (3 == i) {
			_sd_set_err(p_hndl, SD_ERR_CARD_CC);
			return SD_ERR;
		}
	} else {
		p_hndl->rca[0] = 1;   /* fixed 1 */
		if (_sd_card_send_cmd_arg(p_hndl, CMD3, SD_RSP_R1, p_hndl->rca[0], 0x0000)
				!= SD_OK) {
			return SD_ERR;
		}
	}

	/* ==== stand-by state  ==== */

	/* ---- get CSD (issue CMD9) ---- */
	if (_sd_card_send_cmd_arg(p_hndl, CMD9, SD_RSP_R2_CSD, p_hndl->rca[0], 0x0000)
			!= SD_OK) {
		return SD_ERR;
	}

	p_hndl->dsr[0] = 0x0000;

	if (p_hndl->media_type & SD_MEDIA_MEM) {
		/* is DSR implimented? */
		if (p_hndl->csd[3] & 0x0010u) {		/* implimented */
			/* set DSR (issue CMD4) */
			p_hndl->dsr[0] = 0x0404;
			if (_sd_card_send_cmd_arg(p_hndl, CMD4, SD_RSP_NON, p_hndl->dsr[0], 0x0000)
					!= SD_OK) {
				return SD_ERR;
			}
		}
	}

	return SD_OK;
}
/******************************************************************************
 End of function _sd_card_init_get_rca
 *****************************************************************************/

/******************************************************************************
 * Function Name: _sd_mem_mount
 * Description  : mount memory card.
 *              : mount memory part from stand-by to transfer state
 * Arguments    : st_sdhndl_t *p_hndl : SD handle
 * Return Value : SD_OK : end of succeed
 *              : SD_ERR: end of error
 * Remark       : Added processing to select the physical partition #1
 *              : If you can select #1, issue CMD45.
 *              : After that, the currently selected the physical partition is obtained
 *              : and saved in the internal variable hndl->partition_id.
 *              : *** About reason not to issue CMD45 unconditionally
 *              : *** See _esd_card_select_partition() function column
 *****************************************************************************/
static int32_t _sd_mem_mount(st_sdhndl_t *p_hndl)
{
	/* case of combo, already supplied data transfer clock */
	if ((p_hndl->media_type & SD_MEDIA_IO) == 0) {
		/* ---- supply clock (data-transfer ratio) ---- */
		if (p_hndl->csd_tran_speed > SD_CLK_25MHZ) {
			p_hndl->csd_tran_speed = SD_CLK_25MHZ;

			/* Herein after, if switch-function(cmd6) is pass,      */
			/* p_hndl->csd_tran_speed is set to SD_CLK_50MHz          */
		}

		/* Cast to an appropriate type */
		if (_sd_set_clock(p_hndl, (int32_t)p_hndl->csd_tran_speed, SD_CLOCK_ENABLE) != SD_OK) {
			return _sd_mem_mount_error(p_hndl);
		}
	}

	/* ==== data-transfer mode(Transfer State) ==== */
	if (_sd_card_send_cmd_arg(p_hndl, CMD7, SD_RSP_R1B, p_hndl->rca[0], 0x0000)
			!= SD_OK) {
		return _sd_mem_mount_error(p_hndl);
	}

	if ((p_hndl->resp_status & 0x02000000)) {
		_sd_set_err(p_hndl, SD_ERR_CARD_LOCK);
		return SD_OK;
	}

	/* select the physical partition #1 */
	if (_esd_card_select_partition(p_hndl, 1) == SD_OK) {
		/* Get changed partition ID from device */
		_esd_get_partition_id(p_hndl, &p_hndl->partition_id);
	} else {
		/* It is NG in the internal function, but it is treated as SD_OK considering the subsequent processing */
		p_hndl->error = SD_OK;
		/* Don't save ID */
	}

	/* ---- set block length (issue CMD16) ---- */
	if (_sd_card_send_cmd_arg(p_hndl, CMD16, SD_RSP_R1, 0x0000, 0x0200) != SD_OK) {
		return _sd_mem_mount_error(p_hndl);
	}

	/* if 4bits transfer supported (SD memory card mandatory), change bus width 4bits */
	if (p_hndl->media_type & SD_MEDIA_SD) {
		_sd_set_port(p_hndl, p_hndl->sup_if_mode);
	}

	/* clear pull-up DAT3 */
	if (p_hndl->media_type & SD_MEDIA_SD) {
		if (_sd_send_acmd(p_hndl, ACMD42, 0, 0) != SD_OK) {
			return _sd_mem_mount_error(p_hndl);
		}

		/* check R1 resp */
		if (_sd_get_resp(p_hndl, SD_RSP_R1) != SD_OK) {
			return _sd_mem_mount_error(p_hndl);
		}
	}

	/* if SD memory card, get SD Status */
	if (p_hndl->media_type & SD_MEDIA_SD) {
		if (_sd_card_get_status(p_hndl) != SD_OK) {
			return _sd_mem_mount_error(p_hndl);
		}

		/* get protect area size */
		if (_sd_get_size(p_hndl, SD_PROT_AREA) != SD_OK) {
			return _sd_mem_mount_error(p_hndl);
		}
	}

	return SD_OK;
}
/******************************************************************************
 End of function _sd_mem_mount
 *****************************************************************************/

/******************************************************************************
 * Function Name: _sd_mem_mount_error
 * Description  : mount memory card error.
 * Arguments    : st_sdhndl_t *p_hndl : SD handle
 * Return Value : SD_OK : end of succeed
 *              : SD_ERR: end of error
 *****************************************************************************/
static int32_t _sd_mem_mount_error(st_sdhndl_t *p_hndl)
{
	/* ---- halt clock ---- */
	_sd_set_clock(p_hndl, 0, SD_CLOCK_DISABLE);
	return p_hndl->error;
}
/******************************************************************************
 End of function _sd_mem_mount_error
 *****************************************************************************/

/******************************************************************************
 * Function Name: _sd_card_get_status
 * Description  : get SD Status (issue ACMD13)
 * Arguments    : st_sdhndl_t *p_hndl : SD handle
 * Return Value : SD_OK : end of succeed
 *              : SD_ERR: end of error
 *****************************************************************************/
static int32_t _sd_card_get_status(st_sdhndl_t *p_hndl)
{
	int32_t  ret;
	int32_t  i;
	uint8_t  *p_rw_buff;

	/* Cast to an appropriate type */
	p_rw_buff = (uint8_t *)&s_stat_buff[p_hndl->sd_port][0];

	/* ---- get SD Status (issue ACMD13) ---- */
	if (_sd_read_byte(p_hndl, ACMD13, 0, 0, p_rw_buff, SD_STATUS_BYTE) != SD_OK) {
		return SD_ERR;
	}

	/* ---- distinguish SD ROM card ---- */
	if ((p_rw_buff[2] & 0xffu) == 0x00) {	/* [495:488] = 0x00 */
		ret = SD_OK;
		if ((p_rw_buff[3] & 0xffu) == 0x01) {
			p_hndl->write_protect |= SD_WP_ROM;
		}
	} else {
		ret = SD_ERR;
		_sd_set_err(p_hndl, SD_ERR_CARD_ERROR);
	}

	p_hndl->speed_class = p_rw_buff[8];
	p_hndl->perform_move = p_rw_buff[9];

	/* ---- save SD STATUS ---- */
	for (i = 0; i < (16 / sizeof(uint16_t)); i++) {
		p_hndl->sdstatus[i] = (s_stat_buff[p_hndl->sd_port][i] << 8) | (s_stat_buff[p_hndl->sd_port][i] >> 8);
	}

	return ret;
}
/******************************************************************************
 End of function _sd_card_get_status
 *****************************************************************************/

/******************************************************************************
 * Function Name: _sd_card_get_scr
 * Description  : get SCR register (issue ACMD51).
 * Arguments    : st_sdhndl_t *p_hndl : SD handle
 * Return Value : SD_OK : end of succeed
 *****************************************************************************/
static int32_t _sd_card_get_scr(st_sdhndl_t *p_hndl)
{
	uint8_t  *p_rw_buff;

	/* Cast to an appropriate type */
	p_rw_buff = (uint8_t *)&s_stat_buff[p_hndl->sd_port][0];

	/* ---- get SCR register (issue ACMD51) ---- */
	if (_sd_read_byte(p_hndl, ACMD51, 0, 0, p_rw_buff, SD_SCR_REGISTER_BYTE) != SD_OK) {
		return SD_ERR;
	}

	/* ---- save SCR register ---- */
	p_hndl->scr[0] = (s_stat_buff[p_hndl->sd_port][0] << 8) | (s_stat_buff[p_hndl->sd_port][0] >> 8);
	p_hndl->scr[1] = (s_stat_buff[p_hndl->sd_port][1] << 8) | (s_stat_buff[p_hndl->sd_port][1] >> 8);
	p_hndl->scr[2] = (s_stat_buff[p_hndl->sd_port][2] << 8) | (s_stat_buff[p_hndl->sd_port][2] >> 8);
	p_hndl->scr[3] = (s_stat_buff[p_hndl->sd_port][3] << 8) | (s_stat_buff[p_hndl->sd_port][3] >> 8);

	return SD_OK;
}
/******************************************************************************
 End of function _sd_card_get_scr
 *****************************************************************************/

/******************************************************************************
 * Function Name: _sd_read_byte
 * Description  : read byte data from card
 *              : issue byte data read command and read data from SD_BUF
 *              : using following commands
 *              : SD STATUS(ACMD13),SCR(ACMD51),NUM_WRITE_BLOCK(ACMD22),
 * Arguments    : st_sdhndl_t *p_hndl : SD handle
 *              : uint16_t cmd        : command code
 *              : uint16_t h_arg      : command argument high [31:16]
 *              : uint16_t l_arg      : command argument low [15:0]
 *              : uint8_t *readbuff   : read data buffer
 *              : uint16_t byte       : the number of read bytes
 * Return Value : SD_OK : end of succeed
 * Remark       : transfer type is PIO
 *****************************************************************************/
static int32_t _sd_read_byte(st_sdhndl_t *p_hndl, uint16_t cmd, uint16_t h_arg,
						uint16_t l_arg, uint8_t *readbuff, uint16_t byte)
{
	/* ---- disable SD_SECCNT ---- */
	SDMMC.SD_STOP.LONGLONG = 0x0000;

	/* ---- set transfer bytes ---- */
	SDMMC.SD_SIZE.LONGLONG = (uint64_t)byte;

	/* ---- issue command ---- */
	if (cmd & 0x0040u) {	/* ACMD13, ACMD22 and ACMD51 */
		if (_sd_send_acmd(p_hndl, cmd, h_arg, l_arg) != SD_OK) {
			if ((SD_ERR_END_BIT == p_hndl->error) ||
					(SD_ERR_CRC == p_hndl->error)) {
				/* continue */
				;
			} else {
				return _sd_read_byte_error(p_hndl);
			}
		}
	} else {
		_sd_set_arg(p_hndl, h_arg, l_arg);
		if (_sd_send_cmd(p_hndl, cmd) != SD_OK) {
			return SD_ERR;
		}
	}

	/* ---- check R1 response ---- */
	if (_sd_get_resp(p_hndl, SD_RSP_R1) != SD_OK) {
		return _sd_read_byte_error(p_hndl);
	}

	/* enable All end, BRE and errors */
	_sd_set_int_mask(p_hndl, SD_INFO1_MASK_DATA_TRNS, SD_INFO2_MASK_BRE);

	/* ---- wait BRE interrupt ---- */
	if (esddev_int_wait(p_hndl->sd_port, SD_TIMEOUT_MULTIPLE) != SD_OK) {
		_sd_set_err(p_hndl, SD_ERR_HOST_TOE);
		return _sd_read_byte_error(p_hndl);
	}

	/* ---- check errors ---- */
	if (p_hndl->int_info2 & SD_INFO2_MASK_ERR) {
		_sd_check_info2_err(p_hndl);
		return _sd_read_byte_error(p_hndl);
	}

	/* Cast to an appropriate type */
	_sd_clear_info(p_hndl, 0x0000, SD_INFO2_MASK_RE); /* clear BRE bit */

	/* transfer data */

	if (esddev_read_data(p_hndl->sd_port, readbuff, (uintptr_t)(&SDMMC.SD_BUF0.LONGLONG), (int32_t)byte) != SD_OK) {
		_sd_set_err(p_hndl, SD_ERR_CPU_IF);
		return _sd_read_byte_error(p_hndl);
	}

	/* wait All end interrupt */
	if (esddev_int_wait(p_hndl->sd_port, SD_TIMEOUT_RESP) != SD_OK) {
		_sd_set_err(p_hndl, SD_ERR_HOST_TOE);
		return _sd_read_byte_error(p_hndl);
	}

	/* ---- check errors ---- */
	if (p_hndl->int_info2 & SD_INFO2_MASK_ERR) {
		_sd_check_info2_err(p_hndl);
		return _sd_read_byte_error(p_hndl);
	}

	/* Cast to an appropriate type */
	_sd_clear_info(p_hndl, SD_INFO1_MASK_DATA_TRNS, SD_INFO2_MASK_ERR); /* clear All end bit */

	/* disable all interrupts */
	_sd_clear_int_mask(p_hndl, SD_INFO1_MASK_DATA_TRNS, SD_INFO2_MASK_BRE);

	return SD_OK;
}
/******************************************************************************
 End of function _sd_read_byte
 *****************************************************************************/


/******************************************************************************
* Function Name: sddev_read_data
* Description  : read from SDHI buffer FIFO
* Arguments    : int32_t sd_port   : channel no (0 or 1)
*              : uint8_t *buff     : buffer addrees to store reading datas
*              : uint32_t reg_addr : SDIP FIFO address
*              : int32_t num       : counts to read(unit:byte)
* Return Value : success : SD_OK
*              : fail    : SD_ERR
******************************************************************************/
static int32_t esddev_read_data(int32_t sd_port, uint8_t *buff, uint32_t reg_addr, int32_t num)
{
	int32_t  i;
	int32_t  cnt;
	uint64_t *p_reg;
	uint64_t *p_l;
	uint8_t  *p_c;
	volatile uint64_t tmp;

	/* Cast to an appropriate type */
	p_reg = (uint64_t *)((uintptr_t)reg_addr);

	cnt = (num / 8);

	/* Cast to an appropriate type */
	if (0uL != ((uintptr_t)buff & 0x7uL)) {
		/* Cast to an appropriate type */
		p_c = (uint8_t *)buff;
		for (i = cnt; i > 0 ; i--) {
			tmp = *p_reg;

			/* Cast to an appropriate type */
			*p_c++ = (uint8_t)(tmp);

			/* Cast to an appropriate type */
			*p_c++ = (uint8_t)(tmp >> 8);

			/* Cast to an appropriate type */
			*p_c++ = (uint8_t)(tmp >> 16);

			/* Cast to an appropriate type */
			*p_c++ = (uint8_t)(tmp >> 24);

			/* Cast to an appropriate type */
			*p_c++ = (uint8_t)(tmp >> 32);

			/* Cast to an appropriate type */
			*p_c++ = (uint8_t)(tmp >> 40);

			/* Cast to an appropriate type */
			*p_c++ = (uint8_t)(tmp >> 48);

			/* Cast to an appropriate type */
			*p_c++ = (uint8_t)(tmp >> 56);
		}

		cnt = (num % 8);
		if (0 != cnt) {
			tmp = *p_reg;
			for (i = cnt; i > 0 ; i--) {
				/* Cast to an appropriate type */
				*p_c++ = (uint8_t)(tmp);
				tmp >>= 8;
			}
		}
	} else {
		/* Cast to an appropriate type */
		p_l = (uint64_t *)buff;
		for (i = cnt; i > 0 ; i--) {
			*p_l++ = *p_reg;
		}

		cnt = (num % 8);
		if (0 != cnt) {
			/* Cast to an appropriate type */
			p_c = (uint8_t *)p_l;
			tmp = *p_reg;
			for (i = cnt; i > 0 ; i--) {
				/* Cast to an appropriate type */
				*p_c++ = (uint8_t)(tmp);
				tmp >>= 8;
			}
		}
	}

	return SD_OK;
}
/*******************************************************************************
 End of function sddev_read_data
 ******************************************************************************/

/******************************************************************************
 * Function Name: _sd_read_byte_error
 * Description  : read byte data error.
 * Arguments    : st_sdhndl_t *p_hndl : SD handle.
 * Return Value : SD_ERR: end of error.
 *****************************************************************************/
static int32_t _sd_read_byte_error(st_sdhndl_t *p_hndl)
{
	/* Cast to an appropriate type */
	SDMMC.SD_STOP.LONGLONG = (uint64_t)0x0001;                       /* stop data transfer   */

	/* Cast to an appropriate type */
	_sd_clear_info(p_hndl, SD_INFO1_MASK_DATA_TRNS, SD_INFO2_MASK_ERR); /* clear All end bit    */

	/* disable all interrupts */
	_sd_clear_int_mask(p_hndl, SD_INFO1_MASK_DATA_TRNS, SD_INFO2_MASK_BRE);

	return SD_ERR;
}
/******************************************************************************
 End of function _sd_read_byte_error
 *****************************************************************************/

/******************************************************************************
 * Function Name: _sd_calc_erase_sector
 * Description  : calculate erase sector.
 *              : This function calculate erase sector for SD Phy Ver2.0.
 * Arguments    : st_sdhndl_t *p_hndl : SD handle
 * Return Value : SD_OK : end of succeed
 *              : SD_ERR: end of error
 * Remark       : transfer type is PIO
 *****************************************************************************/
static int32_t _sd_calc_erase_sector(st_sdhndl_t *p_hndl)
{
	uint16_t au;
	uint16_t erase_size;

	if ((p_hndl->scr[0] & 0x0f00) == 0x0200) {
		/* AU is not defined,set to fixed value */
		p_hndl->erase_sect = SD_ERASE_SECTOR;

		/* get AU size */
		au = p_hndl->sdstatus[5] >> 12;

		if ((au > 0) && (au < 0x0a)) {
			/* get AU_SIZE(sectors) */
			p_hndl->erase_sect = ((8 * 1024) / 512) << au;

			/* get ERASE_SIZE */
			erase_size = (p_hndl->sdstatus[5] << 8) | (p_hndl->sdstatus[6] >> 8);
			if (0 != erase_size) {
				p_hndl->erase_sect *= erase_size;
			}
		}

	} else {
		/* If card is not Ver2.0,it use ERASE_BLK_LEN in CSD *//* DO NOTHING */
		;
	}
	return SD_OK;
}
/******************************************************************************
 End of function _sd_calc_erase_sector
 *****************************************************************************/

/**********************************************************************************************************************
 * Function Name: _esd_card_query_partitions
 * Description  : Issue CMD45 and get QUERY_PARTITIONS information
 *              : If you can get it, it will be saved in the area of p_rw_buff.
 * Arguments    : st_sdhndl_t *p_hndl      : SD handle
 *              : int32_t opcode           : Operation code (0xA1, 0xB1, 0xB2)
 *              : uint8_t *p_rw_buff       : 512byte area
 * Return Value : SD_OK : end of succeed
 *              : SD_ERR: end of error
 *********************************************************************************************************************/
static int32_t _esd_card_query_partitions(st_sdhndl_t *p_hndl, uint32_t opcode, uint8_t *p_rw_buff)
{
	int32_t tmp = SD_OK;

	_sd_read_byte(p_hndl, CMD45, opcode << 8, 0, p_rw_buff, SD_QUERY_PARTITION_SIZE);

	tmp = p_hndl->error;

	/* issue CMD13 to clear the status */
	_sd_card_send_cmd_arg(p_hndl, CMD13, SD_RSP_R1, p_hndl->rca[0], 0x0000);

	if (tmp != SD_OK)
		p_hndl->error = tmp;

	return p_hndl->error;
}
/**********************************************************************************************************************
 * End of function _esd_card_query_partitions
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: _esd_card_select_partition
 * Description  : SELECT PARTITIONS information [issue CMD43].
 * Arguments    : st_sdhndl_t *p_hndl : SD handle
 *              : int32_t id          : Partition ID
 * Return Value : SD_OK               : SELECT_PARTITION success
 *              : SD_ERR_RES_TOE      : SELECT_PARTITION not supported
 *              : SD_ERR_OUT_OF_RANGE : SELECT_PARTITION supported but the specified partition does not exist
 * Remark       : Summary of behavior for #SELECT_PARTITION(CMD43)
 *              : If eSDv2.10(Made by SanDisk eSD) is supported
 *              :  - Corresponding device is forcibly terminated regardless of partition
 *              :  - If it can be changed to the specified partition, next command(CMD13) ends with SD_OK
 *              :  - Next command(CMD13) returns an OUT_OF_RANGE error if the specified partition does not exist
 *              :
 *              : If eSDv2.10(Made by Tosiba eSD/marketing SDSC/SDHC etc)isn't supported
 *              :  - If device is not supported, CMD45 ends with NO_RESPONSE
 *              :  - Since next command responds with an error, issue CMD13 to clear the error
 *              :  - I want to return the error value at the time of CMD43 execution,
 *              :    so temporarily evacuate so that CMD13 is not overwritten
 *********************************************************************************************************************/
static int32_t _esd_card_select_partition(st_sdhndl_t *p_hndl, uint32_t id)
{
	int32_t tmp = SD_OK;

	_sd_card_send_cmd_arg(p_hndl, CMD43, SD_RSP_R1B, id << 8, 0x0000);

	tmp = p_hndl->error;

	/* issue CMD13 to clear the status */
	_sd_card_send_cmd_arg(p_hndl, CMD13, SD_RSP_R1, p_hndl->rca[0], 0x0000);

	if (tmp != SD_OK)
		p_hndl->error = tmp;

	return p_hndl->error;
}
/**********************************************************************************************************************
 * End of function _esd_card_select_partition
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: _esd_get_partition_id
 * Description  : Issue CMD45 to device.
 *              : If failed.
 *              : Terminates with an error and sets nothing to the argument ID.
 * Arguments    : st_sdhndl_t *p_hndl : SD handle
 *              : int32_t *id         : Partition ID
 * Return Value : SD_OK : end of succeed
 *              : SD_ERR : end of error
 *********************************************************************************************************************/
static int32_t _esd_get_partition_id(st_sdhndl_t *p_hndl, int32_t *id)
{
	/* Issue the QUERY_PARTITION_LIST command  */
	if (_esd_card_query_partitions(p_hndl, 0xA1, p_hndl->p_rw_buff) != SD_OK) {
		return p_hndl->error;
	}

	*id = p_hndl->p_rw_buff[511];

	return SD_OK;
}
/**********************************************************************************************************************
 * End of function _esd_get_partition_id
 *********************************************************************************************************************/

 /**********************************************************************************************************************
 * Function Name: esd_get_partition_id
 * Description  : Get the currently selected partitionID from internal variables.
 * Arguments    : int32_t *id       : Partition ID
 * Return Value : SD_OK : end of succeed
 *              : SD_ERR : not initialized
 * Call functions : _sd_get_hndl
 * Remark       : Get directly from device when partition is selected
 *              : Therefore,it does not issue CMD45
 *********************************************************************************************************************/
static int32_t esd_get_partition_id(int32_t sd_port, int32_t *id)
{
    st_sdhndl_t    *p_hndl;

    p_hndl = _sd_get_hndl(sd_port);
    if(p_hndl == 0)
    {
        return SD_ERR;    /* Not initialized */
    }
    if(id != 0)
    {
        *id = p_hndl->partition_id;
    }
    return SD_OK;
}
/**********************************************************************************************************************
 * End of function esd_get_partition_id
 *********************************************************************************************************************/
/******************************************************************************
 * Function Name: _sd_set_clock
 * Description  : control SD clock.
 *              : supply or halt SD clock
 *              : if enable is SD_CLOCK_ENABLE, supply SD clock
 *              : if enable is SD_CLOCK_DISKABLE, halt SD clock
 * Arguments    : st_sdhndl_t *p_hndl : SD handle
 *              : int32_t clock       : SD clock frequency
 *              : int32_t enable      : supply or halt SD clock
 * Return Value : SD_OK : end of succeed
 *              : SD_ERR: end of error
 *****************************************************************************/
static int32_t _sd_set_clock(st_sdhndl_t *p_hndl, int32_t clock, int32_t enable)
{
	uint32_t div;
	int32_t  i;

	if (SD_CLOCK_ENABLE == enable) {
		/* convert clock frequency to clock divide ratio */
		div = esddev_get_clockdiv(p_hndl->sd_port, clock);

		if ((div > SD_DIV_512) && (SD_DIV_1 != div)) {
			_sd_set_err(p_hndl, SD_ERR_CPU_IF);
			return SD_ERR;
		}

		/* SCLKEN = 0 */
		SDMMC.SD_CLK_CTRL.LONGLONG = SDMMC.SD_CLK_CTRL.LONGLONG & (~SD_CLK_CTRL_SCLKEN);

		/* write DIV[7:0] */
		SDMMC.SD_CLK_CTRL.LONGLONG = ((SDMMC.SD_CLK_CTRL.LONGLONG  & (~0x00FFuL)) | div);

		/* SCLKEN = 1 */
		SDMMC.SD_CLK_CTRL.LONGLONG = (SDMMC.SD_CLK_CTRL.LONGLONG | SD_CLK_CTRL_SCLKEN);
	} else {
		for (i = 0; i < SCLKDIVEN_LOOP_COUNT; i++) {
#ifdef USE_INFO2_CBSY
			/* Cast to an appropriate type */
			if ((SDMMC.SD_INFO2.LONGLONG & SD_INFO2_MASK_CBSY) == 0) {
				break;
			}
#else
			/* Cast to an appropriate type */
			if (SDMMC.SD_INFO2.LONGLONG & SD_INFO2_MASK_SCLKDIVEN) {
				break;
			}
#endif
		}
		if (SCLKDIVEN_LOOP_COUNT == i) {
			p_hndl->error = SD_ERR_CBSY_ERROR;
		}

		/* SCLKEN = 0  halt */
		SDMMC.SD_CLK_CTRL.LONGLONG = (SDMMC.SD_CLK_CTRL.LONGLONG & (~SD_CLK_CTRL_SCLKEN));
	}
	return SD_OK;
}
/******************************************************************************
 End of function _sd_set_clock
 *****************************************************************************/


/******************************************************************************
* Function Name: sddev_get_clockdiv
* Description  : Get clock div value.
* Arguments    : int32_t sd_port : channel no (0 or 1)
*              : int32_t clock   : request clock frequency
*              :   SD_CLK_50MHZ
*              :   SD_CLK_25MHZ
*              :   SD_CLK_20MHZ
*              :   SD_CLK_10MHZ
*              :   SD_CLK_5MHZ
*              :   SD_CLK_1MHZ
*              :   SD_CLK_400KHZ
* Return Value : clock div value
*              :   SD_DIV_4   : 1/4   clock
*              :   SD_DIV_8   : 1/8   clock
*              :   SD_DIV_16  : 1/16  clock
*              :   SD_DIV_32  : 1/32  clock
*              :   SD_DIV_256 : 1/256 clock
*              :   SD_DIV_512 : 1/512 clock
******************************************************************************/
static uint32_t esddev_get_clockdiv(int32_t sd_port, int32_t clock)
{
	uint32_t div;

	switch (clock) {
	case SD_CLK_50MHZ:
		div = SD_DIV_4;        /* 133.25MHz/4 = 33.31MHz   */
		break;
	case SD_CLK_25MHZ:
	case SD_CLK_20MHZ:
		div = SD_DIV_8;        /* 133.25MHz/8 = 16.65MHz   */
		break;
	case SD_CLK_10MHZ:
		div = SD_DIV_16;       /* 133.25MHz/16 = 8.32MHz   */
		break;
	case SD_CLK_5MHZ:
		div = SD_DIV_32;       /* 133.25MHz/32 = 4.16MHz   */
		break;
	case SD_CLK_1MHZ:
		div = SD_DIV_256;      /* 133.25MHz/256 = 520.5kHz */
		break;
	case SD_CLK_400KHZ:
		div = SD_DIV_512;      /* 133.25MHz/512 = 260.2kHz */
		break;
	default:
		div = SD_DIV_512;      /* 133.25MHz/512 = 260.2kHz */
		break;
	}

	return div;
}
/*******************************************************************************
 End of function sddev_get_clockdiv
 ******************************************************************************/

/******************************************************************************
 * Function Name: _sd_set_port
 * Description  : control data bus width.
 *              : change data bus width
 *              : if port is SD_PORT_SERIAL, set data bus width 1bit
 *              : if port is SD_PORT_PARALEL, set data bus width 4bits
 *              : change between 1bit and 4bits by ACMD6
 * Arguments    : st_sdhndl_t *p_hndl : SD handle
 *              : int32_t port        : setting bus with
 * Return Value : SD_OK : end of succeed
 *              : SD_ERR: end of error
 * Remark       : before execute this function, check card supporting bus
 *              : width
 *              : SD memory card is 4bits support mandatory
 *****************************************************************************/
static int32_t _sd_set_port(st_sdhndl_t *p_hndl, int32_t port)
{
	uint64_t reg;
	uint16_t arg;

	if (p_hndl->media_type & SD_MEDIA_SD) {	/* SD or COMBO */
		/* ---- check card state ---- */
		if ((p_hndl->resp_status & RES_STATE) == STATE_TRAN) {	/* transfer state */
			if (SD_PORT_SERIAL == port) {
				arg = ARG_ACMD6_1BIT;
			} else {
				arg = ARG_ACMD6_4BIT;
			}

			/* ==== change card bus width (issue ACMD6) ==== */
			if (_sd_send_acmd(p_hndl, ACMD6, 0, arg) != SD_OK) {
				return SD_ERR;
			}
			if (_sd_get_resp(p_hndl, SD_RSP_R1) != SD_OK) {
				return SD_ERR;
			}
		}
	}

	/* ==== change SDHI bus width ==== */
	if (SD_PORT_SERIAL == port) {	/* 1bit */

		/* Cast to an appropriate type */
		reg = SDMMC.SD_OPTION.LONGLONG;

		/* Cast to an appropriate type */
		reg |= SD_OPTION_WIDTH;

		/* Cast to an appropriate type */
		SDMMC.SD_OPTION.LONGLONG = reg;
	} else {	/* 4bits */
		/* Cast to an appropriate type */
		reg = SDMMC.SD_OPTION.LONGLONG;

		/* Cast to an appropriate type */
		reg &= (~SD_OPTION_WIDTH_MASK);

		/* Cast to an appropriate type */
		SDMMC.SD_OPTION.LONGLONG = reg;
	}

	/* Cast to an appropriate type */
	p_hndl->if_mode = (uint8_t)port;

	return SD_OK;
}
/******************************************************************************
 End of function _sd_set_port
 *****************************************************************************/

/******************************************************************************
 * Function Name: _sd_set_err
 * Description  : set errors information.
 *              : set error information (=error) to SD Handle member
 *              : (=p_hndl->error)
 * Arguments    : st_sdhndl_t *p_hndl : SD handle
 *              : int32_t error       : setting error information
 * Return Value : SD_OK : end of succeed
 *              : SD_ERR: end of error
 * Remark       : if p_hndl->error was already set, no overwrite it
 *****************************************************************************/
static int32_t _sd_set_err(st_sdhndl_t *p_hndl, int32_t error)
{
	if (SD_OK == p_hndl->error) {
		p_hndl->error = error;
	}

	return SD_OK;
}
/******************************************************************************
 End of function _sd_set_err
 *****************************************************************************/

/******************************************************************************
 * Function Name: sd_get_type
 * Description  : get card type.
 *              : get mounting card type, current and supported speed mode
 *              : and capacity type
 *              : (if SD memory card)
 *              : following card types are defined
 *              : SD_MEDIA_UNKNOWN : unknown media
 *              : SD_MEDIA_MMC     : MMC card
 *              : SD_MEDIA_SD      : SD Memory card
 *              : SD_MEDIA_COMBO   : SD COMBO card (IO spec ver1.10)
 *              : SD_MEDIA_EMBEDDED:
 * Arguments    : int32_t  sd_port : channel no (0 or 1)
 *              : uint16_t *type   : mounting card type
 *              : uint16_t *speed  : speed mode
 *              : uint8_t  *capa   : card capacity
 *              :   Standard capacity:0, High capacity:1
 * Return Value : SD_OK : end of succeed
 *              : SD_ERR: end of error
 * Remark       : if pointer has NULL ,the value isn't returned
 *              : only SD memory card, speed mode has meaning
 *****************************************************************************/
int32_t esd_get_type(int32_t sd_port, uint16_t *type, uint16_t *speed, uint8_t *capa)
{
	st_sdhndl_t *p_hndl;

	if ((0 != sd_port) && (1 != sd_port)) {
		return SD_ERR;
	}

	p_hndl = SD_GET_HNDLS(sd_port);
	if (0 == p_hndl) {
		return SD_ERR;  /* not initilized */
	}

	if (type) {
		*type = p_hndl->media_type;
	}

	if (p_hndl->partition_id > 0) {
		*type |= SD_MEDIA_EMBEDDED;
	}

	if (speed) {
		*speed = p_hndl->speed_mode;
	}
	if (capa) {
		*capa = p_hndl->csd_structure;
	}
	return SD_OK;
}
/******************************************************************************
 End of function sd_get_type
 *****************************************************************************/

/******************************************************************************
* Function Name: sddev_loc_cpu
* Description  : lock cpu to disable interrupt
* Arguments    : int32_t sd_port : channel no (0 or 1)
* Return Value : success : SD_OK
******************************************************************************/
static int32_t esddev_loc_cpu(int32_t sd_port)
{
	return SD_OK;
}
/*******************************************************************************
 End of function sddev_loc_cpu
 ******************************************************************************/

/******************************************************************************
* Function Name: sddev_init
* Description  : Initialize H/W to use SDHI
* Arguments    : int32_t sd_port : channel no (0 or 1)
* Return Value : success : SD_OK
******************************************************************************/
static int32_t esddev_init(int32_t sd_port)
{
	return SD_OK;
}
/*******************************************************************************
 End of function sddev_init
 ******************************************************************************/


/******************************************************************************
 * Function Name: _sd_send_cmd
 * Description  : issue SD command, hearafter wait recive response
 * Arguments    : st_sdhndl_t *p_hndl : SD handle
 *              : uint16_t cmd        : command code
 * Return Value : SD_OK : end of succeed
 *              : SD_ERR: end of error
 * Remark       : not get response and check response errors
 *****************************************************************************/
static int32_t _sd_send_cmd(st_sdhndl_t *p_hndl, uint16_t cmd)
{
	int32_t time;
	int32_t i;

	p_hndl->error = SD_OK;

	if (CMD38 == cmd) {	/* erase command */
		time = SD_TIMEOUT_ERASE_CMD;    /* extend timeout 1 sec */
	} else if (ACMD46 == cmd) {	/* ACMD46 */
		time = SD_TIMEOUT_MULTIPLE; /* same as write timeout */
	} else if (CMD7 == cmd) {
		time = SD_TIMEOUT_RESP; /* same as write timeout */
	} else if (CMD12 == cmd) {
		time = SD_TIMEOUT_RESP; /* same as write timeout */
	} else if (CMD43 == cmd) {
		time = SD_TIMEOUT_RESP;
	} else if (CMD44 == cmd) {
		time = SD_TIMEOUT_RESP;
	} else if (CMD45 == cmd) {
		time = SD_TIMEOUT_RESP;
	} else {
		time = SD_TIMEOUT_CMD;
	}

	/* enable resp end and illegal access interrupts *//* Cast to an appropriate type */
	_sd_set_int_mask(p_hndl, SD_INFO1_MASK_RESP, 0);

	for (i = 0; i < SCLKDIVEN_LOOP_COUNT; i++) {

		/* Cast to an appropriate type */
		if (SDMMC.SD_INFO2.LONGLONG & SD_INFO2_MASK_SCLKDIVEN) {
			break;
		}
	}
	if (SCLKDIVEN_LOOP_COUNT == i) {
		_sd_set_err(p_hndl, SD_ERR_CBSY_ERROR);       /* treate as CBSY ERROR */
		return p_hndl->error;
	}

	/* ---- issue command ---- */

	/* Cast to an appropriate type */
	SDMMC.SD_CMD.LONGLONG = (uint64_t)cmd;

	/* ---- wait resp end ---- */
	if (esddev_int_wait(p_hndl->sd_port, time) != SD_OK) {
		_sd_set_err(p_hndl, SD_ERR_HOST_TOE);

		/* Cast to an appropriate type */
		_sd_clear_int_mask(p_hndl, SD_INFO1_MASK_RESP, SD_INFO2_MASK_ILA);
		return p_hndl->error;
	}

	/* disable resp end and illegal access interrupts *//* Cast to an appropriate type */
	_sd_clear_int_mask(p_hndl, SD_INFO1_MASK_RESP, SD_INFO2_MASK_ILA);

	_sd_get_info2(p_hndl);    /* get SD_INFO2 register */

	_sd_check_info2_err(p_hndl);  /* check SD_INFO2 error bits */

	/* Cast to an appropriate type */
	if (!(p_hndl->int_info1 & SD_INFO1_MASK_RESP)) {
		_sd_set_err(p_hndl, SD_ERR_NO_RESP_ERROR);    /* no response */
	}

	/* ---- clear previous errors ---- *//* Cast to an appropriate type */
	_sd_clear_info(p_hndl, SD_INFO1_MASK_RESP, SD_INFO2_MASK_ERR);

	return p_hndl->error;
}
/******************************************************************************
 End of function _sd_send_cmd
 *****************************************************************************/

/******************************************************************************
 * Function Name: _sd_send_acmd
 * Description  : issue application specific command, hearafter wait recive
 *              : response
 *              : issue CMD55 preceide application specific command
 * Arguments    : st_sdhndl_t *p_hndl : SD handle
 *              : uint16_t cmd        : command code
 *              : uint16_t h_arg      : command argument high [31:16]
 *              : uint16_t l_arg      : command argument low [15:0]
 * Return Value : SD_OK : end of succeed
 *              : SD_ERR: end of error
 *****************************************************************************/
static int32_t _sd_send_acmd(st_sdhndl_t *p_hndl, uint16_t cmd, uint16_t h_arg,
						uint16_t l_arg)
{
	/* ---- issue CMD 55 ---- */
	_sd_set_arg(p_hndl, p_hndl->rca[0], 0);
	if (_sd_send_cmd(p_hndl, CMD55) != SD_OK) {
		return SD_ERR;
	}

	if (_sd_get_resp(p_hndl, SD_RSP_R1) != SD_OK) {
		return SD_ERR;
	}

	/* ---- issue ACMD ---- */
	_sd_set_arg(p_hndl, h_arg, l_arg);
	if (_sd_send_cmd(p_hndl, cmd) != SD_OK) {
		return SD_ERR;
	}

	return SD_OK;
}
/******************************************************************************
 End of function _sd_send_acmd
 *****************************************************************************/

/******************************************************************************
 * Function Name: _sd_card_send_cmd_arg
 * Description  : issue general SD command.
 *              : issue command specified cmd code
 *              : get and check response
 * Arguments    : st_sdhndl_t *p_hndl : SD handle
 *              : uint16_t cmd        : command code (CMD18 or CMD25)
 *              : int32_t  resp       : command response
 *              : uint16_t h_arg      : command argument high [31:16]
 *              : uint16_t l_arg      : command argument low [15:0]
 * Return Value : SD_OK : end of succeed
 *              : SD_ERR: end of error
 *****************************************************************************/
static int32_t _sd_card_send_cmd_arg(st_sdhndl_t *p_hndl, uint16_t cmd, int32_t resp,
								uint16_t h_arg, uint16_t l_arg)
{
	int32_t ret;

	_sd_set_arg(p_hndl, h_arg, l_arg);

	/* ---- issue command ---- */
	ret = _sd_send_cmd(p_hndl, cmd);
	if (SD_OK == ret) {
		ret = _sd_get_resp(p_hndl, resp); /* get and check response */
	}
	return ret;
}
/******************************************************************************
 End of function _sd_card_send_cmd_arg
 *****************************************************************************/

/******************************************************************************
 * Function Name: _sd_set_arg
 * Description  : set command argument to SDHI
 *              : h_arg means higher 16bits [31:16] and  set SD_ARG0
 *              : l_arg means lower 16bits [15:0] and set SD_ARG1
 * Arguments    : st_sdhndl_t *p_hndl : SD handle
 *              : uint16_t h_arg      : command argument high [31:16]
 *              : uint16_t l_arg      : command argument low [15:0]
 * Return Value : none
 * Remark       : SD_ARG0 and SD_ARG1 are like little endian order
 *****************************************************************************/
void _sd_set_arg(st_sdhndl_t *p_hndl, uint16_t h_arg, uint16_t l_arg)
{
	/* Cast to an appropriate type */
	SDMMC.SD_ARG.WORD.LL = l_arg;

	/* Cast to an appropriate type */
	SDMMC.SD_ARG1.WORD.LL = h_arg;
}
/******************************************************************************
 End of function _sd_set_arg
 *****************************************************************************/

/******************************************************************************
 * Function Name: _sd_card_send_ocr
 * Description  : get OCR register and check card operation voltage
 *              : if type is SD_MEDIA_SD, issue ACMD41
 *              : if type is SD_MEDIA_MMC, issue CMD1
 * Arguments    : st_sdhndl_t *p_hndl : SD handle
 *              : int32_t type        : card type
 * Return Value : SD_OK : end of succeed
 *              : SD_ERR: end of error
 *****************************************************************************/
static int32_t _sd_card_send_ocr(st_sdhndl_t *p_hndl, int32_t type)
{
	int32_t ret;
	int32_t i;
	int32_t j = 0;

	/* ===== distinguish card type issuing CMD5, ACMD41 or CMD1 ==== */
	for (i = 0; i < 200; i++) {
		switch (type) {
		case SD_MEDIA_UNKNOWN:  /* unknown media (read OCR) */

			/* ---- issue CMD5 ---- */
			_sd_set_arg(p_hndl, 0, 0);
			ret = _sd_send_cmd(p_hndl, CMD5);
			if (SD_OK == ret) {
				return _sd_get_resp(p_hndl, SD_RSP_R4);
			} else {
				return ret;
			}
			break;

		case SD_MEDIA_SD:
		case SD_MEDIA_COMBO:
			if (SD_MODE_VER2X == p_hndl->sup_ver) {
				if (p_hndl->sd_spec & SD_SPEC_20) {
					/* cmd8 have response   *//* set HCS bit */
					p_hndl->voltage |= 0x40000000;

				}
			}

			/* ---- issue ACMD41 ---- *//* Cast to an appropriate type */
			ret = _sd_send_acmd(p_hndl, ACMD41, (uint16_t)(p_hndl->voltage >> 16), (uint16_t)p_hndl->voltage);
			break;

		case SD_MEDIA_MMC:  /* MMC */

			/* ---- issue CMD1 ---- *//* Cast to an appropriate type */
			_sd_set_arg(p_hndl, (uint16_t)(p_hndl->voltage >> 16), (uint16_t)p_hndl->voltage);
			ret = _sd_send_cmd(p_hndl, CMD1);
			break;

		default:
			p_hndl->resp_status = 0;

			/* for internal error detail    */
			/* but not need to change       */
			p_hndl->error = SD_ERR_INTERNAL;
			return SD_ERR;
		}

		if (SD_OK == ret) {
			_sd_get_resp(p_hndl, SD_RSP_R3); /* check R3 resp */

			/* ---- polling busy bit ---- */
			if (p_hndl->ocr[0] & 0x8000) {	/* busy cleared */
				break;
			} else {
				ret = SD_ERR;   /* busy */
				esddev_int_wait(p_hndl->sd_port, 5);   /* add wait function because retry interval is too short */
			}
		}

		/* if more than 3 times response timeout occured, retry stop quick distinction to MMC */
		if (SD_ERR_RES_TOE == p_hndl->error) {
			++j;
			if (3 == j) {
				break;
			}
		} else {
			j = 0;
		}
	}
	return ret;
}
/******************************************************************************
 End of function _sd_card_send_ocr
 *****************************************************************************/

/******************************************************************************
 * Function Name: _sd_check_resp_error
 * Description  : distinguish error bit from R1 response
 *              : set the error bit to p_hndl->error
 * Arguments    : st_sdhndl_t *p_hndl : SD handle
 * Return Value : SD_OK : no error detected
 *              : SD_ERR: any errors detected
 *****************************************************************************/
static int32_t _sd_check_resp_error(st_sdhndl_t *p_hndl)
{
	uint16_t status;
	int32_t  bit;

	/* SD or MMC card */
	status = (uint16_t)((p_hndl->resp_status >> 16) & 0xfdffu);

	/* ---- search R1 error bit ---- */
	bit = _sd_bit_search(status);

	if ((-1) != bit) {
		/* R1 resp errors bits but for AKE_SEQ_ERROR */
		_sd_set_err(p_hndl, s_resp_err_tbl[bit]);
		return SD_ERR;
	} else if (p_hndl->resp_status & RES_AKE_SEQ_ERROR) {
		/* authentication process sequence error */
		_sd_set_err(p_hndl, SD_ERR_AKE_SEQ);
		return SD_ERR;
	} else {
		/* DO NOTHING */
		;
	}

	return SD_OK;
}
/******************************************************************************
 End of function _sd_check_resp_error
 *****************************************************************************/

/******************************************************************************
 * Function Name: _sd_get_resp
 * Description  : get response and check response errors.
 *              : get response value from RESP register
 *              : R1, R2, R3,(R4, R5) and R6 types are available
 *              : specify response type by the argument resp
 *              : set response value to SD handle member
 * Arguments    : st_sdhndl_t *p_hndl : SD handle
 *              : int32_t resp        : response type
 * Return Value : SD_OK : end of succeed
 *              : SD_ERR: end of error
 *****************************************************************************/
static int32_t _sd_get_resp(st_sdhndl_t *p_hndl, int32_t resp)
{
	uint32_t status;
	uint16_t *p_ptr;

	/* select RESP register depend on the response type */
	switch (resp) {
	case SD_RSP_NON:   /* no response */

		/* DO NOTHING */
		;
		break;
	case SD_RSP_R1:    /* nomal response (32bits length) */
	case SD_RSP_R1B:   /* nomal response with an optional busy signal */

		/* Cast to an appropriate type */
		status = SDMMC.SD_RSP1.WORD.LL;
		status <<= 16;

		/* Cast to an appropriate type */
		status |= SDMMC.SD_RSP10.WORD.LL;
		p_hndl->resp_status = status;

		if (status & 0xfdffe008) {		/* ignore card locked status    */
			/* any status error */
			return _sd_check_resp_error(p_hndl);
		}

		break;

	case SD_RSP_R1_SCR:    /* nomal response with an optional busy signal */

		/* Cast to an appropriate type */
		p_hndl->scr[0] = SDMMC.SD_RSP1.WORD.LL;

		/* Cast to an appropriate type */
		p_hndl->scr[1] = SDMMC.SD_RSP10.WORD.LL;
		break;

	case SD_RSP_R2_CID:    /* CID register (128bits length) */
		p_ptr = p_hndl->cid;

		/* Cast to an appropriate type */
		*p_ptr++ = SDMMC.SD_RSP7.WORD.LL;

		/* Cast to an appropriate type */
		*p_ptr++ = SDMMC.SD_RSP76.WORD.LL;

		/* Cast to an appropriate type */
		*p_ptr++ = SDMMC.SD_RSP5.WORD.LL;

		/* Cast to an appropriate type */
		*p_ptr++ = SDMMC.SD_RSP54.WORD.LL;

		/* Cast to an appropriate type */
		*p_ptr++ = SDMMC.SD_RSP3.WORD.LL;

		/* Cast to an appropriate type */
		*p_ptr++ = SDMMC.SD_RSP32.WORD.LL;

		/* Cast to an appropriate type */
		*p_ptr++ = SDMMC.SD_RSP1.WORD.LL;

		/* Cast to an appropriate type */
		*p_ptr++ = SDMMC.SD_RSP10.WORD.LL;
		break;

	case SD_RSP_R2_CSD:    /* CSD register (128bits length) */
		p_ptr = p_hndl->csd;

		/* Cast to an appropriate type */
		*p_ptr++ = SDMMC.SD_RSP7.WORD.LL;

		/* Cast to an appropriate type */
		*p_ptr++ = SDMMC.SD_RSP76.WORD.LL;

		/* Cast to an appropriate type */
		*p_ptr++ = SDMMC.SD_RSP5.WORD.LL;

		/* Cast to an appropriate type */
		*p_ptr++ = SDMMC.SD_RSP54.WORD.LL;

		/* Cast to an appropriate type */
		*p_ptr++ = SDMMC.SD_RSP3.WORD.LL;

		/* Cast to an appropriate type */
		*p_ptr++ = SDMMC.SD_RSP32.WORD.LL;

		/* Cast to an appropriate type */
		*p_ptr++ = SDMMC.SD_RSP1.WORD.LL;

		/* Cast to an appropriate type */
		*p_ptr++ = SDMMC.SD_RSP10.WORD.LL;
		break;

	case SD_RSP_R3:    /* OCR register (32bits length) */

		/* Cast to an appropriate type */
		p_hndl->ocr[0] = SDMMC.SD_RSP1.WORD.LL;

		/* Cast to an appropriate type */
		p_hndl->ocr[1] = SDMMC.SD_RSP10.WORD.LL;
		break;

	case SD_RSP_R4:    /* IO OCR register (24bits length) */

		/* Cast to an appropriate type */
		p_hndl->io_ocr[0] = SDMMC.SD_RSP1.WORD.LL;

		/* Cast to an appropriate type */
		p_hndl->io_ocr[1] = SDMMC.SD_RSP10.WORD.LL;
		break;

	case SD_RSP_R6:        /* Published RCA response (32bits length) */

		/* Cast to an appropriate type */
		p_hndl->rca[0] = SDMMC.SD_RSP1.WORD.LL;

		/* Cast to an appropriate type */
		p_hndl->rca[1] = SDMMC.SD_RSP10.WORD.LL;
		break;

	case SD_RSP_R5:        /* IO RW response */

		/* Cast to an appropriate type */
		status = SDMMC.SD_RSP1.WORD.LL;
		status <<= 16;

		/* Cast to an appropriate type */
		status |= SDMMC.SD_RSP10.WORD.LL;
		p_hndl->resp_status = status;

		if (status & 0xcb00) {
			/* any status error */
			return _sd_check_resp_error(p_hndl);
		}
		break;

	case SD_RSP_R7:       /* IF_COND response */

		/* Cast to an appropriate type */
		p_hndl->if_cond[0] = SDMMC.SD_RSP1.WORD.LL;

		/* Cast to an appropriate type */
		p_hndl->if_cond[1] = SDMMC.SD_RSP10.WORD.LL;
		break;

	default:

		/* unknown type */
		p_hndl->resp_status = 0;
		p_hndl->error = SD_ERR_INTERNAL;
		return SD_ERR;
	}

	return SD_OK;
}
/******************************************************************************
 End of function _sd_get_resp
 *****************************************************************************/

/******************************************************************************
 * Function Name: _sd_check_csd
 * Description  : check CSD register and get following information
 *              : Transfer Speed
 *              : Command Class
 *              : Read Block Length
 *              : Copy Bit
 *              : Write Protect Bit
 *              : File Format Group
 *              : Number of Erase Sector
 * Arguments    : st_sdhndl_t *p_hndl : SD handle
 * Return Value : SD_OK : end of succeed
 *              : SD_ERR: end of error
 *****************************************************************************/
static int32_t _sd_check_csd(st_sdhndl_t *p_hndl)
{
	uint32_t transpeed;
	uint32_t timevalue;
	uint32_t erase_sector_size;
	uint32_t erase_group_size;

	/* ---- CSD Structure ---- */
	if (SD_MEDIA_MMC == p_hndl->media_type) {
		p_hndl->csd_structure = 0;
	} else {
		/* Cast to an appropriate type */
		p_hndl->csd_structure = (uint8_t)((p_hndl->csd[0] & 0x00c0u) >> 6u);
		if (1 == p_hndl->csd_structure) {
			if ((SD_SPEC_10 == p_hndl->sd_spec) || (SD_SPEC_11 == p_hndl->sd_spec)) {
				/* if csd_structure is ver1.00 or 1.10, sd_spec has to be phys spec ver1.00 or 1.10 */
				_sd_set_err(p_hndl, SD_ERR_CSD_VER);
				return SD_ERR;
			}
		}
	}

	/* ---- TAAC/NSAC ---- */
	/* no check, to be obsolete */

	/* ---- TRAN_SPEED  ---- */
	transpeed = (p_hndl->csd[2] & 0x0700u) >> 8u;
	timevalue = (p_hndl->csd[2] & 0x7800u) >> 11u;

	/* Cast to an appropriate type */
	transpeed = (uint32_t)(s_tran_speed[transpeed] * s_time_value[timevalue]);

	/* ---- set transfer speed (memory access) ---- */
	if (transpeed >= 5000) {
		p_hndl->csd_tran_speed = SD_CLK_50MHZ;
	} else if (transpeed >= 2500) {
		p_hndl->csd_tran_speed = SD_CLK_25MHZ;
	} else if (transpeed >= 2000) {
		p_hndl->csd_tran_speed = SD_CLK_20MHZ;
	} else if (transpeed >= 1000) {
		p_hndl->csd_tran_speed = SD_CLK_10MHZ;
	} else if (transpeed >= 500) {
		p_hndl->csd_tran_speed = SD_CLK_5MHZ;
	} else if (transpeed >= 100) {
		p_hndl->csd_tran_speed = SD_CLK_1MHZ;
	} else {
		p_hndl->csd_tran_speed = SD_CLK_400KHZ;
	}

	/* ---- CCC  ---- */
	p_hndl->csd_ccc = (uint16_t)(((p_hndl->csd[2] & 0x00ffu) << 4u) |
									((p_hndl->csd[3] & 0xf000u) >> 12u));


	/* ---- COPY ---- */
	p_hndl->csd_copy = (uint8_t)(p_hndl->csd[7] & 0x0040u);

	/* ---- PERM/TMP_WRITE_PROTECT ---- */
	p_hndl->write_protect |= (uint8_t)((p_hndl->csd[7] & 0x0030u) >> 3u);

	/* ---- FILE_FORMAT ---- */
	p_hndl->csd_file_format = (uint8_t)(p_hndl->csd[7] & 0x008cu);
	if (p_hndl->csd_file_format & 0x80u) {
		_sd_set_err(p_hndl, SD_ERR_FILE_FORMAT);
		return SD_ERR;
	}

	/* ---- calculate the number of erase sectors ---- */
	if (p_hndl->media_type & SD_MEDIA_SD) {
		erase_sector_size = ((p_hndl->csd[5] & 0x003fu) << 1u) |
							((p_hndl->csd[6] & 0x8000) >> 15);
		erase_group_size = (p_hndl->csd[6] & 0x7f00u) >> 8u;
	} else {
		erase_sector_size = (p_hndl->csd[5] & 0x007cu) >> 2u;
		erase_group_size = ((p_hndl->csd[5] & 0x0003u) << 3u) |
							((p_hndl->csd[6] & 0xe000u) >> 13u);
	}
	p_hndl->erase_sect = (erase_sector_size + 1) * (erase_group_size + 1);
	return SD_OK;
}
/******************************************************************************
 End of function _sd_check_csd
 *****************************************************************************/

/******************************************************************************
 * Function Name: _sd_check_info2_err
 * Description  : check SD_INFO2 register errors.
 *              : check error bit of SD_INFO2 register
 *              : set the error bit to p_hndl->error
 * Arguments    : st_sdhndl_t *p_hndl : SD handle
 * Return Value : SD_OK : end of succeed
 *****************************************************************************/
static int32_t _sd_check_info2_err(st_sdhndl_t *p_hndl)
{
	uint16_t info2;
	int32_t  bit;

	/* Cast to an appropriate type */
	info2 = (uint16_t)(p_hndl->int_info2 & SD_INFO2_MASK_ERR);

	/* ---- search error bit ---- */
	bit = _sd_bit_search(info2);

	if ((-1) != bit) {
		_sd_set_err(p_hndl, s_info2_err_tbl[bit]);
	}

	return SD_OK;
}
/******************************************************************************
 End of function _sd_check_info2_err
 *****************************************************************************/

/******************************************************************************
 * Function Name: _sd_bit_search
 * Description  : get bit information.
 *              : check every bits of argument (data) from LSB
 *              : return first bit whose value is 1'b
 *              : bit number is big endian (MSB is 0)
 * Arguments    : uint16_t data : checked data
 * Return Value : not less than 0 : bit number has 1'b
 *              : -1 : no bit has 1'b
 * Remark       : just 16bits value can be applied
 *****************************************************************************/
static int32_t _sd_bit_search(uint16_t data)
{
	int32_t i;

	for (i = 15; i >= 0 ; i--) {
		if (data & 1u) {
			return i;
		}
		data >>= 1;
	}

	return -1;
}
/******************************************************************************
 End of function _sd_bit_search
 *****************************************************************************/

/******************************************************************************
 * Function Name: _sd_get_info2
 * Description  : get SD_INFO2 register
 *              : set the register value to p_hndl->int_info2
 * Arguments    : st_sdhndl_t *p_hndl : SD handle
 * Return Value : none
 *****************************************************************************/
static void _sd_get_info2(st_sdhndl_t *p_hndl)
{
	uint64_t info2_reg;

	/* Cast to an appropriate type */
	info2_reg = (uint64_t)(SDMMC.SD_INFO2.LONGLONG & SD_INFO2_MASK_ERR);

	/* Cast to an appropriate type */
	SDMMC.SD_INFO2.LONGLONG = (uint64_t)~info2_reg;

	/* Cast to an appropriate type */
	p_hndl->int_info2 = (uint64_t)(p_hndl->int_info2 | info2_reg);
}
/******************************************************************************
 End of function _sd_get_info2
 *****************************************************************************/

/******************************************************************************
 * Function Name: _sd_set_int_mask
 * Description  : set SD_INFO1 and SD_INFO2 interrupt mask.
 *              : set int_info1_mask and int_info2_mask depend on the mask bits
 *              : value
 *              : if mask bit is one, it is enabled
 *              : if mask bit is zero, it is disabled
 * Arguments    : st_sdhndl_t *p_hndl : SD handle
 *              : uint64_t mask1      : SD_INFO1_MASK1 bits value
 *              : uint64_t mask2      : SD_INFO1_MASK2 bits value
 * Return Value : SD_OK : end of succeed
 *****************************************************************************/
static int32_t _sd_set_int_mask(st_sdhndl_t *p_hndl, uint64_t mask1, uint64_t mask2)
{
	esddev_loc_cpu(p_hndl->sd_port);

	/* ---- set int_info1_mask and int_info2_mask ---- */
	p_hndl->int_info1_mask |= mask1;
	p_hndl->int_info2_mask |= mask2;

	/* ---- set hardware mask ---- */
	SDMMC.SD_INFO1_MASK.LONGLONG = (uint64_t)~(p_hndl->int_info1_mask);

	/* Cast to an appropriate type */
	SDMMC.SD_INFO2_MASK.LONGLONG = (uint64_t)~(p_hndl->int_info2_mask);

	return SD_OK;
}
/******************************************************************************
 End of function _sd_set_int_mask
 *****************************************************************************/

/******************************************************************************
 * Function Name: _sd_clear_int_mask
 * Description  : clear SD_INFO1 and SD_INFO2 interrupt mask.
 *              : clear int_cc_status_mask depend on the mask bits value
 *              : if mask bit is one, it is disabled
 *              : if mask bit is zero, it is enabled
 * Arguments    : st_sdhndl_t *p_hndl : SD handle
 *              : uint64_t mask1      : SD_INFO1_MASK1 bits value
 *              : uint64_t mask2      : SD_INFO1_MASK2 bits value
 * Return Value : SD_OK : end of succeed
 *****************************************************************************/
static int32_t _sd_clear_int_mask(st_sdhndl_t *p_hndl, uint64_t mask1, uint64_t mask2)
{
	esddev_loc_cpu(p_hndl->sd_port);

	/* ---- clear int_info1_mask and int_info2_mask ---- */
	p_hndl->int_info1_mask &= (uint64_t)~mask1;

	/* Cast to an appropriate type */
	p_hndl->int_info2_mask &= (uint64_t)~mask2;

	/* ---- clear hardware mask ---- */
	SDMMC.SD_INFO1_MASK.LONGLONG = (uint64_t)~(p_hndl->int_info1_mask);

	/* Cast to an appropriate type */
	SDMMC.SD_INFO2_MASK.LONGLONG = (uint64_t)~(p_hndl->int_info2_mask);

	return SD_OK;
}
/******************************************************************************
 End of function _sd_clear_int_mask
 *****************************************************************************/

/******************************************************************************
 * Function Name: _sd_clear_info
 * Description  : clear int_info bits.
 *              : clear int_info1 and int_info2 depend on the clear value
 * Arguments    : st_sdhndl_t *p_hndl  : SD handle
 *              : uint64_t clear_info1 : int_info1 clear bits value
 *              : uint64_t clear_info2 : int_info2 clear bits value
 * Return Value : SD_OK : end of succeed
 *              : SD_ERR: end of error
 * Remark       : SD_INFO1 and SD_INFO2 bits are not cleared
 *****************************************************************************/
static int32_t _sd_clear_info(st_sdhndl_t *p_hndl, uint64_t clear_info1, uint64_t clear_info2)
{
	esddev_loc_cpu(p_hndl->sd_port);

	/* ---- clear int_info1 and int_info2 ---- */
	p_hndl->int_info1 &= (uint64_t)~clear_info1;

	/* Cast to an appropriate type */
	p_hndl->int_info2 &= (uint64_t)~clear_info2;

	return SD_OK;
}
/******************************************************************************
 End of function _sd_clear_info
 *****************************************************************************/

/******************************************************************************
 * Function Name: _sd_get_size
 * Description  : get card size.
 *              : get memory card size
 * Arguments    : st_sdhndl_t *p_hndl : SD handle
 *              : int32_t area   : memory area (bit0:user area, bit1:protect area)
 * Return Value : SD_OK : end of succeed
 *              : SD_ERR: end of error
 * Remark       : protect area is just the number of all sectors
 *****************************************************************************/
static int32_t _sd_get_size(st_sdhndl_t *p_hndl, uint32_t area)
{
	uint32_t c_mult;
	uint32_t c_size;
	uint32_t read_bl_len;

	/* ---- READ BL LEN ---- */
	read_bl_len = (p_hndl->csd[3] & 0x0f00u) >> 8;

	/* ---- C_SIZE_MULT ---- */
	c_mult = ((p_hndl->csd[5] & 0x0380u) >> 7);

	if (area & SD_PROT_AREA) {
		/* calculate the number of all sectors */
		if ((SD_MODE_VER2X == p_hndl->sup_ver) && (0x01 == p_hndl->csd_structure)) {
			/* Cast to an appropriate type */
			p_hndl->prot_sector_size = (((uint32_t)p_hndl->sdstatus[2] << 16u) |

										/* Cast to an appropriate type */
										((uint32_t)p_hndl->sdstatus[3])) / 512;
		} else {
			/* Cast to an appropriate type */
			p_hndl->prot_sector_size = (p_hndl->sdstatus[3] *

										/* Cast to an appropriate type */
										((uint32_t)1 << (c_mult + 2)) *

										/* Cast to an appropriate type */
										((uint32_t)1 << read_bl_len)) / 512;
		}
	}

	if (area & SD_USER_AREA) {
		if ((SD_MODE_VER2X == p_hndl->sup_ver) && (0x01 == p_hndl->csd_structure)) {
			/* Cast to an appropriate type */
			c_size = ((((uint32_t)p_hndl->csd[4] & 0x3fffu) << 8u) |

						/* Cast to an appropriate type */
						(((uint32_t)p_hndl->csd[5] & 0xff00u) >> 8u));

			/* memory capacity = C_SIZE*512K byte */
			/* sector_size = memory capacity/512 */
			p_hndl->card_sector_size = ((c_size + 1) << 10u);
		} else {
			/* ---- C_SIZE ---- */
			c_size = ((p_hndl->csd[3] & 0x0003u) << 10) |
						((p_hndl->csd[4] & 0xffc0u) >> 6);

			/* calculate the number of all sectors */
			p_hndl->card_sector_size = ((uint32_t)(c_size + 1) *

										/* Cast to an appropriate type */
										((uint32_t)1 << (c_mult + 2)) * ((uint32_t)1
												<< read_bl_len)) / 512;
		}
	}

	return SD_OK;
}
/******************************************************************************
 End of function _sd_get_size
 *****************************************************************************/

/******************************************************************************
 * Function Name: _sd_iswp
 * Description  : check hardware write protect refer to SDHI register
 *              : if WP pin is disconnected to SDHI, return value has no
 *              : meaning
 * Arguments    : st_sdhndl_t *p_hndl : SD handle
 * Return Value : SD_WP_OFF (0): not write protected
 *              : SD_WP_HW  (1): write protected
 * Remark       : don't check CSD write protect bits and ROM card
 *****************************************************************************/
static int32_t _sd_iswp(st_sdhndl_t *p_hndl)
{
	int32_t wp;
	int32_t layout;

	/* Cast to an appropriate type */
	layout = esddev_wp_layout((int32_t)(p_hndl->sd_port));

	if (SD_OK == layout) {
		/* ===== check SD_INFO1 WP bit ==== */
		wp = (int32_t)(((~SDMMC.SD_INFO1.WORD.LL) & SD_INFO1_MASK_WP) >> 7);
	} else {
		/* Cast to an appropriate type */
		wp = (int32_t)SD_WP_OFF;
	}
	return wp;
}
/******************************************************************************
 End of function _sd_iswp
 *****************************************************************************/

/******************************************************************************
 * Function Name: _sd_init_hndl
 * Description  : initialize SD handle.
 *              : initialize following SD handle members
 *              : media_type       : card type
 *              : write_protect    : write protect
 *              : resp_status      : R1/R1b response status
 *              : error            : error detail information
 *              : stop             : compulsory stop flag
 *              : prot_sector_size : sector size (protect area)
 *              : card registers   : ocr, cid, csd, dsr, rca, scr, sdstatus and
 *              : status_data
 * Arguments    : st_sdhndl_t *p_hndl : SD handle
 *              : uint32_t mode       : driver mode
 *              : uint32_t voltage    : working voltage
 * Return Value : SD_OK : end of succeed
 *              : SD_ERR: end of error
 *****************************************************************************/
static int32_t _sd_init_hndl(st_sdhndl_t *p_hndl, uint32_t mode, uint32_t voltage)
{
	int32_t i;
	int32_t j;

	p_hndl->media_type = SD_MEDIA_UNKNOWN;
	p_hndl->write_protect = 0;
	p_hndl->resp_status = STATE_IDEL;
	p_hndl->error = SD_OK;
	p_hndl->stop = 0;
	p_hndl->prot_sector_size = 0;
	p_hndl->voltage = voltage;
	p_hndl->speed_mode = 0;

	/* Cast to an appropriate type */
	p_hndl->int_mode = (uint8_t)(mode & 0x1u);

	/* Cast to an appropriate type */
	p_hndl->trans_mode = (uint8_t)(mode & SD_MODE_DMA);

	/* Cast to an appropriate type */
	p_hndl->sup_card = (uint8_t)(mode & 0x30u);

	/* Cast to an appropriate type */
	p_hndl->sup_speed = (uint16_t)(mode & 0xF040u);

	/* Cast to an appropriate type */
	p_hndl->sup_ver = (uint8_t)(mode & 0x80u);
	if (mode & SD_MODE_1BIT) {
		p_hndl->sup_if_mode = SD_PORT_SERIAL;
	} else {
		p_hndl->sup_if_mode = SD_PORT_PARALLEL;
	}

	/* initialize card registers */
	for (i = 0; i < (4 / sizeof(uint16_t)); ++i) {
		p_hndl->ocr[i] = 0;
	}
	for (i = 0; i < (16 / sizeof(uint16_t)); ++i) {
		p_hndl->cid[i] = 0;
	}
	for (i = 0; i < (16 / sizeof(uint16_t)); ++i) {
		p_hndl->csd[i] = 0;
	}
	for (i = 0; i < (2 / sizeof(uint16_t)); ++i) {
		p_hndl->dsr[i] = 0;
	}
	for (i = 0; i < (4 / sizeof(uint16_t)); ++i) {
		p_hndl->rca[i] = 0;
	}
	for (i = 0; i < (8 / sizeof(uint16_t)); ++i) {
		p_hndl->scr[i] = 0;
	}
	for (i = 0; i < (14 / sizeof(uint16_t)); ++i) {
		p_hndl->sdstatus[i] = 0;
	}
	for (i = 0; i < (18 / sizeof(uint16_t)); ++i) {
		p_hndl->status_data[i] = 0;
	}
	for (i = 0; i < (4 / sizeof(uint16_t)); ++i) {
		p_hndl->if_cond[i] = 0;
	}

	if (p_hndl->sup_card & SD_MODE_IO) {
		p_hndl->io_flag = 0;
		p_hndl->io_info = 0;

		for (i = 0; i < (4 / sizeof(uint16_t)); ++i) {
			p_hndl->io_ocr[i] = 0;
		}

		for (i = 0; i < 8; ++i) {
			for (j = 0; j < (SDIO_INTERNAL_REG_SIZE / sizeof(uint8_t)); ++j) {
				p_hndl->io_reg[i][j] = 0;
			}

			p_hndl->io_len[i]   = 0;
			p_hndl->io_abort[i] = 0;
		}
	}

	if (SD_MODE_VER2X == p_hndl->sup_ver) {
		p_hndl->if_cond[0] = 0;
		p_hndl->if_cond[1] = 0x00aa;
		if (p_hndl->voltage & 0x00FF8000) {
			p_hndl->if_cond[1] |= 0x0100; /* high volatege : 2.7V-3.6V */
		}
		if (p_hndl->voltage & 0x00000F00) {
			p_hndl->if_cond[1] |= 0x0200; /* low volatege : 1.65V-1.95V */
		}
	}

	return SD_OK;
}
/******************************************************************************
 End of function _sd_init_hndl
 *****************************************************************************/

/******************************************************************************
* Function Name: sddev_wp_layout
* Description  : WP Terminal Support Confirmation
* Arguments    : int32_t sd_port : channel no (0 or 1)
* Return Value : Not Support : SD_ERR
******************************************************************************/
static int32_t esddev_wp_layout(int32_t sd_port)
{
	return SD_ERR;
}
/*******************************************************************************
 End of function sddev_wp_layout
 ******************************************************************************/

/******************************************************************************
* Function Name: sddev_int_wait
* Description  : Waitting for SDHI Interrupt
* Arguments    : int32_t sd_port : channel no (0 or 1)
*              : int32_t time    : time out value to wait interrupt
* Return Value : get interrupt : SD_OK
*              : time out      : SD_ERR
******************************************************************************/
static int32_t esddev_int_wait(int32_t sd_port, int32_t time)
{
	int32_t  ret;
	uint32_t waittime;

	waittime = (uint32_t)time;

	/* interrupt generated? */
	ret = esd_check_int(sd_port);
	while ((SD_ERR == ret) && (waittime > 0uL)) {
		mdelay(1);
		waittime--;

		/* interrupt generated? */
		ret = esd_check_int(sd_port);
	}

	return ret;
}
/*******************************************************************************
 End of function sddev_int_wait
 ******************************************************************************/

/******************************************************************************
 * Function Name: sd_check_int
 * Description  : check SD_INFO1 and SD_INFO2 interrupt elements
 *              : if any interrupt is detected, return SD_OK
 *              : if no interrupt is detected, return SD_ERR
 * Arguments    : int32_t sd_port : channel no (0 or 1)
 * Return Value : SD_OK : end of succeed
 *              : SD_ERR: end of error
 *****************************************************************************/
static int32_t esd_check_int(int32_t sd_port)
{
	st_sdhndl_t *p_hndl;

	if ((0 != sd_port) && (1 != sd_port)) {
		return SD_ERR;
	}

	p_hndl = SD_GET_HNDLS(sd_port);
	if (0 == p_hndl) {
		return SD_ERR;  /* not initilized */
	}

	if (p_hndl->int_mode) {
		/* ---- hardware interrupt mode ---- */
		if (p_hndl->int_info1 || p_hndl->int_info2) {
			return SD_OK;
		} else {
			return SD_ERR;
		}
	}

	/* ---- polling mode ---- */
	return _sd_get_int(p_hndl);
}
/******************************************************************************
 End of function sd_check_int
 *****************************************************************************/

/******************************************************************************
 * Function Name: _sd_get_int
 * Description  : get SD_INFO1 and SD_INFO2 interrupt elements.
 *              : get SD_INFO1 and SD_INFO2 bits
 *              : examine enabled elements
 *              : hearafter, clear SD_INFO1 and SD_INFO2 bits
 *              : save those bits to int_info1 or int_info2
 * Arguments    : st_sdhndl_t *p_hndl : SD handle
 * Return Value : SD_OK : end of succeed
 *              : SD_ERR: end of error
 *****************************************************************************/
static int32_t _sd_get_int(st_sdhndl_t *p_hndl)
{
	uint64_t info1;
	uint64_t info2;

	/* get SD_INFO1 and SD_INFO2 bits */
	info1 = (uint64_t)(SDMMC.SD_INFO1.LONGLONG & p_hndl->int_info1_mask);

	/* Cast to an appropriate type */
	info2 = (uint64_t)(SDMMC.SD_INFO2.LONGLONG & p_hndl->int_info2_mask);

	/* clear SD_INFO1 and SD_INFO2 bits */
	SDMMC.SD_INFO1.LONGLONG = (uint64_t)~info1;

	/* Cast to an appropriate type */
	SDMMC.SD_INFO2.LONGLONG = (uint64_t)~info2;

	/* save enabled elements */
	p_hndl->int_info1 |= info1;
	p_hndl->int_info2 |= info2;
	if (info1 || info2) {
		return SD_OK;   /* any interrupt occured */
	}

	return SD_ERR;  /* no interrupt occured */
}
/******************************************************************************
 End of function _sd_get_int
 *****************************************************************************/

/******************************************************************************
 * Function Name: sd_read_sect
 * Description  : read sector data from card.
 *              : read sector data from physical sector number (=psn) by the
 *              : number of sectors (=cnt)
 *              : if SD Driver mode is SD_MODE_SW, data transfer by
 *              : sddev_read_data function
 *              : if SD Driver mode is SD_MODE_DMA, data transfer by DMAC
 * Arguments    : int32_t sd_port : channel no (0 or 1)
 *              : uint8_t *buff   : read data buffer
 *              : uint32_t psn    : read physical sector number
 *              : int32_t cnt     : number of read sectors
 * Return Value : SD_OK : end of succeed
 *              : SD_ERR: end of error
 *****************************************************************************/
int32_t esd_read_sect(int32_t sd_port, uint8_t *buff, uint32_t psn, int32_t cnt)
{
	st_sdhndl_t *p_hndl;
	int32_t     i;
	int32_t     j;
	int32_t     ret;
	int32_t     mode = SD_MODE_SW;
	int32_t     mmc_lastsect = 0;
	uint64_t    info1_back;
	uint64_t    opt_back;

	if ((0 != sd_port) && (1 != sd_port)) {
		return SD_ERR;
	}

	p_hndl = SD_GET_HNDLS(sd_port);
	if (0 == p_hndl) {
		return SD_ERR;  /* not initilized */
	}

	/* Cast to an appropriate type */
	if (NULL == buff) {
		return SD_ERR;
	}

	p_hndl->error = SD_OK;

	/* ---- check card is mounted ---- */
	if (SD_MOUNT_UNLOCKED_CARD != p_hndl->mount) {
		_sd_set_err(p_hndl, SD_ERR);
		return p_hndl->error; /* not mounted yet */
	}

	/* ---- is stop compulsory? ---- */
	if (p_hndl->stop) {
		p_hndl->stop = 0;
		_sd_set_err(p_hndl, SD_ERR_STOP);
		return SD_ERR_STOP;
	}

	/* access area check */
	if ((psn >= p_hndl->card_sector_size) || ((psn + cnt) > p_hndl->card_sector_size)) {
		_sd_set_err(p_hndl, SD_ERR);
		return p_hndl->error; /* out of area */
	}

	/* if DMA transfer, buffer boundary is octlet unit */
	if ((p_hndl->trans_mode & SD_MODE_DMA) && (((uintptr_t)buff & 0x07u) == 0)) {
		mode = SD_MODE_DMA; /* set DMA mode */
	}

	/* transfer size is fixed (512 bytes) */
	SDMMC.SD_SIZE.LONGLONG = (uint64_t)512;

	/* ---- supply clock (data-transfer ratio) ---- */
	if (_sd_set_clock(p_hndl, (int32_t)p_hndl->csd_tran_speed, SD_CLOCK_ENABLE) != SD_OK) {
		return p_hndl->error;
	}

	/* ==== check status precede read operation ==== */
	if (_sd_card_send_cmd_arg(p_hndl, CMD13, SD_RSP_R1, p_hndl->rca[0], 0x0000)
			== SD_OK) {
		if ((p_hndl->resp_status & RES_STATE) != STATE_TRAN) {	/* not transfer state */
			p_hndl->error = SD_ERR;
			return _sd_read_sect_error(p_hndl, mode);
		}
	} else {	/* SDHI error */
		return _sd_read_sect_error(p_hndl, mode);
	}

	/* ==== execute multiple transfer by 256 sectors ==== */
	for (i = cnt; i > 0;
			i -= TRANS_SECTORS, psn += TRANS_SECTORS, buff += (TRANS_SECTORS * 512)) {

		/* set transfer sector numbers to SD_SECCNT */
		cnt = i - TRANS_SECTORS;
		if (cnt < 0) {	/* remaining sectors are less than TRANS_SECTORS */
			cnt = i;
		} else {
			cnt = TRANS_SECTORS;
		}

		if (cnt <= 2) {
			/* disable SD_SECCNT */
			SDMMC.SD_STOP.LONGLONG = 0x0000;
			for (j = cnt; j > 0; j--, psn++, buff += 512) {
				ret = _sd_single_read(p_hndl, buff, psn, mode);
				if (SD_OK != ret) {
					/* Cast to an appropriate type */
					opt_back = SDMMC.SD_OPTION.LONGLONG;

					/* Cast to an appropriate type */
					SDMMC.SOFT_RST.LONGLONG = SOFT_RST_SDRST_RESET;

					/* Cast to an appropriate type */
					SDMMC.SOFT_RST.LONGLONG = SOFT_RST_SDRST_RELEASED;

					/* Cast to an appropriate type */
					SDMMC.SD_OPTION.LONGLONG = opt_back;
					break;
				}
			}

			/* ---- halt clock ---- */
			_sd_set_clock(p_hndl, 0, SD_CLOCK_DISABLE);

			return p_hndl->error;
		}

		/* enable SD_SECCNT */
		SDMMC.SD_STOP.LONGLONG = (uint64_t)0x0100;

		/* issue CMD12 not automatically, if MMC last sector access */
		mmc_lastsect = 0;
		if ((SD_MEDIA_MMC == (p_hndl->media_type)) && ((psn + cnt) == (p_hndl->card_sector_size))) {
			mmc_lastsect = 1;
		}

		/* Cast to an appropriate type */
		SDMMC.SD_SECCNT.LONGLONG = (uint64_t)cnt;

		/* ---- enable RespEnd and ILA ---- */
		_sd_set_int_mask(p_hndl, SD_INFO1_MASK_RESP, 0);

		/* issue CMD18 (READ_MULTIPLE_BLOCK) */
		if (0 != mmc_lastsect) {	/* MMC last sector access */
			if (_sd_send_mcmd(p_hndl, CMD18 | SDR104_READ_CMD, SET_ACC_ADDR) != SD_OK) {
				return _sd_read_sect_error(p_hndl, mode);
			}
		} else {
			if (_sd_send_mcmd(p_hndl, CMD18, SET_ACC_ADDR) != SD_OK) {
				return _sd_read_sect_error(p_hndl, mode);
			}
		}

		/* ---- disable RespEnd and ILA ---- */
		_sd_clear_int_mask(p_hndl, SD_INFO1_MASK_RESP, SD_INFO2_MASK_ILA);

		if (SD_MODE_SW == mode) {	/* ==== PIO ==== */
			/* enable All end, BRE and errors */
			_sd_set_int_mask(p_hndl, SD_INFO1_MASK_DATA_TRNS, SD_INFO2_MASK_BRE);

			/* software data transfer */
			ret = _sd_software_trans(p_hndl, buff, cnt, SD_TRANS_READ);
		} else {	/* ==== DMA ==== */
			/* disable card ins&rem interrupt for FIFO */
			info1_back = (uint64_t)(p_hndl->int_info1_mask & SD_INFO1_MASK_DET_CD);

			/* Cast to an appropriate type */
			_sd_clear_int_mask(p_hndl, SD_INFO1_MASK_DET_CD, 0);

			/* enable All end and errors */
			_sd_set_int_mask(p_hndl, SD_INFO1_MASK_DATA_TRNS, SD_INFO2_MASK_ERR);

			_sd_set_int_mask(p_hndl, info1_back, 0);
		}

		if (SD_OK != ret) {
			return _sd_read_sect_error(p_hndl, mode);
		}

		/* ---- wait All end interrupt ---- */
		if (esddev_int_wait(sd_port, SD_TIMEOUT_RESP) != SD_OK) {
			_sd_set_err(p_hndl, SD_ERR_HOST_TOE);
			return _sd_read_sect_error(p_hndl, mode);
		}

		/* ---- check errors ---- */
		if (p_hndl->int_info2 & SD_INFO2_MASK_ERR) {
			_sd_check_info2_err(p_hndl);
			return _sd_read_sect_error(p_hndl, mode);
		}

		/* clear All end bit */
		_sd_clear_info(p_hndl, SD_INFO1_MASK_DATA_TRNS, 0x0000);

		/* disable All end, BRE and errors */
		_sd_clear_int_mask(p_hndl, SD_INFO1_MASK_DATA_TRNS, SD_INFO2_MASK_BRE);

		if (mmc_lastsect) {
			if (_sd_card_send_cmd_arg(p_hndl, 12, SD_RSP_R1B, 0, 0) != SD_OK) {
				/* check OUT_OF_RANGE error */
				/* ignore errors during last block access */
				if (p_hndl->resp_status & 0xffffe008ul) {
					if ((psn + cnt) != p_hndl->card_sector_size) {
						return _sd_read_sect_error(p_hndl, mode);  /* but for last block */
					}
					if (p_hndl->resp_status & 0x7fffe008ul) {
						return _sd_read_sect_error(p_hndl, mode);  /* not OUT_OF_RANGE error */
					}

					/* clear OUT_OF_RANGE error */
					p_hndl->resp_status &= 0x1f00u;
					p_hndl->error = SD_OK;
				} else {	/* SDHI error, ex)timeout error so on */
					return _sd_read_sect_error(p_hndl, mode);
				}
			}
		}

		/* ==== check status after read operation ==== */
		if (_sd_card_send_cmd_arg(p_hndl, CMD13, SD_RSP_R1, p_hndl->rca[0], 0x0000)
				!= SD_OK) {
			/* check OUT_OF_RANGE error */
			/* ignore errors during last block access */
			if (p_hndl->resp_status & 0xffffe008ul) {
				if ((psn + cnt) != p_hndl->card_sector_size) {
					return _sd_read_sect_error(p_hndl, mode);  /* but for last block */
				}
				if (p_hndl->resp_status & 0x7fffe008ul) {
					return _sd_read_sect_error(p_hndl, mode);  /* not OUT_OF_RANGE error */
				}

				/* clear OUT_OF_RANGE error */
				p_hndl->resp_status &= 0x1f00u;
				p_hndl->error = SD_OK;
			} else {	/* SDHI error, ex)timeout error so on */
				return _sd_read_sect_error(p_hndl, mode);
			}
		}

		if ((p_hndl->resp_status & RES_STATE) != STATE_TRAN) {
			p_hndl->error = SD_ERR;
			return _sd_read_sect_error(p_hndl, mode);
		}

		/* ---- is stop compulsory? ---- */
		if (p_hndl->stop) {
			p_hndl->stop = 0;

			/* data transfer stop (issue CMD12) */
			SDMMC.SD_STOP.LONGLONG = (uint64_t)0x0001;
			i = 0;  /* set zero to break loop */
			_sd_set_err(p_hndl, SD_ERR_STOP);
		}
	}

	/* ---- halt clock ---- */
	_sd_set_clock(p_hndl, 0, SD_CLOCK_DISABLE);

	return p_hndl->error;
}
/******************************************************************************
 End of function sd_read_sect
 *****************************************************************************/

/******************************************************************************
 * Function Name: sd_write_sect
 * Description  : write sector data to card.
 *              : write sector data from physical sector number (=psn) by the
 *              : number of sectors (=cnt)
 *              : if SD Driver mode is SD_MODE_SW, data transfer by
 *              : sddev_read_data function
 *              : if SD Driver mode is SD_MODE_DMA, data transfer by DMAC
 * Arguments    : int32_t sd_port   : channel no (0 or 1).
 *              : uint8_t *buff     : write data buffer
 *              : uint32_t psn      : write physical sector number
 *              : int32_t cnt       : number of write sectors
 *              : int32_t writemode : memory card write mode
 *              :   SD_WRITE_WITH_PREERASE : pre-erease write
 *              :   SD_WRITE_OVERWRITE     : overwrite
 * Return Value : SD_OK : end of succeed.
 *              : SD_ERR: end of error
 *****************************************************************************/
int32_t esd_write_sect(int32_t sd_port, uint8_t *buff, uint32_t psn, int32_t cnt, int32_t writemode)
{
	st_sdhndl_t *p_hndl;

	if ((0 != sd_port) && (1 != sd_port)) {
		return SD_ERR;
	}

	p_hndl = SD_GET_HNDLS(sd_port);
	if (0 == p_hndl) {
		return SD_ERR;  /* not initilized */
	}

	/* Cast to an appropriate type */
	if (NULL == buff) {
		return SD_ERR;
	}

	p_hndl->error = SD_OK;

	/* ---- check card is mounted ---- */
	if (SD_MOUNT_UNLOCKED_CARD != p_hndl->mount) {
		_sd_set_err(p_hndl, SD_ERR);
		return p_hndl->error; /* not mounted yet */
	}

	/* ---- check write protect ---- */
	if (p_hndl->write_protect) {
		_sd_set_err(p_hndl, SD_ERR_WP);
		return p_hndl->error; /* write protect error */
	}

	/* ---- is stop compulsory? ---- */
	if (p_hndl->stop) {
		p_hndl->stop = 0;
		_sd_set_err(p_hndl, SD_ERR_STOP);
		return p_hndl->error;
	}

	/* ==== write sector data to card ==== */
	_sd_write_sect(p_hndl, buff, psn, cnt, writemode);

	return p_hndl->error;
}
/******************************************************************************
 End of function sd_write_sect
 *****************************************************************************/

/******************************************************************************
 * Function Name: _sd_send_mcmd
 * Description  : issue multiple command (CMD18 or CMD25)
 *              : wait response
 *              : set read start address to startaddr
 *              : after this function finished, start data transfer
 * Arguments    : st_sdhndl_t *p_hndl : SD handle
 *              : uint16_t cmd        : command code (CMD18 or CMD25)
 *              : uint32_t startaddr  : data address (command argument)
 * Return Value : SD_OK : end of succeed
 *              : SD_ERR: end of error
 *****************************************************************************/
static int32_t _sd_send_mcmd(st_sdhndl_t *p_hndl, uint16_t cmd, uint32_t startaddr)
{
	int32_t i;

	/* Cast to an appropriate type */
	_sd_set_arg(p_hndl, (uint16_t)(startaddr >> 16), (uint16_t)startaddr);

	for (i = 0; i < SCLKDIVEN_LOOP_COUNT; i++) {
		/* Cast to an appropriate type */
		if (SDMMC.SD_INFO2.LONGLONG & SD_INFO2_MASK_SCLKDIVEN) {
			break;
		}
	}
	if (SCLKDIVEN_LOOP_COUNT == i) {
		_sd_set_err(p_hndl, SD_ERR_CBSY_ERROR);       /* treate as CBSY ERROR */
		return p_hndl->error;
	}

	/* ---- issue command ---- */
	SDMMC.SD_CMD.LONGLONG = (uint64_t)cmd;

	/* ---- wait resp end ---- */
	if (esddev_int_wait(p_hndl->sd_port, SD_TIMEOUT_CMD) != SD_OK) {
		_sd_set_err(p_hndl, SD_ERR_HOST_TOE);
		return p_hndl->error;
	}

	_sd_get_info2(p_hndl);    /* get SD_INFO2 register */

	_sd_check_info2_err(p_hndl);  /* check SD_INFO2 error bits */

	/* Cast to an appropriate type */
	if (p_hndl->int_info1 & SD_INFO1_MASK_RESP) {
		if (!p_hndl->error) {
			_sd_get_resp(p_hndl, SD_RSP_R1); /* check R1 resp */
		}
	} else {
		_sd_set_err(p_hndl, SD_ERR_NO_RESP_ERROR);    /* no response */
	}

	/* ---- clear previous errors ---- *//* Cast to an appropriate type */
	_sd_clear_info(p_hndl, SD_INFO1_MASK_RESP, SD_INFO2_MASK_ERR);

	return p_hndl->error;
}
/******************************************************************************
 End of function _sd_send_mcmd
 *****************************************************************************/

/******************************************************************************
 * Function Name: _sd_read_sect_error
 * Description  : read sector data error.
 * Arguments    : st_sdhndl_t *p_hndl : SD handle
 *              : int32_t mode        : data transfer mode
 *              :   SD_MODE_SW  : software
 *              :   SD_MODE_DMA : DMA
 * Return Value : SD_OK : end of succeed
 *              : SD_ERR: end of error
 *****************************************************************************/
static int32_t _sd_read_sect_error(st_sdhndl_t *p_hndl, int32_t mode)
{
	uint64_t sd_option;
	uint64_t sd_clk_ctrl;

	mode = p_hndl->error;

	/* ---- clear error bits ---- */
	_sd_clear_info(p_hndl, SD_INFO1_MASK_TRNS_RESP, SD_INFO2_MASK_ALL);

	/* ---- disable all interrupts ---- */
	_sd_clear_int_mask(p_hndl, SD_INFO1_MASK_TRNS_RESP, SD_INFO2_MASK_ALL);

	/* Cast to an appropriate type */
	if ((SDMMC.SD_INFO2.LONGLONG & SD_INFO2_MASK_CBSY) == SD_INFO2_MASK_CBSY) {
		/* ---- enable All end ---- */
		_sd_set_int_mask(p_hndl, SD_INFO1_MASK_DATA_TRNS, 0);

		/* ---- data transfer stop (issue CMD12) ---- */
		SDMMC.SD_STOP.LONGLONG = (uint64_t)0x0001;

		/* ---- wait All end ---- */
		esddev_int_wait(p_hndl->sd_port, SD_TIMEOUT_RESP);

		/* Cast to an appropriate type */
		_sd_clear_info(p_hndl, SD_INFO1_MASK_TRNS_RESP, SD_INFO2_MASK_ALL);

		/* Cast to an appropriate type */
		_sd_clear_int_mask(p_hndl, SD_INFO1_MASK_DATA_TRNS, 0);

		esddev_loc_cpu(p_hndl->sd_port);

		/* Cast to an appropriate type */
		sd_option   = SDMMC.SD_OPTION.LONGLONG;

		/* Cast to an appropriate type */
		sd_clk_ctrl = SDMMC.SD_CLK_CTRL.LONGLONG;

		/* Cast to an appropriate type */
		SDMMC.SOFT_RST.LONGLONG = SOFT_RST_SDRST_RESET;

		/* Cast to an appropriate type */
		SDMMC.SOFT_RST.LONGLONG = SOFT_RST_SDRST_RELEASED;

		/* Cast to an appropriate type */
		SDMMC.SD_STOP.LONGLONG = 0x0000;

		/* Cast to an appropriate type */
		SDMMC.SD_OPTION.LONGLONG = sd_option;

		/* Cast to an appropriate type */
		SDMMC.SD_CLK_CTRL.LONGLONG = sd_clk_ctrl;
	}

	/* Cast to an appropriate type */
	SDMMC.SD_STOP.LONGLONG = (uint64_t)0x0001;

	/* Cast to an appropriate type */
	SDMMC.SD_STOP.LONGLONG = 0x0000;

	/* Check Current State */
	if (_sd_card_send_cmd_arg(p_hndl, CMD13, SD_RSP_R1, p_hndl->rca[0], 0x0000) == SD_OK) {
		/* not transfer state? */
		if ((p_hndl->resp_status & RES_STATE) != STATE_TRAN) {
			/* if not tran state, issue CMD12 to transit the SD card to tran state */
			_sd_card_send_cmd_arg(p_hndl, CMD12, SD_RSP_R1B, p_hndl->rca[0], 0x0000);

			/* not check error because already checked */
		}
	}

	p_hndl->error = mode;

	/* Cast to an appropriate type */
	_sd_clear_int_mask(p_hndl, SD_INFO1_MASK_TRNS_RESP, SD_INFO2_MASK_ALL);

	/* ---- halt clock ---- */
	_sd_set_clock(p_hndl, 0, SD_CLOCK_DISABLE);

	return p_hndl->error;
}
/******************************************************************************
 End of function _sd_read_sect_error
 *****************************************************************************/

/******************************************************************************
 * Function Name: _sd_single_read
 * Description  : read sector data from card by single block transfer.
 *              : read sector data from physical sector number (=psn) by the
 *              : single block transfer
 *              : if SD Driver mode is SD_MODE_SW, data transfer by
 *              : esddev_read_data function
 *              : if SD Driver mode is SD_MODE_DMA, data transfer by DMAC
 * Arguments    : st_sdhndl_t *p_hndl : SD handle
 *              : uint8_t *buff       : read data buffer
 *              : uint32_t psn        : read physical sector number
 *              : int32_t mode        : data transfer mode
 *              :   SD_MODE_SW  : software
 *              :   SD_MODE_DMA : DMA
 * Return Value : SD_OK : end of succeed
 *              : SD_ERR: end of error
 *****************************************************************************/
static int32_t _sd_single_read(st_sdhndl_t *p_hndl, uint8_t *buff, uint32_t psn,
								int32_t mode)
{
	uint64_t info1_back;

	/* ---- enable RespEnd and ILA ---- */
	_sd_set_int_mask(p_hndl, SD_INFO1_MASK_RESP, SD_INFO2_MASK_ILA);

	/* issue CMD17 (READ_SINGLE_BLOCK) */
	if (_sd_send_mcmd(p_hndl, CMD17, SET_ACC_ADDR) != SD_OK) {
		return _sd_single_read_error(p_hndl, mode);
	}

	/* ---- disable RespEnd and ILA ---- */
	_sd_clear_int_mask(p_hndl, SD_INFO1_MASK_RESP, SD_INFO2_MASK_ILA);

	if (SD_MODE_SW == mode) {	/* ==== PIO ==== */
		/* enable All end, BRE and errors */
		_sd_set_int_mask(p_hndl, SD_INFO1_MASK_DATA_TRNS, SD_INFO2_MASK_BRE);

		/* software data transfer */
		_sd_software_trans(p_hndl, buff, 1, SD_TRANS_READ);
	} else {	/* ==== DMA ==== */
		/* disable card ins&rem interrupt for FIFO */
		info1_back = (uint64_t)(p_hndl->int_info1_mask & SD_INFO1_MASK_DET_CD);

		/* Cast to an appropriate type */
		_sd_clear_int_mask(p_hndl, SD_INFO1_MASK_DET_CD, 0);

		/* enable All end and errors */
		_sd_set_int_mask(p_hndl, SD_INFO1_MASK_DATA_TRNS, SD_INFO2_MASK_ERR);

		_sd_set_int_mask(p_hndl, info1_back, 0);
	}

	/* ---- wait All end interrupt ---- */
	if (esddev_int_wait(p_hndl->sd_port, SD_TIMEOUT_RESP) != SD_OK) {
		_sd_set_err(p_hndl, SD_ERR_HOST_TOE);
		return _sd_single_read_error(p_hndl, mode);
	}

	/* ---- check errors ---- */
	if (p_hndl->int_info2 & SD_INFO2_MASK_ERR) {
		_sd_check_info2_err(p_hndl);
		return _sd_single_read_error(p_hndl, mode);
	}

	/* clear All end bit */
	_sd_clear_info(p_hndl, SD_INFO1_MASK_DATA_TRNS, 0x0000);

	/* disable All end, BRE and errors */
	_sd_clear_int_mask(p_hndl, SD_INFO1_MASK_DATA_TRNS, SD_INFO2_MASK_BRE);


	/* ==== check status after read operation ==== */
	if (_sd_card_send_cmd_arg(p_hndl, CMD13, SD_RSP_R1, p_hndl->rca[0], 0x0000) != SD_OK) {
		/* check OUT_OF_RANGE error */
		/* ignore errors during last block access */
		if (p_hndl->resp_status & 0xffffe008ul) {
			if ((psn + 1) != p_hndl->card_sector_size) {
				return _sd_single_read_error(p_hndl, mode);  /* but for last block */
			}
			if (p_hndl->resp_status & 0x7fffe008ul) {
				return _sd_single_read_error(p_hndl, mode);  /* not OUT_OF_RANGE error */
			}

			/* clear OUT_OF_RANGE error */
			p_hndl->resp_status &= 0x1f00u;
			p_hndl->error = SD_OK;
		} else {	/* SDHI error, ex)timeout error so on */
			return _sd_single_read_error(p_hndl, mode);
		}
	}

	return p_hndl->error;
}
/******************************************************************************
 End of function _sd_single_read
 *****************************************************************************/

/******************************************************************************
 * Function Name: _sd_single_read_error
 * Description  : read sector data error.
 * Arguments    : st_sdhndl_t *p_hndl : SD handle
 *              : int32_t mode        : data transfer mode
 *              :   SD_MODE_SW  : software
 *              :   SD_MODE_DMA : DMA
 * Return Value : SD_OK : end of succeed
 *              : SD_ERR: end of error
 *****************************************************************************/
static int32_t _sd_single_read_error(st_sdhndl_t *p_hndl, int32_t mode)
{
	int32_t error;

	error = p_hndl->error;

	/* Cast to an appropriate type */
	_sd_clear_info(p_hndl, SD_INFO1_MASK_TRNS_RESP, SD_INFO2_MASK_ALL);

	/* Cast to an appropriate type */
	_sd_clear_int_mask(p_hndl, SD_INFO1_MASK_TRNS_RESP, SD_INFO2_MASK_ALL);

	/* Cast to an appropriate type */
	_sd_set_int_mask(p_hndl, SD_INFO1_MASK_DATA_TRNS, 0);

	/* Cast to an appropriate type */
	_sd_card_send_cmd_arg(p_hndl, CMD13, SD_RSP_R1, p_hndl->rca[0], 0x0000);

	/* Cast to an appropriate type */
	_sd_clear_int_mask(p_hndl, SD_INFO1_MASK_TRNS_RESP, SD_INFO2_MASK_ALL);

	p_hndl->error = error;

	return p_hndl->error;
}
/******************************************************************************
 End of function _sd_single_read_error
 *****************************************************************************/

/******************************************************************************
 * Function Name: _sd_write_sect
 * Description  : write sector data to card.
 *              : write sector data from physical sector number (=psn) by the
 *              : number of sectors (=cnt)
 *              : if SD Driver mode is SD_MODE_SW, data transfer by
 *              : sddev_read_data function
 *              : if SD Driver mode is SD_MODE_DMA, data transfer by DMAC
 * Arguments    : st_sdhndl_t *p_hndl : SD handle
 *              : uint8_t *buff       : write data buffer
 *              : uint32_t psn        : write physical sector number
 *              : int32_t cnt         : number of write sectors
 *              : int32_t writemode   : memory card write mode
 *              :   SD_WRITE_WITH_PREERASE : pre-erease write
 *              :   SD_WRITE_OVERWRITE     : overwrite
 * Return Value : SD_OK : end of succeed
 *              : SD_ERR: end of error
 *****************************************************************************/
static int32_t _sd_write_sect(st_sdhndl_t *p_hndl, uint8_t *buff, uint32_t psn, int32_t cnt, int32_t writemode)
{
	int32_t  i;
	int32_t  j;
	int32_t  ret;
	int32_t  trans_ret;
	int32_t  mode = SD_MODE_SW;
	uint8_t  wb[4];
	uint32_t writeblock;
	uint64_t info1_back;
	uint64_t opt_back;

	/* access area check */
	if ((psn >= p_hndl->card_sector_size) || ((psn + cnt) > p_hndl->card_sector_size)) {
		_sd_set_err(p_hndl, SD_ERR);
		return p_hndl->error; /* out of area */
	}

	/* if DMA transfer, buffer boundary is octlet unit */
	if ((p_hndl->trans_mode & SD_MODE_DMA) && (((uintptr_t)buff & 0x07u) == 0)) {
		mode = SD_MODE_DMA; /* set DMA mode */
	}

	/* ---- supply clock (data-transfer ratio) ---- */
	if (_sd_set_clock(p_hndl, (int32_t)p_hndl->csd_tran_speed, SD_CLOCK_ENABLE) != SD_OK) {
		return p_hndl->error;
	}

	/* ==== check status precede write operation ==== */
	if (_sd_card_send_cmd_arg(p_hndl, CMD13, SD_RSP_R1, p_hndl->rca[0], 0x0000)
			== SD_OK) {
		if ((p_hndl->resp_status & RES_STATE) != STATE_TRAN) {	/* not transfer state */
			p_hndl->error = SD_ERR;
			return _sd_write_sect_error(p_hndl, mode);
		}
	} else {	/* SDHI error */
		return _sd_write_sect_error(p_hndl, mode);
	}

	/* ==== execute multiple transfer by 256 sectors ==== */
	for (i = cnt; i > 0;
			i -= TRANS_SECTORS, psn += TRANS_SECTORS, buff += (TRANS_SECTORS * 512)) {

		cnt = i - TRANS_SECTORS;
		if (cnt < 0) {	/* remaining sectors is less than TRANS_SECTORS */
			cnt = i;
		} else {
			cnt = TRANS_SECTORS;
		}

		/* if card is SD Memory card and pre-erease write, issue ACMD23 */
		if ((p_hndl->media_type & SD_MEDIA_SD) &&
				(SD_WRITE_WITH_PREERASE == writemode)) {
			/* Cast to an appropriate type */
			if (_sd_send_acmd(p_hndl, ACMD23, 0, (uint16_t)cnt) != SD_OK) {
				return _sd_write_sect_error(p_hndl, mode);
			}
			if (_sd_get_resp(p_hndl, SD_RSP_R1) != SD_OK) {
				return _sd_write_sect_error(p_hndl, mode);
			}
		}

		/* transfer size is fixed (512 bytes) */
		SDMMC.SD_SIZE.LONGLONG = (uint64_t)512;

		/* 1 or 2 blocks, apply single read */
		if (cnt <= 2) {
			/* disable SD_SECCNT */
			SDMMC.SD_STOP.LONGLONG = 0x0000;
			for (j = cnt; j > 0; j--, psn++, buff += 512) {
				trans_ret = _sd_single_write(p_hndl, buff, psn, mode);
				if (SD_OK != trans_ret) {
					/* Cast to an appropriate type */
					opt_back = SDMMC.SD_OPTION.LONGLONG;

					/* Cast to an appropriate type */
					SDMMC.SOFT_RST.LONGLONG = SOFT_RST_SDRST_RESET;

					/* Cast to an appropriate type */
					SDMMC.SOFT_RST.LONGLONG = SOFT_RST_SDRST_RELEASED;

					/* Cast to an appropriate type */
					SDMMC.SD_OPTION.LONGLONG = opt_back;
					break;
				}
			}

			/* ---- halt clock ---- */
			_sd_set_clock(p_hndl, 0, SD_CLOCK_DISABLE);

			return p_hndl->error;
		}

		/* enable SD_SECCNT */
		SDMMC.SD_STOP.LONGLONG = (uint64_t)0x0100;

		/* set number of transfer sectors */
		SDMMC.SD_SECCNT.LONGLONG = (uint64_t)cnt;

		/* ---- enable RespEnd and ILA ---- */
		_sd_set_int_mask(p_hndl, SD_INFO1_MASK_RESP, 0);

		/* issue CMD25 (WRITE_MULTIPLE_BLOCK) */
		ret = _sd_send_mcmd(p_hndl, CMD25, SET_ACC_ADDR);
		if (SD_OK != ret) {
			return _sd_write_sect_error(p_hndl, mode);
		}

		/* ---- disable RespEnd and ILA ---- */
		_sd_clear_int_mask(p_hndl, SD_INFO1_MASK_RESP, SD_INFO2_MASK_ILA);

		info1_back = 0;

		if (SD_MODE_SW == mode) {	/* ==== PIO ==== */
			/* enable All end, BWE and errors */
			_sd_set_int_mask(p_hndl, SD_INFO1_MASK_DATA_TRNS, SD_INFO2_MASK_BWE);

			/* software data transfer */
			trans_ret = _sd_software_trans(p_hndl, buff, cnt, SD_TRANS_WRITE);
		} else {	/* ==== DMA ==== */
			/* disable card ins&rem interrupt for FIFO */
			info1_back = (uint64_t)(p_hndl->int_info1_mask & SD_INFO1_MASK_DET_CD);

			/* Cast to an appropriate type */
			_sd_clear_int_mask(p_hndl, SD_INFO1_MASK_DET_CD, 0);

			/* enable All end and errors */
			_sd_set_int_mask(p_hndl, SD_INFO1_MASK_DATA_TRNS, SD_INFO2_MASK_ERR);
		}

		/* ---- wait All end interrupt ---- */
		ret = esddev_int_wait(p_hndl->sd_port, SD_TIMEOUT_RESP);

		if (SD_MODE_DMA == mode) {
			_sd_set_int_mask(p_hndl, info1_back, 0);
		}

		/* ---- check result of transfer ---- */
		if (SD_OK != trans_ret) {
			return _sd_write_sect_error(p_hndl, mode);
		}

		/* ---- check result of wait All end interrupt ---- */
		if (SD_OK != ret) {
			_sd_set_err(p_hndl, SD_ERR_HOST_TOE);
			return _sd_write_sect_error(p_hndl, mode);
		}

		/* ---- check errors ---- */
		if (p_hndl->int_info2 & SD_INFO2_MASK_ERR) {
			_sd_check_info2_err(p_hndl);
			return _sd_write_sect_error(p_hndl, mode);
		}

		/* clear All end bit */
		_sd_clear_info(p_hndl, SD_INFO1_MASK_DATA_TRNS, 0x0000);

		/* disable All end, BWE and errors */
		_sd_clear_int_mask(p_hndl, SD_INFO1_MASK_DATA_TRNS, SD_INFO2_MASK_BWE);

		if (p_hndl->media_type & SD_MEDIA_SD) {
			if (_sd_get_resp(p_hndl, SD_RSP_R1) != SD_OK) {
				/* check number of write complete block */
				if (_sd_read_byte(p_hndl, ACMD22, 0, 0, wb, 4) != SD_OK) {
					return _sd_write_sect_error(p_hndl, mode);
				}

				/* type cast (uint32_t) remove compiler dependence */
				writeblock = ((uint32_t)wb[0] << 24) | ((uint32_t)wb[1] << 16) |

								/* Cast to an appropriate type */
								((uint32_t)wb[2] << 8)  | (uint32_t)wb[3];

				if (cnt != writeblock) {	/* no write complete block */
					_sd_set_err(p_hndl, SD_ERR);
					return _sd_write_sect_error(p_hndl, mode);
				}
			}
		}

		/* ==== check status after write operation ==== */
		if (_sd_card_send_cmd_arg(p_hndl, CMD13, SD_RSP_R1, p_hndl->rca[0], 0x0000)
				!= SD_OK) {
			/* check OUT_OF_RANGE error */
			/* ignore errors during last block access */
			if (p_hndl->resp_status & 0xffffe008ul) {
				if ((psn + cnt) != p_hndl->card_sector_size) {
					return _sd_write_sect_error(p_hndl, mode);   /* but for last block */
				}
				if (p_hndl->resp_status & 0x7fffe008ul) {
					return _sd_write_sect_error(p_hndl, mode);   /* not OUT_OF_RANGE error */
				}

				/* clear OUT_OF_RANGE error */
				p_hndl->resp_status &= 0x1f00u;
				p_hndl->error = SD_OK;
			} else {	/* SDHI error, ex)timeout error so on */
				return _sd_write_sect_error(p_hndl, mode);
			}
		}

		if ((p_hndl->resp_status & RES_STATE) != STATE_TRAN) {
			p_hndl->error = SD_ERR;
			return _sd_write_sect_error(p_hndl, mode);
		}

		/* ---- is stop compulsory? ---- */
		if (p_hndl->stop) {
			p_hndl->stop = 0;

			/* data transfer stop (issue CMD12) */
			SDMMC.SD_STOP.LONGLONG = (uint64_t)0x0001;
			i = 0;  /* set zero to break loop */
			_sd_set_err(p_hndl, SD_ERR_STOP);
		}
	}

	/* ---- halt clock ---- */
	_sd_set_clock(p_hndl, 0, SD_CLOCK_DISABLE);

	return p_hndl->error;

	/*
		CMD12 may not be issued when an error occurs.
		When the SDHI does not execute the command sequence and the SD card is not in the transfer state,
		CMD12 must be issued.

		Conditions for error processing
		1.The SD card is not in the transfer state when CMD13 is issued.
		2.CMD13 issuing process is failed.

		3.DMA transfer processing is failed.
		  The sequence is suspended by the implementation failure of the user-defined function or INFO2 error.
		4.Interrupt for access end or response end is not generated.
		  Implementation failure of the user-defined function caused timeout during the command sequence execution.
		5.Error bit of INFO2 is set.
		6.ACMD22 is in error.
		7.The sector count for writing obtained by ACMD22 and the transmitted sector count are different.
		8.Error bit of CMD13 response is set.
		9.CMD13 issuing process is failed.
		10.The SD card is not in the transfer state when CMD13 is issued.
	*/
}
/******************************************************************************
 End of function _sd_write_sect
 *****************************************************************************/

/******************************************************************************
 * Function Name: _sd_write_sect_error
 * Description  : write sector data error.
 * Arguments    : st_sdhndl_t *p_hndl : SD handle
 *              : int32_t mode        : data transfer mode
 *              :   SD_MODE_SW  : software
 *              :   SD_MODE_DMA : DMA
 * Return Value : SD_OK : end of succeed
 *              : SD_ERR: end of error
 *****************************************************************************/
static int32_t _sd_write_sect_error(st_sdhndl_t *p_hndl, int32_t mode)
{
	volatile int32_t error;
	uint64_t sd_option;
	uint64_t sd_clk_ctrl;

	error = p_hndl->error;

	/* ---- clear error bits ---- */
	_sd_clear_info(p_hndl, SD_INFO1_MASK_TRNS_RESP, SD_INFO2_MASK_ALL);

	/* ---- disable all interrupts ---- */
	_sd_clear_int_mask(p_hndl, SD_INFO1_MASK_TRNS_RESP, SD_INFO2_MASK_ALL);

	/* Cast to an appropriate type */
	if ((SDMMC.SD_INFO2.LONGLONG & SD_INFO2_MASK_CBSY) == SD_INFO2_MASK_CBSY) {
		/* ---- enable All end ---- */
		_sd_set_int_mask(p_hndl, SD_INFO1_MASK_DATA_TRNS, 0);

		/* ---- data transfer stop (issue CMD12) ---- */
		SDMMC.SD_STOP.LONGLONG = (uint64_t)0x0001;

		/* ---- wait All end ---- */
		esddev_int_wait(p_hndl->sd_port, SD_TIMEOUT_RESP);

		/* Cast to an appropriate type */
		_sd_clear_info(p_hndl, SD_INFO1_MASK_TRNS_RESP, SD_INFO2_MASK_ALL);

		/* Cast to an appropriate type */
		_sd_clear_int_mask(p_hndl, SD_INFO1_MASK_DATA_TRNS, 0);

		esddev_loc_cpu(p_hndl->sd_port);

		/* Cast to an appropriate type */
		sd_option   = SDMMC.SD_OPTION.LONGLONG;

		/* Cast to an appropriate type */
		sd_clk_ctrl = SDMMC.SD_CLK_CTRL.LONGLONG;

		/* Cast to an appropriate type */
		SDMMC.SOFT_RST.LONGLONG = SOFT_RST_SDRST_RESET;

		/* Cast to an appropriate type */
		SDMMC.SOFT_RST.LONGLONG = SOFT_RST_SDRST_RELEASED;

		/* Cast to an appropriate type */
		SDMMC.SD_STOP.LONGLONG =  0x0000;

		/* Cast to an appropriate type */
		SDMMC.SD_OPTION.LONGLONG = sd_option;

		/* Cast to an appropriate type */
		SDMMC.SD_CLK_CTRL.LONGLONG = sd_clk_ctrl;
	}

	/* Check Current State */
	if (_sd_card_send_cmd_arg(p_hndl, CMD13, SD_RSP_R1, p_hndl->rca[0], 0x0000) == SD_OK) {
		/* not transfer state? */
		if ((p_hndl->resp_status & RES_STATE) != STATE_TRAN) {
			/* if not tran state, issue CMD12 to transit the SD card to tran state */
			_sd_card_send_cmd_arg(p_hndl, CMD12, SD_RSP_R1B, p_hndl->rca[0], 0x0000);

			/* not check error because already checked */
		}
	}

	p_hndl->error = error;

	/* Cast to an appropriate type */
	_sd_clear_int_mask(p_hndl, SD_INFO1_MASK_TRNS_RESP, SD_INFO2_MASK_ALL);

	/* ---- halt clock ---- */
	_sd_set_clock(p_hndl, 0, SD_CLOCK_DISABLE);

	return p_hndl->error;

}
/******************************************************************************
 End of function _sd_write_sect_error
 *****************************************************************************/

/******************************************************************************
 * Function Name: _sd_single_write
 * Description  : write sector data to card by single block transfer.
 *              : read sector data from physical sector number (=psn) by the
 *              : single block transfer
 *              : if SD Driver mode is SD_MODE_SW, data transfer by
 *              : sddev_read_data function
 *              : if SD Driver mode is SD_MODE_DMA, data transfer by DMAC
 * Arguments    : st_sdhndl_t *p_hndl : SD handle
 *              : uint8_t *buff       : write data buffer
 *              : uint32_t psn        : write physical sector number
 *              : int32_t mode        : data transfer mode
 *              :   SD_MODE_SW  : software
 *              :   SD_MODE_DMA : DMA
 * Return Value : SD_OK : end of succeed
 *              : SD_ERR: end of error
 *****************************************************************************/
static int32_t _sd_single_write(st_sdhndl_t *p_hndl, uint8_t *buff, uint32_t psn,
								int32_t mode)
{
	uint64_t info1_back;

	/* ---- enable RespEnd and ILA ---- */
	_sd_set_int_mask(p_hndl, SD_INFO1_MASK_RESP, SD_INFO2_MASK_ILA);

	/* issue CMD24 (WRITE_SIGLE_BLOCK) */
	if (_sd_send_mcmd(p_hndl, CMD24, SET_ACC_ADDR) != SD_OK) {
		return _sd_single_write_error(p_hndl, mode);
	}

	/* ---- disable RespEnd and ILA ---- */
	_sd_clear_int_mask(p_hndl, SD_INFO1_MASK_RESP, SD_INFO2_MASK_ILA);

	info1_back = 0;

	if (SD_MODE_SW == mode) {	/* ==== PIO ==== */
		/* enable All end, BWE and errors */
		_sd_set_int_mask(p_hndl, SD_INFO1_MASK_DATA_TRNS, SD_INFO2_MASK_BWE);

		/* software data transfer */
		_sd_software_trans(p_hndl, buff, 1, SD_TRANS_WRITE);
	} else {/* ==== DMA ==== */
		/* disable card ins&rem interrupt for FIFO */
		info1_back = (uint64_t)(p_hndl->int_info1_mask & SD_INFO1_MASK_DET_CD);

		/* Cast to an appropriate type */
		_sd_clear_int_mask(p_hndl, SD_INFO1_MASK_DET_CD, 0);

		/* enable All end and errors */
		_sd_set_int_mask(p_hndl, SD_INFO1_MASK_DATA_TRNS, SD_INFO2_MASK_ERR);
	}

	/* ---- wait All end interrupt ---- */
	esddev_int_wait(p_hndl->sd_port, SD_TIMEOUT_RESP);

	if (SD_MODE_DMA == mode) {
		_sd_set_int_mask(p_hndl, info1_back, 0);
	}


	/* ---- check errors ---- */
	if (p_hndl->int_info2 & SD_INFO2_MASK_ERR) {
		_sd_check_info2_err(p_hndl);
		return _sd_single_write_error(p_hndl, mode);
	}

	/* clear All end bit */
	_sd_clear_info(p_hndl, SD_INFO1_MASK_DATA_TRNS, 0x0000);

	/* disable All end, BWE and errors */
	_sd_clear_int_mask(p_hndl, SD_INFO1_MASK_DATA_TRNS, SD_INFO2_MASK_BWE);

	/* ==== skip write complete block number check ==== */

	/* ==== check status after write operation ==== */
	if (_sd_card_send_cmd_arg(p_hndl, CMD13, SD_RSP_R1, p_hndl->rca[0], 0x0000)
			!= SD_OK) {
		/* check OUT_OF_RANGE error */
		/* ignore errors during last block access */
		if (p_hndl->resp_status & 0xffffe008ul) {
			if ((psn + 1) != p_hndl->card_sector_size) {
				return _sd_single_write_error(p_hndl, mode);  /* but for last block */
			}
			if (p_hndl->resp_status & 0x7fffe008ul) {
				return _sd_single_write_error(p_hndl, mode);  /* not OUT_OF_RANGE error */
			}

			/* clear OUT_OF_RANGE error */
			p_hndl->resp_status &= 0x1f00u;
			p_hndl->error = SD_OK;
		} else {	/* SDHI error, ex)timeout error so on */
			return _sd_single_write_error(p_hndl, mode);
		}
	}

	return p_hndl->error;
}
/******************************************************************************
 End of function _sd_single_write
 *****************************************************************************/

/******************************************************************************
 * Function Name: _sd_single_write_error
 * Description  : write sector data to card error.
 * Arguments    : st_sdhndl_t *p_hndl : SD handle.
 *              : int32_t mode        : data transfer mode
 *              :   SD_MODE_SW  : software
 *              :   SD_MODE_DMA : DMA
 * Return Value : SD_OK : end of succeed.
 *              : SD_ERR: end of error
 *****************************************************************************/
static int32_t _sd_single_write_error(st_sdhndl_t *p_hndl, int32_t mode)
{
	int32_t temp_error;

	temp_error = p_hndl->error;

	/* ---- clear error bits ---- */
	_sd_clear_info(p_hndl, SD_INFO1_MASK_TRNS_RESP, SD_INFO2_MASK_ALL);

	/* ---- disable all interrupts ---- */
	_sd_clear_int_mask(p_hndl, SD_INFO1_MASK_TRNS_RESP, SD_INFO2_MASK_ALL);

	/* ---- enable All end ---- */
	_sd_set_int_mask(p_hndl, SD_INFO1_MASK_DATA_TRNS, 0);

	/* ---- data transfer stop (issue CMD12) ---- */
	SDMMC.SD_STOP.LONGLONG = (uint64_t)0x0001;

	/* ---- wait All end ---- */
	esddev_int_wait(p_hndl->sd_port, SD_TIMEOUT_RESP);

	/* Cast to an appropriate type */
	_sd_clear_info(p_hndl, SD_INFO1_MASK_TRNS_RESP, SD_INFO2_MASK_ALL);
	_sd_card_send_cmd_arg(p_hndl, CMD13, SD_RSP_R1, p_hndl->rca[0], 0x0000);
	p_hndl->error = temp_error;

	/* Cast to an appropriate type */
	_sd_clear_int_mask(p_hndl, SD_INFO1_MASK_TRNS_RESP, SD_INFO2_MASK_ALL);

	/* ---- halt clock ---- */
	_sd_set_clock(p_hndl, 0, SD_CLOCK_DISABLE);

	return p_hndl->error;
}
/******************************************************************************
 End of function _sd_single_write_error
 *****************************************************************************/

/******************************************************************************
 * Function Name: _sd_software_trans
 * Description  : transfer data by software.
 *              : transfer data to/from card by software
 *              : this operations are used multiple command data phase
 *              : if dir is SD_TRANS_READ, data is from card to host
 *              : if dir is SD_TRANS_WRITE, data is from host to card
 * Arguments    : st_sdhndl_t *p_hndl : SD handle
 *              : uint8_t *buff       : destination/source data buffer
 *              : int32_t cnt         : number of transfer bytes
 *              : int32_t dir         : transfer direction
 * Return Value : p_hndl->error  : SD handle error value
 *              : SD_OK : end of succeed
 *              : SD_ERR: end of error
 * Remark       : transfer finished, check CMD12 sequence refer to All end
 *****************************************************************************/
static int32_t _sd_software_trans(st_sdhndl_t *p_hndl, uint8_t *buff, int32_t cnt, int32_t dir)
{
	int32_t j;
	int32_t (*func)(int32_t sd_port, uint8_t *buff, uint32_t reg_addr, int32_t num);

	if (SD_TRANS_READ == dir) {
		func = esddev_read_data;
	} else {
		func = esddev_write_data;
	}

	for (j = cnt; j > 0 ; j--) {
		/* ---- wait BWE/BRE interrupt ---- */
		if (esddev_int_wait(p_hndl->sd_port, SD_TIMEOUT_MULTIPLE) != SD_OK) {
			_sd_set_err(p_hndl, SD_ERR_HOST_TOE);
			break;
		}

		/* Cast to an appropriate type */
		if (p_hndl->int_info2 & SD_INFO2_MASK_ERR) {
			_sd_check_info2_err(p_hndl);
			break;
		}

		if (SD_TRANS_READ == dir) {
			/* Cast to an appropriate type */
			_sd_clear_info(p_hndl, 0x0000, SD_INFO2_MASK_RE); /* clear BRE and errors bit */
		} else {
			/* Cast to an appropriate type */
			_sd_clear_info(p_hndl, 0x0000, SD_INFO2_MASK_WE); /* clear BWE and errors bit */
		}

		/* write/read to/from SD_BUF by 1 sector */

		if ((*func)(p_hndl->sd_port, buff, (uintptr_t)(&SDMMC.SD_BUF0.LONGLONG), 512) != SD_OK) {
			_sd_set_err(p_hndl, SD_ERR_CPU_IF);
			break;
		}

		/* update buffer */
		buff += 512;

	}

	return p_hndl->error;
}
/******************************************************************************
 End of function _sd_software_trans
 *****************************************************************************/

/******************************************************************************
* Function Name: sddev_write_data
* Description  : write to SDHI buffer FIFO
* Arguments    : int32_t sd_port   : channel no (0 or 1)
*              : uint8_t *buff     : buffer addrees to store writting datas
*              : uint32_t reg_addr : SDIP FIFO address
*              : int32_t num       : counts to write(unit:byte)
* Return Value : success : SD_OK
*              : fail    : SD_ERR
******************************************************************************/
static int32_t esddev_write_data(int32_t sd_port, uint8_t *buff, uint32_t reg_addr, int32_t num)
{
	int32_t  i;

	/* Cast to an appropriate type */
	uint64_t *p_reg = (uint64_t *)((uintptr_t)reg_addr);

	/* Cast to an appropriate type */
	uint64_t *p_buff = (uint64_t *)buff;
	uint64_t tmp;

	/* dont care non 8byte alignment data */
	num += 7;
	num /= 8;

	/* Cast to an appropriate type */
	if (((uintptr_t)buff & 0x7uL) != 0uL) {
		for (i = num; i > 0 ; i--) {
			/* Cast to an appropriate type */
			tmp  = (uint64_t)(*buff++);

			/* Cast to an appropriate type */
			tmp |= ((uint64_t)(*buff++) << 8);

			/* Cast to an appropriate type */
			tmp |= ((uint64_t)(*buff++) << 16);

			/* Cast to an appropriate type */
			tmp |= ((uint64_t)(*buff++) << 24);

			/* Cast to an appropriate type */
			tmp |= ((uint64_t)(*buff++) << 32);

			/* Cast to an appropriate type */
			tmp |= ((uint64_t)(*buff++) << 40);

			/* Cast to an appropriate type */
			tmp |= ((uint64_t)(*buff++) << 48);

			/* Cast to an appropriate type */
			tmp |= ((uint64_t)(*buff++) << 56);
			*p_reg = tmp;
		}
	} else {
		for (i = num; i > 0 ; i--) {
			*p_reg = *p_buff++;
		}
	}

	return SD_OK;
}
/*******************************************************************************
 End of function sddev_write_data
 ******************************************************************************/

/**********************************************************************************************************************
 * Function Name: esd_main
 * Description  : Initializes and mounts the working memory of the SD driver.
 * Arguments    : void
 * Return Value : SD_OK : end of succeed
 *              : SD_ERR: end of error
 *********************************************************************************************************************/
int32_t esd_main(void)
{
	int32_t subret;
	uint16_t ubcardtype;
	uint16_t ubcardspeed;
	uint8_t  ubcardcapacity;
	int32_t  ipartition_number;

	const struct sdhi_config_t * sdhi_fconf_cfg = sdhi_config_getter();

	subret=0;
	subret = esd_init(DEV_SD0, sdhi_fconf_cfg->mmc_base, sd_drv_work_area, SD_CD_SOCKET);
	if (SD_OK != subret) {
		NOTICE("BL2: Failed to esd_init.\n");
		return SD_ERR;
    }

	subret=0;
	subret = esd_set_buffer(DEV_SD0, (void *)sd_drv_rw_buffer,     /* Specify format buffer that has 512byte */
							sizeof(sd_drv_rw_buffer));    /* format buffer size is specify */
	if (SD_OK != subret) {
		NOTICE("BL2: Failed to esd_set_buffer.\n");
		return SD_ERR;
	}

	subret=0;
	subret = esd_check_media(DEV_SD0);
	if (SD_OK != subret) {
		NOTICE("BL2: Failed to esd_check_media.\n");
		return SD_ERR;
	}

	subret=0;
	subret = esd_mount(DEV_SD0, (SD_MODE_POLL |  SD_MODE_SW  |  SD_MODE_DS | SD_MODE_VER2X), SD_VOLT_3_3);
	if (SD_OK != subret) {
		NOTICE("BL2: Failed to esd_mount.\n");
		return SD_ERR;
	}

	subret=0;
	ubcardtype=0;
	ubcardspeed=0;
	ubcardcapacity=0;
	subret = esd_get_type(DEV_SD0, (uint16_t *)&ubcardtype,
							(uint16_t *) &ubcardspeed,
							(uint8_t *) &ubcardcapacity);
	if( SD_OK != subret ) {
		NOTICE("BL2: Failed to esd_get_type.\n");
		return SD_ERR;
	}
	else if( ( ubcardtype & SD_MEDIA_SD ) == SD_MEDIA_SD ) {
		/* none */
	}
	else if( ( ubcardtype & SD_MEDIA_EMBEDDED ) == SD_MEDIA_EMBEDDED ) {
		/* No operation */
	}
	else {
		/* Error break with invalid card type */
		NOTICE("BL2: Failed to esd_get_type.\n");
		return SD_ERR;
    }

	subret=0;
	ipartition_number = 0;
	subret = esd_get_partition_id(DEV_SD0, &ipartition_number);
	if (SD_OK != subret) {
		NOTICE("BL2: Failed to esd_get_partition_id.\n");
		return SD_ERR;
	}

	return SD_OK;
}

/* End of File */
