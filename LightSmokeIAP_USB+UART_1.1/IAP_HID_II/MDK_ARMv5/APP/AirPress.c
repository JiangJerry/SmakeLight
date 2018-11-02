#include <string.h>
#include "BMP280Drive.h"
#include  <math.h>    //Keil library  
#include  <stdlib.h>  //Keil library  
#include  <stdio.h>   //Keil library

#include "USART0Drive.h"

#define   uchar unsigned char
#define   uint unsigned int	

#define	BMP280_SlaveAddress   0xEE	  //����������IIC�����еĴӵ�ַ                               
    //�洢BMP180���ݵĽṹ
_bmp180 bmp180;

#define OSS 0	// Oversampling Setting (note: code is not set up to use other OSS values)


long PressSum = 0,PressCnt = 0,PressCurrent;
extern u8 DutyBak,Duty,EnPressFlag;

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
//��ʼ��BMP280��������Ҫ��ο�pdf�����޸�**************
	unsigned short temp = 0;
void Init_BMP280(void)
{

    while(temp != 0X58)
    {    
        temp = BMP_ReadOneByte(0xd0);//��ȡid���ж�iic�Ƿ�����ģ���Ƿ���������ȡ�����ĵ�ַ��0x58���ǶԵ�
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



	BMP_WriteOneByte(0xf4,0xff);
//	write_byte = 0x00;//����
	BMP_WriteOneByte(0xf5,0X80);//5<<2
	GetInitPressure();//��ȡ��ʼֵ
//	Delay_N1ms(200);	
}
//***********************************************************************

long adc_T;
long adc_P;
long var1, var2,t_fine,T,p; 
long bmp280Convert(void)
{
	adc_T = Multiple_three_read(0xFA);    // ��ȡ�¶�
	adc_P = Multiple_three_read(0xF7); // ��ȡѹǿ
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
//����ԭʼֵ
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
//����ȥ����4λ���ȵ�ֵ
void Del1Out(void)
{
	while (!USART_GetFlagStatus(HTCFG_USART, USART_FLAG_TXC));
	USART_SendData(HTCFG_USART, 0xa2); 
	PressSum = PressSum>>4;    
	while (!USART_GetFlagStatus(HTCFG_USART, USART_FLAG_TXC));
	USART_SendData(HTCFG_USART, ((PressSum>>8)&0xff));    
	while (!USART_GetFlagStatus(HTCFG_USART, USART_FLAG_TXC));
	USART_SendData(HTCFG_USART, ((PressSum)&0xff));    	
}
long PressSumInitValu;
u8 PressDnFlag = 0;
void GetPressure(void)
{ 	
	u8 PressValu;
    PressSum = PressSum + bmp280Convert();   
    PressCnt++;
    if(PressCnt >= 10)  //10��ƽ��ֵ        
    {
		PressCurrent = (PressSum/10);
		PressCurrent = PressCurrent>>4;//ȥ��4λ����
//		Del1Out();
		if(PressSumInitValu > PressCurrent)
		{
			PressDn = PressSumInitValu - PressCurrent;
			if((PressDn > 20)&&(PressDnFlag == 0))	//������ѹ��־�����ݱ仯ǰ������
			{
				PressDnFlag = 1;
				ColorRadioDnBak();
			}
		}
		else
		{
			
			PressDn = 0;
		}
		while (!USART_GetFlagStatus(HTCFG_USART, USART_FLAG_TXC));
		USART_SendData(HTCFG_USART, PressDn);    
		
        PressSum = 0;
        PressCnt = 0;
	}   
} 
//������¼��ʼֵ
void GetInitPressure(void)
{
	u8 i = 0;
	PressSum = 0;
	for(i=0;i<10;i++)
	{
		PressSum = PressSum + bmp280Convert(); 
	}		
	PressSumInitValu = PressSum/10;
	PressSumInitValu = PressSumInitValu>>4;	//ȥ��4λ����
}
 //    USART_SendData(HTCFG_USART, (u8)T);