#include "ht32.h"
#include "ht32_board.h"
#include "main.h"
#include "BMP280Drive.h"


u8 TaskKeyRdy;
extern FlagStatus EnterSleepPreFlag;

void TaskKey(void)
{
    if(TaskKeyRdy)
    {
        TaskKeyRdy = 0;
        GetKey();    
#if EN_PRESS_FUNC > 0    
        GetPressure();  //获取压力值  
#endif        
        if((LEDDispOrNot == RESET)&&(ChargeFlag == RESET)&&(StdbyFlag == RESET))
        {
            SleepPrepare();
        }
        else
        {
            EnterSleepPreFlag = RESET;
        }
    }
    
}
