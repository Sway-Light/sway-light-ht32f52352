/*********************************************************************************************************//**
 * @file    IP/Example/main.c
 * @version $Rev:: 4722         $
 * @date    $Date:: 2020-04-06 #$
 * @brief   Main program.
 *************************************************************************************************************
 * @attention
 *
 * Firmware Disclaimer Information
 *
 * 1. The customer hereby acknowledges and agrees that the program technical documentation, including the
 *    code, which is supplied by Holtek Semiconductor Inc., (hereinafter referred to as "HOLTEK") is the
 *    proprietary and confidential intellectual property of HOLTEK, and is protected by copyright law and
 *    other intellectual property laws.
 *
 * 2. The customer hereby acknowledges and agrees that the program technical documentation, including the
 *    code, is confidential information belonging to HOLTEK, and must not be disclosed to any third parties
 *    other than HOLTEK and the customer.
 *
 * 3. The program technical documentation, including the code, is provided "as is" and for customer reference
 *    only. After delivery by HOLTEK, the customer shall use the program technical documentation, including
 *    the code, at their own risk. HOLTEK disclaims any expressed, implied or statutory warranties, including
 *    the warranties of merchantability, satisfactory quality and fitness for a particular purpose.
 *
 * <h2><center>Copyright (C) Holtek Semiconductor Inc. All rights reserved</center></h2>
 ************************************************************************************************************/
// <<< Use Configuration Wizard in Context Menu >>>

/* Includes ------------------------------------------------------------------------------------------------*/
#include "ht32.h"
#include "ht32_board.h"

#include "arm_math.h"
#include "arm_const_structs.h"

#include "ws2812.h"

#include "_ht32_project_source.h"

/** @addtogroup Project_Template Project Template
  * @{
  */

/** @addtogroup IP_Examples IP
  * @{
  */

/** @addtogroup Example
  * @{
  */


/* Settings ------------------------------------------------------------------------------------------------*/
/* Private types -------------------------------------------------------------------------------------------*/
typedef union {
	struct {
		unsigned long Key1:  1;
		unsigned long Key2:  1;
		unsigned long Key3:  1;
		unsigned long Key4:  1;
		unsigned long Key5:  1;
		unsigned long Key6:  1;
		unsigned long Key7:  1;
		unsigned long Key8:  1;
		unsigned long Key9:  1;
		unsigned long Key10: 1;
		unsigned long Key11: 1;
		unsigned long Key12: 1;
		unsigned long Key13: 1;
		unsigned long Key14: 1;
		unsigned long Key15: 1;
		unsigned long Key16: 1;
	} Bit;
	u16 Data;
} TouchKey_TypeDef;

// Touch key
const u8 zoom = 3, slide = 2, press = 1, none = 0;
u8 slideValue = 24, zoomValue = 2;
u8 status = none;
TouchKey_TypeDef Touch;
bool TK_CHECK = FALSE, TK_1SEC = FALSE;
u8 TK_L = 0, TK_R = 0;
u16 TK_COUNT = 0, i = 0;

// WS2812B
u8 ws_white[3] = {255, 255, 255}, ws_clean[3] = {0, 0, 0};
u8 wsLevel[16] = {1};
u8 wsLevelTM[16] = {0};

// Touch key I2C
#define I2C_TOUCHKEY_SPEED         (50000)          /*!< I2C speed                                          */
#define I2C_TOUCHKEY_DEV_ADDR      (0x50)           /*!< I2C device address                                 */
#define KEYSTATUS_CMD              (0x08)

// FFT
#define TEST_LENGTH_SAMPLES 128

/* Private function prototypes -----------------------------------------------------------------------------*/
void NVIC_Configuration(void);
void CKCU_Configuration(void);
void GPIO_Configuration(void);
void GPTM1_Configuration(void);
void I2C_Configuration(void);
void ADC_Configuration(void);
void TM_Configuration(void);

#if (ENABLE_CKOUT == 1)
void CKOUTConfig(void);
#endif

u32 Touchkey_ButtonRead(void);
void _I2C_Touchkey_AckPolling(void);
void get_TKLR(void);
void Slide(u32, u32, u8*);
void Zoom(u32, u32, u8*);
void wsUpdateMag(u8);
void ADC_MainRoutine(void);
void RUN_FFT(void);
static void delay(u32 count);

/* Private macro -------------------------------------------------------------------------------------------*/
/* Global variables ----------------------------------------------------------------------------------------*/
s32 gADC_Result;
vu32 gADC_CycleEndOfConversion;
/* -------------------------------------------------------------------
* External Input and Output buffer Declarations for FFT Bin Example
* ------------------------------------------------------------------- */
bool sampleFlag = FALSE, startShow = FALSE, initFlag = FALSE;
s32 InputSignal[TEST_LENGTH_SAMPLES];
float32_t fftData[TEST_LENGTH_SAMPLES];
static float32_t OutputSignal[TEST_LENGTH_SAMPLES / 2];

/* ------------------------------------------------------------------
* Global variables for FFT Bin Example
* ------------------------------------------------------------------- */
uint32_t fftSize = TEST_LENGTH_SAMPLES / 2;
uint32_t ifftFlag = 0;
uint32_t doBitReverse = 1;

const u8 WS_LED[9][16] = {
	{ 15,  14,  13,  12,  11,  10,   9,   8,   7,   6,   5,   4,   3,   2,   1,   0},
	{ 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31},
	{ 47,  46,  45,  44,  43,  42,  41,  40,  39,  38,  37,  36,  35,  34,  33,  32},
	{ 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63},
	{ 79,  78,  77,  76,  75,  74,  73,  72,  71,  70,  69,  68,  67,  66,  65,  64},
	{ 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95},
	{111, 110, 109, 108, 107, 106, 105, 104, 103, 102, 101, 100,  99,  98,  97,  96},
	{112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127},
	{143, 142, 141, 140, 139, 138, 137, 136, 135, 134, 133, 132, 131, 130, 129, 128}
};

s16 j = 0;

/* Private variables ---------------------------------------------------------------------------------------*/
/* Global functions ----------------------------------------------------------------------------------------*/
/*********************************************************************************************************//**
  * @brief  Main program.
  * @retval None
  ***********************************************************************************************************/
int main(void) {
	u32 uCounter;
	u8 mode = 2;
	
	NVIC_Configuration();               /* NVIC configuration                                                 */
	CKCU_Configuration();               /* System Related configuration                                       */
	GPIO_Configuration();               /* GPIO Related configuration                                         */
	RETARGET_Configuration();           /* Retarget Related configuration                                     */
	
	GPTM1_Configuration();
	TM_Configuration();
	I2C_Configuration();
	ADC_Configuration();
	
	wsInit();
	
	wsBlinkAll(30);
	
	initFlag = TRUE;
	
	ADC_Cmd(HT_ADC0, ENABLE);
	TM_Cmd(HT_GPTM0, ENABLE);
	while (1) {
		if (!GPIO_ReadInBit(HT_GPIOB, GPIO_PIN_1)) {
			while (!GPIO_ReadInBit(HT_GPIOB, GPIO_PIN_1));
			if (mode == 1) mode = 2;
			else if (mode == 2) {
				mode = 1;
				i = 0;
			}
			wsUpdateMag(mode);
			printf("\rmode = %d", mode);
		}
		
		if (mode == 1) {
			if (!GPIO_ReadInBit(HT_GPIOC, GPIO_PIN_0)) {
				TK_CHECK = TRUE;
				Touch.Data = Touchkey_ButtonRead();
				get_TKLR();
				printf("\rStatus = %d, DATA = %04X, slideValue = %3d, zoomValue = %3d", status, Touch.Data, slideValue, zoomValue);
				if (status == slide) Slide(TK_L, TK_R, &slideValue);
				else if (status == zoom) Zoom(TK_L, TK_R, &zoomValue);
				
				wsUpdateMag(mode);
				
				uCounter = (HSE_VALUE >> 4);
				while (uCounter--);
			} else {
				TK_CHECK = FALSE;
				TK_1SEC = TRUE;
				TK_COUNT = 0;
				status = none;
			}
		}else if(mode == 2) {
			ADC_MainRoutine();
			if (sampleFlag) {
				for (j = 0; j < TEST_LENGTH_SAMPLES; j += 1) fftData[j] = ((float)InputSignal[j]) / 2048.0;
				RUN_FFT();
				if (mode == 2) {
					wsUpdateMag(mode);
					delay(300);
				}
			}
		}
	}
}

void NVIC_Configuration(void) {
	NVIC_SetVectorTable(NVIC_VECTTABLE_FLASH, 0x0);     /* Set the Vector Table base location at 0x00000000   */
}

void CKCU_Configuration(void) {
	#if 1
	CKCU_PeripClockConfig_TypeDef CKCUClock = {{ 0 }};
	CKCUClock.Bit.AFIO = 1;
	CKCU_PeripClockConfig(CKCUClock, ENABLE);
	#endif

	#if (ENABLE_CKOUT == 1)
	CKOUTConfig();
	#endif
}

void GPIO_Configuration(void) {
	{ /* Enable peripheral clock                                                                              */
		CKCU_PeripClockConfig_TypeDef CKCUClock = {{ 0 }};
		CKCUClock.Bit.PA = 1;
		CKCUClock.Bit.PB = 1;
		CKCUClock.Bit.PC = 1;
		CKCUClock.Bit.PD = 1;
		CKCU_PeripClockConfig(CKCUClock, ENABLE);
	}
	
	GPIO_DirectionConfig(HT_GPIOC, GPIO_PIN_0, GPIO_DIR_IN);
	GPIO_InputConfig(HT_GPIOC, GPIO_PIN_0, ENABLE);
	
	GPIO_DirectionConfig(HT_GPIOB, GPIO_PIN_1, GPIO_DIR_IN);
	GPIO_InputConfig(HT_GPIOB, GPIO_PIN_1, ENABLE);
	
#if (RETARGET_PORT == RETARGET_USART0)
	AFIO_GPxConfig(GPIO_PA, AFIO_PIN_2 | AFIO_PIN_3, AFIO_FUN_USART_UART);
#endif

#if (RETARGET_PORT == RETARGET_USART1)
	AFIO_GPxConfig(GPIO_PA, AFIO_PIN_4 | AFIO_PIN_5, AFIO_FUN_USART_UART);
#endif

#if (RETARGET_PORT == RETARGET_UART0)
	AFIO_GPxConfig(GPIO_PC, AFIO_PIN_4 | AFIO_PIN_5, AFIO_FUN_USART_UART);
#endif

#if (RETARGET_PORT == RETARGET_UART1)
	AFIO_GPxConfig(GPIO_PC, AFIO_PIN_1 | AFIO_PIN_3, AFIO_FUN_USART_UART);
#endif
}

void GPTM1_Configuration(void) {
	TM_TimeBaseInitTypeDef TimeBaseInit;
	
	{ /* Enable peripheral clock                                                                              */
		CKCU_PeripClockConfig_TypeDef CKCUClock = {{ 0 }};
		CKCUClock.Bit.GPTM1 = 1;
		CKCU_PeripClockConfig(CKCUClock, ENABLE);
	}
	
	TM_TimeBaseStructInit(&TimeBaseInit);                // Init GPTM1 time-base
	TimeBaseInit.CounterMode = TM_CNT_MODE_UP;           // up count mode
	TimeBaseInit.CounterReload = 36000;                  // interrupt in every 500us
	TimeBaseInit.Prescaler = 5;
	TimeBaseInit.PSCReloadTime = TM_PSC_RLD_IMMEDIATE;   // reload immediately
	TM_TimeBaseInit(HT_GPTM1, &TimeBaseInit);            // write the parameters into GPTM1
	TM_ClearFlag(HT_GPTM1, TM_FLAG_UEV);                 // Clear Update Event Interrupt flag
	TM_IntConfig(HT_GPTM1, TM_INT_UEV, ENABLE);          // interrupt by GPTM update
	TM_Cmd(HT_GPTM1, ENABLE);                            // enable the counter 1
	
	NVIC_EnableIRQ(GPTM1_IRQn);
}

void I2C_Configuration(void) {
	I2C_InitTypeDef I2C_InitStructure = { 0 };
	
	{ /* Enable peripheral clock                                                                              */
		CKCU_PeripClockConfig_TypeDef CKCUClock = {{ 0 }};
		CKCUClock.Bit.I2C1 = 1;
		CKCU_PeripClockConfig(CKCUClock, ENABLE);
	}

	/* Configure I2C SCL pin, I2C SDA pin                                                                     */
	AFIO_GPxConfig(GPIO_PA, AFIO_PIN_0, AFIO_FUN_I2C);
	AFIO_GPxConfig(GPIO_PA, AFIO_PIN_1, AFIO_FUN_I2C);

	/* I2C configuration                                                                                      */
	I2C_InitStructure.I2C_GeneralCall = I2C_GENERALCALL_DISABLE;
	I2C_InitStructure.I2C_AddressingMode = I2C_ADDRESSING_7BIT;
	I2C_InitStructure.I2C_Acknowledge = I2C_ACK_DISABLE;
	I2C_InitStructure.I2C_OwnAddress = 0x00;
	I2C_InitStructure.I2C_Speed = I2C_TOUCHKEY_SPEED;
	I2C_InitStructure.I2C_SpeedOffset = 0;
	I2C_Init(HT_I2C1, &I2C_InitStructure);
	I2C_Cmd(HT_I2C1, ENABLE);
}

void ADC_Configuration(void) {
	{ /* Enable peripheral clock                                                                              */
		CKCU_PeripClockConfig_TypeDef CKCUClock = {{ 0 }};
		CKCUClock.Bit.ADC0 = 1;
		CKCU_PeripClockConfig(CKCUClock, ENABLE);
	}

	/* Configure AFIO mode as ADC function                                                                    */
	AFIO_GPxConfig(GPIO_PA, AFIO_PIN_6, AFIO_MODE_2);

	{ /* ADC related settings                                                                                 */
		/* CK_ADC frequency is set to (CK_AHB / 32)                                                             */
		CKCU_SetADCnPrescaler(CKCU_ADCPRE_ADC0, CKCU_ADCPRE_DIV32);

		/* One Shot mode, sequence length = 1                                                                   */
		ADC_RegularGroupConfig(HT_ADC0, ONE_SHOT_MODE, 1, 0);

		/* ADC conversion time = (Sampling time + Latency) / CK_ADC = (1.5 + ADST + 12.5) / CK_ADC              */
		/* Set ADST = 0, sampling time = 1.5 + ADST                                                             */
		#if (LIBCFG_ADC_SAMPLE_TIME_BY_CH)
		// The sampling time is set by the last parameter of the function "ADC_RegularChannelConfig()".
		#else
		ADC_SamplingTimeConfig(HT_ADC0, 0);
		#endif

		/* Set ADC conversion sequence as channel n                                                             */
		ADC_RegularChannelConfig(HT_ADC0, ADC_CH_6, 0, 0);

		/* Set GPTM0 CH3O as ADC trigger source                                                                 */
		ADC_RegularTrigConfig(HT_ADC0, ADC_TRIG_GPTM0_CH3O);
	}

	/* Enable ADC single/cycle end of conversion interrupt                                                    */
	ADC_IntConfig(HT_ADC0, ADC_INT_SINGLE_EOC | ADC_INT_CYCLE_EOC, ENABLE);

	/* Enable the ADC interrupts                                                                              */
	NVIC_EnableIRQ(ADC0_IRQn);
}

void TM_Configuration(void) {
	{ /* Enable peripheral clock                                                                              */
		CKCU_PeripClockConfig_TypeDef CKCUClock = {{ 0 }};
		CKCUClock.Bit.GPTM0 = 1;
		CKCU_PeripClockConfig(CKCUClock, ENABLE);
	}

	{ /* Time base configuration                                                                              */
		TM_TimeBaseInitTypeDef TimeBaseInit;
		TimeBaseInit.Prescaler = (SystemCoreClock / 21000) - 1;
		TimeBaseInit.CounterReload = 6 - 1;
		TimeBaseInit.RepetitionCounter = 0;
		TimeBaseInit.CounterMode = TM_CNT_MODE_UP;
		TimeBaseInit.PSCReloadTime = TM_PSC_RLD_IMMEDIATE;
		TM_TimeBaseInit(HT_GPTM0, &TimeBaseInit);
	}

	{ /* Channel n output configuration                                                                       */
		TM_OutputInitTypeDef OutInit;
		OutInit.Channel = TM_CH_3;
		OutInit.OutputMode = TM_OM_PWM2;
		OutInit.Control = TM_CHCTL_ENABLE;
		OutInit.ControlN = TM_CHCTL_DISABLE;
		OutInit.Polarity = TM_CHP_NONINVERTED;
		OutInit.PolarityN = TM_CHP_NONINVERTED;
		OutInit.IdleState = MCTM_OIS_LOW;
		OutInit.IdleStateN = MCTM_OIS_HIGH;
		OutInit.Compare = 3 - 1;
		OutInit.AsymmetricCompare = 0;
		TM_OutputInit(HT_GPTM0, &OutInit);
	}

	TM_IntConfig(HT_GPTM0, TM_INT_CH3CC, ENABLE);
	NVIC_EnableIRQ(GPTM0_IRQn);
}

#if (ENABLE_CKOUT == 1)
/*********************************************************************************************************//**
  * @brief  Configure the debug output clock.
  * @retval None
  ***********************************************************************************************************/
void CKOUTConfig(void) {
	CKCU_CKOUTInitTypeDef CKOUTInit;

	CKCU_PeripClockConfig_TypeDef CKCUClock = {{ 0 }};
	CKCUClock.Bit.AFIO = 1;
	CKCU_PeripClockConfig(CKCUClock, ENABLE);

	AFIO_GPxConfig(GPIO_PA, AFIO_PIN_9, AFIO_MODE_15);
	CKOUTInit.CKOUTSRC = CKCU_CKOUTSRC_HCLK_DIV16;
	CKCU_CKOUTConfig(&CKOUTInit);
}
#endif

#if (HT32_LIB_DEBUG == 1)
/*********************************************************************************************************//**
  * @brief  Report both the error name of the source file and the source line number.
  * @param  filename: pointer to the source file name.
  * @param  uline: error line source number.
  * @retval None
  ***********************************************************************************************************/
void assert_error(u8* filename, u32 uline) {
	/*
	 This function is called by IP library that the invalid parameters has been passed to the library API.
	 Debug message can be added here.
	 Example: printf("Parameter Error: file %s on line %d\r\n", filename, uline);
	*/

	while (1) {
	}
}
#endif

/* Private functions ---------------------------------------------------------------------------------------*/
u32 Touchkey_ButtonRead(void) {
	u32 uData;

	/* Touchkey addressread                                                                                   */
	_I2C_Touchkey_AckPolling();

	while (!I2C_CheckStatus(HT_I2C1, I2C_MASTER_TX_EMPTY));
	I2C_SendData(HT_I2C1, KEYSTATUS_CMD);

	/* Touchkey addressread                                                                                   */
	I2C_TargetAddressConfig(HT_I2C1, I2C_TOUCHKEY_DEV_ADDR, I2C_MASTER_READ);
	while (!I2C_CheckStatus(HT_I2C1, I2C_MASTER_RECEIVER_MODE));

	/* enable master receiver ACK                                                                             */
	I2C_AckCmd(HT_I2C1, ENABLE);

	/* sequential read                                                                                        */
	while (!I2C_CheckStatus(HT_I2C1, I2C_MASTER_RX_NOT_EMPTY));
	uData = I2C_ReceiveData(HT_I2C1);
	I2C_AckCmd(HT_I2C1, DISABLE);
	while (!I2C_CheckStatus(HT_I2C1, I2C_MASTER_RX_NOT_EMPTY));
	uData |= (I2C_ReceiveData(HT_I2C1) << 8);

	/* end of read                                                                                            */
	I2C_GenerateSTOP(HT_I2C1);

	return uData;
}

void _I2C_Touchkey_AckPolling(void) {
	u32 reg;
	/* wait if bus busy                                                                                       */
	while (I2C_GetFlagStatus(HT_I2C1, I2C_FLAG_BUSBUSY));

	/* send slave address until ack reply                                                                     */
	while (1) {
		/* send slave address                                                                                   */
		I2C_TargetAddressConfig(HT_I2C1, I2C_TOUCHKEY_DEV_ADDR, I2C_MASTER_WRITE);

		while (1) {
			reg = HT_I2C1->SR;
			if (reg & I2C_FLAG_ADRS) return;
			if (reg & I2C_FLAG_RXNACK) {
				I2C_ClearFlag(HT_I2C1, I2C_FLAG_RXNACK);
				break;
			}
		}
	}
}

void get_TKLR(void) {
	int index, check = 0x0001;
	
	for (index = 0; index < 16; index++) {
		if ((Touch.Data & check) == check) {
			TK_L = index;
			break;
		} else {
			check <<= 1;
		}
	}
	check = 0x8000;
	for (index = 15; index >= 0; index--) {
		if ((Touch.Data & check) == check) {
			TK_R = index;
			break;
		} else {
			check >>= 1;
		}
	}
}

void Slide(u32 L, u32 R, u8 *Value) {
	static u32 prevL = 0, prevR = 0;
	
	if (L != prevL || R != prevR) {
		if (L < prevL || R < prevR) {
			if (*Value <= 0) *Value = 0;
			else (*Value) -= 2;
		} else if (L > prevL || R > prevR) {
			if (*Value >= 100) *Value = 100;
			else (*Value) += 2;
		}
		prevL = L;
		prevR = R;
	}
}

void Zoom(u32 L, u32 R, u8* Value) {
	static u32 prevL = 0, prevR = 0;
	
	if (L != prevL || R != prevR) {
		if (L > prevL || R < prevR) {
			if (*Value <= 2) *Value = 2;
			else (*Value) -= 2;
		} else if (L < prevL || R > prevR) {
			if (*Value >= 16) *Value = 16;
			else (*Value) += 2;
		}
		prevL = L;
		prevR = R;
	}
}

void wsUpdateMag(u8 mode) {
	u8 i, j;
	
	if (mode == 1) {
		// Light Source Mode
		u8 rows[16] = {0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0};
		for (i = 0; i < 16; i += 1) {
			if (zoomValue <= 2) {
				if (i >= 7 && i <= 8) rows[i] = 1;
				else rows[i] = 0;
			} else if (zoomValue <= 4) {
				if (i >= 6 && i <= 9) rows[i] = 1;
				else rows[i] = 0;
			} else if (zoomValue <= 6) {
				if (i >= 5 && i <= 10) rows[i] = 1;
				else rows[i] = 0;
			} else if (zoomValue <= 8) {
				if (i >= 4 && i <= 11) rows[i] = 1;
				else rows[i] = 0;
			} else if (zoomValue <= 10) {
				if (i >= 3 && i <= 12) rows[i] = 1;
				else rows[i] = 0;
			} else if (zoomValue <= 12) {
				if (i >= 2 && i <= 13) rows[i] = 1;
				else rows[i] = 0;
			} else if (zoomValue <= 14) {
				if (i >= 1 && i <= 14) rows[i] = 1;
				else rows[i] = 0;
			} else if (zoomValue <= 16) {
				rows[i] = 1;
			}
		}
		for (i = 0; i < 16; i += 1) {
			for (j = 0; j < 8; j += 1) {
				if (rows[i] == 1) wsSetColor(WS_LED[j][i], ws_white, ((float)slideValue) / 100.0);
				else wsSetColor(WS_LED[j][i], ws_clean, 0);
			}
		}
	} else if (mode == 2) {
		// Music Mode
		u8 level;
		u8 musicColor[9][3] = {
				{0, 255, 0},
				{128, 255, 0},
				{200, 255, 0},
				{255, 255, 0},
				{200, 255, 0},
				{255, 200, 0},
				{255, 128, 0},
				{255, 80, 0},
				{255, 0, 0}
		};
		
		for (i = 0; i < 16; i += 1) {
			if (OutputSignal[i*2 + 1] < 3) level = 1;
			else if(OutputSignal[i*2 + 1] < 5) level = 2;
			else if(OutputSignal[i*2 + 1] < 8) level = 3;
			else if(OutputSignal[i*2 + 1] < 11) level = 4;
			else if(OutputSignal[i*2 + 1] < 14) level = 5;
			else if(OutputSignal[i*2 + 1] < 17) level = 6;
			else if(OutputSignal[i*2 + 1] < 20) level = 7;
			else level = 8;
			
			for (j = 0; j < 8; j += 1) {
				if (j < level) wsSetColor(WS_LED[j][i], musicColor[j], 0.3);
				else wsSetColor(WS_LED[j][i], musicColor[j], 0);
				if(j == wsLevel[i] - 1)
					wsSetColor(WS_LED[j][i], musicColor[j], 0.3);
			}
			
			if(level > wsLevel[i]) {
				wsLevel[i] = level;
				wsLevelTM[i] = 30;
			}
			if(wsLevelTM[i] == 0) {
				if(wsLevel[i] >= level) wsLevel[i]--;
//				wsLevel[i] = level;
				wsLevelTM[i] = 8;
			}
		}
	}
	
	startShow = TRUE;
}

void ADC_MainRoutine(void) {
	if (gADC_CycleEndOfConversion) {
		if (i < TEST_LENGTH_SAMPLES) {
			InputSignal[i] = gADC_Result;
			InputSignal[i + 1] = 0;
			i += 2;
		} else {
			sampleFlag = TRUE;
		}
		gADC_CycleEndOfConversion = FALSE;
	}
}

void RUN_FFT(void) {
	/* Process the data through the CFFT/CIFFT module */
	arm_cfft_f32(&arm_cfft_sR_f32_len64, fftData, ifftFlag, doBitReverse);

	/* Process the data through the Complex Magnitude Module for
	calculating the magnitude at each bin */
	arm_cmplx_mag_f32(fftData, OutputSignal, fftSize);
}

static void delay(u32 count) {
	while (count--) {
		__NOP(); // Prevent delay loop be optimized
	}
}


/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */
