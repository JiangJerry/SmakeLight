/*********************************************************************************************************//**
 * @file    Src_AP/ap_handler.c
 * @version $Rev:: 296          $
 * @date    $Date:: 2016-03-10 #$
 * @brief   This file contains IAP handler.
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
#include "ht32.h"
#include "ht32_board.h"
#include "ht32_board_config.h"

#include <string.h>
#include "ap_uart_handler.h"
#include "uart.h"

#include "ap_crc16.h"
#include "rough_delay.h"
#include "timebase.h"

/* Private typedef -----------------------------------------------------------------------------------------*/
typedef u32 (*pFunction)();

/* Private constants ---------------------------------------------------------------------------------------*/
#define __DBG_IAPPrintf(...)
#define delayXuS                RoughDelay_XuS

#if (IAP_DEBUG_MODE == 1)
  #undef __DBG_IAPPrintf
  #define __DBG_IAPPrintf printf
  static _IAP_DumpData(u32 uSatrt, u32 uLength);
  #warning "================================ IAP Debug Mode Warning ================================"
  #warning " 1. IAP debug mode has been enable which degrade the performance.                       "
  #warning " 2. Note that print debug message too much may cause the IAP command not synchronized.  "
  #warning " 3. Remember to modify Host/Device IAP_CODE_PAGE define according to the IAP Size       "
  #warning "    after enable IAP debug mode.                                                        "
  #warning " 4. After all debug operation is finished, please remember to turn off IAP debug mode.  "
  #warning "========================================================================================"
#endif

#define HEADER_LEN              (5)
#define MAX_RETURN_PAYLOAD      (12)
#define CRC_LEN                 (2)
#define MAX_RETURN_LEN          (HEADER_LEN + MAX_RETURN_PAYLOAD + CRC_LEN) // Header + MaxPayload + crc

#define MAX_CMD_LEN             (HEADER_LEN + 4 + 4 + CMD_PAYLOAD_LEN + CRC_LEN)
#define MAX_TOKENS              (3)
#define IAP_CMD_COUNT           (10)
#define CMD_COUNT               (IAP_CMD_COUNT + USER_CMD_COUNT)

#define CMD_SUCCESS             ('O')
#define CMD_FAILED              ('F')

#define USER_CMD_START          (0x50)

#define CMD_PAYLOAD_ADDR        (HEADER_LEN + 4 + 4)
#define CMD_PAYLOAD_LEN         (64)

#define PREFIX                  (0x55)

#define ERROR_SUCCESS           (0x0)

/* Private function prototypes -----------------------------------------------------------------------------*/
static u32 _AP_CMD0(u32 type, u32 saddr, u32 eaddr);
static u32 AP_Connect(void);
static u32 AP_GetBootMode(void);
  
static u8 CalculateHeaderCheckSum(u8* pCmdBuffer);
static void AdjustReturnPackage(u8* pBuf, u8 length);
static void ParseCmd(void);

#if (USER_CMD_COUNT > 0)  
static u32 AP_User_Cmd(u32 uPar0, u32 uPar1, u32 uPar2);
#endif

/* Private macro -------------------------------------------------------------------------------------------*/
#define WriteByte(data)         IAP_UART_Send((u8*)&data, 1)
#define WriteBytes(pData, len)  IAP_UART_Send(pData, len)
#define ReadByte(pData)         IAP_UART_Get(pData, 1, 1)

/* Global variables ----------------------------------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------------------------------------*/
__ALIGN4 static u8 gu8CmdBuffer[MAX_CMD_LEN];
static u8 gu8ReturnBuffer[MAX_RETURN_LEN];
static u32 u32BufferIndex;
static u32 u32ReturnBufferIndex;
static vu16 gRxCmdTimeoutCnt = 0;
static vu16 gHeartbeatTimeoutCnt = 0;

static const pFunction pFComHandlerTable[CMD_COUNT] =
{
    (pFunction)_AP_CMD0,
    (pFunction)_AP_CMD0,
    (pFunction)_AP_CMD0,
    (pFunction)_AP_CMD0,
    (pFunction)AP_Uart_Reset,
    (pFunction)_AP_CMD0,
    (pFunction)AP_Connect,
    (pFunction)_AP_CMD0,
    (pFunction)AP_GetBootMode,

#if (USER_CMD_COUNT > 0)  
    (pFunction)AP_User_Cmd
#endif  
};


/* Global functions ----------------------------------------------------------------------------------------*/

/*********************************************************************************************************//**
  * @brief  AP mode initialization.
  * @retval None
  ***********************************************************************************************************/
void AP_Uart_Init(void)
{
    IAP_UART_Init();  
//    Timebase_Init();
//    Timebase_InterruptCmd(ENABLE);
}

/*********************************************************************************************************//**
  * @brief  TBD
  * @retval None
  ***********************************************************************************************************/
void AP_TimebaseHandler(void)
{
  if(gRxCmdTimeoutCnt)
    gRxCmdTimeoutCnt--;
  
  if(gHeartbeatTimeoutCnt)
    gHeartbeatTimeoutCnt--;
}

/*********************************************************************************************************//**
  * @brief  IAP mode handler.
  * @retval None
  ***********************************************************************************************************/
void AP_Uart_Handler(void)
{ 
    static u8 gRxCmdState = 0;
    u8 c;
    while(ReadByte(&c)) //读取缓存的一个字节
    {
        switch (gRxCmdState)
        {
            case 0:
            {
                if(c == 0x55)   //IAP头定义55
                {
                  u32BufferIndex = 0;
                  gu8CmdBuffer[u32BufferIndex++] = c;
                  gRxCmdState++;
                  gRxCmdTimeoutCnt = 10;
                  continue;
                }
                APPFrameRxCallBack(c);
                return;
            }
            case 1:
            {
                gu8CmdBuffer[u32BufferIndex++] = c;
                gRxCmdState++;
                gRxCmdTimeoutCnt = 10;
                continue;
            }
            case 2:
            {
                gRxCmdTimeoutCnt = 500;

                gu8CmdBuffer[u32BufferIndex++] = c;

                if(u32BufferIndex >= gu8CmdBuffer[1])
                {
                  ParseCmd();
                  gRxCmdState = 0;
                  gHeartbeatTimeoutCnt = 100;
                  return;
                }

                continue;
            }
            default:
            {
                gRxCmdState = 0;

                break;
            }
        }
    }

    if(gRxCmdTimeoutCnt == 0)
    {
        gRxCmdState = 0;
    }

#if 0
    if(gHeartbeatTimeoutCnt == 0)
    {    
        gu8ReturnBuffer[2] = 'C';
        gu8ReturnBuffer[3] = 0x0;
        memcpy((u8*)&gu8ReturnBuffer[5], (u8*)&"HT32 AP", 8);
        AdjustReturnPackage(gu8ReturnBuffer, 5+8);

        WriteBytes(gu8ReturnBuffer, gu8ReturnBuffer[1]);

        gHeartbeatTimeoutCnt = 100;
    }  
#endif
}

/*********************************************************************************************************//**
  * @brief  Reset Command.
  * @param  uMode: Mode after reset
  * @retval FALSE or TRUE
  ***********************************************************************************************************/
u32 AP_Uart_Reset(u32 uMode)
{
  if (uMode == 0)
  {
    #if 0
    // 2017/01/19 發現似乎不需要這個確認，如果有做通訊混肴時還有可能造成沒辦法Reset MCU使停在IAP連不上的情況
    if ((IAP_isAPValid() == FALSE) || (IAP_isVersionValid() == FALSE) )
    {
      /* AP image is invalid. Jump to AP is not allow                                                       */
      __DBG_IAPPrintf("AP incorrect\n\r");
      return FALSE;
    }
    #endif
    
    ww(BOOT_MODE_ID_ADDR, BOOT_MODE_AP);
  }
  else
  {
    /*------------------------------------------------------------------------------------------------------*/
    /* Disconnect USB and wait for PC aware it                                                              */
    /*------------------------------------------------------------------------------------------------------*/
    ww(BOOT_MODE_ID_ADDR, BOOT_MODE_IAP);
  }

  gu8ReturnBuffer[2] = CMD_SUCCESS;
  AdjustReturnPackage(gu8ReturnBuffer, 5);
  WriteBytes(gu8ReturnBuffer, gu8ReturnBuffer[1]);
  
  //delayXuS(100);
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

/*********************************************************************************************************//**
  * @brief  TBD
  * @retval None
  ***********************************************************************************************************/
static u8 CalculateHeaderCheckSum(u8* pCmdBuffer)
{
  u8 i;
  u8 sum = 0;
  
  for(i=0 ; i < 4 ; i++)
  {
    sum += pCmdBuffer[i];
  }
  sum = (~sum ) + 1;
  return sum;
}

/*********************************************************************************************************//**
  * @brief  TBD
  * @retval None
  ***********************************************************************************************************/
static void AdjustReturnPackage(u8* pBuf, u8 length)
{
    u16 crc;

    pBuf[0] = 0x55;
    pBuf[1] = length + 2; // Add CRC length
    pBuf[4] = CalculateHeaderCheckSum(pBuf);

    crc = CRC16(0, (u8 *)(&pBuf[0]), length);
    memcpy((u8*)&pBuf[length], (u8*)&crc, 2);
}

/*********************************************************************************************************//**
  * @brief  TBD
  * @retval None
  ***********************************************************************************************************/
static void ParseCmd(void)
{
  u32 u32Parameter[MAX_TOKENS];
  u16 crc;
  u16 crcValue;
  u8 len;
  
  u32ReturnBufferIndex = 5;
  gu8ReturnBuffer[3] = 0x0;
  
  /*------------------------------------------------------------------------------------------------------*/
  /* Check CRC value of command packet                                                                    */
  /*------------------------------------------------------------------------------------------------------*/
  len = gu8CmdBuffer[1];
  crc = gu8CmdBuffer[len-2] | ((u16)gu8CmdBuffer[len-1] << 8);
  crcValue = CRC16(0, (u8 *)(&gu8CmdBuffer[0]), len-2);

  if (gu8CmdBuffer[2] >= USER_CMD_START)
  {
    gu8CmdBuffer[2] = gu8CmdBuffer[2] - USER_CMD_START + IAP_CMD_COUNT;
  }
  
  /*------------------------------------------------------------------------------------------------------*/
  /* Check command is valid and CRC is correct                                                            */
  /*------------------------------------------------------------------------------------------------------*/
  if (gu8CmdBuffer[2] > CMD_COUNT)
  {
    /*----------------------------------------------------------------------------------------------------*/
    /* Command invalid or CRC error. Return 'F' and clear command buffer                                  */
    /*----------------------------------------------------------------------------------------------------*/
    gu8ReturnBuffer[2] = CMD_FAILED;
    gu8ReturnBuffer[3] = 0x1;
  }
  else if (crc != crcValue)
  {
    gu8ReturnBuffer[2] = CMD_FAILED;
    gu8ReturnBuffer[3] = 0x1;
  }
  else
  {    
    /*----------------------------------------------------------------------------------------------------*/
    /* Prepare parameter and execution command                                                            */
    /*----------------------------------------------------------------------------------------------------*/
    u32Parameter[0] = gu8CmdBuffer[3];
    memcpy((u8*)&u32Parameter[1], (u8*)&gu8CmdBuffer[5], 4);
    memcpy((u8*)&u32Parameter[2], (u8*)&gu8CmdBuffer[9], 4);
    
    gu8ReturnBuffer[2] = (*pFComHandlerTable[gu8CmdBuffer[2]])(u32Parameter[0], u32Parameter[1], u32Parameter[2]);
  }

  /*------------------------------------------------------------------------------------------------------*/
  /* Send Result to Host                                                                                  */
  /*------------------------------------------------------------------------------------------------------*/  
  AdjustReturnPackage(gu8ReturnBuffer, u32ReturnBufferIndex);
  WriteBytes(gu8ReturnBuffer, gu8ReturnBuffer[1]);
}

/*********************************************************************************************************//**
  * @brief  
  * @retval 
  ***********************************************************************************************************/
static u32 AP_Connect(void)
{
  return CMD_SUCCESS;
}


/*********************************************************************************************************//**
  * @brief  
  * @retval 
  ***********************************************************************************************************/
static u32 AP_GetBootMode(void)
{
  gu8ReturnBuffer[u32ReturnBufferIndex++] = 0xF0; // AP mode
  
	return CMD_SUCCESS;
}

/*********************************************************************************************************//**
  * @brief  Download image for program or verify.
  * @param  uPar0: User define paramemter0 (8 bit)
  * @param  uPar1: User define paramemter1 (32 bit)
  * @param  uPar2: User define paramemter2 (32 bit)
  * @retval CMD_SUCCESS or CMD_FAILED
  ***********************************************************************************************************/
#if (USER_CMD_COUNT > 0)  
static u32 AP_User_Cmd(u32 uPar0, u32 uPar1, u32 uPar2)
{
  extern u32 gParameter;

  switch (uPar0)
  {
    case 0x0: // Get FW Information
    {
      return CMD_SUCCESS;
    }
    case 0x1: // Write Parameter
    {
      gParameter = uPar1;
      return CMD_SUCCESS;
    }
    case 0x2: // Read Parameter
    {
      gu8ReturnBuffer[u32ReturnBufferIndex++] = gParameter;
      return CMD_SUCCESS;
    }
  }


  return CMD_FAILED;
}
#endif
