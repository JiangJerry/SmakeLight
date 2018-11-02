/*********************************************************************************************************//**
 * @file    IAP/IAP_HID_II/Src_AP/ap_config.h
 * @version $Rev:: 2524         $
 * @date    $Date:: 2018-01-29 #$
 * @brief   The configuration file.
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
#ifndef __IAP_CONFIG_H
#define __IAP_CONFIG_H

/* Includes ------------------------------------------------------------------------------------------------*/

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


/* Settings ------------------------------------------------------------------------------------------------*/
/*  Buffer                                                                                                  */
#define TX_BUFFER_SIZE                (68)                          /* Tx Buffer size                       */
#define RX_BUFFER_SIZE                (1024)                        /* Rx Buffer size                       */

/* IAP                                                                                                      */
#define LOADER_VERSION                (0x100)

//<o0.0> Enable wait IAP command timeout function
// <i> If IAP Loader did not received any valid IAP command after specific timeout time, it starts AP automatically.
//<o1> Wait IAP command timeout value (ms) <1-5000:1>
#define IAP_TIMEOUT_EN                (0)
#define IAP_TIMEOUT                   (5000) //2000                       /* IAP Command timeout based on ms      */

#define EXT_FLASH_ENABLE              (0)
#define EXT_FLASH_PAGE_SIZE           (1024 * 4)
#define EXT_FALSH_ADDRESS             (0x01000000)


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
