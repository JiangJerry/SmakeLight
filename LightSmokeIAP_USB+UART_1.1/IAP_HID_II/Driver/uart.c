/*********************************************************************************************************//**
 * @file    Board/drv_uart.c
 * @version $Rev:: 93           $
 * @date    $Date:: 2015-11-24 #$
 * @brief   This file provides all the CRC firmware functions.
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

/* Includes ------------------------------------------------------------------------------------------------*/
#include "uart.h"
#include "buffer.h"
#include "rough_delay.h"

/* Settings ------------------------------------------------------------------------------------------------*/
#define DBG_EN  0

#define BRUD_RATE   115200

/*  Buffer Size                                                                                             */
#define IAP_UART_TX_BUFFER_SIZE    (68)                         /* Tx Buffer size                */
#define IAP_UART_RX_BUFFER_SIZE    (512)                        /* Rx Buffer size                */

/* I/O */
#define IAP_UART_TX_GPIO_PORT_NAME  A
#define IAP_UART_TX_GPIO_PIN_ID     2
#define IAP_UART_RX_GPIO_PORT_NAME  A
#define IAP_UART_RX_GPIO_PIN_ID     3

#define IAP_UART_NAME               USART0
#define IAP_UART_IRQn               STRCAT2(IAP_UART_NAME,  _IRQn)
#define IAP_UART                    STRCAT2(HT_,            IAP_UART_NAME)
#define IAP_UART_IRQHandler         STRCAT2(IAP_UART_NAME,  _IRQHandler)

#define IAP_UART_TX_GPIO_PORT_ID    STRCAT2(GPIO_P,         IAP_UART_TX_GPIO_PORT_NAME)
#define IAP_UART_RX_GPIO_PORT_ID    STRCAT2(GPIO_P,         IAP_UART_RX_GPIO_PORT_NAME)

#define IAP_UART_RX_GPIO_CLK        STRCAT2(P,              IAP_UART_RX_GPIO_PORT_NAME)
#define IAP_UART_RX_GPIO_PORT       STRCAT2(HT_GPIO,        IAP_UART_RX_GPIO_PORT_NAME)
#define IAP_UART_RX_GPIO_PIN        STRCAT2(GPIO_PIN_,      IAP_UART_RX_GPIO_PIN_ID)

#define delayXuS(XuS)               RoughDelay_XuS(XuS)

u8 IAP_UART_RxBufferMem[IAP_UART_RX_BUFFER_SIZE];
Buffer_TypeDef IAP_UART_RxBuffer;

/* Global functions ----------------------------------------------------------------------------------------*/

/*********************************************************************************************************//**
  * @brief  TBD
  * @param  TBD
  * @retval None
  ***********************************************************************************************************/
void IAP_UART_Init(void)
{
    USART_InitTypeDef init;

    CKCU_PeripClockConfig_TypeDef CKCUClock = {{0}};

    CKCUClock.Bit.IAP_UART_NAME         = 1;
    CKCUClock.Bit.AFIO                  = 1;
    CKCUClock.Bit.IAP_UART_RX_GPIO_CLK  = 1;
    CKCU_PeripClockConfig(CKCUClock, ENABLE);

    AFIO_GPxConfig(IAP_UART_TX_GPIO_PORT_ID, AFIO_PIN_0 << IAP_UART_TX_GPIO_PIN_ID, AFIO_FUN_USART_UART);
    AFIO_GPxConfig(IAP_UART_RX_GPIO_PORT_ID, AFIO_PIN_0 << IAP_UART_RX_GPIO_PIN_ID, AFIO_FUN_USART_UART);

    GPIO_PullResistorConfig(IAP_UART_RX_GPIO_PORT, IAP_UART_RX_GPIO_PIN, GPIO_PR_UP);

    Buffer_Init(&IAP_UART_RxBuffer, IAP_UART_RxBufferMem, IAP_UART_RX_BUFFER_SIZE);   

    USART_DeInit(IAP_UART);

    init.USART_BaudRate   = BRUD_RATE;
    init.USART_WordLength = USART_WORDLENGTH_8B;
    init.USART_Parity     = USART_PARITY_NO;
    init.USART_StopBits   = USART_STOPBITS_1;
    init.USART_Mode       = USART_MODE_NORMAL;
    USART_Init(IAP_UART, &init);

    /* Enable RX Data Received Interrupt                                                                      */
    USART_IntConfig(IAP_UART, USART_INT_RXDR, ENABLE);

    USART_TxCmd(IAP_UART, ENABLE);
    USART_RxCmd(IAP_UART, ENABLE);

    NVIC_EnableIRQ(IAP_UART_IRQn);
}

/*********************************************************************************************************//**
  * @brief  TBD
  * @param  TBD
  * @retval None
  ***********************************************************************************************************/
ErrStatus IAP_UART_PutChar(u8 c)
{
    u16 timeout = 10000;

    IAP_UART->DR = c;
    while (timeout--) 
    {
        if(((IAP_UART->SR & USART_FLAG_TXC) != 0))
        {
            return SUCCESS;
        }
    }

    return ERROR;
}
  
/*********************************************************************************************************//**
  * @brief  TBD
  * @param  TBD
  * @retval None
  ***********************************************************************************************************/
u16 IAP_UART_Send(u8 *puData, u16 length)
{
    u16 cnt;

    for(cnt = 0 ; cnt < length ; cnt++)
    {
        if(IAP_UART_PutChar(puData[cnt]) == ERROR)
        {
            break;
        }
    }

    return cnt;
}

/*********************************************************************************************************//**
  * @brief  TBD
  * @param  TBD
  * @retval None
  ***********************************************************************************************************/
u16 IAP_UART_Get(u8 *puData, u16 length, u16 timeout_ms)
{
    u16 cnt = 0;

    do
    {
        cnt += Buffer_Read(&IAP_UART_RxBuffer, puData, length-cnt);
        if(cnt >= length)
          break;

        delayXuS(1000);
    } while(timeout_ms--);

    return cnt;
}

/*********************************************************************************************************//**
  * @brief  UART ISR.
  * @param  TBD
  * @retval None
  ***********************************************************************************************************/
void USART0_IRQHandler(void)
{
//  u8 uTempData;
  
  /*--------------------------------------------------------------------------------------------------------*/
  /* Move data from USART FIFO to buffer when Rx Data Received                                              */
  /*--------------------------------------------------------------------------------------------------------*/
  if ((IAP_UART->IER & USART_INT_RXDR) && (IAP_UART->SR & USART_FLAG_RXDNE))
  {
    #if (DBG_EN != 0)
    if (Buffer_IsFull(&IAP_UART_RxBuffer))
    {
      /*----------------------------------------------------------------------------------------------------*/
      /* Should not reach here! It means the buffer for USART is full.                                      */
      /*----------------------------------------------------------------------------------------------------*/
      while (1);
    }
    #endif
    Buffer_WriteByte(&IAP_UART_RxBuffer, (IAP_UART->DR & (u32)0x000000FF));
  }
}
