#include "ht32.h"
#include "ht32_board.h"
#include "main.h"

/*********************************************************************************************************//**
  * @brief  Configure the system clocks.
  * @retval None
  ***********************************************************************************************************/
void CKCU_Configuration(void)
{
  CKCU_PeripClockConfig_TypeDef CKCUClock = {{ 0 }};
  CKCUClock.Bit.PDMA       = 1;
  CKCUClock.Bit.USBD       = 1;
  CKCUClock.Bit.CKREF      = 1;
  CKCUClock.Bit.EBI        = 0;
  CKCUClock.Bit.CRC        = 0;
  CKCUClock.Bit.PA         = 1;
  CKCUClock.Bit.PB         = 1;
  CKCUClock.Bit.PC         = 1;
  CKCUClock.Bit.PD         = 0;
  CKCUClock.Bit.I2C0       = 0;
  CKCUClock.Bit.I2C1       = 0;
  CKCUClock.Bit.SPI0       = 0;
  CKCUClock.Bit.SPI1       = 0;
  CKCUClock.Bit.USART0     = 1;
  CKCUClock.Bit.USART1     = 1;
  CKCUClock.Bit.UART0      = 1;
  CKCUClock.Bit.UART1      = 1;
  CKCUClock.Bit.AFIO       = 1;
  CKCUClock.Bit.EXTI       = 1;
  CKCUClock.Bit.SCI0       = 0;
  CKCUClock.Bit.SCI1       = 0;
  CKCUClock.Bit.I2S        = 0;
  CKCUClock.Bit.MCTM0      = 0;
  CKCUClock.Bit.WDT        = 1;
  CKCUClock.Bit.BKP        = 1;
  CKCUClock.Bit.GPTM0      = 1;
  CKCUClock.Bit.GPTM1      = 0;
  CKCUClock.Bit.BFTM0      = 0;
  CKCUClock.Bit.BFTM1      = 0;
  CKCUClock.Bit.CMP        = 0;
  CKCUClock.Bit.ADC        = 1;
  CKCUClock.Bit.SCTM0      = 0;
  CKCUClock.Bit.SCTM1      = 0;
  CKCUClock.Bit.SCTM2      = 0;
  CKCUClock.Bit.SCTM3      = 0;
//  CKCUClock.Bit.PDMA       = 0;
//  CKCUClock.Bit.USBD       = 0;
//  CKCUClock.Bit.CKREF      = 0;
//  CKCUClock.Bit.EBI        = 0;
//  CKCUClock.Bit.CRC        = 0;
//  CKCUClock.Bit.PA         = 0;
//  CKCUClock.Bit.PB         = 0;
//  CKCUClock.Bit.PC         = 0;
//  CKCUClock.Bit.PD         = 0;
//  CKCUClock.Bit.I2C0       = 0;
//  CKCUClock.Bit.I2C1       = 0;
//  CKCUClock.Bit.SPI0       = 0;
//  CKCUClock.Bit.SPI1       = 0;
//  CKCUClock.Bit.USART0     = 0;
//  CKCUClock.Bit.USART1     = 0;
//  CKCUClock.Bit.UART0      = 0;
//  CKCUClock.Bit.UART1      = 0;
//  CKCUClock.Bit.AFIO       = 0;
//  CKCUClock.Bit.EXTI       = 0;
//  CKCUClock.Bit.SCI0       = 0;
//  CKCUClock.Bit.SCI1       = 0;
//  CKCUClock.Bit.I2S        = 0;
//  CKCUClock.Bit.MCTM0      = 0;
//  CKCUClock.Bit.WDT        = 0;
//  CKCUClock.Bit.BKP        = 0;
//  CKCUClock.Bit.GPTM0      = 0;
//  CKCUClock.Bit.GPTM1      = 0;
//  CKCUClock.Bit.BFTM0      = 0;
//  CKCUClock.Bit.BFTM1      = 0;
//  CKCUClock.Bit.CMP        = 0;
//  CKCUClock.Bit.ADC        = 0;
//  CKCUClock.Bit.SCTM0      = 0;
//  CKCUClock.Bit.SCTM1      = 0;
//  CKCUClock.Bit.SCTM2      = 0;
//  CKCUClock.Bit.SCTM3      = 0;

  CKCU_PeripClockConfig(CKCUClock, ENABLE);
}
