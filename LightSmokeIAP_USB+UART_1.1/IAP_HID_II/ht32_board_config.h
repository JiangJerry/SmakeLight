/*********************************************************************************************************//**
 * @file    IAP_HID/IAP_HID_II/ht32_board_config.h
 * @version $Rev:: 2113         $
 * @date    $Date:: 2017-11-13 #$
 * @brief   The header file of board configuration.
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
/* Define to prevent recursive inclusion -------------------------------------------------------------------*/
#ifndef __HT32_BOARD_CONFIG_H
#define __HT32_BOARD_CONFIG_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Settings ------------------------------------------------------------------------------------------------*/
#if defined(USE_HT32F52230_SK)
  #error "This example code does not apply to the chip you selected."
#endif

#if defined(USE_HT32F52241_SK)
  #error "This example code does not apply to the chip you selected."
#endif

#if defined(USE_HT32F52341_SK)
#endif

#if defined(USE_HT32F52352_SK)
#endif

#define MCU_FLASH_PAGE_SIZE           (1024)   
#define IAP_CODE_PAGE                 (10)
#define IAP_CODE_SIZE                 (IAP_CODE_PAGE * MCU_FLASH_PAGE_SIZE)  /* IAP code size in Bytes     */
       
#define IAP_VERSION_SIZE              (LIBCFG_FLASH_PAGESIZE)

#define IAP_APFLASH_SIZE              (LIBCFG_FLASH_SIZE)
#define IAP_APSRAM_SIZE               (LIBCFG_RAM_SIZE)

#define IAP_APFLASH_START             (IAP_CODE_SIZE + IAP_VERSION_SIZE)
#define IAP_APSRAM_START              (HT_SRAM_BASE)

#define BOOT_MODE_AP                  (0x55AAFAF0)
#define BOOT_MODE_IAP                 (0x55AAFAF5)

#define BOOT_MODE_ID_ADDR             (HT_SRAM_BASE)

#define BOOT_MODE                     ((*((u32 volatile *) (BOOT_MODE_ID_ADDR))))


#ifdef __cplusplus
}
#endif

#endif
