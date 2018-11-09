#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------------------------------------*/
#include "ht32.h"

/* Exported types ------------------------------------------------------------------------------------------*/
typedef struct
{
  u8 uRed;
  u8 uGreen;
  u8 uBlue;
} LED_RGB_Data_TypeDef;

typedef struct 
{
	u8	RecSta;
	u8	DataSize;
	u8	DataCnt;
	u8	DataChk;
	u8	TxSta;
    u8 	TxBusy;
    u8        Tx_Buf[20]; 
    u8        Rx_Buf[20]; 
    u8        Rx_Buf_Invalid[20];
    u8        Receive_Bytes;
    u8        Receive_Sign;
    u8        MS_1_Receive;
    u8        Receive_Bytes_Temp;
    u8        Receive_Enable;
}Communicate; 	  	

typedef enum
{
    NOSTATUS = 0,
    CHARG,
    STDBY,
    LOWBAT,
}_BAT_STATUS;

extern _BAT_STATUS BatSta;
extern Communicate APPFrame;

/* Exported constants --------------------------------------------------------------------------------------*/
#define      OUTPUT_TABLE1      1
#define      OUTPUT_TABLE2      2
#define      RESET_TABLE        3
#define      OFF_CONST          4

#define CODE_0      15
#define CODE_1      45

#define GPTMx       HT_GPTM0

/* Exported macro ------------------------------------------------------------------------------------------*/
#define LED_NUM 72

#define LED_MODE1   0
#define LED_MODE2   1
#define LED_MODE3   2
#define LED_MODE4   3
#define LED_MODE5   4
#define LED_MODE6   5

#define LED_ALTER       15

#define CONFIG_IN_NUM   1

#define EN_PRESS_FUNC   1

/* Exported functions --------------------------------------------------------------------------------------*/
void NVIC_Configuration(void);
void CKCU_Configuration(void);
void GPIO_Configuration(void);
void GPTM_Configuration(void);
void GPTM_PDMA_Configuration(void);
void LEDDisp(void);
void KeyIoInit(void);
void USART1Init(void);
void Tab0Encode(u8 bRED,u8 bGREEN,u8 bBLUE);
void NextLEDRGBEncode(u8 bRED,u8 bGREEN,u8 bBLUE);
void ColorDataRdy3(u8 Ratio);
void ColorDataRdy2(u8 Ratio);
void ColorDataRdy1(u8 Ratio,u8 type);
void ColorDataRdy4(u8 Ratio);
void ColorDataRdy5(u8 Ratio);
void ColorDataRdy6(u8 Ratio);
void ModifyColorTable1(void);
void ModifyColorTable2(void);
void ModifyColorTable3(void);
void ModifyColorTable4(void);    
void ModifyColorTable5(void);
void ModifyColorTable6(void);    
void GetKey(void);
void AckDataToBleModle(void);
void LEDModeDatePre(u8 DutyData);
void ColorRadioDnBak(void);
void PressBrightAdj(u8 PressData);
void ModityDnUpTable(void);
void WaitWakeUpKeyReset(void);
void AP_TimebaseHandler(void);
void ADCInit(void);
void ADCStart(void);
void ADCStop(void);
void ADCEnable(void);
void ADCDisable(void);


void TaskKey(void);
void TaskADC(void);
void TaskLEDLine(void);
void EnterSleep(void);

FlagStatus GetCHRGSta(void);
FlagStatus GetSTDBYSta(void);



void BatLedOrg(void);
void BatLedGreen(void);
void BatLedRed(void);

void BatLedOff(void);

#include "Sleep.h"
#include "AirPress.h"
#include "LedLine.h"
#include "BatManage.h"
#include "TaskLEDLine.h"
#include "TaskBLE.h"
#include "TaskLEDLine.h"
#include "KeyInput.h"

#ifdef __cplusplus
}
#endif

#endif
