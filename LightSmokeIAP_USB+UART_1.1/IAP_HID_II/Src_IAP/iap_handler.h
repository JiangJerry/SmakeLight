/*********************************************************************************************************//**
 * @file    IAP/IAP_HID_II/Src_IAP/iap_handler.h
 * @version $Rev:: 2524         $
 * @date    $Date:: 2018-01-29 #$
 * @brief   The header file of IAP Handler.
 *************************************************************************************************************
 * @attention
 *
 * Firmware Disclaimer Information
 *
 * 1. The customer hereby acknowledges and agrees that the program technical documentation, including the
 *    code, which is supplied by Holtek Semiconductor Inc., (hereinafter referred to as "HOLTEK") is the
 *    proprietary and confidential intellectual property of HOLTEK, and is protected by copyright law and
 *    other intellectual property laws.
 *
 * 2. The customer hereby acknowledges and agrees that the program technical documentation, including the
 *    code, is confidential information belonging to HOLTEK, and must not be disclosed to any third parties
 *    other than HOLTEK and the customer.
 *
 * 3. The program technical documentation, including the code, is provided "as is" and for customer reference
 *    only. After delivery by HOLTEK, the customer shall use the program technical documentation, including
 *    the code, at their own risk. HOLTEK disclaims any expressed, implied or statutory warranties, including
 *    the warranties of merchantability, satisfactory quality and fitness for a particular purpose.
 *
 * <h2><center>Copyright (C) Holtek Semiconductor Inc. All rights reserved</center></h2>
 ************************************************************************************************************/
// <<< Use Configuration Wizard in Context Menu >>>
/* Define to prevent recursive inclusion -------------------------------------------------------------------*/
#ifndef __IAP_HANDLER_H
#define __IAP_HANDLER_H

/* Includes ------------------------------------------------------------------------------------------------*/
#include "ht32.h"
#include "ht32_board.h"

/** @addtogroup HT32_Series_Peripheral_Examples HT32 Peripheral Examples
  * @{
  */

/** @addtogroup IAP_HID IAP HID
  * @{
  */

/** @addtogroup IAP_HID_II IAP HID II (with USB Check buffer)
  * @{
  */

/** @addtogroup IAP_HID_II_IAP IAP Example
  * @{
  */
  
extern u32 IAP_Update_Port;

/* Settings ------------------------------------------------------------------------------------------------*/
//<o0.0> Enable IAP Debug mode
// <i> Enable IAP Debug mode which output debug message to retarget.
#define IAP_DEBUG_MODE                (0)

/* Exported constants --------------------------------------------------------------------------------------*/
#define IAP_TIMEOUT_OFF               (-1)

/* Exported functions --------------------------------------------------------------------------------------*/
void IAP_Init(void);
void IAP_Handler(void);
u32 IAP_Reset(u32 uMode);
u32 IAP_isAPValid(void);
u32 IAP_isVersionValid(void);
void IAP_GoCMD(u32 address);


/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#endif
