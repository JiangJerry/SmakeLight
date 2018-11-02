/*********************************************************************************************************//**
 * @file    debug.c
 * @version V1.00
 * @date    9/18/2016
 * @brief   The source file of debug.
 *************************************************************************************************************
 * @attention
 *
 * Firmware Disclaimer Information
 *
 * 1. The customer hereby acknowledges and agrees that the program technical documentation, including the
 *    code, which is supplied by Holtek Semiconductor Inc., (hereinafter referred to as "HOLTEK") is the
 *    proprietary and confidential intellectual property of HOLTEK, and is protected by copyright law and
 *    other intellectual property laws.
 *
 * 2. The customer hereby acknowledges and agrees that the program technical documentation, including the
 *    code, is confidential information belonging to HOLTEK, and must not be disclosed to any third parties
 *    other than HOLTEK and the customer.
 *
 * 3. The program technical documentation, including the code, is provided "as is" and for customer reference
 *    only. After delivery by HOLTEK, the customer shall use the program technical documentation, including
 *    the code, at their own risk. HOLTEK disclaims any expressed, implied or statutory warranties, including
 *    the warranties of merchantability, satisfactory quality and fitness for a particular purpose.
 *
 * <h2><center>Copyright (C) Holtek Semiconductor Inc. All rights reserved</center></h2>
 ************************************************************************************************************/
// <<< Use Configuration Wizard in Context Menu >>>

/* Includes ------------------------------------------------------------------------------------------------*/
#include "debug.h"
#include <stdarg.h>
#include <stdio.h>

/*----------------------------------------------------------------------------------------------------------*/
/* Debug Configuration Setting                                                                              */
/*----------------------------------------------------------------------------------------------------------*/
//  <o0>    Debug USART Channel <0=> USART0 <1=> USART1 <2=> UART0 <3=> UART1
//  <o1>    Debug USART Baoud Rate <110-921600>
//  <o2>    Debug Print Buffer Size <16-256>
//  <o3.0>  Enable Tx
//  <o4>    Tx Port    <0=> GPIOA      <1=> GPIOB      <2=> GPIOC      <3=> GPIOD      <4=> GPIOE
//  <o5>    Tx Pin ID  <0-15>
//  <o6.0>  Enable Rx
//  <o7>    Rx Port    <0=> GPIOA      <1=> GPIOB      <2=> GPIOC      <3=> GPIOD      <4=> GPIOE
//  <o8>    Rx Pin ID  <0-15>
#define DBG_USART_CH                    3
#define DBG_USART_BAOUD                 115200
#define DBG_PRINT_BUFFER_SIZE           64
#define DBG_USART_TX_EN                 1
#define DBG_USART_TX_GPIO_PORT_ID       0
#define DBG_USART_TX_GPIO_PIN_ID        4
#define DBG_USART_RX_EN                 0
#define DBG_USART_RX_GPIO_PORT_ID       0
#define DBG_USART_RX_GPIO_PIN_ID        5

#ifndef USART_PORT
#define USART_PORT(port_id)     ((port_id < 2)? \
                                 ((HT_USART_TypeDef*)(HT_USART0_BASE + ((u32)port_id << 18))) : \
                                 ((HT_USART_TypeDef*)(HT_UART0_BASE + ((u32)(port_id - 2) << 18))))
#endif

#define DBG_UART_PORT           USART_PORT(DBG_USART_CH)

// <<< end of configuration section >>>

/* Private constants ---------------------------------------------------------------------------------------*/
static char string_tmp[DBG_PRINT_BUFFER_SIZE];
static unsigned int print_counter = 0;

/*********************************************************************************************************//**
 * @brief DBG_Init
 * @param  None
 * @return None
 ************************************************************************************************************/
void _DBG_Init(void)
{
  USART_InitTypeDef USART_InitStructure;

  /* Enable AFIO and USART clock                                                                            */
  {
    CKCU_PeripClockConfig_TypeDef CKCUClock = {{ 0 }};
    u8 RegCK[4] = {0};
    RegCK[DBG_USART_CH] = 1;
    CKCUClock.Bit.AFIO       = 1;
    CKCUClock.Bit.USART0     = RegCK[0];
    CKCUClock.Bit.USART1     = RegCK[1];
    CKCUClock.Bit.UART0      = RegCK[2];
    CKCUClock.Bit.UART1      = RegCK[3];
    CKCU_PeripClockConfig(CKCUClock, ENABLE);
  }

  #if (DBG_USART_TX_EN != 0 )
  AFIO_GPxConfig(DBG_USART_TX_GPIO_PORT_ID, (AFIO_PIN_0 << DBG_USART_TX_GPIO_PIN_ID), AFIO_FUN_USART_UART);
  #endif
  #if (DBG_USART_RX_EN != 0 )
  AFIO_GPxConfig(DBG_USART_RX_GPIO_PORT_ID, (AFIO_PIN_0 << DBG_USART_RX_GPIO_PIN_ID), AFIO_FUN_USART_UART);
  #endif

  /* USART configuration                                                                                    */
  USART_InitStructure.USART_BaudRate = DBG_USART_BAOUD;
  USART_InitStructure.USART_WordLength = USART_WORDLENGTH_8B;
  USART_InitStructure.USART_StopBits = USART_STOPBITS_1;
  USART_InitStructure.USART_Parity = USART_PARITY_NO;
  USART_InitStructure.USART_Mode = USART_MODE_NORMAL;
  USART_Init(DBG_UART_PORT, &USART_InitStructure);

  #if (DBG_USART_TX_EN != 0 )
  /* USART Tx enable                                                                                        */
  USART_TxCmd(DBG_UART_PORT, ENABLE);
  #endif
  #if (DBG_USART_RX_EN != 0 )
  /* USART Rx enable                                                                                        */
  USART_RxCmd(DBG_UART_PORT, ENABLE);
  #endif
}

/*********************************************************************************************************//**
 * @brief DBG_PutChar
 * @param  None
 * @return None
 ************************************************************************************************************/
char _DBG_PutChar(char ch)
{
  USART_SendData(DBG_UART_PORT, (u8)ch);
  while (USART_GetFlagStatus(DBG_UART_PORT, USART_FLAG_TXC) == RESET)
  {
  }
  return ch;
}

/*********************************************************************************************************//**
 * @brief DBG_PutString
 * @param  None
 * @return None
 ************************************************************************************************************/
void _DBG_PutString(char* string)
{
  int cnt = DBG_PRINT_BUFFER_SIZE;
  char c;
  
  while(cnt--)
  {
    c = *string;
    string++;
    if(c == 0)
    {
      break;
    }
    else
    {
      DBG_PutChar(c);
    }
  }
  
  if(cnt == -1)
  {
    // Debug Print Error, should not reach hear
    //__NOP();
    while(1);
  }
}

/*********************************************************************************************************//**
 * @brief DBG_PutBuffer
 * @param  None
 * @return None
 ************************************************************************************************************/
void _DBG_PutBuffer(char* buffer, u32 length)
{   
  while(length--)
  {
    DBG_PutChar(*buffer);
    buffer++;
  }
}

/*********************************************************************************************************//**
 * @brief DBG_Error
 * @param  None
 * @return None
 ************************************************************************************************************/
void _DBG_PrintError(char* fmt, ...)
{
  va_list argp;
  
  DBG_PutString("error: ");
  
  va_start(argp, fmt);
  vsprintf(string_tmp, fmt, argp);
  va_end(argp);
  
  sprintf(string_tmp, "\r\n");
  DBG_PutString(string_tmp);
}

/*********************************************************************************************************//**
 * @brief DBG_Print
 * @param  None
 * @return None
 ************************************************************************************************************/
void _DBG_Print(char* fmt, ...)
{
  va_list argp;
  
  sprintf(string_tmp, "%04u: ", print_counter++);
  DBG_PutString(string_tmp);  
    
  va_start(argp, fmt);
  vsprintf(string_tmp, fmt, argp);
  va_end(argp);
  DBG_PutString(string_tmp);  
  
  sprintf(string_tmp, "\r\n");
  DBG_PutString(string_tmp);  
}
