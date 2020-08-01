#include "stdlib.h"
#include "ht32.h"
#include "ws2812.h"

// Data Type Definitions
TM_TimeBaseInitTypeDef 	TM_TimeBaseInitStructure;
TM_OutputInitTypeDef 		TM_OutputInitStructure;
PDMACH_InitTypeDef 			PDMACH_InitStructure;
HT_PDMA_TypeDef 				PDMA_TEST_Struct;

static u32 WS2812[WS_PIXEL + 2][WS_24BITS];
static u8 ws_i;

bool didSetColor;

void wsInit(void) {
	wsCKCUConfig();
	wsAFIOConfig();
	wsPWMConfig();
	wsNVICConfig();
	MCTM_CHMOECmd(HT_MCTM0, ENABLE);
	for(ws_i = 0; ws_i < WS_24BITS; ws_i++) {
		WS2812[WS_PIXEL	+ 0][ws_i] = 0;
		WS2812[WS_PIXEL + 1][ws_i] = 0;
		WS2812[WS_PIXEL + 2][ws_i] = 0;
	}
	wsClearAll();
}

void wsCKCUConfig(void) {
	{ /* Enable peripheral clock                                                                              */
		CKCU_PeripClockConfig_TypeDef CKCUClock = {{ 0 }};
		CKCUClock.Bit.PB = 1;
		CKCUClock.Bit.PDMA = 1;
		CKCUClock.Bit.MCTM0 = 1;
		CKCU_PeripClockConfig(CKCUClock, ENABLE);
	}
}

void wsAFIOConfig(void) {
	AFIO_GPxConfig(GPIO_PB, AFIO_PIN_0, AFIO_MODE_4); 	//Port B -> Pin  0  >>>  TM_CH_1
}

void wsPWMConfig(void) {
	TM_TimeBaseInitStructure.CounterReload = WS_RELOAD;
	TM_TimeBaseInitStructure.Prescaler = WS_PRESCALER;
	TM_TimeBaseInitStructure.RepetitionCounter = 0;
	TM_TimeBaseInitStructure.CounterMode = TM_CNT_MODE_UP;
	TM_TimeBaseInitStructure.PSCReloadTime = TM_PSC_RLD_IMMEDIATE;
	TM_TimeBaseInit(HT_MCTM0, &TM_TimeBaseInitStructure);
	TM_ClearFlag(HT_MCTM0, TM_FLAG_UEV);
	TM_Cmd(HT_MCTM0, ENABLE);
	
	TM_OutputInitStructure.Channel = TM_CH_1;
	TM_OutputInitStructure.OutputMode = TM_OM_PWM1;
	TM_OutputInitStructure.Control = TM_CHCTL_ENABLE;
	TM_OutputInitStructure.ControlN = TM_CHCTL_DISABLE;
	TM_OutputInitStructure.Polarity = TM_CHP_NONINVERTED;
	TM_OutputInitStructure.PolarityN = TM_CHP_NONINVERTED;
	TM_OutputInitStructure.IdleState = MCTM_OIS_LOW;
	TM_OutputInitStructure.IdleStateN = MCTM_OIS_HIGH;
	TM_OutputInitStructure.Compare = 0;
	TM_OutputInit(HT_MCTM0, &TM_OutputInitStructure);
}

void wsPDMAConfig(void) {
	PDMACH_InitStructure.PDMACH_SrcAddr = (u32)WS2812;
	PDMACH_InitStructure.PDMACH_DstAddr = (u32)&(HT_MCTM0 -> CH1CCR);
	PDMACH_InitStructure.PDMACH_BlkCnt = WS_BLOCK;
	PDMACH_InitStructure.PDMACH_BlkLen = 1;
	PDMACH_InitStructure.PDMACH_DataSize = WIDTH_32BIT;
	PDMACH_InitStructure.PDMACH_Priority = VH_PRIO;
	PDMACH_InitStructure.PDMACH_AdrMod = SRC_ADR_LIN_INC | DST_ADR_FIX | AUTO_RELOAD;
	PDMA_Config(PDMA_CH5, &PDMACH_InitStructure);
	PDMA_ClearFlag(PDMA_CH5, PDMA_FLAG_TC);
	PDMA_IntConfig(PDMA_CH5, PDMA_INT_TC, ENABLE);
	TM_PDMAConfig(HT_MCTM0, TM_PDMA_UEV, ENABLE);
}

void wsNVICConfig(void) {
	NVIC_EnableIRQ(PDMACH2_5_IRQn);	// Enable PDMA Channel 2 to 5 interrupt
}

void wsSetColor(u8 pixelNum, u8 color[], float mag) {
	u8 color_bit;
	u8 red = (u8)((float)color[0]*mag);
	u8 green = (u8)((float)color[1]*mag);
	u8 blue = (u8)((float)color[2]*mag);
	
	didSetColor = FALSE;
	for (color_bit = 0; color_bit < 8; color_bit += 1) {
		WS2812[pixelNum][color_bit] = (green >> (7 - color_bit) & 1) == 1 ? WS_LOGIC_1 : WS_LOGIC_0;
	}
	for (color_bit = 7; color_bit < 16; color_bit += 1) {
		WS2812[pixelNum][color_bit] = (red >> (15 - color_bit) & 1) == 1 ? WS_LOGIC_1 : WS_LOGIC_0;
	}
	for (color_bit = 15; color_bit < 24; color_bit += 1) {
		WS2812[pixelNum][color_bit] = (blue >> (23 - color_bit) & 1) == 1 ? WS_LOGIC_1 : WS_LOGIC_0;
	}
	didSetColor = TRUE;
}

void wsShow(void) {
	wsPDMAConfig();
	PDMA_ClearFlag(PDMA_CH5, PDMA_FLAG_TC);												// Clear PDMA_FLAG_TC of PDMA_CH5 before transferring data
	TM_PDMAConfig(HT_MCTM0, TM_PDMA_UEV, ENABLE);
	TM_Cmd(HT_MCTM0, ENABLE);
	PDMA_EnaCmd(PDMA_CH5, ENABLE);
	while (PDMA_GetFlagStatus(PDMA_CH5, PDMA_FLAG_TC) == RESET);	// Wait for transferring data complete
	TM_PDMAConfig(HT_MCTM0, TM_PDMA_UEV, DISABLE);
	TM_Cmd(HT_MCTM0, DISABLE);
	PDMA_EnaCmd(PDMA_CH5, DISABLE);
}

void wsClearAll(void) {
	u8 ws_clear[3] = {0};
	for(ws_i = 0; ws_i < WS_PIXEL; ws_i++) wsSetColor(ws_i, ws_clear, 1);
}

void wsClearBetween(u8 startPixelNum, u8 endPixelNum) {
	u8 i = 0;
	u8 ws_clear[3] = {0};
	for(i = startPixelNum; i <= endPixelNum; i++) wsSetColor(i, ws_clear, 1);
}

void wsBlinkAll(u32 wait) {
	u8 ws_white[3] = {255, 255, 255};
	wait *= 10000;

	for(ws_i = 0; ws_i < WS_PIXEL; ws_i++) wsSetColor(ws_i, ws_white, 0.1);
	wsShow();
	while(wait--);
	wsClearAll();
	wsShow();
}
