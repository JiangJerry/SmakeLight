;********************************************************************************
;* @file        version.s
;* @version     V1.00
;* @date        11/22/2012
;* @brief       Include version.txt
;*
;* @note        Copyright (C) Holtek Semiconductor Inc. All rights reserved.
;*
;* <h2><center>&copy; COPYRIGHT 2012 Holtek</center></h2>
;*
;********************************************************************************

        AREA    VER, DATA, READONLY
        DCD     6          ; Length of verson
        INCBIN  version.txt

        END
