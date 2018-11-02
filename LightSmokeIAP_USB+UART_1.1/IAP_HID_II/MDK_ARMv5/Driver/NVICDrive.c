#include "ht32.h"
#include "ht32_board.h"
#include "main.h"
/*********************************************************************************************************//**
  * @brief  Configure the NVIC vector table.
  * @retval None
  ***********************************************************************************************************/
void NVIC_Configuration(void)
{
    NVIC_SetVectorTable(NVIC_VECTTABLE_FLASH, 0);     /* Set the Vector Table base location at 0x00000000   */
}
