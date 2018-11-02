/*********************************************************************************************************//**
 * @file    mcu_52231_41.h
 * @version V1.0
 * @date    2016-09-18
 * @brief   Header file for mcu related defination.
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

#ifndef __MCU_DEVICE_H
#define __MCU_DEVICE_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------------------------------------*/
#include "ht32.h"

/********************************************************************************************************** */
/* Setting                                                                                                  */
/********************************************************************************************************** */

#define MCU_MAX_HCLK                    (40000000)
#define MCU_WAIT_STATE0_MAX_FREQ        (20000000)
#define MCU_WAIT_STATE1_MAX_FREQ        (40000000)
#define MCU_WAIT_STATE2_MAX_FREQ        (60000000)
#define MCU_WAIT_STATE3_MAX_FREQ        (80000000)

#define MCU_PAGE_NUM_PER_PPBIT          (1)
#define MCU_FLASH_PAGE_SIZE             (1024)
#define MCU_FLASH_PAGES                 (63)
#define MCU_FLASH_SIZE                  (MCU_FLASH_PAGE_SIZE * MCU_FLASH_PAGES)
#define MCU_SRAM_SIZE                   (1024 * 8)
#define MCU_SRAM_START                  (0x20000000)

/* GPIO related definetion                                                                                  */
#define GPIO_NBR                        (3)
#define GPIOA_ID                        (0)
#define GPIOB_ID                        (1)
#define GPIOC_ID                        (2)

#define GPIO_PORT(port_id)              ((HT_GPIO_TypeDef*)(HT_GPIOA_BASE + (0x2000ul * (port_id))))
#define GPIO_PIN(pin_id)                ((u32)GPIO_PIN_0 << (pin_id))

#define IS_3V3_IO_PIN(port_id, pin_id)  (1)

#define SWDIO_PORT_ID                   GPIOA_ID
#define SWDIO_PIN_ID                    13

#define SWCLK_PORT_ID                   GPIOA_ID
#define SWCLK_PIN_ID                    12

#define IS_SW_DEBUG_IO_PIN(port_id, Pin_id) \
  ((((port_id) == SWDIO_PORT_ID) && ((pin_id) == SWDIO_PIN_ID))\
    || (((port_id) == SWCLK_PORT_ID) && ((pin_id) == SWCLK_PIN_ID))\
  )

#define XTAL32KIN_PORT_ID               GPIOB_ID
#define XTAL32KIN_PIN_ID                10

#define XTAL32KOUT_PORT_ID              GPIOB_ID
#define XTAL32KOUT_PIN_ID               11

#define RTCOUT_PORT_ID                  GPIOB_ID
#define RTCOUT_PIN_ID                   12

#define IS_BAK_IO_PIN(port_id, Pin_id) \
  ((((port_id) == XTAL32KIN_PORT_ID) && ((pin_id) == XTAL32KIN_PIN_ID))\
    || (((port_id) == XTAL32KOUT_PORT_ID) && ((pin_id) == XTAL32KOUT_PIN_ID))\
    || (((port_id) == RTCOUT_PORT_ID) && ((pin_id) == RTCOUT_PIN_ID))\
    || (((port_id) == GPIOB_ID) && ((pin_id) == 9))\
  )

#define XTALIN_PORT_ID                  GPIOB_ID
#define XTALIN_PIN_ID                   13

#define XTALOUT_PORT_ID                 GPIOB_ID
#define XTALOUT_PIN_ID                  14

#define IS_XTAL_IO_PIN(port_id, Pin_id) \
  ((((port_id) == XTALIN_PORT_ID) && ((pin_id) == XTALIN_PIN_ID))\
    || (((port_id) == XTALOUT_PORT_ID) && ((pin_id) == XTALOUT_PIN_ID))\
  )

#define SW_BOOT_PORT_ID                GPIOA_ID
#define SW_BOOT_PIN_ID                 8

#define BOOT_PORT_ID                   GPIOA_ID
#define BOOT_PIN_ID                    9
#define HW_BOOT_PORT_ID                BOOT_PORT_ID
#define HW_BOOT_PIN_ID                 BOOT_PIN_ID
#define IS_BOOT_IO_PIN(port_id, Pin_id) \
  (((port_id) == BOOT_PORT_ID) && ((pin_id) == BOOT_PIN_ID))

/* ID0 USART0 / ID1 USART1 / ID2 UART0 / ID3 UART1                                                          */
#define USART_PORT(port_id)             (((port_id) < 2)? \
                                         ((HT_USART_TypeDef*)(HT_USART0_BASE + ((u32)(port_id) << 18))) : \
                                         ((HT_USART_TypeDef*)(HT_UART0_BASE + ((u32)((port_id) - 2) << 18))))

#define SPI_PORT(port_id)               ((HT_SPI_TypeDef*)(HT_SPI0_BASE + ((u32)(port_id) << 18)))
#define I2C_PORT(port_id)               ((HT_I2C_TypeDef*)(HT_I2C0_BASE + ((u32)(port_id) << 12)))

#ifdef __cplusplus
}
#endif


#endif
