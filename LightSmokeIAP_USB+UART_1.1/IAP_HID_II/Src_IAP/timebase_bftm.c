/*********************************************************************************************************//**
 * @file    Src_AP/timebase_bftm.c
 * @version $Rev:: 295          $
 * @date    $Date:: 2016-03-10 #$
 * @brief   This file contains all keyboard body.
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
#include "timebase.h"
#include "iap_uart_handler.h"

/* Setting -------------------------------------------------------------------------------------------------*/

#define Timebase_BFTM_NAME        BFTM0

#define Timebase_BFTM             STRCAT2(HT_,  Timebase_BFTM_NAME)
#define Timebase_BFTM_IRQn        STRCAT2(Timebase_BFTM_NAME, _IRQn)
#define Timebase_BFTM_IRQHandler  STRCAT2(Timebase_BFTM_NAME, _IRQHandler)

#ifndef HCLK
#define HCLK                    (40000000)
#endif

#define RELOAD                  ((HCLK/TIMEBASE_FREQ) - 1)

/* Private typedef -----------------------------------------------------------------------------------------*/
/* Private constants ---------------------------------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------------------------------------*/
/* Global variables ----------------------------------------------------------------------------------------*/ 
volatile bool Timebase_CountFlag;
vu32 Timebase_Counter;

/*********************************************************************************************************//**
  * @brief  Timebase_Init
  * @retval None
  ***********************************************************************************************************/
void Timebase_Init(void)
{
  CKCU_PeripClockConfig_TypeDef CKCUClock = {{ 0 }};
  CKCUClock.Bit.Timebase_BFTM_NAME = 1;
  CKCU_PeripClockConfig(CKCUClock, ENABLE);

  Timebase_InterruptCmd(DISABLE);
  BFTM_SetCompare(Timebase_BFTM, RELOAD);
  BFTM_EnaCmd(Timebase_BFTM, ENABLE);
  NVIC_EnableIRQ(Timebase_BFTM_IRQn);
}

/*********************************************************************************************************//**
  * @brief  TBD
  * @param  TBD
  * @retval None
  ***********************************************************************************************************/
void Timebase_InterruptCmd(ControlStatus NewState)
{
  BFTM_IntConfig(Timebase_BFTM, NewState);
}

/*********************************************************************************************************//**
  * @brief  Timebase_DelayXClocks
  * @param  TBD
  * @retval None
  ***********************************************************************************************************/
#define TOLENCE   (16)
void Timebase_DelayXClocks(u32 X)
{
  u32 start_counter = Timebase_BFTM->CNTR;
  u32 this_cycle_remain;
   
  while(X > RELOAD)
  {
    X -= (RELOAD+1);  
    if(start_counter < TOLENCE)
    {
      while(Timebase_BFTM->CNTR < TOLENCE);
      while(Timebase_BFTM->CNTR >= TOLENCE);
      while(Timebase_BFTM->CNTR < start_counter);
    }
    else if(start_counter > (RELOAD - TOLENCE))
    {
      while(Timebase_BFTM->CNTR > start_counter);
      while(Timebase_BFTM->CNTR < (RELOAD - TOLENCE));
    }
    else
    {
      while(Timebase_BFTM->CNTR > start_counter);
      while(Timebase_BFTM->CNTR < start_counter);
    }
  }

  this_cycle_remain = (RELOAD+1) - start_counter;
    
  if(X > this_cycle_remain)
  {  
    X -= this_cycle_remain;
    if(start_counter < TOLENCE)
    {
      while(Timebase_BFTM->CNTR < TOLENCE);
      while(Timebase_BFTM->CNTR >= TOLENCE);
    }
    else if(start_counter > (RELOAD - TOLENCE))
    {
      while(Timebase_BFTM->CNTR >= (RELOAD - TOLENCE));
    }
    else
    {
      while(Timebase_BFTM->CNTR > start_counter);
    } 

    if(X > (RELOAD - TOLENCE))
    {
      while(Timebase_BFTM->CNTR < (RELOAD - TOLENCE));  
      while(Timebase_BFTM->CNTR > (RELOAD - TOLENCE));  
    }
    else
    {
      while(Timebase_BFTM->CNTR < X);  
    }
  }
  else
  {
    if((this_cycle_remain - X) > TOLENCE)
    {
      u32 tmp = start_counter + X;
      while(Timebase_BFTM->CNTR < tmp);  
    }
    else
    {
      while(Timebase_BFTM->CNTR < (RELOAD - TOLENCE));  
      while(Timebase_BFTM->CNTR > (RELOAD - TOLENCE));        
    }
  }
}

/*********************************************************************************************************//**
  * @brief  Timebase_DelayXuS
  * @param  TBD
  * @retval None
  ***********************************************************************************************************/
void Timebase_DelayXuS(u32 X)
{
  Timebase_DelayXClocks(X * TIMEBASE_1US_CLOCK);
}

/*********************************************************************************************************//**
  * @brief  Timebase_DelayXmS
  * @param  TBD
  * @retval None
  ***********************************************************************************************************/
void Timebase_DelayXmS(u32 X)
{
  while(X--)
  {
    Timebase_DelayXuS(1000);
  }
}

/*********************************************************************************************************//**
  * @brief  Timebase_BFTM_IRQHandler
  * @param  TBD
  * @retval None
  ***********************************************************************************************************/
void Timebase_BFTM_IRQHandler(void)
{
  Timebase_BFTM->SR = 0;  
  Timebase_Counter++;
  Timebase_CountFlag = TRUE;
  
  /* TODO */
  IAP_TimebaseHandler();
}

/* Private functions ---------------------------------------------------------------------------------------*/
