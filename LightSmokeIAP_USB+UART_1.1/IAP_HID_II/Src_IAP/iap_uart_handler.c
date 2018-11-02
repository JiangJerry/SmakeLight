/*********************************************************************************************************//**
 * @file    Src_IAP/iap_handler.c
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

#include "iap_uart_handler.h"
#include "uart.h"

#include "iap_config.h"
#include "arc4.h"
#include "iap_crc16.h"
#include "rough_delay.h"

#include "timebase.h"

/* Private typedef -----------------------------------------------------------------------------------------*/
typedef struct {
  vu32 MDID;                             /* offset  0x180 Flash Manufacturer and Device ID Register (MDID)  */
  vu32 PNSR;                             /* offset  0x184 Flash Page Number Status Register (PNSR)          */
  vu32 PSSR;                             /* offset  0x188 Flash Page Size Status Register (PSSR)            */
  vu32 DID;
} FMCInfo_TypeDef;

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
#define MAX_RETURN_PAYLOAD      (64)
#define CRC_LEN                 (2)
#define MAX_RETURN_LEN          (HEADER_LEN + MAX_RETURN_PAYLOAD + CRC_LEN) // Header + MaxPayload + crc

#define MAX_CMD_LEN             (HEADER_LEN + 4 + 4 + CMD_PAYLOAD_LEN + CRC_LEN)
#define MAX_TOKENS              (3)
#define IAP_CMD_COUNT           (10)
#define CMD_COUNT               (IAP_CMD_COUNT + USER_CMD_COUNT)
#define INF_COUNT               (5)
#define CMD_SUCCESS             ('O')
#define CMD_FAILED              ('F')
#define USER_CMD_START          (0x50)
#define CMD_PAYLOAD_ADDR        (HEADER_LEN + 4 + 4)
#define CMD_PAYLOAD_LEN         (64)

#define PREFIX                  (0x55)

#define CMD_PAGE_ERASE          (1)
#define CMD_MASS_ERASE          (0)

#define CMD_VERIFY              (0)
#define CMD_PROGRAM             (1)
#define CMD_READ                (2)
#define CMD_BLANK               (3)

#define FMCINFO_BASE            (0x40080180)
#define FMCINFO                 ((FMCInfo_TypeDef*) FMCINFO_BASE)

#define LOADER_MODE             (0x1)
#define LOADER_FLASH_START      (IAP_APFLASH_START)
#define LOADER_CHIP_NAME        0//FMCINFO->MDID) & 0x0000FFFF)
#define LOADER_PAGE_SIZE        (FMCINFO->PSSR)
#define LOADER_PPBIT_NUM        (((FMCINFO->PNSR) - IAP_CODE_PAGE) / MCU_PAGE_NUM_PER_PPBIT)
#define LOADER_FLASH_NUM        ((FMCINFO->PNSR) - IAP_CODE_PAGE)
#define LOADER_FULL_CHIP_NAME   (FMCINFO->DID)

#define LOADER_INFO0            (u32)((LOADER_MODE      << 28) | (LOADER_VERSION     << 16) | LOADER_CHIP_NAME)
#define LOADER_INFO1            (u32)((LOADER_PAGE_SIZE << 16) | (LOADER_FLASH_START << 0))
#define LOADER_INFO2            (u32)((LOADER_FLASH_NUM << 16) | (LOADER_PPBIT_NUM   << 0))
#define LOADER_INFO3            (u32)(IAP_CODE_SIZE)
#define LOADER_INFO4            (u32)(LOADER_FULL_CHIP_NAME)

#define FLASH_CMD_PROGRAM       ((u32)0x00000004)
#define FLASH_SEND_MAIN         ((u32)0x00000014)

#define IAP_PAGE_ERASE          (0x8)
#define IAP_MASS_ERASE          (0xA)

#define ERROR_SUCCESS           (0x0)

/* Private function prototypes -----------------------------------------------------------------------------*/
static u32 _IAP_Uart_CMD0(void);
static u32 _IAP_Uart_Erase(u32 type, u32 saddr, u32 eaddr);
static u32 _IAP_Uart_Flash(u32 type, u32 saddr, u32 eaddr);
static u32 _IAP_Uart_CRC(u32 crc, u32 saddr, u32 length);
static u32 _IAP_Uart_Info(void);
static void _IAP_Uart_Exit(void);
static u32 _IAP_Uart_Connect(void);
static u32 _IAP_Uart_GetBootMode(void);

static void FLASH_Operation(u32 type, u32 PageAddress, u32 Data);

#if (USER_CMD_COUNT > 0)  
static u32 _User_Cmd50Example(u32 uPar0, u32 uPar1, u32 uPar2);
#endif
#if (USER_CMD_COUNT > 1)  
static u32 _User_Cmd51Example(u32 uPar0, u32 uPar1, u32 uPar2);
#endif

/* Private macro -------------------------------------------------------------------------------------------*/
#define WriteByte(data)         IAP_UART_Send((u8*)&data, 1)
#define WriteBytes(pData, len)  IAP_UART_Send(pData, len)
#define ReadByte(pData)         IAP_UART_Get(pData, 1, 100)

/* Global variables ----------------------------------------------------------------------------------------*/

#if (IAP_TIMEOUT_EN == 1)
extern s32 gIAPTimeout;
#endif

#if (DIGEST_CHECK_EN == 1)
u8 isDigestCheck = 0;  
u16 iap_crc = 0, digest_crc = 0;
u32 iap_len = 0;
#endif

#if (RESTRAINT_PROGRAMMING_FLOW_EN == 1)
u32 erase_bitmap[8];
#endif

/* Private variables ---------------------------------------------------------------------------------------*/
static u32 gu32Infotable[INF_COUNT];

__ALIGN4 static u8 gu8CmdBuffer[MAX_CMD_LEN];
static u8 gu8ReturnBuffer[MAX_RETURN_LEN];
static u32 u32BufferIndex;
static u32 u32ReturnBufferIndex;
static vu16 gRxCmdTimeoutCnt = 0;
static vu16 gHeartbeatTimeoutCnt = 0;

static u8 CalculateHeaderCheckSum(u8* pCmdBuffer);
static void AdjustReturnPackage(u8* pBuf, u8 length);
static void ParseCmd(void);

static const pFunction pFComHandlerTable[CMD_COUNT] =
{
  (pFunction)_IAP_Uart_Erase,
  (pFunction)_IAP_Uart_Flash,
  (pFunction)_IAP_Uart_CRC,
  (pFunction)_IAP_Uart_Info,
  (pFunction)IAP_Uart_Reset,
  (pFunction)_IAP_Uart_Exit,
 	(pFunction)_IAP_Uart_Connect,
	(pFunction)_IAP_Uart_CMD0,
	(pFunction)_IAP_Uart_GetBootMode,
	(pFunction)_IAP_Uart_CMD0,
  
  #if (USER_CMD_COUNT > 0)  
  (pFunction)_User_Cmd50Example,
  #endif  
  #if (USER_CMD_COUNT > 1)  
  (pFunction)_User_Cmd51Example
  #endif  
};

/* Global functions ----------------------------------------------------------------------------------------*/
#if ((ARC4_DECRYPTIOIN_EN != 0) || (DIGEST_CHECK_EN != 0))
static const unsigned char Encrypt_Key[64] = {
  0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
  0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
  0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
  0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F
};
#endif

#if (COMMAND_KEY_EN != 0)
static const unsigned char Command_Key[64] = {
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
  0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
  0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
  0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F
};
#endif

/*********************************************************************************************************//**
  * @brief  ARC4 decryption.
  * @retval None
  ***********************************************************************************************************/
#if (ARC4_DECRYPTIOIN_EN != 0)
static u32 arc4_decrypt(u8 *ibuf, u8 *obuf, u8 len)
{
  arc4_context ctx;

  arc4_init(&ctx);
  arc4_setup(&ctx, Encrypt_Key, ARC4_KEY_LEN);
  arc4_crypt(&ctx, len, ibuf, obuf);
  arc4_free(&ctx);

  return 0;
}
#endif

/*********************************************************************************************************//**
  * @brief  IAP mode initialization.
  * @retval None
  ***********************************************************************************************************/
void IAP_Uart_Init(void)
{
  Timebase_Init();
  Timebase_InterruptCmd(ENABLE);
  
  gu32Infotable[0] = LOADER_INFO0;
  gu32Infotable[1] = LOADER_INFO1;
  gu32Infotable[2] = LOADER_INFO2;
  gu32Infotable[3] = LOADER_INFO3;
  gu32Infotable[4] = LOADER_INFO4;

  IAP_UART_Init();
}

/*********************************************************************************************************//**
  * @brief  TBD
  * @retval None
  ***********************************************************************************************************/
void IAP_TimebaseHandler(void)
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
void IAP_Uart_Handler(void)
{ 
  static u8 gRxCmdState = 0;
  u8 c;

  if(IAP_Update_Port==1) return;
  
  while(ReadByte(&c))
  {
    switch (gRxCmdState)
    {
      case 0:
      {
        if(c == 0x55)
        {
          u32BufferIndex = 0;
          gu8CmdBuffer[u32BufferIndex++] = c;
          gRxCmdState++;
          gRxCmdTimeoutCnt = 10;
          continue;
        }
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
    memcpy((u8*)&gu8ReturnBuffer[5], (u8*)&"HT32 IAP", 8);
    AdjustReturnPackage(gu8ReturnBuffer, 5+8);
    
    WriteBytes(gu8ReturnBuffer, gu8ReturnBuffer[1]);
    
    gHeartbeatTimeoutCnt = 100;
  }  
  #endif
}

///*********************************************************************************************************//**
//  * @brief  Reset Command.
//  * @param  uMode: Mode after reset
//  * @retval FALSE or TRUE
//  ***********************************************************************************************************/
u32 IAP_Uart_Reset(u32 uMode)
{
  if (uMode == 0)
  {
    #if 0
    // 2017/01/19 發現似乎不需要這個確認，如果有做通訊混肴時還有可能造成沒辦法Reset MCU使停在IAP連不上的情況
    if ((IAP_Uart_isAPValid() == FALSE) || (IAP_isVersionValid() == FALSE) )
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

///*********************************************************************************************************//**
//  * @brief  Check AP is valid or not.
//  * @retval FALSE or TRUE
//  ***********************************************************************************************************/
u32 IAP_Uart_isAPValid(void)
{
  vu32 SP, PC;
  u32 i, ver, len, veraddr;

  /* Check Stack Point in range                                                                             */
  SP = rw(IAP_APFLASH_START);
  if (SP < IAP_SRAM_START || SP > IAP_SRAM_END)
  {
    __DBG_IAPPrintf("AP SP Err[%08X]\n\r", SP);
    //__NOP();
    return FALSE;
  }

  /* Check PC in range                                                                                      */
  PC = rw(IAP_APFLASH_START + 0x4);
  if (PC < IAP_APFLASH_START || PC > IAP_APFLASH_END)
  {
    __DBG_IAPPrintf("AP PC Err[%08X]\n\r", PC);
    //__NOP();
    return FALSE;
  }
  
  veraddr = IAP_CODE_SIZE + 8;
  len = rw(veraddr);
  
  if (len == 0xFFFFFFFF)
  {
    //__NOP();
    return FALSE;
  }
  
  veraddr += 0x4;
  for (i = 0; i < len; i += 4)
  {
    ver = rw(veraddr + i);
    if (ver == 0xFFFFFFFF)
    {
      __DBG_IAPPrintf("Version Err[%08X]\n\r", ver);
      //__NOP();
      return FALSE;
    }
  }

  #if 1 // Check CRC
	{
    
  u32 crc16, len;
	u32 crc16_addr = IAP_CODE_SIZE;
	u32 crc16_len_addr = IAP_CODE_SIZE+4;
	u32 crc16_saddr = IAP_APFLASH_START;
  
  crc16 = rw(crc16_addr);
	len = rw(crc16_len_addr);
	
	if((crc16 == FW_INF_ENGINEERING_CRC) && 
		 (len == FW_INF_ENGINEERING_LENGTH)) 
  {
    //__NOP();
		return TRUE;
  }
	
	if(((crc16_saddr < IAP_APFLASH_START) || (crc16_saddr > IAP_APFLASH_END)) ||
		(len > ((LOADER_FLASH_NUM * MCU_FLASH_PAGE_SIZE) - IAP_APINFO_SIZE)) || 
		((crc16_saddr + len) > (IAP_APFLASH_END + 1)))
	{
    //__NOP();
		return FALSE;
	}
	
	if(len == 0xFFFFFFFF)
	{
    //__NOP();
		return FALSE;
	}
	
	if((u16)crc16 != CRC16(0, (u8 *)crc16_saddr, len))		
  {
    //__NOP();
		return FALSE;
  }
  }
  #endif
  
  return TRUE;
}

///*********************************************************************************************************//**
//  * @brief  Check address is in version region.
//  * @retval FALSE or TRUE
//  ***********************************************************************************************************/
u32 IAP_Uart_isVersionRegion(u32 saddr)
{
  if ((saddr >= IAP_CODE_SIZE) && (saddr < IAP_APFLASH_START))
    return TRUE;
  else
    return FALSE;
}

//#if defined (__CC_ARM)
///*********************************************************************************************************//**
//  * @brief  Jump to user application by change PC.
//  * @param  address: Start address of user application
//  * @retval None
//  ***********************************************************************************************************/
//__asm void IAP_GoCMD(u32 address)
//{
//  LDR R1, [R0]
//  MOV SP, R1
//  LDR R1, [R0, #4]
//  BX R1
//}
//#elif defined (__ICCARM__)
//void IAP_GoCMD(u32 address)
//{
//  __asm("LDR R1, [R0]");
//  __asm("MOV SP, R1");
//  __asm("LDR R1, [R0, #4]");
//  __asm("BX R1");
//}
//#endif

///* Private functions ---------------------------------------------------------------------------------------*/
///*********************************************************************************************************//**
//  * @brief isValidationErase
//  * @param
//  * @retval CMD_SUCCESS or CMD_FAILED
//  ***********************************************************************************************************/
#if (RESTRAINT_PROGRAMMING_FLOW_EN == 1)
static u8 isValidationErase()
{
  u32 idx, shift, count = 0;
  for (idx = 0; idx < 8; idx++)
  {
    u32 tmp = erase_bitmap[idx];
    shift = 32;
    while(shift--)
    {
      count += tmp & 0x1;
      tmp >>= 1;
    }
  }

  if (count >= MINIMUM_ERASE_SIZE)
  {
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}
#endif

///*********************************************************************************************************//**
//  * @brief  TBD
//  * @retval None
//  ***********************************************************************************************************/
u8 CalculateHeaderCheckSum(u8* pCmdBuffer)
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

///*********************************************************************************************************//**
//  * @brief  TBD
//  * @retval None
//  ***********************************************************************************************************/
void AdjustReturnPackage(u8* pBuf, u8 length)
{
  u16 crc;
  
  pBuf[0] = 0x55;
  pBuf[1] = length + 2; // Add CRC length
  pBuf[4] = CalculateHeaderCheckSum(pBuf);
  
  crc = CRC16(0, (u8 *)(&pBuf[0]), length);
  memcpy((u8*)&pBuf[length], (u8*)&crc, 2);
}

///*********************************************************************************************************//**
//  * @brief  TBD
//  * @retval None
//  ***********************************************************************************************************/
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

  #if (COMMAND_KEY_EN == 1)
  crcValue = CRC16(crcValue, (u8 *)Command_Key, sizeof(Command_Key));
  #endif

  if (gu8CmdBuffer[2] >= USER_CMD_START)
  {
    IAP_Update_Port=2;
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
    /* Command is OK, stop SysTick to prevent timeout reset                                               */
    /*----------------------------------------------------------------------------------------------------*/
    #if (IAP_TIMEOUT_EN == 1)
    SYSTICK_CounterCmd(SYSTICK_COUNTER_DISABLE);
    #endif
    
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

///*********************************************************************************************************//**
//  * @brief  
//  * @retval 
//  ***********************************************************************************************************/
static u32 _IAP_Uart_CMD0(void)
{
	return CMD_SUCCESS;
}

///*********************************************************************************************************//**
//  * @brief  Mass/Page Erase.
//  * @param  type: Erase type
//  *         @arg IAP_MASS_ERASE: Mass Erase (Not support in IAP mode)
//  *         @arg IAP_PAGE_ERASE: Page Erase
//  * @param  saddr: Start address
//  * @param  eaddr: End address
//  * @retval CMD_SUCCESS or CMD_FAILED
//  ***********************************************************************************************************/
static u32 _IAP_Uart_Erase(u32 type, u32 saddr, u32 eaddr)
{
  u32 i, j;

  #if (RESTRAINT_PROGRAMMING_FLOW_EN == 1)
  u32 n, sidx = 0, eidx = 0, idx = 0, shift = 0;
  #endif

  if (type == IAP_MASS_ERASE || saddr < IAP_CODE_SIZE)
  {
    gu8ReturnBuffer[3] = 0x3;
    return CMD_FAILED;
  }

  #if (RESTRAINT_PROGRAMMING_FLOW_EN == 1)
  if (saddr == IAP_APFLASH_START)
  {

    for (i = 0; i < 8; i++)
      erase_bitmap[i] = 0;
  }
  #endif

  for (i = saddr, j = 0; i <= eaddr; i += FLASH_PAGE_SIZE, j++)
  {
    FLASH_ErasePage(i);

    #if (WDT_EN != 0)
    if ((j & 0x1F) == 0)
    {
      WDT_RESTART();
    }
    #endif
  }

  #if (RESTRAINT_PROGRAMMING_FLOW_EN == 1)
  if (saddr >= IAP_APFLASH_START)
  {
    sidx = (saddr - IAP_APFLASH_START) >> ( 11 - (1024 / FLASH_PAGE_SIZE)); // (saddr - IAP_APFLASH_START) / FLASH_PAGE_SIZE
    eidx = (eaddr - IAP_APFLASH_START) >> ( 11 - (1024 / FLASH_PAGE_SIZE)); // (eaddr - IAP_APFLASH_START) / FLASH_PAGE_SIZE

    for (n = sidx; n <= eidx; n++)
    {
      idx = n >> 5;       // n / 32
      shift= n & 0x1F;    // n % 32
      erase_bitmap[idx] |= (0x1ul << shift);
    }
  }
  #endif


  #if (DIGEST_CHECK_EN == 1)
  iap_crc = 0;
  iap_len = 0;
  isDigestCheck = 0;
  #endif

  return CMD_SUCCESS;
}

///*********************************************************************************************************//**
//  * @brief  Download image for program or verify.
//  * @param  type: Program or verify
//  *         @arg CMD_PROGRAM: Program mode
//  *         @arg CMD_VERIFY: Verify mode
//  *         @arg CMD_BLANK: Blank check mode
//  *         @arg CMD_READ: Read mode
//  * @param  saddr: Start address
//  * @param  eaddr: End address
//  * @param  buffer: point of data buffer
//  * @retval CMD_SUCCESS or CMD_FAILED
//  ***********************************************************************************************************/
static u32 _IAP_Uart_Flash(u32 type, u32 saddr, u32 eaddr)
{
  u32 i, data;
  u8 *buffer = (u8 *)(&gu8CmdBuffer[13]);
  FLASH_OptionByte Option;

  /*--------------------------------------------------------------------------------------------------------*/
  /* When Security protection is enabled, read operation is not allowed                                     */
  /*--------------------------------------------------------------------------------------------------------*/
  FLASH_GetOptionByteStatus(&Option);

  if (type == CMD_PROGRAM)
  {
    /* Blank Check before programming                                                                       */
    for (i = 0; i < (eaddr-saddr + 1); i += 4)
    {
      if (rw(saddr + i) != 0xFFFFFFFF )
      {
        gu8ReturnBuffer[3] = 0x4;
        return CMD_FAILED;
      }
    }
  }

  /*--------------------------------------------------------------------------------------------------------*/
  /* ARC4 Decryptioin Implementation ...                                                                    */
  /*--------------------------------------------------------------------------------------------------------*/
  switch (type)
  {
    case CMD_PROGRAM:
    case CMD_VERIFY:
    {
      if (IAP_Uart_isVersionRegion(saddr)){
      #if (DIGEST_CHECK_EN == 1)
        if (isDigestCheck == 0)
          return CMD_SUCCESS;
      #endif
      }else{
      #if (ARC4_DECRYPTIOIN_EN == 1)
        arc4_decrypt((u8*)buffer, (u8*)buffer, (eaddr-saddr + 1));
      #endif
      }
      break;
    }

    default:
      break;
  }

  /*--------------------------------------------------------------------------------------------------------*/
  /* Program                                                                                                */
  /*--------------------------------------------------------------------------------------------------------*/
  if (type == CMD_PROGRAM)
  {
    #if (RESTRAINT_PROGRAMMING_FLOW_EN == 1)
    if (!isValidationErase())
      return CMD_SUCCESS;
    #endif

    #if (DIGEST_CHECK_EN == 1)
    if (!IAP_isVersionRegion(saddr)){
      iap_crc = CRC16(iap_crc, (u8 *)buffer, (eaddr-saddr + 1));
      iap_len += (eaddr-saddr + 1);
    }
    #endif

    while (saddr <= eaddr)
    {
      u32 tmp;
      memcpy((u8*)&tmp, (u8*)buffer, 4);

      FLASH_Operation(FLASH_CMD_PROGRAM, saddr, tmp);
      saddr += 4;
      buffer += 4;
    }
  }

  while (saddr <= eaddr)
  {
    if (saddr > 0x1FF003FC)
    {
      data = 0;
    }
    else
    {
      data = rw(saddr);
    }
    /*------------------------------------------------------------------------------------------------------*/
    /* Verify                                                                                               */
    /*------------------------------------------------------------------------------------------------------*/
    if (type == CMD_VERIFY)
    {
      u32 tmp;
      memcpy((u8*)&tmp, (u8*)buffer, 4);

      if (data != tmp)
      {
        gu8ReturnBuffer[3] = 0x4;
        return CMD_FAILED;
      }
    }
    /*------------------------------------------------------------------------------------------------------*/
    /* Blank                                                                                                */
    /*------------------------------------------------------------------------------------------------------*/
    else if (type == CMD_BLANK)
    {
      if (data != 0xFFFFFFFF)
      {
        gu8ReturnBuffer[3] = 0x4;
        return CMD_FAILED;
      }
    }
    /*--------------------------------------------------------------------------------------------------------*/
    /* Read                                                                                                   */
    /*--------------------------------------------------------------------------------------------------------*/   
    else
    {
      if (Option.MainSecurity == 1)
      {
        if ((saddr >= IAP_CODE_SIZE) && (saddr < (IAP_CODE_SIZE + IAP_APINFO_SIZE)) )
        {
          goto readok;
        }

        if (saddr < 0x1FF00000 || saddr >= 0x1FF00050)
        {
          data = 0;
        }
      }      

readok:
      memcpy((u8*)&gu8ReturnBuffer[u32ReturnBufferIndex], (u8*)&data, 4);
      u32ReturnBufferIndex += 4;
    }    

    saddr += 4;
    buffer += 4;
  }

  return CMD_SUCCESS;
}

///*********************************************************************************************************//**
//  * @brief  Calculate CRC value.
//  * @param  crc: Iinitial value of CRC (Usually as 0)
//  * @param  saddr: Start address
//  * @param  length: Length for CRC calculation
//  * @retval Always success (CMD_SUCCESS)
//  ***********************************************************************************************************/
static u32 _IAP_Uart_CRC(u32 crc, u32 saddr, u32 length)
{
  crc = CRC16(crc, (u8 *)saddr, length);

  #if (DIGEST_CHECK_EN == 1)
  digest_crc = (u16)(gu8CmdBuffer[13] | (gu8CmdBuffer[14] << 8));
  
  iap_crc = CRC16(iap_crc, (u8*)Encrypt_Key, ARC4_KEY_LEN);
  
  if (iap_crc == digest_crc)
  {
    // Validation Digest
    isDigestCheck = 1;
  }
  else
  {
    // Invalidation Digest
    // Erase AP code...
    _IAP_Erase(IAP_PAGE_ERASE, IAP_APFLASH_START, IAP_APFLASH_START + iap_len);
  }
  #endif

  memcpy((u8*)&gu8ReturnBuffer[u32ReturnBufferIndex], (u8*)&crc, 2);
  u32ReturnBufferIndex += 2;
  
  return CMD_SUCCESS;
}

///*********************************************************************************************************//**
//  * @brief  Send information to Host.
//  * @retval Always success (CMD_SUCCESS)
//  ***********************************************************************************************************/
static u32 _IAP_Uart_Info(void)
{
  gu8ReturnBuffer[2] = CMD_SUCCESS;

  memcpy((u8*)&gu8ReturnBuffer[5], (u8*)&gu32Infotable[0], 12);
  AdjustReturnPackage(gu8ReturnBuffer, 5+12);
  WriteBytes(gu8ReturnBuffer, gu8ReturnBuffer[1]);

  memcpy((u8*)&gu8ReturnBuffer[u32ReturnBufferIndex], (u8*)&gu32Infotable[3], 8);
  u32ReturnBufferIndex += 8;
  
  return CMD_SUCCESS;
}

///*********************************************************************************************************//**
//  * @brief  Exit Loader mode.
//  * @retval None
//  ***********************************************************************************************************/
static void _IAP_Uart_Exit(void)
{
  gu8ReturnBuffer[2] = CMD_SUCCESS;
  AdjustReturnPackage(gu8ReturnBuffer, 5);
  WriteBytes(gu8ReturnBuffer, gu8ReturnBuffer[1]);
  
  while (1);
}

///*********************************************************************************************************//**
//  * @brief  
//  * @retval 
//  ***********************************************************************************************************/
static u32 _IAP_Uart_Connect(void)
{
  return CMD_SUCCESS;
}

///*********************************************************************************************************//**
//  * @brief  
//  * @retval 
//  ***********************************************************************************************************/
static u32 _IAP_Uart_GetBootMode(void)
{
  gu8ReturnBuffer[u32ReturnBufferIndex++] = 0xF5; // IAP mode
  
	return CMD_SUCCESS;
}

///*********************************************************************************************************//**
//  * @brief  Download image for program or verify.
//  * @param  uPar0: User define paramemter0 (8 bit)
//  * @param  uPar1: User define paramemter1 (32 bit)
//  * @param  uPar2: User define paramemter2 (32 bit)
//  * @retval CMD_SUCCESS or CMD_FAILED
//  ***********************************************************************************************************/
#if (USER_CMD_COUNT > 0)  
static u32 _User_Cmd50Example(u32 uPar0, u32 uPar1, u32 uPar2)
{
  switch (uPar0)
  {
    case 0x1:
    {
      // GPIO_WriteOutBits(HT_GPIOA, GPIO_PIN_15, (FlagStatus)!uPar1);
      return CMD_SUCCESS;
    }
    case 0x2:
    {
      // GPIO_WriteOutBits(HT_GPIOB, GPIO_PIN_0, (FlagStatus)!uPar1);
      return CMD_SUCCESS;
    }
    case 0x3:
    {
      // GPIO_WriteOutBits(HT_GPIOB, GPIO_PIN_1, (FlagStatus)!uPar1);
      return CMD_SUCCESS;
    }
  }

  return CMD_FAILED;
}
#endif

#if (USER_CMD_COUNT > 1)  
///*********************************************************************************************************//**
//  * @brief  Download image for program or verify.
//  * @param  uPar0: User define paramemter0 (8 bit)
//  * @param  uPar1: User define paramemter1 (32 bit)
//  * @param  uPar2: User define paramemter2 (32 bit)
//  * @retval CMD_SUCCESS or CMD_FAILED
//  ***********************************************************************************************************/
static u32 _User_Cmd51Example(u32 uPar0, u32 uPar1, u32 uPar2)
{
  u8 uUserData[12];
  u8 *pPayload = (u8 *)(&gu8CmdBuffer[CMD_PAYLOAD_ADDR]);
  u32 i;

  switch (uPar0)
  {
    case 0x0:
    {
      for(i = 0; i < 12; i++)
      {
        uUserData[i] = *pPayload++;
      }
      return CMD_SUCCESS;
    }
    case 0x1:
    {
      for(i = 0; i < CMD_PAYLOAD_LEN; i+=4)
      {
        *ptr++ = *((u32 *)(&uUserData[i]));
      }

      memcpy((u8*)&gu8ReturnBuffer[u32ReturnBufferIndex], (u8*)&uUserData[0], 12);
      u32ReturnBufferIndex += 12;     
      
      return CMD_SUCCESS;
    }
  }

  return CMD_FAILED;
}
#endif

///*********************************************************************************************************//**
//  * @brief  FLASH_Operation
//  * @retval None
//  ***********************************************************************************************************/
static void FLASH_Operation(u32 type, u32 PageAddress, u32 Data)
{
  HT_FLASH->OCMR = type;
  HT_FLASH->TADR = PageAddress;
  HT_FLASH->WRDR = Data;
  HT_FLASH->OPCR = FLASH_SEND_MAIN;
}
