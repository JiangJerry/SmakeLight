/*********************************************************************************************************//**
 * @file    buffer.h
 * @version $Rev:: 2524         $
 * @date    $Date:: 2018-01-29 #$
 * @brief   The header file of buffer.
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
#ifndef __BUFFER_H
#define __BUFFER_H

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


/* Exported types ------------------------------------------------------------------------------------------*/
/**
 * @brief Definition of buffer index
 */
typedef struct
{
  u32 uRead;
  vu32 uWrite;
  u8 *puMemory;
  u32 uSize;
} Buffer_TypeDef;

/* Exported functions --------------------------------------------------------------------------------------*/
void Buffer_Init(Buffer_TypeDef *pBuffer, u8 *puMemory, u32 uSize);
u32 Buffer_ReadByte(Buffer_TypeDef *pBuffer, u8 *puData);
u32 Buffer_WriteByte(Buffer_TypeDef *pBuffer, u32 uData);
u32 Buffer_Read(Buffer_TypeDef *pBuffer, u8 *puData, u32 uLength);
u32 Buffer_Write(Buffer_TypeDef *pBuffer, u8 *puData, u32 uLength);
u32 Buffer_isEmpty(Buffer_TypeDef *pBuffer);
u32 Buffer_isFull(Buffer_TypeDef *pBuffer);
void Buffer_Discard(Buffer_TypeDef *pBuffer);


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
