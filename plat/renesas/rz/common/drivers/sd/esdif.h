/*
 * Copyright (c) 2022, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**********************************************************************************************************************
 * File Name    : esdif.h
 * Version      : 1.0
 * Description  : SD Driver interface header file.
 *********************************************************************************************************************/
/**********************************************************************************************************************
 * History : DD.MM.YYYY Version  Description
 *         : 01.09.2020 1.00     First Release
 *********************************************************************************************************************/

/**********************************************************************************************************************
 Includes   <System Includes> , "Project Includes"
 *********************************************************************************************************************/

/**********************************************************************************************************************
 Macro definitions
 *********************************************************************************************************************/
#ifndef SDDRV_H
#define SDDRV_H

/* ==== Define  ==== */
/* ---- SD Driver work buffer ---- */

/* ---- SD Driver work buffer ---- */
#define SD_SIZE_OF_INIT           (856)

#define DEV_SD0     (0)
#define DEV_SD1     (1)

/* ---- error code ---- */
#define SD_OK_LOCKED_CARD         (1)                   /* OK but card is locked status */
#define SD_OK                     (0)                   /* OK */
#define SD_ERR                    (-1)                  /* general error */
#define SD_ERR_WP                 (-2)                  /* write protect error */
/* 3 */
#define SD_ERR_RES_TOE            (-4)                  /* response time out error */
#define SD_ERR_CARD_TOE           (-5)                  /* card time out error */
#define SD_ERR_END_BIT            (-6)                  /* end bit error */
#define SD_ERR_CRC                (-7)                  /* CRC error */
#define SD_ERR_ILL_ACCESS         (-8)                  /* illegal access error */
#define SD_ERR_HOST_TOE           (-9)                  /* host time out error */
#define SD_ERR_CARD_ERASE         (-10)                 /* card erase error */
#define SD_ERR_CARD_LOCK          (-11)                 /* card lock error */
#define SD_ERR_CARD_UNLOCK        (-12)                 /* card unlock error */
#define SD_ERR_HOST_CRC           (-13)                 /* host CRC error */
#define SD_ERR_CARD_ECC           (-14)                 /* card internal ECC error */
#define SD_ERR_CARD_CC            (-15)                 /* card internal error */
#define SD_ERR_CARD_ERROR         (-16)                 /* unknown card error */
#define SD_ERR_CARD_TYPE          (-17)                 /* non support card type */
#define SD_ERR_NO_CARD            (-18)                 /* no card */
#define SD_ERR_ILL_READ           (-19)                 /* illegal buffer read */
#define SD_ERR_ILL_WRITE          (-20)                 /* illegal buffer write */
#define SD_ERR_AKE_SEQ            (-21)                 /* the sequence of authentication process */
#define SD_ERR_OVERWRITE          (-22)                 /* CID/CSD overwrite error */
/* 23-29 */
#define SD_ERR_CPU_IF             (-30)                 /* target CPU interface function error  */
#define SD_ERR_STOP               (-31)                 /* user stop */
/* 32-49 */
#define SD_ERR_CSD_VER            (-50)                 /* CSD register version error */
#define SD_ERR_SCR_VER            (-51)                 /* SCR register version error */
#define SD_ERR_FILE_FORMAT        (-52)                 /* CSD register file format error  */
#define SD_ERR_NOTSUP_CMD         (-53)                 /* not supported command  */
/* 54-59 */
#define SD_ERR_ILL_FUNC           (-60)                 /* invalid function request error */
#define SD_ERR_IO_VERIFY          (-61)                 /* direct write verify error */
#define SD_ERR_IO_CAPAB           (-62)                 /* IO capability error */
/* 63-69 */
#define SD_ERR_IFCOND_VER         (-70)                 /* Interface condition version error */
#define SD_ERR_IFCOND_VOLT        (-71)                 /* Interface condition voltage error */
#define SD_ERR_IFCOND_ECHO        (-72)                 /* Interface condition echo back pattern error */
/* 73-79 */
#define SD_ERR_OUT_OF_RANGE       (-80)                 /* the argument was out of range */
#define SD_ERR_ADDRESS_ERROR      (-81)                 /* misassigned address */
#define SD_ERR_BLOCK_LEN_ERROR    (-82)                 /* transfered block length is not allowed */
#define SD_ERR_ILLEGAL_COMMAND    (-83)                 /* Command not legal  */
#define SD_ERR_RESERVED_ERROR18   (-84)                 /* Reserved bit 18 Error */
#define SD_ERR_RESERVED_ERROR17   (-85)                 /* Reserved bit 17 Error */
#define SD_ERR_CMD_ERROR          (-86)                 /* SD_INFO2 bit  0 CMD error */
#define SD_ERR_CBSY_ERROR         (-87)                 /* SD_INFO2 bit 14 CMD Type Reg Busy error */
#define SD_ERR_NO_RESP_ERROR      (-88)                 /* SD_INFO1 bit  0 No Response error */
/* 89-95 */
#define SD_ERR_ERROR              (-96)                 /* SDIO ERROR */
#define SD_ERR_FUNCTION_NUMBER    (-97)                 /* SDIO FUNCTION NUMBER ERROR */
#define SD_ERR_COM_CRC_ERROR      (-98)                 /* SDIO CRC ERROR */
#define SD_ERR_INTERNAL           (-99)                 /* driver software internal error */

/* ---- driver mode ---- */
#define SD_MODE_POLL              (0x0000ul)            /* status check mode is software polling */
#define SD_MODE_HWINT             (0x0001ul)            /* status check mode is hardware interrupt */
#define SD_MODE_SW                (0x0000ul)            /* data transfer mode is software */
#define SD_MODE_DMA               (0x0002ul)            /* data transfer mode is DMA */

/* ---- support mode ---- */
#define SD_MODE_MEM               (0x0000ul)            /* memory cards only are supported */
#define SD_MODE_IO                (0x0010ul)            /* memory and io cards are supported */
#define SD_MODE_COMBO             (0x0030ul)            /* memory ,io and combo cards are supported */
#define SD_MODE_DS                (0x0000ul)            /* only default speed mode is supported */
#define SD_MODE_HS                (0x0040ul)            /* high speed mode is also supported */
#define SD_MODE_SDR12             (0x1000ul)            /* SDR12 mode is also supported */
#define SD_MODE_SDR25             (0x2000ul)            /* SDR25 mode is also supported */
#define SD_MODE_SDR50             (0x4000ul)            /* SDR50 mode is also supported */
#define SD_MODE_SDR104            (0x8000ul)            /* SDR104 mode is also supported */
#define SD_MODE_VER1X             (0x0000ul)            /* ver1.1 host */
#define SD_MODE_VER2X             (0x0080ul)            /* ver2.x host (high capacity and dual voltage) */
#define SD_MODE_1BIT              (0x0100ul)            /* SD Mode 1bit only is supported */
#define SD_MODE_4BIT              (0x0000ul)            /* SD Mode 1bit and 4bit is supported */

/* ---- media voltage ---- */
#define SD_VOLT_1_7               (0x00000010ul)        /* low voltage card minimum */
#define SD_VOLT_1_8               (0x00000020ul)
#define SD_VOLT_1_9               (0x00000040ul)
#define SD_VOLT_2_0               (0x00000080ul)
#define SD_VOLT_2_1               (0x00000100ul)        /* basic communication minimum */
#define SD_VOLT_2_2               (0x00000200ul)
#define SD_VOLT_2_3               (0x00000400ul)
#define SD_VOLT_2_4               (0x00000800ul)
#define SD_VOLT_2_5               (0x00001000ul)
#define SD_VOLT_2_6               (0x00002000ul)
#define SD_VOLT_2_7               (0x00004000ul)
#define SD_VOLT_2_8               (0x00008000ul)        /* memory access minimum */
#define SD_VOLT_2_9               (0x00010000ul)
#define SD_VOLT_3_0               (0x00020000ul)
#define SD_VOLT_3_1               (0x00040000ul)
#define SD_VOLT_3_2               (0x00080000ul)
#define SD_VOLT_3_3               (0x00100000ul)
#define SD_VOLT_3_4               (0x00200000ul)
#define SD_VOLT_3_5               (0x00400000ul)
#define SD_VOLT_3_6               (0x00800000ul)

/* ---- memory card write mode ---- */
#define SD_WRITE_WITH_PREERASE    (0x0000u)             /* pre-erease write */
#define SD_WRITE_OVERWRITE        (0x0001u)             /* overwrite  */

/* ---- io register write mode ---- */
#define SD_IO_SIMPLE_WRITE        (0x0000u)             /* just write */
#define SD_IO_VERIFY_WRITE        (0x0001u)             /* read after write */

/* ---- io operation code ---- */
#define SD_IO_FIXED_ADDR          (0x0000u)             /* R/W fixed address */
#define SD_IO_INCREMENT_ADDR      (0x0001u)             /* R/W increment address */
#define SD_IO_FORCE_BYTE          (0x0010u)             /* byte access only  */

/* ---- media type ---- */
#define SD_MEDIA_UNKNOWN          (0x0000u)             /* unknown media */
#define SD_MEDIA_MMC              (0x0010u)             /* MMC card */
#define SD_MEDIA_SD               (0x0020u)             /* SD Memory card */
#define SD_MEDIA_IO               (0x0001u)             /* SD IO card */
#define SD_MEDIA_MEM              (0x0030u)             /* Memory card */
#define SD_MEDIA_COMBO            (0x0021u)             /* SD COMBO card */
#define SD_MEDIA_EMBEDDED         (0x8000u)             /* Embedded media */

/* ---- write protect info --- */
#define SD_WP_OFF                 (0x0000u)             /* card is not write protect */
#define SD_WP_HW                  (0x0001u)             /* card is H/W write protect */
#define SD_WP_TEMP                (0x0002u)             /* card is TEMP_WRITE_PROTECT */
#define SD_WP_PERM                (0x0004u)             /* card is PERM_WRITE_PROTECT */
#define SD_WP_ROM                 (0x0010u)             /* card is SD-ROM */

/* ---- SD clock div ---- */    /* IMCLK is host controller clock */
#define SD_DIV_512                (0x0080u)             /* SDCLOCK = IMCLK/512 */
#define SD_DIV_256                (0x0040u)             /* SDCLOCK = IMCLK/256 */
#define SD_DIV_128                (0x0020u)             /* SDCLOCK = IMCLK/128 */
#define SD_DIV_64                 (0x0010u)             /* SDCLOCK = IMCLK/64 */
#define SD_DIV_32                 (0x0008u)             /* SDCLOCK = IMCLK/32 */
#define SD_DIV_16                 (0x0004u)             /* SDCLOCK = IMCLK/16 */
#define SD_DIV_8                  (0x0002u)             /* SDCLOCK = IMCLK/8 */
#define SD_DIV_4                  (0x0001u)             /* SDCLOCK = IMCLK/4 */
#define SD_DIV_2                  (0x0000u)             /* SDCLOCK = IMCLK/2 */
#define SD_DIV_1                  (0x00FFu)             /* SDCLOCK = IMCLK (option) */

/* ---- SD clock define ---- */                 /* Max frequency */
#define SD_CLK_400KHZ           (0x0000u)       /* 400kHz */
#define SD_CLK_1MHZ             (0x0001u)       /* 1MHz */
#define SD_CLK_5MHZ             (0x0002u)       /* 5MHZ */
#define SD_CLK_10MHZ            (0x0003u)       /* 10MHZ */
#define SD_CLK_20MHZ            (0x0004u)       /* 20MHZ */
#define SD_CLK_25MHZ            (0x0005u)       /* 25MHZ */
#define SD_CLK_50MHZ            (0x0006u)       /* 50MHZ (phys spec ver1.10) */

/* ---- speed class ---- */
#define SD_SPEED_CLASS_0          (0x00u)               /* not defined, or less than ver2.0 */
#define SD_SPEED_CLASS_2          (0x01u)               /* 2MB/sec */
#define SD_SPEED_CLASS_4          (0x02u)               /* 4MB/sec */
#define SD_SPEED_CLASS_6          (0x03u)               /* 6MB/sec */
/* ---- IO initialize flags define ---- */    /* add for IO */
#define SD_IO_INT_ENAB            (0x10u)               /* interrupt enable */
#define SD_IO_POWER_INIT          (0x04u)               /* power on initialized */
#define SD_IO_MEM_INIT            (0x02u)               /* memory initialized */
#define SD_IO_FUNC_INIT           (0x01u)               /* io func initialized */

/* ---- IO function's information ---- */    /* add for IO */
#define SD_IO_FUNC_READY          (0x80u)               /* io redy */
#define SD_IO_FUNC_NUM            (0x70u)               /* number of io func */
#define SD_IO_FUNC_EXISTS         (0x04u)               /* memory present */

/* ---- SD port mode ---- */
#define SD_PORT_SERIAL          (0x0000u)       /* 1bit mode */
#define SD_PORT_PARALLEL        (0x0001u)       /* 4bits mode */

/* ---- SD Card detect port ---- */
#define SD_CD_SOCKET            (0x0000u)       /* CD pin */
#define SD_CD_DAT3              (0x0001u)       /* DAT3 pin */

/* ---- SD Card detect interrupt ---- */
#define SD_CD_INT_DISABLE       (0x0000u)       /* Card detect interrupt disable */
#define SD_CD_INT_ENABLE        (0x0001u)       /* Card detect interrupt enable */

/* ---- lock/unlock mode ---- */
#define SD_FORCE_ERASE            (0x08)
#define SD_LOCK_CARD              (0x04)
#define SD_UNLOCK_CARD            (0x00)
#define SD_CLR_PWD                (0x02)
#define SD_SET_PWD                (0x01)

/* ---- Format mode ---- */
#define SD_FORMAT_QUICK         (0x0000u)       /* Quick format */
#define SD_FORMAT_FULL          (0x0001u)       /* Full format */

/* ---- SD Driver work buffer ---- */
#define ESD_SIZE_OF_INIT        (SD_SIZE_OF_INIT+4)
/* ---- Media type ---- */
#define SD_MEDIA_EMBEDDED       (0x8000u)       /* Embedded media */

/******************************************************************************
Typedef definitions
******************************************************************************/
/* ---- User Configuration ---- */
/* SD card detection option */
typedef enum {
	SD_CD_ENABLED,      /* SD card detection is enabled. */
	SD_CD_DISABLED      /* When SD card detection is disabled,
						   the status is always loading. */
} e_sd_cd_layout_t;

/* SD write protection signal detection option */
typedef enum {
	SD_WP_ENABLED,      /* Write protection signal detection is enabled. */
	SD_WP_DISABLED      /* When write protection signal detection is disabled,
						   the status is always write protection signal off. */
} e_sd_wp_layout_t;

typedef int32_t (*p_intCallbackFunc)(int32_t sd_port, int32_t cd);
typedef int32_t (*p_fmtCallbackFunc)(uint32_t secno, uint32_t size);
typedef int32_t (*p_intIoCallbackFunc)(int32_t sd_port);

/**********************************************************************************************************************
 Global Typedef definitions
 *********************************************************************************************************************/

/**********************************************************************************************************************
 External global variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 Exported global functions
 *********************************************************************************************************************/
/* ==== API prototype ===== */
/* ---- Access library I/F ---- */
extern int32_t esd_main(void);
extern int32_t esd_init(int32_t sd_port, uint32_t base, void *workarea, int32_t cd_port);
extern int32_t esd_check_media(int32_t sd_port);
extern int32_t esd_read_sect(int32_t sd_port, uint8_t *buff, uint32_t psn, int32_t cnt);
extern int32_t esd_set_buffer(int32_t sd_port, void *buff, uint32_t size);
extern int32_t esd_get_type(int32_t sd_port, uint16_t *type, uint16_t *speed, uint8_t *capa);
extern int32_t esd_mount(int32_t sd_port, uint32_t mode, uint32_t voltage);
extern int32_t esd_write_sect(int32_t sd_port, uint8_t *buff, uint32_t psn, int32_t cnt, int32_t writemode);

#endif /* SDDRV_H */
/* End of File */