/*
 * File      : msd.h
 * SPI mode SD Card Driver
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2012, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-04-17     Bernard      first version.
 */

#ifndef _BMP280_H
#define _BMP280_H
					   
#include "ht32.h"
#include "ht32_board.h"
#include "main.h"
							   
typedef unsigned char  BYTE;
typedef unsigned short WORD;
   	


#define SUCCESS  0
#define ERROR    -1


#define START_ADDR          0x20

/* ÆøÑ¹´«¸ÐÆ÷ */
//sbit SDA=P4^1;		
//sbit SCL=P4^0;

#define VERSION1  0

#define SDA_PORT       (HT_GPIOB)
#define SCK_PORT       (HT_GPIOB)

#if VERSION1 > 0

#define SDA_PIN       (GPIO_PIN_4)
#define SCK_PIN       (GPIO_PIN_5)

#else

#define SDA_PIN       (GPIO_PIN_8)
#define SCK_PIN       (GPIO_PIN_7)

#endif



#define EN_PIN          (GPIO_PIN_8)

#define KEY_PORT       (HT_GPIOB)
#define KEY_PIN        (GPIO_PIN_12)

#define DC_PORT       (HT_GPIOC)
#define DC_PIN        (GPIO_PIN_4)

#define LED_CTR         (GPIO_PIN_5)
#define WAKE_UP         (GPIO_PIN_0)
#define WAKE_UP_PORT   (HT_GPIOA)

#define BLEWAKEUP_PORT      HT_GPIOA
#define BLEWAKEUP_PIN       GPIO_PIN_0

#define TP4056_CHRG_PORT    HT_GPIOB
#define TP4056_CHRG_PIN     GPIO_PIN_5

#define TP4056_STDBY_PORT   HT_GPIOB
#define TP4056_STDBY_PIN    GPIO_PIN_4

typedef struct __BMP180
{
	short AC1;
	short AC2;
	short AC3;
	unsigned short AC4;
	unsigned short AC5;
	unsigned short AC6;
	short B1;
	short B2;
	short MB;
	short MC;
	short MD;
	long UT;
	long UP;
	long X1;
	long X2;
	long X3;
	long B3;
	unsigned long B4;
	long B5;
	long B6;
	long B7;
	long p;
	long Temp;
	float altitude;
}_bmp180;

extern _bmp180 bmp180;
extern u8 EnPressFlag;

void BMP_Init(void);
uint8_t BMP_ReadOneByte(uint8_t ReadAddr);
void BMP_WriteOneByte(uint8_t WriteAddr,uint8_t DataToWrite);
short BMP_ReadTwoByte(uint8_t ReadAddr);
void BMP_ReadCalibrationData(void);
long BMP_Read_UT(void);
long BMP_Read_UP(void);
void BMP_UncompemstatedToTrue(void);
//#define	FRAME_SIZE			20
//------------------------------------
void BMP280_Start(void);
void BMP280_Stop(void);


void BMP280_SendByte(BYTE bat);
BYTE BMP280_RecvByte(void);
void BMP280_ReadPage(void);
void BMP280_WritePage(void);
void BMP280_SendACK(void);
//bit BMP280_RecvACK();
int	bmp280_init(void);
void Init_BMP280(void);
void GetPressure(void);
long bmp280Convert(void);
long Multiple_three_read(unsigned char ST_Address);
#endif // MSD_H_INCLUDED

