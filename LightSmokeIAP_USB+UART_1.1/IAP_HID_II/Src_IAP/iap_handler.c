/*********************************************************************************************************//**
 * @file    IAP/IAP_HID_II/Src_IAP/iap_handler.c
 * @version $Rev:: 2524         $
 * @date    $Date:: 2018-01-29 #$
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

/* Includes ------------------------------------------------------------------------------------------------*/
#include "ht32.h"
#include "ht32_board.h"
#include "ht32_board_config.h"
#include "ht32_usbd_core.h"

#include "iap_handler.h"
#include "_HT32_USBBufCheck.h"
#include "iap_config.h"
#include "buffer.h"
#include "iap_crc16.h"



/** @addtogroup HT32_Series_Peripheral_Examples HT32 Peripheral Examples
  * @{
  */

/** @addtogroup IAP_HID IAP HID
  * @{
  */

/** @addtogroup IAP_HID_II IAP HID II (with USB Check buffer)
  * @{
  */

/** @addtogroup IAP_HID_II_IAP IAP Example
  * @{
  */


/* Private types -------------------------------------------------------------------------------------------*/
typedef struct {
  vu32 MDID;                             /* offset  0x180 Flash Manufacturer and Device ID Register (MDID)  */
  vu32 PNSR;                             /* offset  0x184 Flash Page Number Status Register (PNSR)          */
  vu32 PSSR;                             /* offset  0x188 Flash Page Size Status Register (PSSR)            */
  vu32 DID;
} FMCInfo_TypeDef;

typedef u32 (*pFunction)();

/* Private constants ---------------------------------------------------------------------------------------*/
#define __DBG_IAPPrintf(...)
#define __DBG_IAP_DumpData(...)

#if (IAP_DEBUG_MODE == 1)
  #undef __DBG_IAPPrintf
  #undef __DBG_IAP_DumpData
  #define __DBG_IAPPrintf printf
  static void __DBG_IAP_DumpData(uc8 *memory, u32 len);
  #warning "================================ IAP Debug Mode Warning ================================"
  #warning " 1. IAP debug mode has been enable which degrade the performance.                       "
  #warning " 2. Note that print debug message too much may cause the IAP command not synchronized.  "
  #warning " 3. Remember to modify Host/Device IAP_CODE_PAGE define according to the IAP Size       "
  #warning "    after enable IAP debug mode.                                                        "
  #warning " 4. After all debug operation is finished, please remember to turn off IAP debug mode.  "
  #warning "========================================================================================"
#endif

/*  User Command count                                                                                      */
#define USER_CMD_COUNT                (2)

#define MAX_CMD_LEN             (64)
#define MAX_TOKENS              (3)
#define ISP_CMD_COUNT           (6)
#define CMD_COUNT               (ISP_CMD_COUNT + USER_CMD_COUNT)
#define INF_COUNT               (5)
#define CMD_SUCCESS             ('O')
#define CMD_FAILED              ('F')
#define USER_CMD_START          (0x50)
#define CMD_PAYLOAD_ADDR        (12)
#define CMD_PAYLOAD_LEN         (52)

#define CMD_PAGE_ERASE          (1)
#define CMD_MASS_ERASE          (0)

#define CMD_VERIFY              (0)
#define CMD_PROGRAM             (1)
#define CMD_READ                (2)
#define CMD_BLANK               (3)

#define FMCINFO_BASE            (0x40080180)
#define FMCINFO                 ((FMCInfo_TypeDef*) FMCINFO_BASE)

#if defined(LIBCFG_FLASH_2PAGE_PER_WPBIT)
  #define PPBIT_PRE_PAGE         2
#else
  #define PPBIT_PRE_PAGE         1
#endif

#define LOADER_MODE             (0x1)
#define LOADER_FLASH_START      (IAP_CODE_SIZE + IAP_VERSION_SIZE)
#define LOADER_CHIP_NAME        (0)
#define LOADER_PAGE_SIZE        (FMCINFO->PSSR)
#define LOADER_PPBIT_NUM        ((FMCINFO->PNSR) - (IAP_CODE_SIZE/LIBCFG_FLASH_PAGESIZE/PPBIT_PRE_PAGE) - 1)
#define LOADER_FLASH_NUM        ((LIBCFG_FLASH_SIZE - IAP_CODE_SIZE - IAP_VERSION_SIZE)/LIBCFG_FLASH_PAGESIZE)
#define LOADER_FULL_CHIP_NAME   ((FMCINFO->DID) & 0x0FFFFFFF)

#define LOADER_INFO0            (u32)((LOADER_MODE      << 28) | (LOADER_VERSION     << 16) | LOADER_CHIP_NAME)
#define LOADER_INFO1            (u32)((LOADER_PAGE_SIZE << 16) | (LOADER_FLASH_START << 0))
#define LOADER_INFO2            (u32)((LOADER_FLASH_NUM << 16) | (LOADER_PPBIT_NUM   << 0))
#define LOADER_INFO3            (u32)(IAP_CODE_SIZE)
#define LOADER_INFO4            (u32)(LOADER_FULL_CHIP_NAME)

#define FLASH_CMD_PROGRAM       ((u32)0x00000004)
#define FLASH_SEND_MAIN         ((u32)0x00000014)

#define IAP_PAGE_ERASE          (0x8)
#define IAP_MASS_ERASE          (0xA)

#define IAP_APFLASH_END         (IAP_APFLASH_SIZE - 1)
#define IAP_APSRAM_END          (HT_SRAM_BASE + IAP_APSRAM_SIZE)

#define IAP_VERFLASH_START      (IAP_CODE_SIZE)

/* Private function prototypes -----------------------------------------------------------------------------*/
static u32 _IAP_Erase(u32 type, u32 saddr, u32 eaddr);
static u32 _IAP_Flash(u32 type, u32 saddr, u32 eaddr);
static u32 _IAP_CRC(u32 crc, u32 saddr, u32 length);
static u32 _IAP_Info(void);
static void _IAP_Exit(void);

static void _IAP_SendDataAndWaitFinish(void);
void FLASH_Operation(u32 type, u32 PageAddress, u32 Data);

#if (EXT_FLASH_ENABLE == 1)
static u32 _IAP_EXT_Flash(u32 type, u32 saddr, u32 eaddr);
static u32 _IAP_EXT_Erase(u32 type, u32 saddr, u32 eaddr);
#endif

static u32 _User_Cmd50Example(u32 uPar0, u32 uPar1, u32 uPar2);
static u32 _User_Cmd51Example(u32 uPar0, u32 uPar1, u32 uPar2);

/* Private macro -------------------------------------------------------------------------------------------*/
#define WriteByte(data)         Buffer_WriteByte(&gTx, data)
#define ReadByte(data)          Buffer_ReadByte(&gRx, data)

/* Global variables ----------------------------------------------------------------------------------------*/
Buffer_TypeDef gTx;
Buffer_TypeDef gRx;

/* Private variables ---------------------------------------------------------------------------------------*/
__ALIGN4 static u8 guRxBuffer[RX_BUFFER_SIZE];
__ALIGN4 static u8 guTxBuffer[TX_BUFFER_SIZE];
static u32 gu32Infotable[INF_COUNT];
__ALIGN4 static u8 gu8CmdBuffer[MAX_CMD_LEN];
static u32 u32BufferIndex;
u32 *ptr = (u32 *)_EP3STADR;

static u8 uUserData[52];

static const pFunction pFComHandlerTable[CMD_COUNT] =
{
  (pFunction)_IAP_Erase,
  (pFunction)_IAP_Flash,
  (pFunction)_IAP_CRC,
  (pFunction)_IAP_Info,
  (pFunction)IAP_Reset,
  (pFunction)_IAP_Exit,
  (pFunction)_User_Cmd50Example,
  (pFunction)_User_Cmd51Example
};


/* Global functions ----------------------------------------------------------------------------------------*/
/*********************************************************************************************************//**
  * @brief  IAP mode initialization.
  * @retval None
  ***********************************************************************************************************/
void IAP_Init(void)
{
  #if (IAP_DEBUG_MODE == 1)
  RETARGET_Configuration();
  #endif

  gu32Infotable[0] = LOADER_INFO0;
  gu32Infotable[1] = LOADER_INFO1;
  gu32Infotable[2] = LOADER_INFO2;
  gu32Infotable[3] = LOADER_INFO3;
  gu32Infotable[4] = LOADER_INFO4;

  Buffer_Init(&gTx, guTxBuffer, sizeof(guTxBuffer));
  Buffer_Init(&gRx, guRxBuffer, sizeof(guRxBuffer));
}

/*********************************************************************************************************//**
  * @brief  IAP mode handler.
  * @retval None
  ***********************************************************************************************************/
void IAP_Handler(void)
{
  u32 u32Parameter[MAX_TOKENS];
  u32 u32CommandExecuteResult;
  u16 crc;
  u16 crcValue;

  if(IAP_Update_Port!=2)  
  {
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

    if (gu8CmdBuffer[0] >= USER_CMD_START)
    {
      IAP_Update_Port=1;
      gu8CmdBuffer[0] = gu8CmdBuffer[0] - USER_CMD_START + ISP_CMD_COUNT;
    }

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
u32 IAP_Reset(u32 uMode)
{
  s32 i;

  _HT32_USBBufCheck_DeInit();

  if (uMode == 0)
  {
    if ((IAP_isAPValid() == FALSE) || (IAP_isVersionValid() == FALSE))
    {
      /* AP image is invalid. Jump to AP is not allow                                                       */
      __DBG_IAPPrintf("AP incorrect\n\r");
      return FALSE;
    }
    BOOT_MODE = BOOT_MODE_AP;
  }
  else
  {
    BOOT_MODE = BOOT_MODE_IAP;
  }

  /*--------------------------------------------------------------------------------------------------------*/
  /* Disconnect USB and wait for PC aware it                                                                */
  /*--------------------------------------------------------------------------------------------------------*/
  NVIC_DisableIRQ(USB_IRQn);
  HT32F_DVB_USBDisConnect();
  for (i = 400000; i != 0; i--);
  NVIC_SystemReset();

  return TRUE;
}

/*********************************************************************************************************//**
  * @brief  Check AP is valid or not.
  * @retval FALSE or TRUE
  ***********************************************************************************************************/
u32 IAP_isAPValid(void)
{
  u32 SP, PC;

  /* Check Stack Point in range                                                                             */
  SP = rw(IAP_APFLASH_START);
  if (SP < IAP_APSRAM_START || SP > IAP_APSRAM_END)
  {
    __DBG_IAPPrintf("AP SP Err[%08X]\n\r", SP);
    return FALSE;
  }

  /* Check PC in range                                                                                      */
  PC = rw(IAP_APFLASH_START + 0x4);
  if (PC < IAP_APFLASH_START || PC > IAP_APFLASH_END)
  {
    __DBG_IAPPrintf("AP PC Err[%08X]\n\r", PC);
    return FALSE;
  }

  return TRUE;
}

/*********************************************************************************************************//**
  * @brief  Check version is valid or not.
  * @retval FALSE or TRUE
  ***********************************************************************************************************/
u32 IAP_isVersionValid(void)
{
  u32 i, ver, len;
  u32 veraddr = IAP_CODE_SIZE;
  len = rw(veraddr);
  if (len == 0xFFFFFFFF)
  {
    return FALSE;
  }
  veraddr += 0x4;
  for (i = 0; i < len; i += 4)
  {
    ver = rw(veraddr + i);
    if (ver == 0xFFFFFFFF)
    {
      __DBG_IAPPrintf("Version Err[%08X]\n\r", ver);
      return FALSE;
    }
  }

  return TRUE;
}

#if defined (__CC_ARM)
/*********************************************************************************************************//**
  * @brief  Jump to user application by change PC.
  * @param  address: Start address of user application
  * @retval None
  ***********************************************************************************************************/
__asm void IAP_GoCMD(u32 address)
{
  LDR R1, [R0]
  MOV SP, R1
  LDR R1, [R0, #4]
  BX R1
}
#elif defined (__ICCARM__)
void IAP_GoCMD(u32 address)
{
  __asm("LDR R1, [R0]");
  __asm("MOV SP, R1");
  __asm("LDR R1, [R0, #4]");
  __asm("BX R1");
}
#endif

/* Private functions ---------------------------------------------------------------------------------------*/
/*********************************************************************************************************//**
  * @brief  Mass/Page Erase.
  * @param  type: Erase type
  *         @arg IAP_MASS_ERASE: Mass Erase (Not support in IAP mode)
  *         @arg IAP_PAGE_ERASE: Page Erase
  * @param  saddr: Start address
  * @param  eaddr: End address
  * @retval CMD_SUCCESS or CMD_FAILED
  ***********************************************************************************************************/
static u32 _IAP_Erase(u32 type, u32 saddr, u32 eaddr)
{
  u32 i;
  #if (EXT_FLASH_ENABLE == 1)
  if (saddr >= EXT_FALSH_ADDRESS)
  {
    return (_IAP_EXT_Erase(type, saddr, eaddr));
  }
  #endif

  if (type == IAP_MASS_ERASE || saddr < IAP_CODE_SIZE)
  {
    return CMD_FAILED;
  }

  for (i = saddr; i <= eaddr; i += FLASH_PAGE_SIZE)
  {
    FLASH_ErasePage(i);
  }
  return CMD_SUCCESS;
}

/*********************************************************************************************************//**
  * @brief  Download image for program or verify.
  * @param  type: Program or verify
  *         @arg CMD_PROGRAM: Program mode
  *         @arg CMD_VERIFY: Verify mode
  *         @arg CMD_BLANK: Blank check mode
  *         @arg CMD_READ: Read mode
  * @param  saddr: Start address
  * @param  eaddr: End address
  * @param  buffer: point of data buffer
  * @retval CMD_SUCCESS or CMD_FAILED
  ***********************************************************************************************************/
static u32 _IAP_Flash(u32 type, u32 saddr, u32 eaddr)
{
  u32 *buffer = (u32 *)(&gu8CmdBuffer[CMD_PAYLOAD_ADDR]);
  u32 i = 0, j = 0, data;
  u32 isReadProtection = 0;
  FLASH_OptionByte Option;
  FLASH_GetOptionByteStatus(&Option);

  #if (EXT_FLASH_ENABLE == 1)
  if (saddr >= EXT_FALSH_ADDRESS)
  {
    return (_IAP_EXT_Flash(type, saddr, eaddr));
  }
  #endif

  /*--------------------------------------------------------------------------------------------------------*/
  /* When Security protection is enabled, Flash operation is not allowed                                    */
  /*--------------------------------------------------------------------------------------------------------*/
  if (Option.MainSecurity == ENABLE)
  {
    isReadProtection = 1;
  }

  if (type == CMD_PROGRAM && isReadProtection == 1)
  {
    for (i = 0; i < (eaddr - saddr + 1); i += 4)
    {
      if (rw(saddr + i) != 0xFFFFFFFF )
        return CMD_FAILED;
    }
  }

  /*--------------------------------------------------------------------------------------------------------*/
  /* Program                                                                                                */
  /*--------------------------------------------------------------------------------------------------------*/
  if (type == CMD_PROGRAM)
  {
    while (saddr <= eaddr)
    {
      FLASH_Operation(FLASH_CMD_PROGRAM, saddr, *buffer);
      saddr += 4;
      buffer++;
    }
  }

  /*--------------------------------------------------------------------------------------------------------*/
  /* Read                                                                                                   */
  /*--------------------------------------------------------------------------------------------------------*/
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
      if (data != *buffer)
      {
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
        return CMD_FAILED;
      }
    }
    /*------------------------------------------------------------------------------------------------------*/
    /* Read                                                                                                 */
    /*------------------------------------------------------------------------------------------------------*/
    else
    {
      if (isReadProtection == 1)
      {
        if ((saddr >= IAP_CODE_SIZE) && (saddr < (IAP_CODE_SIZE + 1024)))
        {
          goto readok;
        }

        if (saddr < 0x1FF00000 || saddr >= 0x1FF00050)
        {
          data = 0;
        }
      }

readok:
      *(ptr + j) = data;
      j++;

      if (j  == 16)
      {
        j = 0;
        _IAP_SendDataAndWaitFinish();
      }
    }

    saddr += 4;
    buffer++;
  }

  return CMD_SUCCESS;
}

#if (EXT_FLASH_ENABLE == 1)
/*********************************************************************************************************//**
  * @brief  Mass/Page Erase EXT Flash.
  * @param  type: Erase type
  *         @arg IAP_MASS_ERASE: Mass Erase (Not support in IAP mode)
  *         @arg IAP_PAGE_ERASE: Page Erase
  * @param  saddr: Start address
  * @param  eaddr: End address
  * @retval CMD_SUCCESS or CMD_FAILED
  ***********************************************************************************************************/
static u32 _IAP_EXT_Erase(u32 type, u32 saddr, u32 eaddr)
{
  u32 i;
  if (type == IAP_MASS_ERASE)
  {
    SPI_FLASH_ChipErase();
  }
  else
  {
    for (i = saddr; i<= eaddr; i += EXT_FLASH_PAGE_SIZE)
    {
        SPI_FLASH_SectorErase(i - EXT_FALSH_ADDRESS);
    }
  }
  return CMD_SUCCESS;
}

/*********************************************************************************************************//**
  * @brief  Download image into external flash for program or verify.
  * @param  type: Program or verify
  *         @arg CMD_PROGRAM: Program mode
  *         @arg CMD_VERIFY: Verify mode
  *         @arg CMD_BLANK: Blank check mode
  *         @arg CMD_READ: Read mode
  * @param  saddr: Start address
  * @param  eaddr: End address
  * @param  buffer: point of data buffer
  * @retval CMD_SUCCESS or CMD_FAILED
  ***********************************************************************************************************/
static u32 _IAP_EXT_Flash(u32 type, u32 saddr, u32 eaddr)
{
  u32 *buffer = (u32 *)(&gu8CmdBuffer[CMD_PAYLOAD_ADDR]);
  u32 ReadBuffer[52/4];
  u32 j = 0;
  u32 *data = (u32 *)ReadBuffer;

  if (type == CMD_PROGRAM)
  {
    SPI_FLASH_BufferWrite((u8* )buffer, saddr - EXT_FALSH_ADDRESS, eaddr - saddr + 1);
    return CMD_SUCCESS;
  }

  SPI_FLASH_BufferRead((u8* )ReadBuffer, saddr - EXT_FALSH_ADDRESS, eaddr - saddr + 1);

  while (saddr <= eaddr)
  {

    /*------------------------------------------------------------------------------------------------------*/
    /* Verify                                                                                               */
    /*------------------------------------------------------------------------------------------------------*/
    if (type == CMD_VERIFY)
    {
      if (*data != *buffer)
      {
        return CMD_FAILED;
      }
    }
    /*------------------------------------------------------------------------------------------------------*/
    /* Blank                                                                                                */
    /*------------------------------------------------------------------------------------------------------*/
    else if (type == CMD_BLANK)
    {
      if (*data != 0xFFFFFFFF)
      {
        return CMD_FAILED;
      }
    }
    /*------------------------------------------------------------------------------------------------------*/
    /* Read                                                                                                 */
    /*------------------------------------------------------------------------------------------------------*/
    else
    {
      *(ptr + j) = *data;
      j++;

      if (j  == 16)
      {
        j = 0;
        _IAP_SendDataAndWaitFinish();
      }
    }

    saddr += 4;
    buffer++;
    data++;
  }

  return CMD_SUCCESS;
}
#endif

/*********************************************************************************************************//**
  * @brief  Calculate CRC value.
  * @param  crc: Iinitial value of CRC (Usually as 0)
  * @param  saddr: Start address
  * @param  length: Length for CRC calculation
  * @retval Always success (CMD_SUCCESS)
  ***********************************************************************************************************/
static u32 _IAP_CRC(u32 crc, u32 saddr, u32 length)
{
  crc = CRC16(crc, (u8 *)saddr, length);

  WriteByte(crc);
  WriteByte(crc >> 8);

  return CMD_SUCCESS;
}

/*********************************************************************************************************//**
  * @brief  Send information to Host.
  * @retval Always success (CMD_SUCCESS)
  ***********************************************************************************************************/
static u32 _IAP_Info(void)
{
  USBDCore_EPTWriteINData(USBD_EPT3, gu32Infotable, 64);
  return CMD_SUCCESS;
}

/*********************************************************************************************************//**
  * @brief  Exit Loader mode.
  * @retval None
  ***********************************************************************************************************/
static void _IAP_Exit(void)
{
  HT32F_DVB_USBDisConnect();
  while (1);
}

/*********************************************************************************************************//**
  * @brief  Download image for program or verify.
  * @param  uPar0: User define paramemter0 (8 bit)
  * @param  uPar1: User define paramemter1 (32 bit)
  * @param  uPar2: User define paramemter2 (32 bit)
  * @retval CMD_SUCCESS or CMD_FAILED
  ***********************************************************************************************************/
static u32 _User_Cmd50Example(u32 uPar0, u32 uPar1, u32 uPar2)
{
  switch (uPar0)
  {
    case 0x1:
    {
      (uPar1 == 1) ? HT32F_DVB_LEDOn(HT_LED1) : HT32F_DVB_LEDOff(HT_LED1);
      return CMD_SUCCESS;
    }
    case 0x2:
    {
      (uPar1 == 1) ? HT32F_DVB_LEDOn(HT_LED2) : HT32F_DVB_LEDOff(HT_LED2);
      return CMD_SUCCESS;
    }
    case 0x3:
    {
      (uPar1 == 1) ? HT32F_DVB_LEDOn(HT_LED3) : HT32F_DVB_LEDOff(HT_LED3);
      return CMD_SUCCESS;
    }
  }

  return CMD_FAILED;
}

/*********************************************************************************************************//**
  * @brief  Download image for program or verify.
  * @param  uPar0: User define paramemter0 (8 bit)
  * @param  uPar1: User define paramemter1 (32 bit)
  * @param  uPar2: User define paramemter2 (32 bit)
  * @retval CMD_SUCCESS or CMD_FAILED
  ***********************************************************************************************************/
static u32 _User_Cmd51Example(u32 uPar0, u32 uPar1, u32 uPar2)
{
  u8 *pPayload = (u8 *)(&gu8CmdBuffer[CMD_PAYLOAD_ADDR]);
  u32 i;

  switch (uPar0)
  {
    case 0x0:
    {
      for (i = 0; i < CMD_PAYLOAD_LEN; i++)
      {
        uUserData[i] = *pPayload++;
      }
      return CMD_SUCCESS;
    }
    case 0x1:
    {
      for (i = 0; i < CMD_PAYLOAD_LEN; i += 4)
      {
        *ptr++ = *((u32 *)(&uUserData[i]));
      }
      _IAP_SendDataAndWaitFinish();
      return CMD_SUCCESS;
    }
  }

  return CMD_FAILED;
}

/*********************************************************************************************************//**
  * @brief  Send data to USB endpoint and wait until transmission is finished.
  * @retval None
  ***********************************************************************************************************/
static void _IAP_SendDataAndWaitFinish(void)
{
  /*--------------------------------------------------------------------------------------------------------*/
  /* Start transmission, wait until 64 bytes transmitted by USB                                             */
  /*--------------------------------------------------------------------------------------------------------*/
  HT_USB->EP3ISR = IDTXIF;
  HT_USBEP3->TCR = 64;
  HT_USBEP3->CSR = NAKTX;
  while (((HT_USB->EP3ISR) & IDTXIF) == 0);
}

/*********************************************************************************************************//**
  * @brief  Flsh operation
  * @param  type: Flash operation type
  * @param  PageAddress: Target address
  * @param  Data: Source data
  * @retval None
  ***********************************************************************************************************/
void FLASH_Operation(u32 type, u32 PageAddress, u32 Data)
{

  HT_FLASH->OCMR = type;
  HT_FLASH->TADR = PageAddress;
  HT_FLASH->WRDR = Data;
  HT_FLASH->OPCR = FLASH_SEND_MAIN;
}

#if (IAP_DEBUG_MODE == 1)
/*********************************************************************************************************//**
  * @brief  Dump data for debug.
  * @param  memory
  * @param  Len
  * @retval None
  ***********************************************************************************************************/
static void __DBG_IAP_DumpData(uc8 *memory, u32 len)
{
  u32 i;
  for (i = 0; i < len; i++)
  {
    if (i % 8 == 0)
    {
      if (i != 0)
      {
        __DBG_IAPPrintf("\r\n");
      }
      __DBG_IAPPrintf("\t\t");
    }
    __DBG_IAPPrintf("%02X ", *((u8 *)(memory + i)));
  }
  __DBG_IAPPrintf("\r\n");

  return;
}
#endif

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
