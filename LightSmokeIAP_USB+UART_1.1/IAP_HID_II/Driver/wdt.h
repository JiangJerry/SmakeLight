/*********************************************************************************************************//**
 * @file    wdt.h
 * @version $Rev:: 823          $
 * @date    $Date:: 2016-06-20 #$
 * @brief   The header file of USB Device Class.
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
#ifndef __WDT_H
#define __WDT_H

/* ---------------------------------------------------------------------------------------------------------*/

//  <o0.0> Watch Dog Timer Enable
//  <o1.0> Watch Dog Timer Test Enable
#define WDT_EN                  (0)
#define WDT_TST                 (0)
#define WDT_CLR_LOOPS           (1000)

#if (WDT_EN == 1)
  #define WDT_RESTART() WDT_Restart();
#else
  #define WDT_RESTART(...)
#endif

#endif
