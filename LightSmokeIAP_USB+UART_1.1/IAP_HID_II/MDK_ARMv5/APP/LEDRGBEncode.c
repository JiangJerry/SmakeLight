#include "ht32.h"
#include "ht32_board.h"
#include "main.h"
#include "BMP280Drive.h"



extern vu32 PDMAIntCnt, LEDTabIndex;
extern vu16 LEDEncodeTab[2][24], ghLED_RESET_TABLE[24];
extern vu16 *pLED_Display, *LEDCodeUpdate;

void NextLEDRGBEncode(u8 bRED,u8 bGREEN,u8 bBLUE)
{
    LEDCodeUpdate[23] = (bGREEN&0x80) ? CODE_1: CODE_0;
    LEDCodeUpdate[22] = (bGREEN&0x40) ? CODE_1: CODE_0;
    LEDCodeUpdate[21] = (bGREEN&0x20) ? CODE_1: CODE_0;
    LEDCodeUpdate[20] = (bGREEN&0x10) ? CODE_1: CODE_0;
    LEDCodeUpdate[19] = (bGREEN&0x08) ? CODE_1: CODE_0;
    LEDCodeUpdate[18] = (bGREEN&0x04) ? CODE_1: CODE_0;
    LEDCodeUpdate[17] = (bGREEN&0x02) ? CODE_1: CODE_0;
    LEDCodeUpdate[16] = (bGREEN&0x01) ? CODE_1: CODE_0;
    LEDCodeUpdate[15] = (bRED&0x80)   ? CODE_1: CODE_0;
    LEDCodeUpdate[14] = (bRED&0x40)   ? CODE_1: CODE_0;
    LEDCodeUpdate[13] = (bRED&0x20)   ? CODE_1: CODE_0;
    LEDCodeUpdate[12] = (bRED&0x10)   ? CODE_1: CODE_0;
    LEDCodeUpdate[11] = (bRED&0x08)   ? CODE_1: CODE_0;
    LEDCodeUpdate[10] = (bRED&0x04)   ? CODE_1: CODE_0;
    LEDCodeUpdate[9]  = (bRED&0x02)   ? CODE_1: CODE_0;
    LEDCodeUpdate[8]  = (bRED&0x01)   ? CODE_1: CODE_0;
    LEDCodeUpdate[7]  = (bBLUE&0x80)  ? CODE_1: CODE_0;
    LEDCodeUpdate[6]  = (bBLUE&0x40)  ? CODE_1: CODE_0;
    LEDCodeUpdate[5]  = (bBLUE&0x20)  ? CODE_1: CODE_0;
    LEDCodeUpdate[4]  = (bBLUE&0x10)  ? CODE_1: CODE_0;
    LEDCodeUpdate[3]  = (bBLUE&0x08)  ? CODE_1: CODE_0;
    LEDCodeUpdate[2]  = (bBLUE&0x04)  ? CODE_1: CODE_0;
    LEDCodeUpdate[1]  = (bBLUE&0x02)  ? CODE_1: CODE_0;
    LEDCodeUpdate[0]  = (bBLUE&0x01)  ? CODE_1: CODE_0;  
}

void Tab0Encode(u8 bRED,u8 bGREEN,u8 bBLUE)
{
    LEDEncodeTab[0][23] = (bGREEN&0x80) ? CODE_1: CODE_0;
    LEDEncodeTab[0][22] = (bGREEN&0x40) ? CODE_1: CODE_0;
    LEDEncodeTab[0][21] = (bGREEN&0x20) ? CODE_1: CODE_0;
    LEDEncodeTab[0][20] = (bGREEN&0x10) ? CODE_1: CODE_0;
    LEDEncodeTab[0][19] = (bGREEN&0x08) ? CODE_1: CODE_0;
    LEDEncodeTab[0][18] = (bGREEN&0x04) ? CODE_1: CODE_0;
    LEDEncodeTab[0][17] = (bGREEN&0x02) ? CODE_1: CODE_0;
    LEDEncodeTab[0][16] = (bGREEN&0x01) ? CODE_1: CODE_0;
    LEDEncodeTab[0][15] = (bRED&0x80)   ? CODE_1: CODE_0;
    LEDEncodeTab[0][14] = (bRED&0x40)   ? CODE_1: CODE_0;
    LEDEncodeTab[0][13] = (bRED&0x20)   ? CODE_1: CODE_0;
    LEDEncodeTab[0][12] = (bRED&0x10)   ? CODE_1: CODE_0;
    LEDEncodeTab[0][11] = (bRED&0x08)   ? CODE_1: CODE_0;
    LEDEncodeTab[0][10] = (bRED&0x04)   ? CODE_1: CODE_0;
    LEDEncodeTab[0][9]  = (bRED&0x02)   ? CODE_1: CODE_0;
    LEDEncodeTab[0][8]  = (bRED&0x01)   ? CODE_1: CODE_0;
    LEDEncodeTab[0][7]  = (bBLUE&0x80)  ? CODE_1: CODE_0;
    LEDEncodeTab[0][6]  = (bBLUE&0x40)  ? CODE_1: CODE_0;
    LEDEncodeTab[0][5]  = (bBLUE&0x20)  ? CODE_1: CODE_0;
    LEDEncodeTab[0][4]  = (bBLUE&0x10)  ? CODE_1: CODE_0;
    LEDEncodeTab[0][3]  = (bBLUE&0x08)  ? CODE_1: CODE_0;
    LEDEncodeTab[0][2]  = (bBLUE&0x04)  ? CODE_1: CODE_0;
    LEDEncodeTab[0][1]  = (bBLUE&0x02)  ? CODE_1: CODE_0;
    LEDEncodeTab[0][0]  = (bBLUE&0x01)  ? CODE_1: CODE_0;  
}
