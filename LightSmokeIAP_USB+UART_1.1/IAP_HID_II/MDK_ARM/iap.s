;********************************************************************************
;* @file        iap.s
;* @version     $Rev:: 1178         $
;* @date        $Date:: 2016-10-12 #$
;* @brief       Include IAP image
;*
;* @note        Copyright (C) Holtek Semiconductor Inc. All rights reserved.
;*
;* <h2><center>&copy; COPYRIGHT Holtek</center></h2>
;*
;********************************************************************************
;*----------- <<< Use Configuration Wizard in Context Menu >>> ------------------
;********************************************************************************
;// <q> Include IAP image into user's application
INCLUDE_IAP      EQU     1

        AREA    IAP, DATA, READONLY

        IF INCLUDE_IAP <> 0
          IF USE_HT32_CHIP=4
          INCBIN  HT32\52352\IAP\Obj\IAP.axf.bin
          ENDIF
          IF USE_HT32_CHIP=3
          INCBIN  HT32\52341\IAP\Obj\IAP.axf.bin
          ENDIF
        ENDIF

        END
