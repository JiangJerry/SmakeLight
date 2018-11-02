#include "ht32.h"
#include "ht32_board.h"
#include "main.h"
#include "BMP280Drive.h"


void KeyIoInit(void)
{
    /* Configure AFIO mode of output pins                                                       */   
           
    AFIO_GPxConfig(GPIO_PA, GPIO_PIN_1, AFIO_MODE_DEFAULT);
    GPIO_DirectionConfig(HT_GPIOA, GPIO_PIN_1, GPIO_DIR_IN);
    /* Configure LED output pins                                                                */
    
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

    /* Configure GPIO direction of output pins                                                                */
    AFIO_GPxConfig(GPIO_PB, SCK_PIN, AFIO_MODE_DEFAULT);
    AFIO_GPxConfig(GPIO_PB, SDA_PIN, AFIO_MODE_DEFAULT);
    GPIO_DirectionConfig(SCK_PORT, SCK_PIN, GPIO_DIR_OUT);
    GPIO_DirectionConfig(SDA_PORT, SDA_PIN, GPIO_DIR_OUT);
    
    AFIO_GPxConfig(GPIO_PA, EN_PIN, AFIO_MODE_DEFAULT);                         //使能LED焕彩灯控制
    GPIO_DirectionConfig(SDA_PORT, EN_PIN, GPIO_DIR_OUT);
    
    AFIO_GPxConfig(GPIO_PA, BLEWAKEUP_PIN, AFIO_MODE_DEFAULT);                  //蓝牙唤醒控制
    GPIO_DirectionConfig(BLEWAKEUP_PORT, BLEWAKEUP_PIN, GPIO_DIR_OUT);          
    GPIO_WriteOutBits(BLEWAKEUP_PORT, BLEWAKEUP_PIN,RESET);
   
    
    AFIO_GPxConfig(GPIO_PB, KEY_PIN, AFIO_MODE_1);                              //按键输入
    GPIO_DirectionConfig(KEY_PORT, KEY_PIN, GPIO_DIR_IN);                       
    GPIO_InputConfig(KEY_PORT, KEY_PIN, ENABLE);


//    AFIO_GPxConfig(GPIO_PB, TP4056_CHRG_PIN, AFIO_MODE_DEFAULT);
//    GPIO_DirectionConfig(TP4056_CHRG_PORT, TP4056_CHRG_PIN, GPIO_DIR_IN);       //正在充电中状态
//    GPIO_InputConfig(TP4056_CHRG_PORT, TP4056_CHRG_PIN, ENABLE);
//    GPIO_PullResistorConfig(TP4056_CHRG_PORT, TP4056_CHRG_PIN, GPIO_PR_DISABLE);

//    AFIO_GPxConfig(GPIO_PB, TP4056_STDBY_PIN, AFIO_MODE_DEFAULT);
//    GPIO_DirectionConfig(TP4056_STDBY_PORT, TP4056_STDBY_PIN, GPIO_DIR_IN);     //充电完成状态
//    GPIO_InputConfig(TP4056_STDBY_PORT, TP4056_STDBY_PIN, ENABLE);
//    GPIO_PullResistorConfig(TP4056_STDBY_PORT, TP4056_STDBY_PIN, GPIO_PR_DISABLE);


    AFIO_GPxConfig(GPIO_PC, DC_PIN, AFIO_MODE_DEFAULT);
    GPIO_DirectionConfig(DC_PORT, DC_PIN, GPIO_DIR_IN);                         //DC输入
    GPIO_InputConfig(DC_PORT, DC_PIN, ENABLE);
    GPIO_PullResistorConfig(DC_PORT, DC_PIN, GPIO_PR_UP);
    
    HT32F_DVB_PBInit(DC_CHARGE, BUTTON_MODE_EXTI);      //充电检测中断输入
    HT32F_DVB_PBInit(BUTTON_WAKEUP, BUTTON_MODE_EXTI);       //按键中断输入
//    BatLedRed();
}

FlagStatus ChargeFlag = RESET,StdbyFlag = RESET;

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

static unsigned char StaCnt[4];

u8 G_Input_Flag[4];
FlagStatus NewSta,OldSta[4];

void WaitWakeUpKeyReset(void)
{
	while(GPIO_ReadInBit(HT_GPIOB,KEY_PIN) == SET);			//等待按键抬起
	OldSta[0] = 2;
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
//读取键值
void GetKey(void)
{
    u8 i;
    for(i=0;i<1;i++)
    {
        if(i == 0)
            NewSta = !GPIO_ReadInBit(HT_GPIOB,KEY_PIN);
        if(i == 1)
            NewSta = GPIO_ReadInBit(HT_GPIOC,DC_PIN);
        if(i == 2)
            NewSta = GPIO_ReadInBit(HT_GPIOB,TP4056_CHRG_PIN);
        if(i == 3)
            NewSta = GPIO_ReadInBit(HT_GPIOB,TP4056_STDBY_PIN);
            
        if(OldSta[i] != NewSta)
            StaCnt[i] ++;
        else
            StaCnt[i] = 0;

        if(StaCnt[i] > 10)
        {
            if(NewSta == RESET)
            {
                G_Input_Flag[i] = 1;
            }
            else
            {
                G_Input_Flag[i] = 2;
            }
            OldSta[i] = NewSta;
        }
    }
}
