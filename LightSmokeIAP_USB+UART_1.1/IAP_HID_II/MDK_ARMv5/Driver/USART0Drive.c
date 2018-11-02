#include "ht32.h"
#include "ht32_board.h"
#include "main.h"
#include "USART0Drive.h"


const char HelloString[] = "Hello, this is USART Tx/Rx FIFO example. Please enter 4 characters...\r\n";
unsigned char UR0RxBuf[USART_BUFSIZE];
unsigned char UR0TxBuf[USART_TXSIZE];
Communicate APPFrame;

#define HEAD0   0XEA
#define HEAD1   0X15

extern u8 TaskLEDRdy,LedMode,i,Duty,EnPressFlag,ShutDownFlag,AlertFlag;
void USART_Tx(const char* TxBuffer, u32 length);

void AckDataToBleModle(void)
{
    u8 i;
    UR0TxBuf[0] = HEAD0; 
    UR0TxBuf[1] = HEAD1; 
    UR0TxBuf[2] = 0; 
    UR0TxBuf[3] = 13; 
    UR0TxBuf[3] = 0xb0; 
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
