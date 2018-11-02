/*********************************************************************************************************//**
 * @file    timebase.h
 * @version $Rev:: 295          $
 * @date    $Date:: 2016-03-10 #$
 * @brief   The header file of keyboard realted function.
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
#ifndef __TIMEBASE_H
#define __TIMEBASE_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------------------------------------*/
#include "ht32.h"

/* Setting -------------------------------------------------------------------------------------------------*/

//  <o0> Timebase Frequency (Hz)
//  <o1> Timebase 1 uS used clocks
#define TIMEBASE_FREQ                  (1000)
#define TIMEBASE_1US_CLOCK             (40)
#define TIMEBASE_US                    (1000000 / TIMEBASE_FREQ)

// <<< end of configuration section >>>

/* Exported variables --------------------------------------------------------------------------------------*/
extern volatile bool Timebase_CountFlag;
extern vu32 Timebase_Counter;
   
/* Exported functions --------------------------------------------------------------------------------------*/
void Timebase_Init(void);
void Timebase_InterruptCmd(ControlStatus NewStat);

void Timebase_DelayXClocks(u32 X);
void Timebase_DelayXuS(u32 X);
void Timebase_DelayXmS(u32 X);

#ifdef __cplusplus
}
#endif

#endif
