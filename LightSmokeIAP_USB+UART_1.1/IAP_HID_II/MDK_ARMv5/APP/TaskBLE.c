#include "ht32.h"
#include "ht32_board.h"
#include "main.h"
#include "BMP280Drive.h"
#include "USART0Drive.h"

#define SET_MODE            0xA0
#define SET_BRIGHTNESS      0XA1
#define SET_LED_LINE_OFF    0XA2
#define SET_PRESSER_SENSOR  0XA3
#define SET_ALTER_TIME      0XA4

u8 TaskBLERdy;

#define HEAD0   0XEA
#define HEAD1   0X15

extern u8 TaskLEDRdy,LedMode,i,Duty,EnPressFlag,ShutDownFlag,AlertFlag;
void USART_Tx(const char* TxBuffer, u32 length);
unsigned char UR0TxBuf[USART_TXSIZE];


void SetBleStatus(u8 Sta)
{
    u8 i;
    UR0TxBuf[0] = HEAD0; 
    UR0TxBuf[1] = HEAD1; 
    UR0TxBuf[2] = 0; 
    UR0TxBuf[3] = 9; 
    UR0TxBuf[4] = 0x30; 
    UR0TxBuf[5] = Sta; 
    UR0TxBuf[6] = 0; 
    for(i=0;i<7;i++)    
        UR0TxBuf[6] += UR0TxBuf[i]; 
    UR0TxBuf[7] = 0xf5; 
    UR0TxBuf[8] = 0x5f; 
    
    USART_Tx(&UR0TxBuf[0],9);    
}


void AckDataToBleModle(void)
{
    u8 i;
    UR0TxBuf[0] = HEAD0; 
    UR0TxBuf[1] = HEAD1; 
    UR0TxBuf[2] = 0; 
    UR0TxBuf[3] = 13; 
    UR0TxBuf[4] = 0xb0; 
    UR0TxBuf[5] = LedMode; 
    UR0TxBuf[6] = Duty; 
    UR0TxBuf[7] = ShutDownFlag; 
    UR0TxBuf[8] = EnPressFlag; 
    UR0TxBuf[9] = AlertFlag; 
        UR0TxBuf[10] = 0; 
    for(i=0;i<10;i++)    
        UR0TxBuf[10] += UR0TxBuf[i]; 
    UR0TxBuf[11] = 0xf5; 
    UR0TxBuf[12] = 0x5f; 
    
    USART_Tx(&UR0TxBuf[0],USART_TXSIZE);
}

Communicate APPFrame;

void APPFrameRxCallBack(u8 RxBuf)
{
    switch (APPFrame.RecSta) 
    {
        case REC_INIT:
            if(RxBuf == SYN_HEAD)
            {
                APPFrame.RecSta = REC_CMD;
                APPFrame.DataChk = RxBuf;                
            }
            else
            {
                APPFrame.RecSta = REC_INIT;
            }
            break;                   
        case REC_CMD:
            if(RxBuf == SYN_CMD)
            {
                APPFrame.RecSta = REC_DATA_SIZE0;
                APPFrame.DataChk += RxBuf;                
            }
            else
            {
                APPFrame.RecSta = REC_INIT;
            }
            break;
        case REC_DATA_SIZE0:
            APPFrame.RecSta = REC_DATA_SIZE1;
            APPFrame.DataChk += RxBuf; 
            break;
        case REC_DATA_SIZE1:
        	if((RxBuf == DATA_SIZE))
        	{
	            APPFrame.RecSta = REC_DATA_CTX;
                APPFrame.DataSize = 2;
                APPFrame.DataCnt = 0;
                APPFrame.DataChk += RxBuf; 
	       	}
	       	else
	       	{
                APPFrame.RecSta = REC_INIT;	       	
	       	}
	            
            break;
        case REC_DATA_CTX:
            if(APPFrame.DataCnt >= (APPFrame.DataSize))
            {
            	if(APPFrame.DataChk == RxBuf)
				{
                    APPFrame.Receive_Sign = 1;
				}
                APPFrame.RecSta = REC_INIT;
            }
            else
            {
	            APPFrame.DataChk += RxBuf;
	        }
            APPFrame.Rx_Buf[APPFrame.DataCnt++] = RxBuf;
	        break;
        default:
            APPFrame.RecSta = REC_INIT;
            break;
    }
}


void TaskBLE(void)
{
    if(TaskBLERdy)
    {
        if(APPFrame.Receive_Sign == 1)              //收到帧数据
        {
            APPFrame.Receive_Sign = 0;
            switch(APPFrame.Rx_Buf[0])
            {
                case SET_MODE:                      //模式命令
                    LedMode = APPFrame.Rx_Buf[1] - 1;
                    LEDModeDatePre(Duty);               
                    break;
                case SET_BRIGHTNESS:                //亮度命令
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
                    break;
                case SET_LED_LINE_OFF:              //关机
                    LEDALLOff();					//灭灯
                    EnterSleep();                   //睡眠
                    break;
                case SET_PRESSER_SENSOR:            //气压传感器有效命令
                    if((APPFrame.Rx_Buf[1] == 1))
                    {
                        EnPressFlag = 1;
                    }
                    else
                    {
                        EnPressFlag = 0;
                    }
                    break;                
                case SET_ALTER_TIME:                //设置报警时间
                    DlySet = APPFrame.Rx_Buf[1];
                    DlySet = (DlySet<<4) - DlySet;  //实现*15 
                    DlyTime = DlySet;                  
                    break;
            }
            AckDataToBleModle();                    //反馈数据给BLE模块
        }  

    }
    
}
