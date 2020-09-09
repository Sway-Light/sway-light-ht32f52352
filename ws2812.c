#include "stdlib.h"
#include "ht32.h"
#include "ws2812.h"

// Data Type Definitions
SPI_InitTypeDef         SPI_InitStructure;
PDMACH_InitTypeDef      PDMACH_InitStructure;
HT_PDMA_TypeDef         PDMA_TEST_Struct;

u8 WS2812[WS_PIXEL + 3][WS_24BITS];

bool didSetColor;

void wsInit(void) {
	u16 i = 0, j = 0;
	wsCKCUConfig();
	wsAFIOConfig();
	wsPDMAConfig();
	wsSPIConfig();
	for(i = 0; i < WS_PIXEL; i++) {
		for(j = 0; j < WS_24BITS; j++) {
			WS2812[i][j] = WS_LOGIC_0;	
		}
	}
	for(i = 0; i < WS_24BITS; i++) {
		WS2812[WS_PIXEL	+ 0][i] = 0;
		WS2812[WS_PIXEL	+ 1][i] = 0;
		WS2812[WS_PIXEL	+ 2][i] = 0;
	}
	wsClearAll();
	wsShow();
}

void wsCKCUConfig(void) {
	{ /* Enable peripheral clock                                                                              */
		CKCU_PeripClockConfig_TypeDef CKCUClock = {{ 0 }};
		CKCUClock.Bit.PDMA = 1;
		CKCUClock.Bit.SPI0 = 1;
		CKCU_PeripClockConfig(CKCUClock, ENABLE);
	}
}

void wsAFIOConfig(void) {
	AFIO_GPxConfig(GPIO_PB, AFIO_PIN_2, AFIO_FUN_SPI);
	AFIO_GPxConfig(GPIO_PB, AFIO_PIN_3, AFIO_FUN_SPI);
	AFIO_GPxConfig(GPIO_PB, AFIO_PIN_4 , AFIO_FUN_SPI);
	AFIO_GPxConfig(GPIO_PB, AFIO_PIN_5 , AFIO_FUN_SPI);
}

void wsPDMAConfig(void) {
	/* Master Master Tx PDMA channel configuration                                                              */
	PDMACH_InitStructure.PDMACH_SrcAddr = (u32) &WS2812;
	PDMACH_InitStructure.PDMACH_DstAddr = (u32) &HT_SPI0->DR;
	PDMACH_InitStructure.PDMACH_BlkCnt = WS_BLOCK;
	PDMACH_InitStructure.PDMACH_BlkLen = 1;
	PDMACH_InitStructure.PDMACH_DataSize = WIDTH_8BIT;
	PDMACH_InitStructure.PDMACH_Priority = VH_PRIO;
	PDMACH_InitStructure.PDMACH_AdrMod = SRC_ADR_LIN_INC | DST_ADR_FIX;
	PDMA_Config(PDMA_CH1, &PDMACH_InitStructure);

	/* Enable Master Tx, Rx GE & TC interrupt                                                                   */
	PDMA_IntConfig(PDMA_CH1, PDMA_INT_GE | PDMA_INT_TC, ENABLE);

	/* Enable the corresponding PDMA channel                                                                  */
	PDMA_EnaCmd(PDMA_CH1, ENABLE);
}

void wsSPIConfig(void) {
	/* MASTER_SEL idle state is HIGH                                                                            */
	GPIO_PullResistorConfig(HT_GPIOB, AFIO_PIN_2, GPIO_PR_UP);

	/* Master configuration                                                                                     */
	SPI_InitStructure.SPI_Mode = SPI_MASTER;
	SPI_InitStructure.SPI_FIFO = SPI_FIFO_DISABLE;
	SPI_InitStructure.SPI_DataLength = SPI_DATALENGTH_8;
	SPI_InitStructure.SPI_SELMode = SPI_SEL_HARDWARE;
	SPI_InitStructure.SPI_SELPolarity = SPI_SELPOLARITY_LOW;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_LOW;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_FIRST;
	SPI_InitStructure.SPI_FirstBit = SPI_FIRSTBIT_MSB;
	SPI_InitStructure.SPI_RxFIFOTriggerLevel = 0;
	SPI_InitStructure.SPI_TxFIFOTriggerLevel = 0;
	SPI_InitStructure.SPI_ClockPrescaler = WS_PRESCALER;
	SPI_Init(HT_SPI0, &SPI_InitStructure);

	/* Enable Master & Slave                                                                                     */
	SPI_Cmd(HT_SPI0, ENABLE);

	/* Set SS as output mode for slave select                                                                 */
	SPI_SELOutputCmd(HT_SPI0, ENABLE);

	/* Enable master Master PDMA requests later                                                                 */
	SPI_PDMACmd(HT_SPI0, SPI_PDMAREQ_TX | SPI_PDMAREQ_RX, ENABLE);
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
	wsSPIConfig();
	while (!PDMA_GetFlagStatus(PDMA_CH1, PDMA_FLAG_TC));
}

void wsClearAll(void) {
	u16 i = 0;
	u8 ws_clear[3] = {0};
	for(i = 0; i < WS_PIXEL; i++) wsSetColor(i, ws_clear, 0);
}

void wsBlinkAll(u32 wait) {
	u16 i = 0;
	u8 ws_white[3] = {255, 255, 255};
	u32 w;
	for(i = 0; i < WS_PIXEL; i++) {
		w = wait * 10000;
		wsClearAll();
		wsSetColor(i, ws_white, 0.1);
		wsShow();
		while(w--);
	}
	wsClearAll();
	wsShow();
}
