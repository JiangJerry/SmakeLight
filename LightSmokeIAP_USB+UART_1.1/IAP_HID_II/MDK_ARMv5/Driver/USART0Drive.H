#ifndef __UART0_H
#define __UART0_H
#define BRAND (9600)

#define USART_BUFSIZE (4)

#define USART_TXSIZE (13)

#define HTCFG_USART_TX_GPIO_ID                          (GPIO_PA)
#define HTCFG_USART_RX_GPIO_ID                          (GPIO_PA)
#define HTCFG_USART_TX_AFIO                             (AFIO_PIN_2)
#define HTCFG_USART_RX_AFIO                             (AFIO_PIN_3)
#define HTCFG_USART                                     (HT_USART0)
#define HTCFG_USART_IRQn                                (USART0_IRQn)
#define HTCFG_USART_CLK(CK)                             (CK.Bit.USART0)
#define HTCFG_USART_IRQHandler                          (USART0_IRQHandler)

#define FSA_INIT     			1
#define FSA_COMMAND  			2
#define FSA_DATASIZE 			3
#define FSA_DATA 	 			4
#define FSA_CHK 	 			5
#define FSA_END		 			6
#define SYN_HEAD  				0xEA 	//?
#define SYN_CMD                 0X15

#define CLOUD_STA			0X0c
#define CLOUD_CMD			0X04
#define DATA_SIZE			0X09
#define	OPEN_CMD			0X61
#define	CLOSE_CMD			0X62
#define	OPEN_ONLY_CMD		0X63
#define UPDATE_TIPS			0X64

#define REC_INIT 				1		//?車那?那??t角角D赤㏒?1?a那??t㏒?2?aACK
#define REC_CMD                 2
#define REC_DATA_SIZE0			3		//
#define REC_DATA_SIZE1		    4		//
#define REC_DATA_CTX		    5		//?車那?﹞⊿?赤那y?Y?迆豕Y
#define REC_END                 6       //?芍那??車那?
#endif
