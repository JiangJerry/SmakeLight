#include "ht32.h"
#include "ht32_board.h"
#include "main.h"
#include "BMP280Drive.h"

void AlterDataRdy(u8 Radio);

#define CODE_RESET  0

vu32 PDMAIntCnt, LEDTabIndex;
vu16 LEDEncodeTab[2][24], ghLED_RESET_TABLE[24],OFFTable[24];
vu16 *LEDCodeDisp, *LEDCodeUpdate;

vu8  wLED_Length = LED_NUM;

LED_RGB_Data_TypeDef  LEDLightAllBuffer[72],LEDLightAllBufferBak[72];

LED_RGB_Data_TypeDef  Light1_PingPongBuff[sizeof(LEDLightAllBuffer)/sizeof(LED_RGB_Data_TypeDef)];

u8 TaskLEDLineRdy,TaskLEDUpdate,LedMode = 0,i,Duty = 50,DutyBak,ShutDownFlag,EnPressFlag = 0,ShutDownFlag,AlertFlag;

LED_RGB_Data_TypeDef   *LEDDispBuffPtr;

FlagStatus LEDDispOrNot = RESET,DCInFlag = RESET,DCExitSleep = RESET;
u16 DlySet = 255,DlyTime;

extern u8 G_Input_Flag[];
LED_RGB_Data_TypeDef *PtrBak;
/*********************************************************************************************************
** ��������: LEDALLOff
** ��������: ���
** �䡡��: 	 i
** �䡡�� :  ��
** ȫ�ֱ���: ��							  
** ����ģ��: ��
********************************************************************************************************/
void LEDALLOff(void)
{   
    PDMAIntCnt = 0;
    LEDTabIndex  = OFF_CONST;                       //����ʱ�䷢�͸�λ���ݣ�����DMA�жϷ��͵����ݾ�һֱ������������
    while(PDMAIntCnt < 200);
    LEDTabIndex  = RESET_TABLE;                     //����ʱ�䷢�͸�λ���ݣ�����DMA�жϷ��͵����ݾ�һֱ������������
    while(PDMAIntCnt < 200);   
    LEDDispOrNot = RESET;									//��Ʊ�־    
}
/*********************************************************************************************************
** ��������: LEDALLOff
** ��������: ���
** �䡡��: 	 i
** �䡡�� :  ��
** ȫ�ֱ���: ��							  
** ����ģ��: ��
********************************************************************************************************/
void LEDModeDatePre(u8 DutyData)
{
	u8 i;
	LED_RGB_Data_TypeDef *PtrCurrent;
	PtrCurrent = &LEDLightAllBuffer[0];
	
    switch(LedMode)
    {
        case LED_MODE1:
            ColorDataRdy1(DutyData,0);
            break;
        case LED_MODE2:
            ColorDataRdy2(DutyData);
            break;
        case LED_MODE3:
            ColorDataRdy3(DutyData);         //��һ��ģʽȫ����ʾ           
            break;                           
        case LED_MODE4:
            ColorDataRdy4(DutyData);         //��һ��ģʽȫ����ʾ           
            break;
        case LED_MODE5:
            ColorDataRdy5(DutyData);         //��һ��ģʽȫ����ʾ           
            break;
        case LED_MODE6:
            ColorDataRdy6(DutyData);         //��һ��ģʽȫ����ʾ           
            break;
        case LED_ALTER:                     //��ʱʱ���ľ���
            AlterDataRdy(100);  
            break;
    }
	for(i=0;i<72;i++)
	{
		LEDLightAllBufferBak[i].uBlue = LEDLightAllBuffer[i].uBlue;
		LEDLightAllBufferBak[i].uGreen = LEDLightAllBuffer[i].uGreen;
		LEDLightAllBufferBak[i].uRed = LEDLightAllBuffer[i].uRed;
	}
}
/*********************************************************************************************************
** ��������: LEDLineInit
** ��������: LED����IO��ʼ��
** �䡡��: 	 i
** �䡡�� :  ��
** ȫ�ֱ���: ��							  
** ����ģ��: ��
********************************************************************************************************/
void LEDLineInit(void)
{ 
    AFIO_GPxConfig(GPIO_PA, AFIO_PIN_4, AFIO_MODE_4);   // GPTM AFIO pin Setting 
    GPTM_Configuration();                               //PWM��ʱ����ʼ����
    GPTM_PDMA_Configuration();                          //DMA��ʼ����
    for (i = 0; i < 24; i++)                            //Reset 24bit LED output data buff
    {
        LEDEncodeTab[0][i] = 0;
        LEDEncodeTab[1][i] = 0;
        LEDEncodeTab[2][i] = 0;

        LEDEncodeTab[0][i] = 0;
        LEDEncodeTab[1][i] = 0; 
        LEDEncodeTab[2][i] = 0;

        ghLED_RESET_TABLE[i] = 0;
        OFFTable[i] = CODE_0;   //��״̬���븳ֵ
    }
    LEDALLOff();
	DutyBak = Duty;
	Duty = Duty * 0.8;

    LedMode = LED_MODE1;
    PtrBak = &LEDLightAllBufferBak[0];
    LEDDispBuffPtr = (LED_RGB_Data_TypeDef*)&LEDLightAllBuffer[0];  //��ɫָ��   
    LEDModeDatePre(Duty);  
#if EN_PRESS_FUNC > 0                    
	GetInitPressure();                                              //��ȡ��ʼֵ	
#endif    
}
/*********************************************************************************************************
** ��������: LEDDisp
** ��������: LED��ʾ
** �䡡��: 	 i
** �䡡�� :  ��
** ȫ�ֱ���: ��							  
** ����ģ��: ��
********************************************************************************************************/
extern u8 SFlag;
extern u8 PressDn,PressDnFlag,LowBatFlag;
u8 PressDnBak = 0,BATSta = 0;


extern u8 WakeUpType;

_BAT_STATUS BatSta;

void Delay(u8 i)
{
    for(;i>0;i--);
}


extern FlagStatus TaskSleepRdy;

void TaskLEDLine(void)
{
	
    static u8 AlterBrighss = 100,AlterTimes = 0,LedModeBak,Flag;
    u8 i,NextDataIndex,NextOutputTab,LEDTIndex;
    float Temp = 0;
    
    if(TaskLEDLineRdy)
    {
        TaskLEDLineRdy = 0;        
        if(G_Input_Flag[0] == 1)    //��������
        {
            G_Input_Flag[0] = 0;
#if EN_PRESS_FUNC > 0                
			GetInitPressure();		//��ȡ��ʼֵ
#endif            
            if(LEDDispOrNot == SET)
            {
                LEDALLOff();											//���
                TaskSleepRdy = SET;
            }
            else
            {
                DCExitSleep = RESET;
                LEDVCCOpen();
                GPIO_WriteOutBits(BLEWAKEUP_PORT, BLEWAKEUP_PIN, RESET);    //��������	   
                LEDDispOrNot = SET;
                DlyTime = DlySet;
                LEDModeDatePre(Duty);            
            }
        }

        if(LEDDispOrNot == SET)
        {
            if((DlyTime == 0)&& (DlySet != 3825))
            {
                DlyTime = DlySet;     //��ֹ�ٴν���
                LedModeBak = LedMode;
                LedMode = LED_ALTER;
                LEDModeDatePre(Duty); 
            }
           
            Tab0Encode(LEDDispBuffPtr[0].uRed,LEDDispBuffPtr[0].uGreen,LEDDispBuffPtr[0].uBlue);//��һ���Ƶ���ɫת��Ϊ24bit����

            NextOutputTab = OUTPUT_TABLE1;                      //֪ͨ��ʾ�ǵ�һ�ű�

            for (LEDTIndex = 0; LEDTIndex < wLED_Length; LEDTIndex++)
            {                 
                PDMAIntCnt   = 0;
                LEDTabIndex  = NextOutputTab;
                while (LEDTabIndex == NextOutputTab);           //�ȴ�����ʾ��
                if ((LEDTIndex % 2) == 0)                       //��һ�鸳ֵ
                {
                    LEDCodeDisp = (vu16*)(&LEDEncodeTab[0][0]);
                    LEDCodeUpdate = (vu16*)(&LEDEncodeTab[1][0]);
                    NextOutputTab = OUTPUT_TABLE2;
                }
                else if ((LEDTIndex % 2) == 1)                  //�ڶ��鸳ֵ
                {
                    LEDCodeDisp = (vu16*)(&LEDEncodeTab[1][0]);
                    LEDCodeUpdate = (vu16*)(&LEDEncodeTab[0][0]);
                    NextOutputTab = OUTPUT_TABLE1;
                }

                if ((LEDTIndex + 1) < wLED_Length )             //׼���´���ʾ������
                {
                    NextDataIndex   = LEDTIndex + 1;
                    NextLEDRGBEncode(LEDDispBuffPtr[NextDataIndex].uRed,LEDDispBuffPtr[NextDataIndex].uGreen,LEDDispBuffPtr[NextDataIndex].uBlue);              
                }
            }
                                                                               
            LEDTabIndex  = RESET_TABLE;                         //����ʱ�䷢�͸�λ���ݣ�����DMA�жϷ��͵����ݾ�һֱ������������
			if((TaskLEDUpdate)||(LedMode == LED_ALTER))
			{
				TaskLEDUpdate = 0;
				if(PressDnFlag == 1)		//��ѹ��С��־��λ
				{
					ModityDnUpTable();
				}
				else
				{
					switch(LedMode)                     //�ı���ɫ
					{
						case LED_MODE1:
							ModifyColorTable1();					
							break;
						case LED_MODE2:
							ModifyColorTable2();
							break;
						case LED_MODE3:
							ModifyColorTable3();
							break;
						case LED_MODE4:
							ModifyColorTable4();
							break;
						case LED_MODE5:
							ModifyColorTable5();
							break;
						case LED_MODE6:
							ColorDataRdy6(Duty);         //ÿ�α任��ɫ�����¸�ֵ����Ȼ���ֲ����������ԭ��û���ҵ�                            
							break;
						case LED_ALTER:
                            if(AlterTimes == 0)
                            {
                                AlterDataRdy(AlterBrighss--);
                                if(AlterBrighss == 0)
                                {
                                    AlterTimes = 1;
                                }                               
                            }
                            else
                            {
                                AlterDataRdy(AlterBrighss++);
                                if(AlterBrighss == 100)
                                {
									AlterTimes = 0;                            
									LedMode = LedModeBak;
									LEDModeDatePre(Duty);
									DlyTime = DlySet;                            
                                }
                            }
							break;
					}
				}
			}
        }
    }          
}
