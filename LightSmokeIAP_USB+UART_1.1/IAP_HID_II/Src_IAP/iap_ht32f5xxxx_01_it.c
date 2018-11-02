/*********************************************************************************************************//**
 * @file    IAP/IAP_HID_II/Src_IAP/iap_ht32f5xxxx_01_it.c
 * @version $Rev:: 2524         $
 * @date    $Date:: 2018-01-29 #$
 * @brief   This file provides all interrupt service routine.
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

/* Includes ------------------------------------------------------------------------------------------------*/
#include "ht32.h"
#include "ht32_usbd_core.h"
#include "iap_handler.h"
#include "iap_config.h"

#include "iap_uart_handler.h"
#include "rough_delay.h"

#define delayXuS(X)               RoughDelay_XuS(X)

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


/* Global functions ----------------------------------------------------------------------------------------*/
/*********************************************************************************************************//**
 * @brief  This function handles SysTick Handler.
 * @retval None
 ************************************************************************************************************/
void SysTick_Handler(void)
{
  #if (IAP_TIMEOUT_EN == 1)
  extern s32 gIAPTimeout;
  if (gIAPTimeout > 0)
  {
    gIAPTimeout--;
    if (gIAPTimeout == 0)
    {
      /* Wait IAP command timeout. Reset and start application                                              */
      //IAP_Reset(0);
      if(IAP_Reset(0) == CMD_SUCCESS)
			{			
        ww(BOOT_MODE_ID_ADDR, BOOT_MODE_APP);        
			
				delayXuS(100000);
        
				NVIC_SystemReset();
			}
    }
  }
  #endif
}

/*********************************************************************************************************//**
 * @brief   This function handles USB interrupt.
 * @retval  None
 ************************************************************************************************************/
void USB_IRQHandler(void)
{
  __ALIGN4 extern USBDCore_TypeDef gUSBCore;
  USBDCore_IRQHandler(&gUSBCore);
}


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
