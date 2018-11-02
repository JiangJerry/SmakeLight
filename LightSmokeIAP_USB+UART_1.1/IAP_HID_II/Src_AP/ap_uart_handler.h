/*********************************************************************************************************//**
 * @file    Src_AP/ap_handler.h
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
#ifndef __AP_UART_HANDLER_H
#define __AP_UART_HANDLER_H

/* Includes ------------------------------------------------------------------------------------------------*/
//#include "mcu.h"

/* Settings ------------------------------------------------------------------------------------------------*/

//<o0.0> Enable IAP Debug mode
// <i> Enable IAP Debug mode which output debug message to retarget COM port or SWV (ITM).
#define IAP_DEBUG_MODE                (0)

//<o0> User Command count <0-2>
#define USER_CMD_COUNT                (1)

/*  Buffer                                                                                                  */
//#define TX_BUFFER_SIZE                (68)                          /* Tx Buffer size                       */
//#define RX_BUFFER_SIZE                (256)                         /* Rx Buffer size                       */

/* Exported types ------------------------------------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------------------------------------*/

#define IAP_SRAM_START                (MCU_SRAM_START)
#define IAP_SRAM_END                  (MCU_SRAM_START + MCU_SRAM_SIZE - 1)

//#define BOOT_MODE_APP                 (0x55AAFAF0)                  /* Magic number for SBVT1               */
//#define BOOT_MODE_IAP                 (0x55AAFAF5)

//#define BOOT_MODE_ID_ADDR             (IAP_SRAM_END - 3) /* SRAM last 4 bytes as boot mode ID, replace the original SBVT[1] */

/* Exported macro ------------------------------------------------------------------------------------------*/

/* Global variables ----------------------------------------------------------------------------------------*/

/* Exported functions --------------------------------------------------------------------------------------*/
void AP_Uart_Init(void);
void AP_Uart_Handler(void);
u32 AP_Uart_Reset(u32 uMode);
void AP_TimebaseHandler(void);



#endif /* __IAP_HANDLER_H ----------------------------------------------------------------------------------*/
