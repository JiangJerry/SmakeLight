#include "ht32.h"
#include "ht32_board.h"
#include "main.h"

/*
* The PDMA channel is used to update led lights data.
* PDMA_CHx will modify 24 times GT_CHxCCR data to display a LED light when PDMA_CHx is started.
* PDMA_CH0: Update GT_CH1CCR by GT_CH3 compare match event (CCR3 = 0)
* PDMA_CH1: Update GT_CH0CCR by GT_UEV update event
*/

extern vu16 ghLED_RESET_TABLE[];

void GPTM_PDMA_Configuration(void)
{
//  vu32 *pCHxCCR = (vu32*)&GPTMx->CH0CCR + GPTM_CH_0;
    PDMACH_InitTypeDef DmaInit;

    // WHY? RSTCU_AHBPeripReset(RSTCU_AHBRST_PDMA, ENABLE);

    /* DMA Channel_0 Setting, SRC_Addr: RESET, DST_Addr: GT_CR1, Data_Blcok = 24, Data_Length= 16bit x1       */
/*    DmaInit.PDMACH_SrcAddr = (u32)&ghLED_RESET_TABLE[23];    //  Reset Signal
    DmaInit.PDMACH_DstAddr = (u32)&(GPTMx->CH1CCR);
    DmaInit.PDMACH_BlkCnt  = 24;
    DmaInit.PDMACH_BlkLen  = 1;
    DmaInit.PDMACH_DataSize = WIDTH_16BIT;
    DmaInit.PDMACH_Priority = VH_PRIO;
    DmaInit.PDMACH_AdrMod   = SRC_ADR_LIN_DEC | DST_ADR_FIX | AUTO_RELOAD;
    PDMA_Config(PDMA_CH0, &DmaInit);*/

    /* DMA Channel_1 Setting, SRC_Addr: RESET, DST_Addr: GT_CR0, Data_Blcok = 24, Data_Length= 16bit x1       */
    DmaInit.PDMACH_SrcAddr = (u32)&ghLED_RESET_TABLE[23];    //  Reset Signal
    DmaInit.PDMACH_DstAddr = (u32)&(GPTMx->CH0CCR);
    DmaInit.PDMACH_BlkCnt  = 24;
    DmaInit.PDMACH_BlkLen  = 1;
    DmaInit.PDMACH_DataSize = WIDTH_16BIT;
    DmaInit.PDMACH_Priority = VH_PRIO;
    DmaInit.PDMACH_AdrMod   = SRC_ADR_LIN_DEC | DST_ADR_FIX | AUTO_RELOAD;
    PDMA_Config(PDMA_CH1, &DmaInit);

    /* Enable PDMA Interrupt                                                                                  */
    NVIC_EnableIRQ(PDMACH0_1_IRQn);

    PDMA_IntConfig(PDMA_CH0, PDMA_INT_TC | PDMA_INT_GE, ENABLE);
    PDMA_IntConfig(PDMA_CH1, PDMA_INT_TC | PDMA_INT_GE, ENABLE);

    /* Enable PDMA Channel                                                                                    */
    PDMA_EnaCmd(PDMA_CH0, ENABLE);    /* GT_CC1 use PDMA_CH0                                                  */
    PDMA_EnaCmd(PDMA_CH1, ENABLE);    /* GT_UEV use PDMA_CH1                                                  */
}
