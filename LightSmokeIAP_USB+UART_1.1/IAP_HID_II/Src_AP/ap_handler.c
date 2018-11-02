/*********************************************************************************************************//**
 * @file    IAP/IAP_HID_II/Src_AP/ap_handler.c
 * @version $Rev:: 817          $
 * @date    $Date:: 2016-06-19 #$
 * @brief   This file contains AP handler.
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
#include "ht32.h"
#include "ht32_board.h"
#include "ht32_board_config.h"
#include "ht32_usbd_core.h"

#include "ap_handler.h"
#include "ap_config.h"
#include "buffer.h"
#include "ap_crc16.h"

/** @addtogroup HT32_Series_Peripheral_Examples HT32 Peripheral Examples
  * @{
  */

/** @addtogroup IAP_HID IAP HID
  * @{
  */

/** @addtogroup IAP_HID_II IAP HID II (with USB Check buffer)
  * @{
  */

/** @addtogroup IAP_HID_II_AP AP Example
  * @{
  */


/* Private types -------------------------------------------------------------------------------------------*/
typedef u32 (*pFunction)();

/* Private constants ---------------------------------------------------------------------------------------*/
#define __DBG_IAPPrintf(...)
#define __DBG_IAP_DumpData(...)

#if (AP_DEBUG_MODE == 1)
  #undef __DBG_APPrintf
  #undef __DBG_AP_DumpData
  #define __DBG_APPrintf printf
  static _DBG_AP_DumpData(uc8 *memory, u32 len);
  #warning "================================= AP Debug Mode Warning ================================"
  #warning " 1. AP debug mode has been enable which degrade the performance.                        "
  #warning " 2. Note that print debug message too much may cause the AP command not synchronized.   "
  #warning " 4. After all debug operation is finished, please remember to turn off AP debug mode.   "
  #warning "========================================================================================"
#endif


#define MAX_CMD_LEN             (64)
#define MAX_TOKENS              (3)
#define CMD_COUNT               (6)

#define CMD_SUCCESS             ('O')
#define CMD_FAILED              ('F')

/* Private function prototypes -----------------------------------------------------------------------------*/
static u32 _AP_CMD0(u32 type, u32 saddr, u32 eaddr);

/* Private macro -------------------------------------------------------------------------------------------*/
#define WriteByte(data)         Buffer_WriteByte(&gTx, data)
#define ReadByte(data)          Buffer_ReadByte(&gRx, data)

/* Global variables ----------------------------------------------------------------------------------------*/
Buffer_TypeDef gTx;
Buffer_TypeDef gRx;

/* Private variables ---------------------------------------------------------------------------------------*/
__ALIGN4 static u8 guRxBuffer[RX_BUFFER_SIZE];
__ALIGN4 static u8 guTxBuffer[TX_BUFFER_SIZE];
__ALIGN4 static u8 gu8CmdBuffer[MAX_CMD_LEN];
static u32 u32BufferIndex;

static const pFunction pFComHandlerTable[CMD_COUNT] =
{
  (pFunction)_AP_CMD0,
  (pFunction)_AP_CMD0,
  (pFunction)_AP_CMD0,
  (pFunction)_AP_CMD0,
  (pFunction)AP_Reset,
  (pFunction)_AP_CMD0
};

/* Global functions ----------------------------------------------------------------------------------------*/
/*********************************************************************************************************//**
  * @brief  AP mode initialization.
  * @retval None
  ***********************************************************************************************************/
void AP_Init(void)
{
    #if (IAP_DEBUG_MODE == 1)
    RETARGET_Configuration();
    #endif
    Buffer_Init(&gTx, guTxBuffer, sizeof(guTxBuffer));
    Buffer_Init(&gRx, guRxBuffer, sizeof(guRxBuffer));
}

/*********************************************************************************************************//**
  * @brief  AP mode handler.
  * @retval None
  ***********************************************************************************************************/
void AP_Handler(void)
{
  extern vu32 gIsCMDReady;
  u32 u32Parameter[MAX_TOKENS];
  u32 u32CommandExecuteResult;
  u16 crc;
  u16 crcValue;
  if (gIsCMDReady == TRUE)
  {
    gIsCMDReady = FALSE;
    u32BufferIndex = 0;
    while (u32BufferIndex < MAX_CMD_LEN)
    {
      ReadByte(&gu8CmdBuffer[u32BufferIndex++]);
    }
    /*------------------------------------------------------------------------------------------------------*/
    /* Check CRC value of command packet                                                                    */
    /*------------------------------------------------------------------------------------------------------*/
    crc = gu8CmdBuffer[2] | ((u16)gu8CmdBuffer[3] << 8);
    gu8CmdBuffer[2] = gu8CmdBuffer[3] = 0;
    crcValue = CRC16(0, (u8 *)(&gu8CmdBuffer[0]), 64);

    /*------------------------------------------------------------------------------------------------------*/
    /* Check command is valid and CRC is correct                                                            */
    /*------------------------------------------------------------------------------------------------------*/
    if (gu8CmdBuffer[0] > CMD_COUNT)
    {
      /*----------------------------------------------------------------------------------------------------*/
      /* Command invalid or CRC error. Return 'F' and clear command buffer                                  */
      /*----------------------------------------------------------------------------------------------------*/
      u32CommandExecuteResult = CMD_FAILED;
      Buffer_Discard(&gRx);
    }
    else if (crc != crcValue)
    {
      u32CommandExecuteResult = CMD_FAILED;
      Buffer_Discard(&gRx);
    }
    else
    {
      /*----------------------------------------------------------------------------------------------------*/
      /* Command is OK, stop SysTick to prevent timeout reset                                               */
      /*----------------------------------------------------------------------------------------------------*/
      SYSTICK_CounterCmd(SYSTICK_COUNTER_DISABLE);
      /*----------------------------------------------------------------------------------------------------*/
      /* Prepare parameter and execution command                                                            */
      /*----------------------------------------------------------------------------------------------------*/
      u32Parameter[0] = gu8CmdBuffer[1];
      u32Parameter[1] = *((u32 *)(&(gu8CmdBuffer[4])));
      u32Parameter[2] = *((u32 *)(&(gu8CmdBuffer[8])));

      u32CommandExecuteResult = (*pFComHandlerTable[gu8CmdBuffer[0]])(u32Parameter[0],
                                                                      u32Parameter[1],
                                                                      u32Parameter[2]
                                                                      );
    }
    /*------------------------------------------------------------------------------------------------------*/
    /* Send Result to Host                                                                                  */
    /*------------------------------------------------------------------------------------------------------*/
    WriteByte(u32CommandExecuteResult);

  }
}

/*********************************************************************************************************//**
  * @brief  Reset Command.
  * @param  uMode: Mode after reset
  * @retval FALSE or TRUE
  ***********************************************************************************************************/
u32 AP_Reset(u32 uMode)
{
  s32 i;

  BOOT_MODE = BOOT_MODE_IAP;

  /*--------------------------------------------------------------------------------------------------------*/
  /* Disconnect USB and wait for PC aware it                                                                */
  /*--------------------------------------------------------------------------------------------------------*/
  NVIC_DisableIRQ(USB_IRQn);
  HT32F_DVB_USBDisConnect();
  for (i = 400000; i != 0; i--);
  NVIC_SystemReset();

  return TRUE;
}

/* Private functions ---------------------------------------------------------------------------------------*/
/*********************************************************************************************************//**
  * @brief  _AP_CMD0.
  * @param  type: TBD
  *         @arg TBD
  *         @arg TBD
  * @param  saddr: Start address
  * @param  eaddr: End address
  * @retval CMD_SUCCESS or CMD_FAILED
  ***********************************************************************************************************/
static u32 _AP_CMD0(u32 type, u32 saddr, u32 eaddr)
{
  //return CMD_FAILED;
  return CMD_SUCCESS;
}



/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */
