#include <string.h>
#include "BMP280Drive.h"
#include  <math.h>    //Keil library  
#include  <stdlib.h>  //Keil library  
#include  <stdio.h>   //Keil library

#include "USART0Drive.h"

#define   uchar unsigned char
#define   uint unsigned int	

#define	BMP280_SlaveAddress   0xEE	  //定义器件在IIC总线中的从地址                               
    //存储BMP180数据的结构
_bmp180 bmp180;

#define OSS 0	// Oversampling Setting (note: code is not set up to use other OSS values)


long PressSum = 0,PressCnt = 0,PressCurrent;
extern u8 DutyBak,Duty,EnPressFlag;
long PressSumInitValu;

u8 PressDn;

unsigned short dig_T1;
short dig_T2;
short dig_T3;
unsigned short dig_P1;
short dig_P2;
short dig_P3;
short dig_P4;
short dig_P5;
short dig_P6;
short dig_P7;
short dig_P8;
short dig_P9;

//**************************************************************
//初始化BMP280，根据需要请参考pdf进行修改**************
void AirPressInit(void)
{
	unsigned short temp = 0;
    /* Configure GPIO direction of output pins                                                                */
    AFIO_GPxConfig(GPIO_PB, SCK_PIN, AFIO_MODE_DEFAULT);
    AFIO_GPxConfig(GPIO_PB, SDA_PIN, AFIO_MODE_DEFAULT);
    GPIO_DirectionConfig(SCK_PORT, SCK_PIN, GPIO_DIR_OUT);
    GPIO_DirectionConfig(SDA_PORT, SDA_PIN, GPIO_DIR_OUT);
    
    while(temp != 0X58)
    {    
        temp = BMP_ReadOneByte(0xd0);//读取id、判断iic是否正常模块是否正常，读取出来的地址是0x58就是对的
    }
	dig_T1 = BMP_ReadTwoByte(0x88);
	dig_T2 = BMP_ReadTwoByte(0x8A);
	dig_T3 = BMP_ReadTwoByte(0x8C);
	dig_P1 = BMP_ReadTwoByte(0x8E);
	dig_P2 = BMP_ReadTwoByte(0x90);
	dig_P3 = BMP_ReadTwoByte(0x92);
	dig_P4 = BMP_ReadTwoByte(0x94);
	dig_P5 = BMP_ReadTwoByte(0x96);
	dig_P6 = BMP_ReadTwoByte(0x98);
	dig_P7 = BMP_ReadTwoByte(0x9A);
	dig_P8 = BMP_ReadTwoByte(0x9C);
	dig_P9 = BMP_ReadTwoByte(0x9E);

	BMP_WriteOneByte(0xe0,0xb6);	//复位
	BMP_WriteOneByte(0xf4,0xff);	//设置采集精度
	//t_sb[7-5]:000 0.5ms转换一次 filter[4-2] 111 
	BMP_WriteOneByte(0xf5,0X1c);	//设置滤波值
	GetInitPressure();//获取初始值
	EnPressFlag = 1;
	
}
//***********************************************************************

long adc_T;
long adc_P;
long var1, var2,t_fine,T,p;

long bmp280Convert(void)
{
	adc_T = Multiple_three_read(0xFA);    // 读取温度
	adc_P = Multiple_three_read(0xF7); // 读取压强
	if(adc_P == 0)
	{
		return 0;
	}
	//Temperature
	var1 = (((double)adc_T)/16384.0-((double)dig_T1)/1024.0)*((double)dig_T2);
	var2 = ((((double)adc_T)/131072.0-((double)dig_T1)/8192.0)*(((double)adc_T)/131072.0-((double)dig_T1)/8192.0))*((double)dig_T3);	
	t_fine = (u32)(var1+var2);	
	T = (var1+var2)/5120.0;
	T = T;	
	var1 = ((double)t_fine/2.0)-64000.0;	
	var2 = var1*var1*((double)dig_P6)/32768.0;	
	var2 = var2+var1*((double)dig_P5)*2.0;	
	var2 = (var2/4.0)+(((double)dig_P4)*65536.0);	
	var1 = (((double)dig_P3)*var1*var1/524288.0+((double)dig_P2)*var1)/524288.0;	
	var1 = (1.0+var1/32768.0)*((double)dig_P1);	
	p = 1048576.0-(double)adc_P;
	p = (p-(var2/4096.0))*6250.0/var1;	
	var1 = ((double)dig_P9)*p*p/2147483648.0;	
	var2 = p*((double)dig_P8)/32768.0;	
	p = p+(var1+var2+((double)dig_P7))/16.0;	 
	return p;
}

void PressBrightAdj(u8 PressData)
{
	static u8 PressBak = 0;
	u8 TempU8 = 0;
	while (!USART_GetFlagStatus(HTCFG_USART, USART_FLAG_TXC));
	USART_SendData(HTCFG_USART, 0xa2); 
	while (!USART_GetFlagStatus(HTCFG_USART, USART_FLAG_TXC));	
	USART_SendData(HTCFG_USART, PressData);    
	while (!USART_GetFlagStatus(HTCFG_USART, USART_FLAG_TXC));
	USART_SendData(HTCFG_USART, DutyBak);    
	while (!USART_GetFlagStatus(HTCFG_USART, USART_FLAG_TXC));
	USART_SendData(HTCFG_USART, TempU8);    
	PressBak = PressData;
	while (!USART_GetFlagStatus(HTCFG_USART, USART_FLAG_TXC));
	USART_SendData(HTCFG_USART, PressBak);    
	while (!USART_GetFlagStatus(HTCFG_USART, USART_FLAG_TXC));
	USART_SendData(HTCFG_USART, 0x00);    	
}
//返回原始值
void OrgiOut1(void)
{
	while (!USART_GetFlagStatus(HTCFG_USART, USART_FLAG_TXC));
	USART_SendData(HTCFG_USART, 0xa2); 
	while (!USART_GetFlagStatus(HTCFG_USART, USART_FLAG_TXC));	
	USART_SendData(HTCFG_USART, PressSum>>16);    
	while (!USART_GetFlagStatus(HTCFG_USART, USART_FLAG_TXC));
	USART_SendData(HTCFG_USART, ((PressSum>>8)&0xff));    
	while (!USART_GetFlagStatus(HTCFG_USART, USART_FLAG_TXC));
	USART_SendData(HTCFG_USART, ((PressSum)&0xff));    	
}
//返回去掉低4位精度的值
void Del1Out(void)
{
	while (!USART_GetFlagStatus(HTCFG_USART, USART_FLAG_TXC));
	USART_SendData(HTCFG_USART, 0xa2); 
	PressSum = PressSum>>4;    
	
	while (!USART_GetFlagStatus(HTCFG_USART, USART_FLAG_TXC));
	USART_SendData(HTCFG_USART, ((PressSumInitValu>>8)&0xff));    
	while (!USART_GetFlagStatus(HTCFG_USART, USART_FLAG_TXC));
	USART_SendData(HTCFG_USART, ((PressSumInitValu)&0xff)); 

	while (!USART_GetFlagStatus(HTCFG_USART, USART_FLAG_TXC));
	USART_SendData(HTCFG_USART, ((PressCurrent>>8)&0xff));    
	while (!USART_GetFlagStatus(HTCFG_USART, USART_FLAG_TXC));
	USART_SendData(HTCFG_USART, ((PressCurrent)&0xff)); 

   	
}
u8 PressDnFlag = 0;
void GetPressure(void)
{ 	
    PressSum = PressSum + bmp280Convert();   
    PressCnt++;
    if(PressCnt >= 10)  //10次平均值        
    {
		PressCurrent = (PressSum/10);
		PressCurrent = PressCurrent>>4;//去掉4位精度
#if EN_PRESS_VALU_OUT > 0		
		Del1Out();
#endif		
		if(PressSumInitValu > PressCurrent)
		{
			PressDn = PressSumInitValu - PressCurrent;
			if((PressDn > AIR_DOWN_VALU)&&(PressDnFlag == 0)&&(EnPressFlag == 1))	//设置气压标志，备份变化前的数据,并且气压检测传感器打开
			{
				PressDnFlag = 1;
				ColorRadioDnBak();  //记录当前光亮度，用于渐暗渐亮
			}
		}
		else
		{		
			PressDn = 0;
		}
        PressSum = 0;
        PressCnt = 0;
	}   
} 
//开机记录初始值
void GetInitPressure(void)
{
	PressSumInitValu = PressCurrent;
}
