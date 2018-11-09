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
    
    AFIO_GPxConfig(GPIO_PA, BLEWAKEUP_PIN, AFIO_MODE_DEFAULT);                  //¿∂—¿ªΩ–—øÿ÷∆
    GPIO_DirectionConfig(BLEWAKEUP_PORT, BLEWAKEUP_PIN, GPIO_DIR_OUT);          
    GPIO_WriteOutBits(BLEWAKEUP_PORT, BLEWAKEUP_PIN,RESET);  
    
    AFIO_GPxConfig(GPIO_PB, KEY_PIN, AFIO_MODE_1);                              //∞¥º¸ ‰»Î
    GPIO_DirectionConfig(KEY_PORT, KEY_PIN, GPIO_DIR_IN);                       
    GPIO_InputConfig(KEY_PORT, KEY_PIN, ENABLE);
}


static unsigned char StaCnt[4];

u8 G_Input_Flag[4];
FlagStatus NewSta,OldSta[4];
//∂¡»°º¸÷µ
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
