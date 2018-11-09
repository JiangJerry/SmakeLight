#include "ht32.h"
#include "ht32_board.h"
#include "main.h"
#include "BMP280Drive.h"


u8 TaskADCRdy;

void TaskADC(void)
{
    if(TaskADCRdy)
    {
        TaskADCRdy = 0;
        ADCStart();
        switch(BatSta)
        {
            case CHARG:
                BatLedOrg();
                break;
            case STDBY:
                BatLedGreen();
                break;
            case LOWBAT:
                BatLedRed();
                break;
            default:
                BatLedOff();
                break;           
        }
    }
    
}
