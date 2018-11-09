
#ifndef _BATMANAGE_H
#define _BATMANAGE_H



#define LED_RED_PORT    (HT_GPIOC)
#define LED_RED_PIN     GPIO_PIN_3
#define LED_GREEN_PORT    (HT_GPIOC)
#define LED_GREEN_PIN     GPIO_PIN_1
#define LED_BLUE_PORT    (HT_GPIOC)
#define LED_BLUE_PIN     GPIO_PIN_2

#define LED_VCC_EN_PORT     (HT_GPIOA)
#define LED_VCC_EN_PIN     (GPIO_PIN_14)

extern FlagStatus ChargeFlag,StdbyFlag;


void LEDVCCClose(void);
void LEDVCCOpen(void);
void BatLedOrg(void);
void BatManageInit(void);


#endif	
