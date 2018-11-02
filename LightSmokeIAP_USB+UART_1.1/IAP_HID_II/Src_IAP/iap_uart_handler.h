/*********************************************************************************************************//**
 * @file    Src_IAP/iap_handler.h
 * @version V1.00
 * @date    11/22/2012
 * @brief   The header file of IAP Handler.
 *************************************************************************************************************
 *
 * <h2><center>Copyright (C) 2012 Holtek Semiconductor Inc. All rights reserved</center></h2>
 *
 ************************************************************************************************************/
// <<< Use Configuration Wizard in Context Menu >>>
/* Define to prevent recursive inclusion -------------------------------------------------------------------*/
#ifndef __IAP_UART_HANDLER_H
#define __IAP_UART_HANDLER_H

/* Includes ------------------------------------------------------------------------------------------------*/
//#include "mcu.h"
#include "ht32.h"
#include "ht32_board.h"
#include "iap_config.h"

/* Settings ------------------------------------------------------------------------------------------------*/
#define MCU_PAGE_NUM_PER_PPBIT          (1)
//#define MCU_FLASH_PAGE_SIZE             (1024)
#define MCU_FLASH_PAGES                 (63)
#define MCU_FLASH_SIZE                  (MCU_FLASH_PAGE_SIZE * MCU_FLASH_PAGES)
#define MCU_SRAM_SIZE                   (1024 * 10)
#define MCU_SRAM_START                  (HT_SRAM_BASE)

/* Settings ------------------------------------------------------------------------------------------------*/
//<o0.0> Enable IAP Debug mode
// <i> Enable IAP Debug mode which output debug message to retarget COM port or SWV (ITM).
#define IAP_DEBUG_MODE                (0)

//  <h> IAP Encryption Setting
//      <h> ARC4
//            <o0.0> ARC4 Decryption Enable
#define ARC4_DECRYPTIOIN_EN                         (0)
#define ARC4_KEY_LEN                                (32)
//      </h>
//      <h> Restraint Flow Check
//            <o0.0> Restraint Programming Flow Enable
//            <o1> Restraint Minimum Erase Size (Unit: KBytes) <1-255>
#define RESTRAINT_PROGRAMMING_FLOW_EN               (0)
#define MINIMUM_ERASE_SIZE_KB                       (3)
#define RESTRAINT_MINIMUM_ERASE_Size                (MINIMUM_ERASE_SIZE_KB * 1024)
#define MINIMUM_ERASE_SIZE                          (RESTRAINT_MINIMUM_ERASE_Size / FLASH_PAGE_SIZE)

//            <o0.0> Digest Check Flow Enable
#define DIGEST_CHECK_EN                             (0)
//        </h>
//      <h> Command Key
//            <o0.0> Command Key Enable
#define COMMAND_KEY_EN                              (0)
//        </h>
//    </h>

//<o0> User Command count <0-2>
#define USER_CMD_COUNT                (0)

//<o0> IAP Region (Number of Pages) <1-127>
//#define IAP_CODE_PAGE                 (10)
//#define IAP_CODE_SIZE                 (IAP_CODE_PAGE * MCU_FLASH_PAGE_SIZE)       /* IAP code size in Bytes             */
#define IAP_APINFO_SIZE               IAP_VERSION_SIZE // The VectorTable must align 128 bytes

//#define LOADER_VERSION                (102)

//<o0.0> Enable wait IAP command timeout function
// <i> If IAP Loader did not received any valid IAP command after specific timeout time, it starts AP automatically.
//<o1> Wait IAP command timeout value (ms) <1-5000:1>
#define IAP_TIMEOUT_EN                (0)
//#define IAP_TIMEOUT                   (5000)                        /* IAP Command timeout based on ms      */

/*  Buffer                                                                                                  */
//#define TX_BUFFER_SIZE                (68)                          /* Tx Buffer size                       */
//#define RX_BUFFER_SIZE                (256)                         /* Rx Buffer size                       */

/* Exported types ------------------------------------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------------------------------------*/
#define IAP_TIMEOUT_OFF               (-1)

//#define IAP_APFLASH_START             (IAP_CODE_SIZE + IAP_APINFO_SIZE)
#define IAP_APFLASH_END               (MCU_FLASH_SIZE - 1)

#define IAP_SRAM_START                (MCU_SRAM_START)
#define IAP_SRAM_END                  (MCU_SRAM_START + MCU_SRAM_SIZE - 1)

//#define BOOT_MODE_APP                 (0x55AAFAF0)                  /* Magic number for SBVT1               */
//#define BOOT_MODE_IAP                 (0x55AAFAF5)

//#define BOOT_MODE_ID_ADDR             (IAP_SRAM_END - 3) /* SRAM last 4 bytes as boot mode ID, replace the original SBVT[1] */

#define FW_INF_ENGINEERING_LENGTH     (0xFFFFA55A)
#define FW_INF_ENGINEERING_CRC        (0x00005AA5)

/* Exported macro ------------------------------------------------------------------------------------------*/

/* Global variables ----------------------------------------------------------------------------------------*/
extern u32 IAP_Update_Port;

/* Exported functions --------------------------------------------------------------------------------------*/
void IAP_Uart_Init(void);
void IAP_Uart_Handler(void);
u32 IAP_Uart_Reset(u32 uMode);
u32 IAP_Uart_isAPValid(void);
//void IAP_Uart_GoCMD(u32 address);
void IAP_TimebaseHandler(void);

#endif /* __IAP_HANDLER_H ----------------------------------------------------------------------------------*/
