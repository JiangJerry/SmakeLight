/**
 @page IAP_HID_II

 @verbatim
 * @file    IAP_HID/IAP_HID_II/readme.txt
 * @version V1.00
 * @date    2015-03-17
 * @brief   Description of HID IAP example.
 @endverbatim

@par Example Description:

<PRE>
This example code shows how to update embedded firmware and external SPI Flash by USB Device HID Class.
The data flow is as below.
  Host                               Device
  ==========                         ==========
  Endpoint 1 Output Report    ->     Command Channel
  Endpoint 0 GetInputReport   <-     Command Response
  Endpoint 2 Input Report     <-     Data for read command

The IAP HID firmware supports the following protocol:


- COMMAND Format:

    0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |     Code      |     uPara     |             uCRC              |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ ---------------|
   |                            uStAddr                            |    Address     |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+       or       |
   |                      uEndAddr / uLength                       | User Parameter |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ ---------------|
   |     Byte 0    |     Byte 1    |     Byte 2    |     Byte 3    |                |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+    52 Bytes    |
   |                              ...                              |    Payload     |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+                |
   |     Byte 48   |     Byte 49   |     Byte 50   |    Byte 51    |                |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ ---------------|

    Field       Size (Bytes)       Descriptions
    ========    ===============    ======================
    Code        1                  Command Code
    uPara       1                  Parameter of command
    uCRC        2                  CRC16 value of (Code, uPara, 0x0000, uStaddr, and uEndAddr)
    uStAddr     4                  Start address of command
    uEndAddr    4                  End address or length of command
    Payload     52                 Data payload
    ------------------------------------------------------
    Total       64


- COMMAND Code:

    Command | Code | Para | Address  | Payload    | Descriptions
    ==========================================================================================================
    Mass    | 0x0  | 0xA  | 0        | 0 Padding  | Mass Erase (Not support in IAP mode).
    Erase   |      |      | 0        |            |
    ----------------------------------------------------------------------------------------------------------
    Page    | 0x0  | 0x8  | uStAddr  | 0 Padding  | Erase specific page which assigned by
    Erase   |      |      | uEndAddr |            | uStAddr and uEndAddr.
    ----------------------------------------------------------------------------------------------------------
    Verify  | 0x1  | 0x0  | uStAddr  | Data       | Verify specific Flash Memory which assigned
            |      |      | uEndAddr |            | by uStAddr and uEndAddr.
    ----------------------------------------------------------------------------------------------------------
    Program | 0x1  | 0x1  | uStAddr  | Data       | Program specific Flash Memory which assigned
            |      |      | uEndAddr |            | by uStAddr and uEndAddr.
    ----------------------------------------------------------------------------------------------------------
    Read    | 0x1  | 0x2  | uStAddr  | 0 Padding  | Read specific Flash Memory which assigned by
            |      |      | uEndAddr |            | uStAddr and uEndAddr.
    ----------------------------------------------------------------------------------------------------------
    Blank   | 0x1  | 0x3  | uStAddr  | 0 Padding  | Blank check specific Flash Memory which
            |      |      | uEndAddr |            | assigned by uStAddr and uEndAddr.
    ----------------------------------------------------------------------------------------------------------
    CRC     | 0x2  | 0x0  | uStAddr  | 0 Padding  | Apply CRC16 checking of specific Flash Memory
            |      |      | uLength  |            | which assigned by uStAddr and uLength.
    ----------------------------------------------------------------------------------------------------------
    Info    | 0x3  | 0x0  | 0        | 0 Padding  | Return 4 words information.
            |      |      | 0        |            |
    ----------------------------------------------------------------------------------------------------------
    Reset   | 0x4  | 0x0  | 0        | 0 Padding  | Reset and start AP mode.
    to AP   |      |      | 0        |            |
    ----------------------------------------------------------------------------------------------------------
    Reaset  | 0x4  | 0x1  | 0        | 0 Padding  | Reset and start IAP mode.
    to IAP  |      |      | 0        |            |
    ----------------------------------------------------------------------------------------------------------
    Exit    | 0x5  | 0x0  | 0        | 0 Padding  | Enter forever loop to exit IAP mode.
            |      |      | 0        |            |
    ----------------------------------------------------------------------------------------------------------
    User    | 0x50 | Par0 | Par1     | User       | User defined command and data.
    Command |   |  |      | Par2     | Data       |
            | 0xFF |      |          |            |
    ----------------------------------------------------------------------------------------------------------

- RETURN Format:

    0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |     Code      |                  Padding                      |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                              ...                              |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                           Padding                             |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

    Field       Size (Bytes)       Descriptions
    ========    ===============    ======================
    Code        1                  Return Code
    Padding     63                 0 Padding


- RETURN Code:

    Return        Code               Descriptions
    ========      ===============    ======================
    OK            0x4F (O)           Command operation Okey
    FAIL          0x46 (F)           Command operation failed


- How to Change Application Start Address

  The application start address shall be modified if the IAP code size is changed. For example, if you
  add more functions into IAP loader which causes the IAP code size changed from 2500 bytes to 3500 bytes
  , you shall modify application start address from 0xC00 (3072 Bytes) to 0x1000 (4096 Bytes).
  Note that the minimum scale of application start address is based on the Flash Page size. Following
  description shows how to modify RO base address of IAP Device, application, and IAP Host.

  IAP_CODE_SIZE:
  ==========================================
    Modify the define value of "IAP_CODE_PAGE" in "ht32_board_config.h" file.
    IAP loader uses this define to confirm PC/SP, apply AP CRC Checking.
    Application uses "IAP_CODE_SIZE" for NVIC VTO setting since the vector is not locate at 0x00000000.

      ht32_board_config.h
        |    ....
        |
        |    #define IAP_CODE_SIZE                 (1024 * 10)          /* IAP code size in Bytes             */
        |
        |    ....

  linker script:
  ==========================================
    Following linker script files define the RO base address. The format of link script is different
    between various compiling tools. We prepare 5k and 10k setting in those files for your reference.
    Please refer to the document of compiling tools for more information.

      MDK_ARM/linker_01.lin
      MDK_ARMv5/linker_01.lin
        |   ....
        |
        |   ;IAP 0x00000000 0x00001400   ; SETTING: 5k for IAP
        |   IAP 0x00000000 0x00002800   ; SETTING: 10 kB for IAP
        |   {
        |
        |   ....
        |
        |   }
        |
        |   ;AP 0x00001800 ; SETTING: User Application start from IAP + Version
        |   AP 0x00002A00   ; SETTING: User Application start from IAP + Version
        |   {
        |
        |   ....


</PRE>

@par Directory contents:

- IAP_HID/IAP_HID_II/ht32_board_config.c                       Board configuration file
- IAP_HID/IAP_HID_II/Src_AP/main.c                             Main program of application
- IAP_HID/IAP_HID_II/Src_AP/ht32fxxxx_it.c                     Interrupt handlers
- IAP_HID/IAP_HID_II/Src_AP/ht32fxxxx_conf.h                   Firmware library configuration file
- IAP_HID/IAP_HID_II/Src_AP/ht32fxxxx_usbdconf.h               USB Device Library configuration file
- IAP_HID/IAP_HID_II/Src_AP/system_ht32fxxxx.c                 System configuration file
- IAP_HID/IAP_HID_II/Src_IAP/iap_main.c                        Main program of IAP
- IAP_HID/IAP_HID_II/Src_IAP/iap_handler.c                     IAP handler who control the image update procedural
- IAP_HID/IAP_HID_II/Src_IAP/iap_handler.h                     Header file of the IAP handlers
- IAP_HID/IAP_HID_II/Src_IAP/iap_buffer.c                      Buffer related functions
- IAP_HID/IAP_HID_II/Src_IAP/iap_buffer.h                      Header file of of the Buffer related functions
- IAP_HID/IAP_HID_II/Src_IAP/iap_crc16.c                       CRC algorithm
- IAP_HID/IAP_HID_II/Src_IAP/iap_crc16.h                       Header file of CRC algorithm
- IAP_HID/IAP_HID_II/Src_IAP/iap_ht32_usbd_class.c             USB Device Class related function
- IAP_HID/IAP_HID_II/Src_IAP/iap_ht32_usbd_class.h             Header file of the USB Device Class
- IAP_HID/IAP_HID_II/Src_IAP/iap_ht32_usbd_descriptor.c        Descriptor of USB Device
- IAP_HID/IAP_HID_II/Src_IAP/iap_ht32_usbd_descriptor.h        Header file of the USB Device Descriptor
- IAP_HID/IAP_HID_II/Src_IAP/iap_ht32fxxxx_it.c                Interrupt handlers
- IAP_HID/IAP_HID_II/Src_IAP/iap_system_ht32fxxxx.c            System configuration file
- IAP_HID/IAP_HID_II/Src_IAP/ht32fxxxx_conf.h                  Firmware library configuration file
- IAP_HID/IAP_HID_II/Src_IAP/ht32fxxxx_usbdconf.h              USB Device Library configuration file
- IAP_HID/IAP_HID_II/Tools/AP                                  Pre-builded AP binady file.
- IAP_HID/IAP_HID_II/Tools/HT32_IAP_Test.exe                   Pre-builded IAP IU.
- IAP_HID/IAP_HID_II/Tools/TPMakerIII_SETTING_EXAMPLE.png      Setting example of TPMaker III.

@par Hardware and Software Environment:

<PRE>
This example runs on HT32 Series Development board. By default, the IAP program uses J3 (PC8-BOOT0)
to decide if user application or IAP mode should be started. An example which using Key1 of HT32F175x/275x
Development board to decide start user application or IAP mode can also be found in "iap_main.c".
This example uses the following endpoints.
  Endpoint    Transfer Type         Buffer Length
  ========    ====================  ==============
  0           Control               64 bytes
  1           Interrupt OUT         64 bytes
  2           Interrupt IN          64 bytes
</PRE>

@note

This example uses multi-project to manage both the IAP program and user's application. Multi-project
helps to build/download/debug both IAP and application in the same time. The following steps show
how to build/download/debug by multi-project setting.
<PRE>
  For Keil uVision:

    Build:    Double click on "MDK_ARM/Project_CHIPNAME.uvmpw" to open multi-project file. Press the
              "Batch Build" icon on the toolbar, or tick "Project -> Batch Build" to open the "Batch Build"
              window. Press "Build" or "Rebuild" button to build both IAP and application. The output file for
              the programming tools is as follows:

                  MDK_ARM/HT32/CHIPNAME/AP/Obj/HT32.hex                 (HEX file of both IAP and application)
                  MDK_ARM/HT32/CHIPNAME/AP/Obj/HT32.axf.bin/AP          (Binary file of application)

    Download: Make sure the active project is "Project_CHIPNAME" by right-clicking on the project name
              "Project_CHIPNAME" in "Project" window and click "Set as Active Project". Press "LOAD" icon
              to download both IAP and application into Flash memory.

    Debug:    Make sure the active project is "Project_CHIPNAME". Enter debug mode by pressing Ctrl+F5.
              The debug symbol of IAP will be loaded by the comamnd below in "HT32Fxxxx_DebugSupport.ini".
                "LOAD HT32\CHIPNAME\IAP\Obj\IAP.axf INCREMENTAL"
              Since both images of IAP and application are loaded into uVision, trace the program switching
              behavior between IAP and application is possible.

</PRE>

@par USB Library debug mode

<PRE>
For debug purpose, your can turn on the USB Library debug mode by setting "USBDCORE_DEBUG" definition
value as 1 (which located at ht32_usbd_core.h). The debug message will be shown via the printf function
to the UART or ITM interface according to the retarget setting of firmware library.

NOTE THAT USB DEBUG MODE DEGRADES THE PERFORMANCE AND MAY CAUSE USB ERROR UNDER SOME CONDITIONS.

You shall turn off the USB debug mode on your final application. We strongly recommend turn off the
"Dump USB Debug data (USBDCORE_DEBUG_DATA)" option when you enable the USB debug mode and retarget the
message to UART.


</PRE>

</PRE>

 * <h2><center>Copyright (C) Holtek Semiconductor Inc. All rights reserved</center></h2>
 */
