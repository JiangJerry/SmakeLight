/*********************************************************************************************************//**
 * @file    IAP/IAP_HID_II/Src_IAP/iap_main.c
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
#include "iap_ht32_usbd_class.h"
#include "iap_ht32_usbd_descriptor.h"
#include "iap_handler.h"
#include "iap_config.h"

#include "iap_uart_handler.h"

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


/* Private constants ---------------------------------------------------------------------------------------*/
#define TIMER_BASE                    (SystemCoreClock * 1 / 1000)  /* 1 ms                                 */

/* Private function prototypes -----------------------------------------------------------------------------*/
void CKCU_Configuration(void);
void USBD_Configuration(void);
void Suspend(u32 uPara);

/* Global variables ----------------------------------------------------------------------------------------*/
__ALIGN4 USBDCore_TypeDef gUSBCore;
USBD_Driver_TypeDef gUSBDriver;
u32 gIsLowPowerAllowed = TRUE;

u32 IAP_Update_Port;  //1=usb, 2=uart

#if (IAP_TIMEOUT_EN == 1)
s32 gIAPTimeout = IAP_TIMEOUT;
#endif

/* Global functions ----------------------------------------------------------------------------------------*/
/*********************************************************************************************************//**
  * @brief  Main program.
  * @retval None
  ***********************************************************************************************************/
int main(void)
{
  CKCU_Configuration();               /* System Related configuration                                       */
  RETARGET_Configuration();
  /*--------------------------------------------------------------------------------------------------------*/
  /* Use BOOT0 to decide start user application or IAP mode.                                                */
  /* Modify it if you want to use another GPIO pin.                                                         */
  /*--------------------------------------------------------------------------------------------------------*/
  #if 1
  if ((HT_FLASH->VMCR & 0x1) == 0x1)     /* The value of BOOT0 will be sampled to the VMCR                  */
                                         /* register of FMC after reset.                                    */
  #else
  /*--------------------------------------------------------------------------------------------------------*/
  /* Example that using Key1 to decide start user application or IAP mode.                                  */
  /* Key1 = Release: User application, Key1 = Pressed: IAP mode,                                            */
  /*--------------------------------------------------------------------------------------------------------*/
  GPIO_DirectionConfig(KEY1_BUTTON_GPIO_PORT, KEY1_BUTTON_GPIO_PIN, GPIO_DIR_IN);
  GPIO_InputConfig(KEY1_BUTTON_GPIO_PORT, KEY1_BUTTON_GPIO_PIN, ENABLE);
  if (GPIO_ReadInBit(KEY1_BUTTON_GPIO_PORT , KEY1_BUTTON_GPIO_PIN))
  #endif
  {
    #if (IAP_TIMEOUT_EN == 1)
    if (BOOT_MODE == BOOT_MODE_AP)
    {
      if ((IAP_isAPValid() == TRUE) && (IAP_isVersionValid() == TRUE))
      {
        /*--------------------------------------------------------------------------------------------------*/
        /* Start user application when                                                                      */
        /*   1. IO = 1 and                                                                                  */
        /*   2. BOOT_MODE == BOOT_MODE_AP (Default IAP) and                                                 */
        /*   3. SP and PC of user's application is in range                                                 */
        /*--------------------------------------------------------------------------------------------------*/
        IAP_GoCMD(IAP_APFLASH_START);
      }
    }
    else if (gIAPTimeout != IAP_TIMEOUT_OFF && BOOT_MODE != BOOT_MODE_IAP)
    {
      SYSTICK_ClockSourceConfig(SYSTICK_SRC_FCLK);
      SYSTICK_SetReloadValue(TIMER_BASE);
      SYSTICK_IntConfig(ENABLE);
      SYSTICK_CounterCmd(SYSTICK_COUNTER_ENABLE);
    }
    #else
    if (BOOT_MODE != BOOT_MODE_IAP)
    {
      if ((IAP_isAPValid() == TRUE) /*&& (IAP_isVersionValid() == TRUE)*/)
      {
        /*--------------------------------------------------------------------------------------------------*/
        /* Start user application when                                                                      */
        /*   1. IO = 1 and                                                                                  */
        /*   2. BOOT_MODE != BOOT_MODE_IAP (Default AP) and                                                 */
        /*   3. SP and PC of user's application is in range                                                 */
        /*--------------------------------------------------------------------------------------------------*/
        IAP_GoCMD(IAP_APFLASH_START);
      }
    }
    #endif
  }

  /*--------------------------------------------------------------------------------------------------------*/
  /* Start IAP mode                                                                                         */
  /*   1. IO = 0 or                                                                                         */
  /*   2. BOOT_MODE = BOOT_MODE_IAP                                                                         */
  /*--------------------------------------------------------------------------------------------------------*/
  IAP_Update_Port=0;
  IAP_Init();
  IAP_Uart_Init();
  
  #if (EXT_FLASH_ENABLE == 1)
  if (SPI_FLASH_Init() == TRUE)
  {
    SPI_FLASH_WriteStatus(0x00);
  }
  #endif

  HT32F_DVB_LEDInit(HT_LED1);
  HT32F_DVB_LEDInit(HT_LED2);
  HT32F_DVB_LEDInit(HT_LED3);

  USBD_Configuration();               /* USB device configuration                                           */
  
  while (1)
  {
    USBDCore_MainRoutine(&gUSBCore);
    IAP_Uart_Handler(); 
    //WDT_Restart();
  }
}

/*********************************************************************************************************//**
  * @brief  Configure the system clocks.
  * @retval None
  ***********************************************************************************************************/
void CKCU_Configuration(void)
{
#if 1
  CKCU_PeripClockConfig_TypeDef CKCUClock = {{ 0 }};
  CKCUClock.Bit.USBD       = 1;
  CKCUClock.Bit.AFIO       = 1;
  CKCUClock.Bit.EXTI       = 1;
  CKCU_PeripClockConfig(CKCUClock, ENABLE);
#endif
}

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
