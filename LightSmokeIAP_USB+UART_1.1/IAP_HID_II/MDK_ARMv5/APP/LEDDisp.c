#include "ht32.h"
#include "ht32_board.h"
#include "main.h"
#include "BMP280Drive.h"



#define CODE_RESET  0

vu32 PDMAIntCnt, LEDTabIndex;
vu16 LEDEncodeTab[2][24], ghLED_RESET_TABLE[24],OFFTable[24];
vu16 *LEDCodeDisp, *LEDCodeUpdate;

vu8  wLED_Length = LED_NUM;


LED_RGB_Data_TypeDef  LEDLightAllBuffer[72],LEDLightAllBufferBak[72];

LED_RGB_Data_TypeDef  Light1_PingPongBuff[sizeof(LEDLightAllBuffer)/sizeof(LED_RGB_Data_TypeDef)];

u8 TaskLEDRdy,TaskLEDUpdate,LedMode = 0,i,Duty = 50,DutyBak,TaskKeyRdy,ShutDownFlag,EnPressFlag = 0,ShutDownFlag,AlertFlag,TaskADCRdy;

LED_RGB_Data_TypeDef   *LEDDispBuffPtr;

FlagStatus LEDDispOrNot = RESET,DCInFlag = RESET,DCExitSleep = RESET;
u16 DlySet = 60,DlyTime;

extern u8 G_Input_Flag[];
LED_RGB_Data_TypeDef *PtrBak;
void LEDALLOff(void)
{
    PDMAIntCnt = 0;
    LEDTabIndex  = OFF_CONST;                       //空闲时间发送复位数据，否则DMA中断发送的数据就一直是上面表的数据
    while(PDMAIntCnt < 200);
    LEDTabIndex  = RESET_TABLE;                     //空闲时间发送复位数据，否则DMA中断发送的数据就一直是上面表的数据
    while(PDMAIntCnt < 200);   
}
//原计划可以通过改变最大最小值，实时变化亮度，用下面这段代码，发现不可行
void LEDModeMaxUpdate(void)
{
    switch(LedMode)
    {
        case LED_MODE1:
            ColorDataRdy1(Duty,1);
            break;
        case LED_MODE2:
            ColorDataRdy2(Duty);
            break;
        case LED_MODE3:
            ColorDataRdy3(Duty);         //第一种模式全亮显示           
            break;                           
        case LED_MODE4:
            ColorDataRdy4(Duty);         //第一种模式全亮显示           
            break;
        case LED_MODE5:
            ColorDataRdy5(Duty);         //第一种模式全亮显示           
            break;
        case LED_MODE6:
            ColorDataRdy6(Duty);         //第一种模式全亮显示           
            break;
        case LED_ALTER:                     //延时时间后的警告
            AlterDataRdy(100);  
            break;
    }
}

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
            ColorDataRdy3(DutyData);         //第一种模式全亮显示           
            break;                           
        case LED_MODE4:
            ColorDataRdy4(DutyData);         //第一种模式全亮显示           
            break;
        case LED_MODE5:
            ColorDataRdy5(DutyData);         //第一种模式全亮显示           
            break;
        case LED_MODE6:
            ColorDataRdy6(DutyData);         //第一种模式全亮显示           
            break;
        case LED_ALTER:                     //延时时间后的警告
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
** 函数名称: LEDInit
** 功能描述: LED数据IO初始化
** 输　入: 	 i
** 输　出 :  无
** 全局变量: 无							  
** 调用模块: 无
********************************************************************************************************/
void LEDInit(void)
{
    
    AFIO_GPxConfig(GPIO_PA, AFIO_PIN_4, AFIO_MODE_4);   // GPTM AFIO pin Setting 

    GPTM_Configuration();       //PWM定时器初始设置
    GPTM_PDMA_Configuration();  //DMA初始设置
    for (i = 0; i < 24; i++)//Reset 24bit LED output data buff
    {
        LEDEncodeTab[0][i] = 0;
        LEDEncodeTab[1][i] = 0;
        LEDEncodeTab[2][i] = 0;

        LEDEncodeTab[0][i] = 0;
        LEDEncodeTab[1][i] = 0; 
        LEDEncodeTab[2][i] = 0;

        ghLED_RESET_TABLE[i] = 0;
        OFFTable[i] = CODE_0;   //灭状态编码赋值
    }
    LEDDispOrNot = RESET;       //初始灭状态
    LEDALLOff();
	EnPressFlag = 1;
	DutyBak = Duty;
	Duty = Duty * 0.8;

    LedMode = LED_MODE1;
    PtrBak = &LEDLightAllBufferBak[0];
    LEDDispBuffPtr = (LED_RGB_Data_TypeDef*)&LEDLightAllBuffer[0];//基色指针   
    LEDModeDatePre(Duty);  
	GetInitPressure();//获取初始值
	
}
/*********************************************************************************************************
** 函数名称: LEDDisp
** 功能描述: LED显示
** 输　入: 	 i
** 输　出 :  无
** 全局变量: 无							  
** 调用模块: 无
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
void EnterSleep(void)
{
    u32 wRtcCounterTmp;
    u8 Flag = 0;
    ADCDisable();
    GPIO_WriteOutBits(BLEWAKEUP_PORT, BLEWAKEUP_PIN, SET);  //蓝牙进入睡眠模式 
    LEDVCCClose();                                          //关闭焕彩灯电源
    WaitWakeUpKeyReset();									//IO等待
    EXTI_WakeupEventConfig(DC_CHRG_EXTI_CHANNEL, EXTI_WAKEUP_LOW_LEVEL, ENABLE);
    EXTI_WakeupEventConfig(WAKEUP_BUTTON_EXTI_CHANNEL, EXTI_WAKEUP_HIGH_LEVEL, ENABLE);
    EXTI_WakeupEventIntConfig(ENABLE);
    PWRCU_DeepSleep1(PWRCU_SLEEP_ENTRY_WFI);    
    ADCEnable();
}

void LEDDisp(void)
{
	
    static u8 AlterBrighss = 100,AlterTimes = 0,LedModeBak,Flag;
    u8 i,NextDataIndex,NextOutputTab,LEDTIndex;
    float Temp = 0;
   
    if(TaskKeyRdy)
    {
        TaskKeyRdy = 0;
        GetKey();    
#if EN_PRESS_FUNC > 0    
        GetPressure();  //获取压力值  
#endif        
//        if((WakeUpType == 1) && (BatSta == NOSTATUS))
//        {
//            EnterSleep();
//        }        
        
    }
    
    if(TaskADCRdy)
    {
        TaskADCRdy = 0;
        ADCStart();
        switch(BatSta)
        {
            case CHARG:
                BatLedOrg();
                break;
            case STDBY:
                BatLedGreen();
                break;
            case LOWBAT:
                BatLedRed();
                break;
            default:
                BatLedOff();
                break;           
        }
    }
    
    if(TaskLEDRdy)
    {
        TaskLEDRdy = 0;        
        if(G_Input_Flag[0] == 1)    //按键按下
        {
            G_Input_Flag[0] = 0;
#if EN_PRESS_FUNC > 0                
			GetInitPressure();		//获取初始值
#endif            
            if(LEDDispOrNot == SET)
            {
                LEDALLOff();											//灭灯
                LEDDispOrNot = RESET;									//灭灯标志
                EnterSleep();
            }
            else
            {
                DCExitSleep = RESET;
                LEDVCCOpen();
                GPIO_WriteOutBits(BLEWAKEUP_PORT, BLEWAKEUP_PIN, RESET);    //唤醒蓝牙	   
                LEDDispOrNot = SET;
                DlyTime = DlySet;
//                LedMode++;
//                LedMode = LedMode%6;
                LEDModeDatePre(Duty);            
            }
        }

        if(LEDDispOrNot == SET)
        {
            if(APPFrame.Receive_Sign == 1)              //收到帧数据
            {
                APPFrame.Receive_Sign = 0;
                if(APPFrame.Rx_Buf[0] == 0xa0)          //模式命令
                {
                    LedMode = APPFrame.Rx_Buf[1]-1;
					LEDModeDatePre(Duty);               
                }
                if(APPFrame.Rx_Buf[0] == 0xa1)          //亮度命令
                {
//                    Temp = APPFrame.Rx_Buf[1];
//                    Temp = (Temp*100)/100;
					if(APPFrame.Rx_Buf[1] > 100)
					{
						APPFrame.Rx_Buf[1] = 100;
					}
                    Duty = APPFrame.Rx_Buf[1];
					DutyBak = Duty;
                    if(Duty < 5)
                    {
                        Duty = 5;
                    }
					LEDModeDatePre(Duty);
                }
				if(APPFrame.Rx_Buf[0] == 0xa2)			//气压传感器有效命令
				{
					if((APPFrame.Rx_Buf[1] == 1)&&(EnPressFlag==0))//打开压力传感器时最大值*0.8
					{
						EnPressFlag = 1;
						DutyBak = Duty;		//备份之前的亮度
						Duty = Duty * 0.8;
					}
					else
					{
						EnPressFlag = 0;
						Duty = DutyBak;		//亮度还原
					}
					LEDModeDatePre(Duty);
				}
                DlyTime = DlySet;                            
                AckDataToBleModle();    //反馈数据给BLE模块
            }
//            if(DlyTime == 0) 
//            {
//                DlyTime = 0xff;     //防止再次进入
//                LedModeBak = LedMode;
//                LedMode = LED_ALTER;
//                LEDModeDatePre(Duty);                                
//            }
           
            Tab0Encode(LEDDispBuffPtr[0].uRed,LEDDispBuffPtr[0].uGreen,LEDDispBuffPtr[0].uBlue);//第一个灯的颜色转换为24bit数据

            NextOutputTab = OUTPUT_TABLE1;                      //通知显示是第一张表

            for (LEDTIndex = 0; LEDTIndex < wLED_Length; LEDTIndex++)
            {                 
                PDMAIntCnt   = 0;
                LEDTabIndex  = NextOutputTab;
                while (LEDTabIndex == NextOutputTab);           //等待表显示完
                if ((LEDTIndex % 2) == 0)                       //第一块赋值
                {
                    LEDCodeDisp = (vu16*)(&LEDEncodeTab[0][0]);
                    LEDCodeUpdate = (vu16*)(&LEDEncodeTab[1][0]);
                    NextOutputTab = OUTPUT_TABLE2;
                }
                else if ((LEDTIndex % 2) == 1)                  //第二块赋值
                {
                    LEDCodeDisp = (vu16*)(&LEDEncodeTab[1][0]);
                    LEDCodeUpdate = (vu16*)(&LEDEncodeTab[0][0]);
                    NextOutputTab = OUTPUT_TABLE1;
                }

                if ((LEDTIndex + 1) < wLED_Length )             //准备下次显示的数据
                {
                    NextDataIndex   = LEDTIndex + 1;
                    NextLEDRGBEncode(LEDDispBuffPtr[NextDataIndex].uRed,LEDDispBuffPtr[NextDataIndex].uGreen,LEDDispBuffPtr[NextDataIndex].uBlue);              
                }
            }
                                                                               
            LEDTabIndex  = RESET_TABLE;                         //空闲时间发送复位数据，否则DMA中断发送的数据就一直是上面表的数据
			if((TaskLEDUpdate)||(LedMode == LED_ALTER))
			{
				TaskLEDUpdate = 0;
				if(PressDnFlag == 1)		//气压变小标志置位
				{
					ModityDnUpTable();
				}
				else
				{
					switch(LedMode)                     //改变颜色
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
							ColorDataRdy6(Duty);         //每次变换颜色需重新赋值，不然出现不亮的情况，原因还没有找到                            
							break;
						case LED_ALTER:
							AlterDataRdy(AlterBrighss--);   
							if(AlterBrighss == 0)
							{
								AlterBrighss = 100;
								AlterTimes ++;
								if(AlterTimes >= 2) //闪烁2次
								{
									AlterTimes = 0;                            
									LedMode = LedModeBak;
		//							LedMode = LedMode%6;
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
