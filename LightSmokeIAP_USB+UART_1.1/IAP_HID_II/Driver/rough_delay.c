/*********************************************************************************************************//**
 * @file    rough_delay.c
 * @version V1.00
 * @date    9/18/2016
 * @brief   The source file of mcu.
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
#include "ht32_board.h"
#include "ht32_board_config.h"
#include "rough_delay.h"

/********************************************************************************************************** */
/* Setting                                                                                                  */
/********************************************************************************************************** */

#ifndef MAX_HCLK
#define MAX_HCLK                (48000000)
#endif

#ifndef HCLK
#define HCLK                    MAX_HCLK
#endif

#ifndef WAIT_STATE0_MAX_FREQ
#define WAIT_STATE0_MAX_FREQ    (24000000)
#endif
#ifndef WAIT_STATE1_MAX_FREQ
#define WAIT_STATE1_MAX_FREQ    (48000000)
#endif
#ifndef WAIT_STATE2_MAX_FREQ
#define WAIT_STATE2_MAX_FREQ    (72000000)
#endif
#ifndef WAIT_STATE3_MAX_FREQ
#define WAIT_STATE3_MAX_FREQ    (96000000)
#endif

/* Private constants ---------------------------------------------------------------------------------------*/
/* Global functions ----------------------------------------------------------------------------------------*/

#pragma push
#pragma Ospace
#pragma O2

/*********************************************************************************************************//**
  * @brief  Dealy function.
  * @param  X: Dealy time based on uS
  * @retval None
  ***********************************************************************************************************/
void RoughDelay_XuS(u32 X)
{
  #if (HCLK > WAIT_STATE3_MAX_FREQ)
  X *= (((HCLK/1000000)/5) + 2);
  #elif (HCLK > WAIT_STATE2_MAX_FREQ)
  X *= (((HCLK/1000000)/4) + 2);
  #elif (HCLK > WAIT_STATE1_MAX_FREQ)
  X *= ((HCLK/1000000)/3);
  #else
  X *= ((HCLK/1000000)/2);
  #endif
  while (X--);
}

/*********************************************************************************************************//**
  * @brief  Dealy function.
  * @param  X: Dealy time based on uS
  * @retval None
  ***********************************************************************************************************/
void RoughDelay_XmS(u32 X)
{
  RoughDelay_XuS(X * 1000);
}
#pragma pop
