#include "ht32.h"
#include "ht32_board.h"
#include "main.h"
#include "BMP280Drive.h"

FlagStatus ChargeFlag = RESET,StdbyFlag = RESET;

void BatManageInit(void)
{
    AFIO_GPxConfig(GPIO_PC, LED_RED_PIN, AFIO_MODE_DEFAULT);                //LEDָʾ�ƽ�λ����
    AFIO_GPxConfig(GPIO_PC, LED_GREEN_PIN, AFIO_MODE_DEFAULT);    
    AFIO_GPxConfig(GPIO_PC, LED_BLUE_PIN, AFIO_MODE_DEFAULT);
    GPIO_DirectionConfig(LED_RED_PORT, LED_RED_PIN, GPIO_DIR_OUT);
    GPIO_DirectionConfig(LED_GREEN_PORT, LED_GREEN_PIN, GPIO_DIR_OUT);
    GPIO_DirectionConfig(LED_BLUE_PORT, LED_BLUE_PIN, GPIO_DIR_OUT);
 

    GPIO_WriteOutBits(LED_RED_PORT, LED_RED_PIN, SET);	   
    GPIO_WriteOutBits(LED_GREEN_PORT, LED_GREEN_PIN, SET);	   
    GPIO_WriteOutBits(LED_BLUE_PORT, LED_BLUE_PIN, SET);	   
    
    GPIO_DirectionConfig(LED_VCC_EN_PORT, LED_VCC_EN_PIN, GPIO_DIR_OUT);        //LED�Ƶ�Դ����
    GPIO_WriteOutBits(LED_VCC_EN_PORT,LED_VCC_EN_PIN, RESET);	   

    HT32F_DVB_PBInit(DC_CHARGE, BUTTON_MODE_EXTI);              //������ж�����
    HT32F_DVB_PBInit(BUTTON_WAKEUP, BUTTON_MODE_EXTI);          //�����ж�����
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
//�رյƴ�VCC��Դ
void LEDVCCClose(void)
{
    GPIO_WriteOutBits(LED_VCC_EN_PORT,LED_VCC_EN_PIN, RESET);
}
//�򿪵ƴ�VCC��Դ
void LEDVCCOpen(void)
{
    GPIO_WriteOutBits(LED_VCC_EN_PORT,LED_VCC_EN_PIN, SET);	 
}


//��ȡ���ŵ�ƽ
FlagStatus GetCHRGSta(void)
{
    return GPIO_ReadInBit(HT_GPIOB,TP4056_CHRG_PIN);
}
//��ȡ�䱥�ŵ�ƽ
FlagStatus GetSTDBYSta(void)
{
    return GPIO_ReadInBit(HT_GPIOB,TP4056_STDBY_PIN);
}
//��ʾ�ȵ�
void BatLedOrg(void)
{
    GPIO_WriteOutBits(LED_RED_PORT, LED_RED_PIN, RESET);	   
    GPIO_WriteOutBits(LED_GREEN_PORT, LED_GREEN_PIN, RESET);	   
    GPIO_WriteOutBits(LED_BLUE_PORT, LED_BLUE_PIN, SET);	       
}
//��ʾ�̵�
void BatLedGreen(void)
{                                                                                                 
    GPIO_WriteOutBits(LED_RED_PORT, LED_RED_PIN, SET);	   
    GPIO_WriteOutBits(LED_GREEN_PORT, LED_GREEN_PIN, RESET);	   
    GPIO_WriteOutBits(LED_BLUE_PORT, LED_BLUE_PIN, SET);	       
}
//��ʾ���
void BatLedRed(void)
{
    GPIO_WriteOutBits(LED_RED_PORT, LED_RED_PIN, RESET);	   
    GPIO_WriteOutBits(LED_GREEN_PORT, LED_GREEN_PIN, SET);
    GPIO_WriteOutBits(LED_BLUE_PORT, LED_BLUE_PIN, SET);	       
}
//���
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

