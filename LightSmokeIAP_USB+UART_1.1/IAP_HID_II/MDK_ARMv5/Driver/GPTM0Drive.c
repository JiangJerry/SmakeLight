#include "ht32.h"
#include "ht32_board.h"
#include "main.h"

/*
* The GT_CH0 and GT_CH1 is used to send two LED lighting data.
* The GPTM timer period = 800K bits, and the lighting data bits will update with CHxCCR preload function.
* The CHxCCR preload data is updated by the corresponded PDMA_CHx which is requested by GT_UEV(GT_CH0)/GT_CH3CC(GT_CH1) event.
*/
void GPTM_Configuration(void)
{
    TM_TimeBaseInitTypeDef   GPTM_TimeBaseInitStructure;
    TM_OutputInitTypeDef     GPTM_OutputInitStructure;
    u32  wGPTM_CRR, wGPTM_Prescaler = 0;

    /* Init GPTM                                                                                              */
    TM_DeInit(GPTMx);

    /* Set PWM period = 800KHz = 1.25us                                                                       */
    SystemCoreClockUpdate();
    wGPTM_CRR = (SystemCoreClock / 1000000) * 1250 / 1000;


    /* Init GPTM time-base                                                                                    */
    GPTM_TimeBaseInitStructure.CounterMode   = TM_CNT_MODE_UP;
    GPTM_TimeBaseInitStructure.CounterReload = wGPTM_CRR;
    GPTM_TimeBaseInitStructure.Prescaler     = wGPTM_Prescaler;
    GPTM_TimeBaseInitStructure.PSCReloadTime = TM_PSC_RLD_UPDATE;
    TM_TimeBaseInit(GPTMx, &GPTM_TimeBaseInitStructure);

    /* Init GPTM Channel 0/1 to output PWM waveform with 0% duty                                              */
    GPTM_OutputInitStructure.Channel    = TM_CH_0;
    GPTM_OutputInitStructure.OutputMode = TM_OM_PWM1;
    GPTM_OutputInitStructure.Control    = TM_CHCTL_ENABLE;
    GPTM_OutputInitStructure.Compare    = 0;
    GPTM_OutputInitStructure.Polarity   = TM_CHP_NONINVERTED;
    GPTM_OutputInitStructure.AsymmetricCompare = 0;
    TM_OutputInit(GPTMx, &GPTM_OutputInitStructure);

    GPTM_OutputInitStructure.Channel    = TM_CH_1;
    TM_OutputInit(GPTMx, &GPTM_OutputInitStructure);

    /* GPTM CH0/1 CCR preload enable                                                                          */
    TM_CHCCRPreloadConfig(GPTMx, TM_CH_0, ENABLE);
    TM_CHCCRPreloadConfig(GPTMx, TM_CH_1, ENABLE);

    GPTMx->CH3CCR = 0;

    /* Clear Update Event Interrupt flag                                                                      */
    GPTMx->EVGR = TM_FLAG_UEV;
    TM_ClearFlag(GPTMx, TM_FLAG_UEV);

    /* GPTM_CH3CC/UEV PDMA Request Setting                                                                    */
    TM_PDMAConfig(GPTMx, TM_PDMA_CH3CC | TM_PDMA_UEV, ENABLE);
    /* Enable GPTM timer                                                                                      */
    GPTMx->CTR     = 0x01;    
}
