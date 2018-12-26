/*********************************************************************************************************//**
 * @file    IAP/IAP_HID_II/Src_AP/ht32f5xxxx_01_it.c
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
#include "ht32_board.h"
#include "ht32_usbd_core.h"
#include "main.h"
#include "BMP280Drive.h"

/*********************************************************************************************************//**
 * @brief   This function handles SysTick Handler.
 * @retval  None,20ms中断一次
 ************************************************************************************************************/
extern u8 TaskLEDLineRdy,TaskKeyRdy,TaskLEDUpdate,Duty,TaskADCRdy,TaskBLERdy,EnterSleepDly;
extern u16 DlySet,DlyTime;
u8 LowBatFlag = 0;
extern FlagStatus TaskSleepRdy,EnterSleepPreFlag;

u8 SFlag = 0;;
#define COUNT_1S    100  

extern FlagStatus ChargeFlag,StdbyFlag;

void SysTick_Handler(void)
{    
    static u16 CntFor1s = 0;
	static u8 CntLEDScan = 0;
	static u8 DutyScnt = 0;
    
    AP_TimebaseHandler();
    if(CntFor1s++ >= COUNT_1S)
    {
        CntFor1s = 0;
        TaskADCRdy = 1;
        if((DlyTime > 0)&&(DlySet != 0xff)&&(LedMode != LED_ALTER)) //模式灯显示时间递减
        {
            DlyTime--;
        }        
        BatSta = NOSTATUS;
        if(ChargeFlag == SET)
        {
            ChargeFlag = RESET;
            BatSta = CHARG;         //充电状态
        }
        if(StdbyFlag == SET)
        {
            StdbyFlag = RESET;           
            BatSta = STDBY;         //充电状态
        }    
        if(LowBatFlag == 1)         //检测是否底电压，亮红灯
        {
            BatSta = LOWBAT;        //底电压状态
        }
        if(EnterSleepPreFlag == SET)
            EnterSleepDly++;
        else
            EnterSleepDly = 0;
    }
	CntLEDScan++;
	if(CntLEDScan >= 4)	    //客户要求的扫描时间
	{
		SFlag = 1;
		TaskLEDLineRdy = 1; //LED扫描任务就绪
		CntLEDScan = 0;
		DutyScnt++;
		if(Duty > 60)
		{
			TaskLEDUpdate = 1;
			DutyScnt = 0;
		}
		else if(Duty >= 40)
		{
			if(DutyScnt >= 2)
			{
				TaskLEDUpdate = 1;
				DutyScnt = 0;				
			}
		}
		else if(Duty >= 30)
		{
			if(DutyScnt >= 3)
			{
				TaskLEDUpdate = 1;
				DutyScnt = 0;				
			}
		}
		else if(Duty >= 20)
		{
			if(DutyScnt >= 5)
			{
				TaskLEDUpdate = 1;
				DutyScnt = 0;				
			}
		}
		else
		{
			if(DutyScnt >= 10)
			{
				TaskLEDUpdate = 1;
				DutyScnt = 0;				
			}
			
		}
	}
    TaskKeyRdy = 1; //按键扫描任务就绪
    TaskBLERdy = 1;
  //HT32F_DVB_LEDToggle((LED_TypeDef)(LedFlag % 1));
  //LedFlag++;
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

#define LOW_BAT 2171    //3.5V
vu32 VbatValu;
/*********************************************************************************************************//**
 * @brief   This function handles ADC interrupt.
 * @retval  None
 ************************************************************************************************************/
void ADC_IRQHandler(void)
{
    static u8 LowBatCnt = 0;
    ADC_ClearIntPendingBit(HT_ADC, ADC_FLAG_SINGLE_EOC);
    VbatValu = ((HT_ADC->DR[0]&0x0FFF));
    if(VbatValu < LOW_BAT)
    {
        LowBatCnt++;
        if(LowBatCnt >= 20) //20秒均小于，判断电池电压底
        {
            LowBatFlag = 1;
        }
    }
    else
    {
        LowBatCnt = 0;
    }
    
}
/*********************************************************************************************************//**
 * @brief   This function handles RTC interrupt.
 * @retval  None
 * @details In RTC interrupt service routine:
 *  - Reset RTC init time when Time is 23:59:59.
 *    - Set RTC Compare register as 86400.
 *    - Reset PWRCU_BAKREG_1 to 0.
 *  - Toggle LED1 each 1s.
 *  - Set gwTimeDisplay to 1 to enable time update.
 *
 ************************************************************************************************************/
void RTC_IRQHandler(void)
{
    extern vu32 gwTimeDisplay;
    static u8 Flag = 0;
    u8 bFlags;

    bFlags = RTC_GetFlagStatus();
//  if((bFlags & 0x2) != 0x0) /* Match flag */
//  {
//    /* Reset RTC init time when Time is 23:59:59 */
//    RTC_SetCompare(86400);
//    TimeReg = 0;
//  }
    if(Flag == 0)
    {
        Flag = 1;
        GPIO_WriteOutBits(LED_GREEN_PORT, LED_GREEN_PIN, RESET);	   
    }
    else
    {
        Flag = 0;
        GPIO_WriteOutBits(LED_GREEN_PORT, LED_GREEN_PIN, SET);	   
    }

  /* Toggle LED1 each 1s */
  HT32F_DVB_LEDToggle(HT_LED1);

  /* Enable time update */
//  gwTimeDisplay = 1;
}

/*********************************************************************************************************//**
 * @brief   This function handles PDMA interrupt.
 * @retval  None
 ************************************************************************************************************/
void PDMA_CH0_1_IRQHandler(void)
{
  extern vu32 PDMAIntCnt, LEDTabIndex;
  extern vu16 LEDEncodeTab[2][24], ghLED_RESET_TABLE[24],OFFTable[24];
/*
*  When PDMA_CHx data is transfer completed(PDMA_CHx TC event),
*  user need to change the PDMA_CHx source address to the LED_x + 1 TX BUFF address
*  to display next LED_x + 1 data.
*
*  LEDEncodeTab[0][0][X]: GT_CH0 TX BUFF[0]
*  LEDEncodeTab[0][1][X]: GT_CH0 TX BUFF[1]
*  LEDEncodeTab[1][0][X]: GT_CH1 TX BUFF[0]
*  LEDEncodeTab[1][1][X]: GT_CH1 TX BUFF[1]
*  ghLED_RESET_TABLE[X]:             RESET STATE BUFF
*  PDMAIntCnt[0]:                PDMA_CH1(GT_CH0) interrupt counter
*  PDMAIntCnt[1]:                PDMA_CH0(GT_CH1) interrupt counter
*/
    if ((HT_PDMA->ISR & 0x0120) )
    {
        HT_PDMA->ISCR = 0x0100;  // clear PDMA_CH1 TC_Flag

        if (LEDTabIndex == OUTPUT_TABLE1)
        {
            HT_PDMA->PDMACH1.SADR = (u32)&LEDEncodeTab[0][23];
            LEDTabIndex = 0;     //中断完成后清内存标识位
        }
        else if (LEDTabIndex == OUTPUT_TABLE2)
        {
            HT_PDMA->PDMACH1.SADR = (u32)&LEDEncodeTab[1][23];
            LEDTabIndex = 0; //中断完成后清内存标识位
        }
        else if (LEDTabIndex == RESET_TABLE)
        {
            HT_PDMA->PDMACH1.SADR = (u32)&ghLED_RESET_TABLE[23];
            LEDTabIndex = RESET_TABLE;
        }
        else if(LEDTabIndex == OFF_CONST)
        {
            HT_PDMA->PDMACH1.SADR = (u32)&OFFTable[23];
            LEDTabIndex = OFF_CONST;
            
        }
        PDMAIntCnt++;       //DMA中断次数累加
    }
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


///*********************************************************************************************************//**
// * @file    IAP/IAP_HID_II/Src_AP/ht32f5xxxx_01_it.c
// * @version $Rev:: 2524         $
// * @date    $Date:: 2018-01-29 #$
// * @brief   This file provides all interrupt service routine.
// *************************************************************************************************************
// * @attention
// *
// * Firmware Disclaimer Information
// *
// * 1. The customer hereby acknowledges and agrees that the program technical documentation, including the
// *    code, which is supplied by Holtek Semiconductor Inc., (hereinafter referred to as "HOLTEK") is the
// *    proprietary and confidential intellectual property of HOLTEK, and is protected by copyright law and
// *    other intellectual property laws.
// *
// * 2. The customer hereby acknowledges and agrees that the program technical documentation, including the
// *    code, is confidential information belonging to HOLTEK, and must not be disclosed to any third parties
// *    other than HOLTEK and the customer.
// *
// * 3. The program technical documentation, including the code, is provided "as is" and for customer reference
// *    only. After delivery by HOLTEK, the customer shall use the program technical documentation, including
// *    the code, at their own risk. HOLTEK disclaims any expressed, implied or statutory warranties, including
// *    the warranties of merchantability, satisfactory quality and fitness for a particular purpose.
// *
// * <h2><center>Copyright (C) Holtek Semiconductor Inc. All rights reserved</center></h2>
// ************************************************************************************************************/

///* Includes ------------------------------------------------------------------------------------------------*/
//#include "ht32.h"
//#include "ht32_board.h"
//#include "ht32_usbd_core.h"

///** @addtogroup HT32_Series_Peripheral_Examples HT32 Peripheral Examples
//  * @{
//  */

///** @addtogroup IAP_HID IAP HID
//  * @{
//  */

///** @addtogroup IAP_HID_II IAP HID II (with USB Check buffer)
//  * @{
//  */

///** @addtogroup IAP_HID_II_AP AP Example
//  * @{
//  */


///* Private variables ---------------------------------------------------------------------------------------*/
//static u32 LedFlag = 0;

///* Global functions ----------------------------------------------------------------------------------------*/
///*********************************************************************************************************//**
// * @brief   This function handles SysTick Handler.
// * @retval  None
// ************************************************************************************************************/
//void SysTick_Handler(void)
//{
//  HT32F_DVB_LEDToggle((LED_TypeDef)(LedFlag % 3));
//  LedFlag++;
//}

///*********************************************************************************************************//**
// * @brief   This function handles USB interrupt.
// * @retval  None
// ************************************************************************************************************/
//void USB_IRQHandler(void)
//{
//  __ALIGN4 extern USBDCore_TypeDef gUSBCore;
//  USBDCore_IRQHandler(&gUSBCore);
//}


///**
//  * @}
//  */

///**
//  * @}
//  */

///**
//  * @}
//  */

///**
//  * @}
//  */
