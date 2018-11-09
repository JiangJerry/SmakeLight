#include "ht32.h"
#include "ht32_board.h"
#include "main.h"
#include "BMP280Drive.h"

FlagStatus ChargeFlag = RESET,StdbyFlag = RESET;

void BatManageInit(void)
{
    AFIO_GPxConfig(GPIO_PC, LED_RED_PIN, AFIO_MODE_DEFAULT);                //LED指示灯脚位配置
    AFIO_GPxConfig(GPIO_PC, LED_GREEN_PIN, AFIO_MODE_DEFAULT);    
    AFIO_GPxConfig(GPIO_PC, LED_BLUE_PIN, AFIO_MODE_DEFAULT);
    GPIO_DirectionConfig(LED_RED_PORT, LED_RED_PIN, GPIO_DIR_OUT);
    GPIO_DirectionConfig(LED_GREEN_PORT, LED_GREEN_PIN, GPIO_DIR_OUT);
    GPIO_DirectionConfig(LED_BLUE_PORT, LED_BLUE_PIN, GPIO_DIR_OUT);
 

    GPIO_WriteOutBits(LED_RED_PORT, LED_RED_PIN, SET);	   
    GPIO_WriteOutBits(LED_GREEN_PORT, LED_GREEN_PIN, SET);	   
    GPIO_WriteOutBits(LED_BLUE_PORT, LED_BLUE_PIN, SET);	   
    
    GPIO_DirectionConfig(LED_VCC_EN_PORT, LED_VCC_EN_PIN, GPIO_DIR_OUT);        //LED灯电源控制
    GPIO_WriteOutBits(LED_VCC_EN_PORT,LED_VCC_EN_PIN, RESET);	   

    HT32F_DVB_PBInit(DC_CHARGE, BUTTON_MODE_EXTI);              //充电检测中断输入
    HT32F_DVB_PBInit(BUTTON_WAKEUP, BUTTON_MODE_EXTI);          //按键中断输入
}


void EXTI4_15_IRQHandler(void)
{
    if (EXTI_GetEdgeFlag(WAKEUP_BUTTON_EXTI_CHANNEL))
    {
        EXTI_ClearEdgeFlag(WAKEUP_BUTTON_EXTI_CHANNEL);
    }
    if (EXTI_GetEdgeFlag(DC_STDBY_EXTI_CHANNEL))
    {
        EXTI_ClearEdgeFlag(DC_STDBY_EXTI_CHANNEL);
        StdbyFlag = SET;
    }
    if (EXTI_GetEdgeFlag(DC_CHRG_EXTI_CHANNEL))
    {
        EXTI_ClearEdgeFlag(DC_CHRG_EXTI_CHANNEL);
        ChargeFlag = SET;
    }
}
//关闭灯带VCC电源
void LEDVCCClose(void)
{
    GPIO_WriteOutBits(LED_VCC_EN_PORT,LED_VCC_EN_PIN, RESET);
}
//打开灯带VCC电源
void LEDVCCOpen(void)
{
    GPIO_WriteOutBits(LED_VCC_EN_PORT,LED_VCC_EN_PIN, SET);	 
}


//读取充电脚电平
FlagStatus GetCHRGSta(void)
{
    return GPIO_ReadInBit(HT_GPIOB,TP4056_CHRG_PIN);
}
//读取充饱脚电平
FlagStatus GetSTDBYSta(void)
{
    return GPIO_ReadInBit(HT_GPIOB,TP4056_STDBY_PIN);
}
//显示橙灯
void BatLedOrg(void)
{
    GPIO_WriteOutBits(LED_RED_PORT, LED_RED_PIN, RESET);	   
    GPIO_WriteOutBits(LED_GREEN_PORT, LED_GREEN_PIN, RESET);	   
    GPIO_WriteOutBits(LED_BLUE_PORT, LED_BLUE_PIN, SET);	       
}
//显示绿灯
void BatLedGreen(void)
{                                                                                                 
    GPIO_WriteOutBits(LED_RED_PORT, LED_RED_PIN, SET);	   
    GPIO_WriteOutBits(LED_GREEN_PORT, LED_GREEN_PIN, RESET);	   
    GPIO_WriteOutBits(LED_BLUE_PORT, LED_BLUE_PIN, SET);	       
}
//显示红灯
void BatLedRed(void)
{
    GPIO_WriteOutBits(LED_RED_PORT, LED_RED_PIN, RESET);	   
    GPIO_WriteOutBits(LED_GREEN_PORT, LED_GREEN_PIN, SET);
    GPIO_WriteOutBits(LED_BLUE_PORT, LED_BLUE_PIN, SET);	       
}
//灭灯
void BatLedOff(void)
{
    GPIO_WriteOutBits(LED_RED_PORT, LED_RED_PIN, SET);	   
    GPIO_WriteOutBits(LED_GREEN_PORT, LED_GREEN_PIN, SET);	   
    GPIO_WriteOutBits(LED_BLUE_PORT, LED_BLUE_PIN, SET);	   
}

FlagStatus GetDCInFlag(void)
{
    return GPIO_ReadInBit(HT_GPIOC,DC_PIN);
}

