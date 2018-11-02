/*********************************************************************************************************//**
 * @file    LCD_Demo/DemoProject/board.h
 * @version V1.0
 * @date    08/28/2013
 * @brief   Header file for board hardware related defination.
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

#ifndef __DEBUG_H
#define __DEBUG_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------------------------------------*/
#include "ht32.h"

//  <e0> Debug Enable
// </e>
#define DBG_EN                          1

#if (DBG_EN == 0)
#define DBG_Init(...)
#define DBG_PutChar(...)
#define DBG_PutString(...)
#define DBG_PutBuffer(...)
#define DBG_PrintError(...)
#define DBG_Print(...)
#else
#define DBG_Init        _DBG_Init
#define DBG_PutChar     _DBG_PutChar
#define DBG_PutString   _DBG_PutString
#define DBG_PutBuffer   _DBG_PutBuffer
#define DBG_PrintError  _DBG_PrintError
#define DBG_Print       _DBG_Print
#endif

// <<< end of configuration section >>>

/* Exported macro ------------------------------------------------------------------------------------------*/
/* Exported functions --------------------------------------------------------------------------------------*/
void _DBG_Init(void);
char _DBG_PutChar(char ch);
void _DBG_PutString(char* string);
void _DBG_PutBuffer(char* buffer, u32 length);
void _DBG_PrintError(char* fmt, ...);
void _DBG_Print(char* fmt, ...);

#ifdef __cplusplus
}
#endif


#endif
