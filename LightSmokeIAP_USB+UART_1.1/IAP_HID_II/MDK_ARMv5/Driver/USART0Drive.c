#include "ht32.h"
#include "ht32_board.h"
#include "main.h"
#include "USART0Drive.h"


const char HelloString[] = "Hello, this is USART Tx/Rx FIFO example. Please enter 4 characters...\r\n";
unsigned char UR0RxBuf[USART_BUFSIZE];


//void HTCFG_USART_IRQHandler(void)
//{
//    /* Rx ,We have received four datas from UART.                                                             */
//    if (USART_GetFlagStatus(HTCFG_USART, USART_FLAG_RXDR))
//    {
//        USART_ClearFlag(HTCFG_USART, USART_FLAG_RXDR);
//        /* Rx, Geting these datas from USART's FIFO                                                             */
//        APPFrameRxCallBack((u8)USART_ReceiveData(HTCFG_USART));
////        /* Transmiting these datas                                                                              */
////        USART_SendData(HTCFG_USART, UR0RxBuf[0]);
////        USART_IntConfig(HTCFG_USART, USART_INT_TXDE, ENABLE);
//        return ;
//    }
//    /*Time out, Reset FIFO                                                                                    */
//    if (USART_GetFlagStatus(HTCFG_USART, USART_FLAG_TOUT))
//    {
//        USART_ClearFlag(HTCFG_USART, USART_FLAG_TOUT);
//        USART_FIFOReset(HTCFG_USART, USART_FIFO_RX);
//        /* Tx finish                                                                                            */

//        USART_SendData(HTCFG_USART, 'T');
//        USART_SendData(HTCFG_USART, 'i');
//        USART_SendData(HTCFG_USART, 'm');
//        USART_SendData(HTCFG_USART, 'e');
//        USART_SendData(HTCFG_USART, 'O');
//        USART_SendData(HTCFG_USART, 'u');
//        USART_SendData(HTCFG_USART, 't');

//    }
//    /* Tx                                                                                                     */
//    if (USART_GetFlagStatus(HTCFG_USART, USART_FLAG_TXDE))
//    {
//        USART_IntConfig(HTCFG_USART, USART_INT_TXDE, DISABLE);
//        return ;
//    }
//}


void USART_Tx(const char* TxBuffer, u32 length)
{
  int i;

  for (i = 0; i < length; i++)
  {
    while (!USART_GetFlagStatus(HTCFG_USART, USART_FLAG_TXC));
    USART_SendData(HTCFG_USART, TxBuffer[i]);
  }
}

void USART1Init(void)
{
    USART_InitTypeDef USART_InitStructure;

    /* Config AFIO mode as HTCFG_USART_Tx function.                                                           */
    AFIO_GPxConfig(HTCFG_USART_TX_GPIO_ID, HTCFG_USART_TX_AFIO, AFIO_FUN_USART_UART);
    AFIO_GPxConfig(HTCFG_USART_RX_GPIO_ID, HTCFG_USART_RX_AFIO, AFIO_FUN_USART_UART);
    USART_InitStructure.USART_BaudRate = BRAND;
    USART_InitStructure.USART_WordLength = USART_WORDLENGTH_8B;
    USART_InitStructure.USART_StopBits = USART_STOPBITS_1;
    USART_InitStructure.USART_Parity = USART_PARITY_NO;
    USART_InitStructure.USART_Mode = USART_MODE_NORMAL;
    USART_Init(HTCFG_USART, &USART_InitStructure);
    /* Seting HTCFG_USART interrupt-flag                                                                      */
    USART_IntConfig(HTCFG_USART, USART_INT_RXDR, ENABLE);
    USART_IntConfig(HTCFG_USART, USART_INT_TOUT, ENABLE);
    /* Seting Rx FIFO Level                                                                                   */
    USART_RXTLConfig(HTCFG_USART, USART_RXTL_01);
    /*Seting TimeOut Counter                                                                                  */
    //USART_SetTimeOutValue(HTCFG_USART, 0x7F);
    /* Enable HTCFG_USART                                                                                     */
    USART_TxCmd(HTCFG_USART, ENABLE);
    USART_RxCmd(HTCFG_USART, ENABLE);
    /* Configure USARTx interrupt                                                                             */
    NVIC_EnableIRQ(HTCFG_USART_IRQn);
//    USART_Tx(HelloString, sizeof(HelloString));
    USART_SendData(HTCFG_USART, 0x55);
    APPFrame.RecSta = REC_INIT;
}
