#include <string.h>
#include "BMP280Drive.h"
#include  <math.h>    //Keil library  
#include  <stdlib.h>  //Keil library  
#include  <stdio.h>   //Keil library

#include "USART0Drive.h"

#define   uchar unsigned char
#define   uint unsigned int	

#define	BMP280_SlaveAddress   0xEE	  //定义器件在IIC总线中的从地址                               

#define OSS 0	// Oversampling Setting (note: code is not set up to use other OSS values)

#define SDA_OUT()   GPIO_DirectionConfig(SDA_PORT, SDA_PIN, GPIO_DIR_OUT)
#define SDA_IN() GPIO_DirectionConfig(SDA_PORT, SDA_PIN, GPIO_DIR_IN);\
                GPIO_InputConfig(SDA_PORT, SDA_PIN, ENABLE);

void delay_us(u8 i)    //略微延时  6us约>4.7us
{
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();

    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();

    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();

    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();

    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();

    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();

    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();

}
//    GPIO_WriteOutBits(SCK_PORT, SCK_PIN, 1);;

//产生IIC起始信号
void IIC_Start(void)
{
	GPIO_WriteOutBits(SDA_PORT, SDA_PIN, SET);	  	  
	GPIO_WriteOutBits(SCK_PORT, SCK_PIN, SET);
	SDA_OUT();     //sda线输出
	delay_us(4);
 	GPIO_WriteOutBits(SDA_PORT, SDA_PIN, RESET);//START:when CLK is high,DATA change form high to low 
	delay_us(4);
	GPIO_WriteOutBits(SCK_PORT, SCK_PIN, RESET);//钳住I2C总线，准备发送或接收数据 
}	  
//产生IIC停止信号
void IIC_Stop(void)
{
	SDA_OUT();//sda线输出
	GPIO_WriteOutBits(SCK_PORT, SCK_PIN, RESET);
	GPIO_WriteOutBits(SDA_PORT, SDA_PIN, RESET);//STOP:when CLK is high DATA change form low to high
 	delay_us(4);
	GPIO_WriteOutBits(SCK_PORT, SCK_PIN, SET); 
	GPIO_WriteOutBits(SDA_PORT, SDA_PIN, SET);//发送I2C总线结束信号
	delay_us(4);							   	
}
//等待应答信号到来
//返回值：1，接收应答失败
//        0，接收应答成功
u8 IIC_Wait_Ack(void)
{
	u8 ucErrTime=0;
	SDA_IN();      //SDA设置为输入  
	GPIO_WriteOutBits(SDA_PORT, SDA_PIN, SET);delay_us(1);	   
	GPIO_WriteOutBits(SCK_PORT, SCK_PIN, SET);delay_us(1);	 
	while(GPIO_ReadInBit(SDA_PORT, SDA_PIN) == SET)
	{
		ucErrTime++;
		if(ucErrTime>250)
		{
			IIC_Stop();
			return 1;
		}
	}
	GPIO_WriteOutBits(SCK_PORT, SCK_PIN, RESET);//时钟输出0 	   
	return 0;  
} 
//产生ACK应答
void IIC_Ack(void)
{
	GPIO_WriteOutBits(SCK_PORT, SCK_PIN, RESET);
	SDA_OUT();
	GPIO_WriteOutBits(SDA_PORT, SDA_PIN, RESET);
	delay_us(2);
	GPIO_WriteOutBits(SCK_PORT, SCK_PIN, SET);
	delay_us(2);
	GPIO_WriteOutBits(SCK_PORT, SCK_PIN, RESET);
}
//不产生ACK应答		    
void IIC_NAck(void)
{
	GPIO_WriteOutBits(SCK_PORT, SCK_PIN, RESET);
	SDA_OUT();
	GPIO_WriteOutBits(SDA_PORT, SDA_PIN, SET);
	delay_us(2);
	GPIO_WriteOutBits(SCK_PORT, SCK_PIN, SET);
	delay_us(2);
	GPIO_WriteOutBits(SCK_PORT, SCK_PIN, RESET);
}					 				     
//IIC发送一个字节
//返回从机有无应答
//1，有应答
//0，无应答			  
void IIC_Send_Byte(u8 txd)
{                        
    u8 t;   
	SDA_OUT(); 	    
    GPIO_WriteOutBits(SCK_PORT, SCK_PIN, RESET);//拉低时钟开始数据传输
    for(t=0;t<8;t++)
    {              
		if((txd&0x80)>>7)
			GPIO_WriteOutBits(SDA_PORT, SDA_PIN, SET);
		else
			GPIO_WriteOutBits(SDA_PORT, SDA_PIN, RESET);
		txd<<=1; 	  
		delay_us(2);   //对TEA5767这三个延时都是必须的
		GPIO_WriteOutBits(SCK_PORT, SCK_PIN, SET);
		delay_us(2); 
		GPIO_WriteOutBits(SCK_PORT, SCK_PIN, RESET);	
		delay_us(2);
    }	 
} 	    
//读1个字节，ack=1时，发送ACK，ack=0，发送nACK   
u8 IIC_Read_Byte(unsigned char ack)
{
	unsigned char i,receive=0;
	SDA_IN();//SDA设置为输入
    for(i=0;i<8;i++ )
	{
        GPIO_WriteOutBits(SCK_PORT, SCK_PIN, RESET); 
        delay_us(2);
		GPIO_WriteOutBits(SCK_PORT, SCK_PIN, SET);
        delay_us(2);
        receive <<= 1;
        if(GPIO_ReadInBit(SDA_PORT, SDA_PIN))
        {
            receive++;   
        }
		delay_us(1); 
    }					 
    if (!ack)
        IIC_NAck();//发送nACK
    else
        IIC_Ack(); //发送ACK   
    return receive;
}

//*********************************************************
//读出BMP085内部数据,连续三个
//*********************************************************
long Multiple_three_read(uchar ST_Address)
{   
	uchar msb, lsb,xlsb;
	long _data = 0;
	
	msb = BMP_ReadOneByte(ST_Address);
	lsb = BMP_ReadOneByte(ST_Address+1);
	xlsb = BMP_ReadOneByte(ST_Address+2);
		
 	_data = msb << 16;	
 	_data |= lsb << 8;	
 	_data |= xlsb;
 	_data = _data >> 4;
	return _data;
}

//写一个数据到BMP180
void BMP_WriteOneByte(uint8_t WriteAddr,uint8_t DataToWrite)
{
	IIC_Start();
	
	IIC_Send_Byte(0xEE);
	IIC_Wait_Ack();
	
	IIC_Send_Byte(WriteAddr);
	IIC_Wait_Ack();
	
	IIC_Send_Byte(DataToWrite);
	IIC_Wait_Ack();
	IIC_Stop();
}

//从BMP180读一个字节数据
uint8_t BMP_ReadOneByte(uint8_t ReadAddr)
{
	uint8_t data = 0;
	
	IIC_Start();
	
	IIC_Send_Byte(0xEE);
	IIC_Wait_Ack();
	
	IIC_Send_Byte(ReadAddr);
	IIC_Wait_Ack();
	
	IIC_Start();
	
	IIC_Send_Byte(0xEF);
	IIC_Wait_Ack();
	
	data = IIC_Read_Byte(0);
	IIC_Stop();
	
	return data;
}

//从BMP180读一个16位的数据
short BMP_ReadTwoByte(uint8_t ReadAddr)
{
	short data;
	uint8_t msb,lsb;
	
	IIC_Start();
	
	IIC_Send_Byte(0xEE);
	IIC_Wait_Ack();
	
	IIC_Send_Byte(ReadAddr);
	IIC_Wait_Ack();
	
	IIC_Start();
	
	IIC_Send_Byte(0xEF);
	IIC_Wait_Ack();
	
	msb = IIC_Read_Byte(1);
	lsb = IIC_Read_Byte(0);
	
	IIC_Stop();
	
	data = msb + lsb*256;
	
	return data;
}
