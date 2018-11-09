#include "ht32.h"
#include "ht32_board.h"
#include "main.h"
#include "BMP280Drive.h"

extern FlagStatus NewSta,OldSta[4];


u8 WakeUpType = 0;

FlagStatus TaskSleepRdy;
/*********************************************************************************************************//**
  * @brief  Configures RTC clock source and prescaler.
  * @retval None
  * @details The RTC configuration as following:
  *   - Check the backup domain(RTC & PWRCU) is ready for access.
  *   - Reset Backup Domain.
  *   - Enable the LSE OSC and wait till LSE is ready.
  *   - Select the RTC Clock Source as LSE.
  *   - Set the RTC time base to 1 second.
  *   - Set Compare value.
  *   - Enable the RTC.
  *   - Enable the RTC Compare Match wakeup event.
  *
  ***********************************************************************************************************/
void RTC_Configuration(void)
{
#ifndef USE_HT32F52230_SK
  /* Check the backup domain(RTC & PWRCU) is ready for access                                               */
    #if (!LIBCFG_NO_PWRCU_TEST_REG)
    if (PWRCU_CheckReadyAccessed() != PWRCU_OK)
    {
      while (1);
    }
    #endif
  /* Reset Backup Domain                                                                                    */
  PWRCU_DeInit();
  /* Enable the LSE OSC                                                                                     */
  RTC_LSESMConfig(RTC_LSESM_NORMAL);
//  RTC_LSECmd(ENABLE);
  /* Wait till LSE is ready                                                                                 */
  while (CKCU_GetClockReadyStatus(CKCU_FLAG_LSIRDY) == RESET);
  /* Select the RTC Clock Source as LSE                                                                     */
  RTC_ClockSourceConfig(RTC_SRC_LSI);
  /* Set the RTC time base to 1s                                                                            */
  RTC_SetPrescaler(RTC_RPRE_32768);
  /* Set Compare value                                                                                      */
  RTC_SetCompare(0xFF);
  /* Enable the RTC                                                                                         */
  RTC_Cmd(ENABLE);
  /* Enable the RTC Compare Match wakeup event                                                              */
  RTC_WakeupConfig(RTC_WAKEUP_CM, DISABLE);
#endif
}


void WakeUpSleepInit(void)
{
    RTC_Configuration();    //RTC时钟设置
    NVIC_SetPriority(EVWUP_IRQn, 0xF);
    NVIC_EnableIRQ(EVWUP_IRQn);
}

void EVWUP_IRQHandler(void)
{     
    if (EXTI_GetWakeupFlagStatus(DC_CHRG_EXTI_CHANNEL))
    {
        EXTI_WakeupEventConfig(DC_CHRG_EXTI_CHANNEL, EXTI_WAKEUP_LOW_LEVEL, DISABLE);
        EXTI_ClearWakeupFlag(DC_CHRG_EXTI_CHANNEL);
        WakeUpType = 1;    
    }  
    if (EXTI_GetWakeupFlagStatus(WAKEUP_BUTTON_EXTI_CHANNEL))
    {
        EXTI_WakeupEventConfig(WAKEUP_BUTTON_EXTI_CHANNEL, EXTI_WAKEUP_HIGH_LEVEL, DISABLE);
        EXTI_ClearWakeupFlag(WAKEUP_BUTTON_EXTI_CHANNEL);
        WakeUpType = 2;    
    }         
}


void WaitWakeUpKeyReset(void)
{
	while(GPIO_ReadInBit(HT_GPIOB,KEY_PIN) == SET);			//等待按键抬起
	OldSta[0] = 2;
}

void EnterSleep(void)
{
    SetBleStatus(0);     //关闭广播
    ADCDisable();
    GPIO_WriteOutBits(BLEWAKEUP_PORT, BLEWAKEUP_PIN, SET);  //蓝牙进入睡眠模式 
    LEDVCCClose();                                          //关闭焕彩灯电源
    WaitWakeUpKeyReset();									//IO等待
    EXTI_WakeupEventConfig(DC_CHRG_EXTI_CHANNEL, EXTI_WAKEUP_LOW_LEVEL, ENABLE);
    EXTI_WakeupEventConfig(WAKEUP_BUTTON_EXTI_CHANNEL, EXTI_WAKEUP_HIGH_LEVEL, ENABLE);
    EXTI_WakeupEventIntConfig(ENABLE);
    PWRCU_DeepSleep1(PWRCU_SLEEP_ENTRY_WFI);
    ADCEnable();
    SetBleStatus(1);     //关闭广播
}
FlagStatus EnterSleepPreFlag = RESET;
u8 EnterSleepDly;

void SleepPrepare(void)
{
    EnterSleepPreFlag = SET;
}

void TaskSleep(void)
{
    if((EnterSleepDly >= 5)&&(LEDDispOrNot == RESET))
    {
        EnterSleepDly = 0;
        EnterSleepPreFlag = RESET;
        TaskSleepRdy = SET;
    }
    if(TaskSleepRdy)
    {
        TaskSleepRdy = RESET;
        EnterSleep();
    }
}
