/*********************************************************************************************************//**
 * @file    mcu.h
 * @version V1.0
 * @date    09/18/2016
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

#ifndef __MCU_H
#define __MCU_H

#ifdef __cplusplus
 extern "C" {
#endif


#if defined(MCU_52220)
#include "mcu_52220.h"
#elif defined(MCU_52230)
#include "mcu_52230.h"
   
#elif defined(MCU_52231)
#include "mcu_52231.h"
#elif defined(MCU_52241)
#include "mcu_52241.h"

#elif defined(MCU_52331) 
#include "mcu_52331.h"  
#elif defined(MCU_52341)
#include "mcu_52341.h"

#elif defined(MCU_52342)
#include "mcu_52342.h"
#elif defined(MCU_52352)
#include "mcu_52352.h"
   
#else
 #error "Need mcu_xxxxx.h header file"
#endif
   
   
/* Exported macro ------------------------------------------------------------------------------------------*/
/* Exported functions --------------------------------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif


#endif
