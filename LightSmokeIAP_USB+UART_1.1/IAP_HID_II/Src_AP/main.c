/*********************************************************************************************************//**
 * @file    IAP/IAP_HID_II/Src_AP/main.c
 * @version $Rev:: 2524         $
 * @date    $Date:: 2018-01-29 #$
 * @brief   The main program of HID IAP example.
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
#include "ht32_usbd_core.h"
#include "ht32_usbd_class.h"
#include "ht32_usbd_descriptor.h"
#include "ap_handler.h"
#include "ap_config.h"

#include "ap_uart_handler.h"

#include "main.h"
#include "USART0Drive.h"


/** @addtogroup HT32_Series_Peripheral_Examples HT32 Peripheral Examples
  * @{
  */

/** @addtogroup IAP_HID IAP HID
  * @{
  */

/** @addtogroup IAP_HID_II IAP HID II (with USB Check buffer)
  * @{
  */

/** @addtogroup IAP_HID_II_AP AP Example
  * @{
  */


/* Private function prototypes -----------------------------------------------------------------------------*/
void CKCU_Configuration(void);
void USBD_Configuration(void);
void Suspend(u32 uPara);

void ShowTestMenu(void);
void FunctionHandler(u32 uCommand);

/* Global variables ----------------------------------------------------------------------------------------*/
__ALIGN4 USBDCore_TypeDef gUSBCore;
USBD_Driver_TypeDef gUSBDriver;
u32 gIsLowPowerAllowed = TRUE;

vu32 gParameter;
vu32 gParameterShadow = 1;
vu32 gTimebaseDownCnt;

/*********************************************************************************************************
** 函数名称: main
** 功能描述: 主函数
** 输　入: 	 无
** 输　出 :  无
** 全局变量: 无							  
** 调用模块: 无
********************************************************************************************************/
int main(void)
{
    NVIC_SetVectorTable(NVIC_VECTTABLE_FLASH, IAP_APFLASH_START);   // Set the Vector Table Offset
    CKCU_Configuration();                                           // System Related configuration
    SYSTICK_SetReloadValue(SystemCoreClock / 8/ 2/ 50);             // (CK_SYS/8/2/50) = 10ms  
    SYSTICK_IntConfig(ENABLE);                                      // Enable SYSTICK Interrupt 
    SYSTICK_CounterCmd(SYSTICK_COUNTER_CLEAR);
    SYSTICK_CounterCmd(SYSTICK_COUNTER_ENABLE);
	USBD_Configuration();   //USB模块初始化
    AP_Init();              //应用程序初始化
    AP_Uart_Init();         //串口初始化
    ADCInit();              //ADC初始化
    BatManageInit();        //电池管理初始化
    KeyIoInit();            //按键IO初始化
#if EN_PRESS_FUNC > 0    
    AirPressInit();         //初始化气压温度传感器
#endif
    LEDLineInit();          //LED灯带数据初始化 
    WakeUpSleepInit();      //睡眠模式1唤醒初始化
    while (1)                          
    {          
        USBDCore_MainRoutine(&gUSBCore);    //USB升级
        AP_Uart_Handler();                  //串口升级及回调函数
        TaskADC();
        TaskKey(); 
        TaskBLE();
        TaskLEDLine();
        TaskSleep();
        WDT_Restart();
    }
}

  

/*********************************************************************************************************//**
  * @brief  Show Test menu.
  * @retval None
  ***********************************************************************************************************/
void ShowTestMenu(void)
{
  printf("\r\n\r\n");
  printf(" Application\r\n");
  printf("--------------------------------\r\n");
  printf(" [1] Start IAP mode\r\n");
  printf(" [2] Hello World\r\n");
  printf(" [3] Enable Write Protection\r\n");
}

/*********************************************************************************************************//**
  * @brief  Function Handler.
  * @param  uCommand
  * @retval None
  ***********************************************************************************************************/
void FunctionHandler(u32 uCommand)
{
  u32 i;

  switch (uCommand)
  {
    case '1':
    {
      /*----------------------------------------------------------------------------------------------------*/
      /* Set BOOT_MODE as BOOT_MODE_IAP and trigger a reset to start IAP mode.                              */
      /*----------------------------------------------------------------------------------------------------*/
      BOOT_MODE = BOOT_MODE_IAP;
      NVIC_SystemReset();           /* Trigger reset                                                        */
      break;
    }
    case '2':
    {
      printf("\r\n");
      for (i = 0; i < 10; i++)
      {
        printf("Hello World %02d!\r\n", i);
      }
      break;
    }
    case '3':
    {
      FLASH_OptionByte Option;
      FLASH_GetOptionByteStatus(&Option);

      if (Option.MainSecurity == ENABLE || Option.OptionProtect == ENABLE)
      {
        printf("Security or Option Protection is enabled. Can not modify Option.\r\n");
      }
      else
      {
        FLASH_EraseOptionByte();
        FLASH_ProgramWordData(OB_PP0, 0x00000000);
        FLASH_ProgramWordData(OB_PP1, 0x00000000);
        FLASH_ProgramWordData(OB_PP2, 0x00000000);
        FLASH_ProgramWordData(OB_PP3, 0x00000000);
        FLASH_ProgramWordData(OB_CHECKSUM, 0x0 + 0x0 + 0x0 + 0x0 + 0xFFFFFFFF);

        printf("Write Protection enabled. Trigger reset to reload Option Byte\r\n");

        NVIC_SystemReset();
      }
      break;
    }
  }
}

/*********************************************************************************************************//**
  * @brief  Configure the system clocks.
  * @retval None
  ***********************************************************************************************************/
/*void CKCU_Configuration(void)
{
#if 1
  CKCU_PeripClockConfig_TypeDef CKCUClock = {{ 0 }};
  CKCUClock.Bit.USBD       = 1;
  CKCUClock.Bit.AFIO       = 1;
  CKCUClock.Bit.EXTI       = 1;
  CKCUClock.Bit.AFIO       = 1; 
  CKCUClock.Bit.PA         = 1;
  CKCUClock.Bit.PC         = 1;
  CKCU_PeripClockConfig(CKCUClock, ENABLE);
#endif
}*/

/*********************************************************************************************************//**
  * @brief  Configure the USB.
  * @retval None
  ***********************************************************************************************************/
void USBD_Configuration(void)
{
  gUSBCore.pDriver = (u32 *)&gUSBDriver;
  gUSBCore.Power.CallBack_Suspend.func  = Suspend;
  //gUSBCore.Power.CallBack_Suspend.uPara = (u32)NULL;
  USBDDesc_Init(&gUSBCore.Device.Desc);
  USBDClass_Init(&gUSBCore.Class);
  USBDCore_Init(&gUSBCore);

  //CKCU_HSIAutoTrimClkConfig(CKCU_ATC_USB);
  //CKCU_HSIAutoTrimCmd(ENABLE);

  NVIC_EnableIRQ(USB_IRQn);

  HT32F_DVB_USBConnect();
}

/*********************************************************************************************************//**
  * @brief  Suspend call back function which enter DeepSleep1
  * @param  uPara: Parameter for Call back function
  * @retval None
  ***********************************************************************************************************/
void Suspend(u32 uPara)
{
  u32 IsRemoteWakeupAllowed;

  if (gIsLowPowerAllowed)
  {
    /* Disable EXTI interrupt to prevent interrupt occurred after wakeup                                    */
    EXTI_IntConfig(WAKEUP_BUTTON_EXTI_CHANNEL, DISABLE);
    IsRemoteWakeupAllowed = USBDCore_GetRemoteWakeUpFeature(&gUSBCore);

    if (IsRemoteWakeupAllowed == TRUE)
    {
      /* Enable EXTI wake event and clear wakeup flag                                                       */
      EXTI_WakeupEventConfig(WAKEUP_BUTTON_EXTI_CHANNEL, EXTI_WAKEUP_LOW_LEVEL, ENABLE);
      EXTI_ClearWakeupFlag(WAKEUP_BUTTON_EXTI_CHANNEL);
    }

    __DBG_USBPrintf("%06ld >DEEPSLEEP\r\n", ++__DBG_USBCount);

    // Add your procedure here which disable related IO to reduce power consumption
    // ..................
    //

    /* For Bus powered device, you must enter DeepSleep1 when device has been suspended. For self-powered   */
    /* device, you may decide to enter DeepSleep1 or not depended on your application.                      */

    /* For the convenient during debugging and evaluation stage, the USBDCore_LowPower() is map to a null   */
    /* function by default. In the real product, you must map this function to the low power function of    */
    /* firmware library by setting USBDCORE_ENABLE_LOW_POWER as 1 (in the ht32fxxxx_usbdconf.h file).       */
    USBDCore_LowPower();

    // Add your procedure here which recovery related IO for application
    // ..................
    //

    __DBG_USBPrintf("%06ld <DEEPSLEEP\r\n", ++__DBG_USBCount);

    if (EXTI_GetWakeupFlagStatus(WAKEUP_BUTTON_EXTI_CHANNEL) == SET)
    {
      __DBG_USBPrintf("%06ld WAKEUP\r\n", ++__DBG_USBCount);
      if (IsRemoteWakeupAllowed == TRUE && USBDCore_IsSuspend(&gUSBCore) == TRUE)
      {
        USBDCore_TriggerRemoteWakeup();
      }
    }

    if (IsRemoteWakeupAllowed == TRUE)
    {
      /* Disable EXTI wake event and clear wakeup flag                                                      */
      EXTI_WakeupEventConfig(WAKEUP_BUTTON_EXTI_CHANNEL, EXTI_WAKEUP_LOW_LEVEL, DISABLE);
      EXTI_ClearWakeupFlag(WAKEUP_BUTTON_EXTI_CHANNEL);
    }

    /* Clear EXTI edge flag and enable EXTI interrupt                                                       */
    EXTI_ClearEdgeFlag(WAKEUP_BUTTON_EXTI_CHANNEL);
    EXTI_IntConfig(WAKEUP_BUTTON_EXTI_CHANNEL, ENABLE);
  }

  return;
}

#if (HT32_LIB_DEBUG == 1)
/*********************************************************************************************************//**
  * @brief  Report both the error name of the source file and the source line number.
  * @param  filename: pointer to the source file name.
  * @param  uline: error line source number.
  * @retval None
  ***********************************************************************************************************/
void assert_error(u8* filename, u32 uline)
{
  /*
     This function is called by IP library that the invalid parameters has been passed to the library API.
     Debug message can be added here.
     Example: printf("Parameter Error: file %s on line %d\r\n", filename, uline);
  */

  while (1)
  {
  }
}
#endif

