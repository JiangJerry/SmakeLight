
#ifndef __TASKBLE_H
#define __TASKBLE_H

#define USART_TXSIZE (13)
#define USART_BUFSIZE (4)


void TaskBLE(void);
void SetBleStatus(unsigned char Sta);
void APPFrameRxCallBack(u8 RxBuf);

extern unsigned char UR0RxBuf[USART_BUFSIZE];
extern unsigned char UR0TxBuf[USART_TXSIZE];
extern Communicate APPFrame;


#endif	
