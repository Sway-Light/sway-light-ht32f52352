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
#include "DFPlayer.h"
#include "74HC4067.h"

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

/* Private function prototypes -----------------------------------------------------------------------------*/
void NVIC_Configuration(void);
void CKCU_Configuration(void);
void GPIO_Configuration(void);
void GPTM1_Configuration(void);
void I2C_Configuration(void);
void ADC_Configuration(void);
void TM_Configuration(void);
void espUART_Configuration(void);

#if (ENABLE_CKOUT == 1)
void CKOUTConfig(void);
#endif

void ledInit(void);
u32 Touchkey_ButtonRead(void);
void _I2C_Touchkey_AckPolling(void);
void get_TKLR(void);
void Slide(u32, u32, u8*);
void Zoom(u32, u32, u8*);
void wsUpdateMag(void);
void ADC_MainRoutine(void);
void RUN_FFT(void);
void speakerEnable(bool enable);
static void delay(u32 count);

/* Global variables ----------------------------------------------------------------------------------------*/
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

typedef enum {
	btRelease,
	btPressed
}BtStatus;
BtStatus mBtStatus = btRelease;

typedef enum {
	btNone,
	btClick,
	btLongClick
}BtAction;
BtAction mBtAction = btNone;

u8 mode = 2;

// Touch key
const u8 zoom = 3, slide = 2, press = 1, none = 0;
u8 slideValue = 24, zoomValue = 2;
u8 status = none;
TouchKey_TypeDef Touch;
bool TK_CHECK = FALSE, TK_1SEC = FALSE;
u8 TK_L = 0, TK_R = 0;
u16 TK_COUNT = 0, adcIndex = 0;
s16 j = 0;

// WS2812B
u8 ws_white[3] = {255, 255, 255}, ws_clean[3] = {0, 0, 0};
u8 wsLevel[16] = {1};
u8 wsLevelTM[16] = {0};

// Touch key
#define I2C_TOUCHKEY_SPEED         (100000)          /*!< I2C speed                                          */
#define I2C_TOUCHKEY_DEV_ADDR      (0x50)           /*!< I2C device address                                 */
#define KEYSTATUS_CMD              (0x08)
bool touchKeyDelayFlag = FALSE;

// FFT
#define TEST_LENGTH_SAMPLES 128
bool sampleFlag = FALSE, startShow = FALSE, initFlag = FALSE;
s32 InputSignal[TEST_LENGTH_SAMPLES];
float32_t fftData[TEST_LENGTH_SAMPLES];
static float32_t OutputSignal[TEST_LENGTH_SAMPLES / 2];
uint32_t fftSize = TEST_LENGTH_SAMPLES / 2;
uint32_t ifftFlag = 0;
uint32_t doBitReverse = 1;

// ADC
s32 gADC_Result;
vu32 gADC_CycleEndOfConversion;

// LED
u8 WS_LED[16][9];

// Button
bool btPress = FALSE;
bool btReleaseFlag = FALSE;
bool longClick = FALSE;
u32 btTM = 0;

// mp3
//static u8 sendData[10] = {0x7E, 0xFF, 0x06, 0x0C, 0x00, 0x00, 0x00, 0xFE, 0xEF, 0xEF};
//static u8 returnData[10];
u8 mp3CmdQueue[QUEUE_MAX_SIZE];
u8 queueSize = 0;

// esp8266
bool espFlag = FALSE;
bool errorFlag = FALSE;
u8 data_from_esp[9], data_to_esp[9];
u8 recieve_index = 0, send_index = 0;

/* Private variables ---------------------------------------------------------------------------------------*/
/* Global functions ----------------------------------------------------------------------------------------*/
/*********************************************************************************************************//**
  * @brief  Main program.
  * @retval None
  ***********************************************************************************************************/
int main(void) {
	u32 uCounter;
	
	NVIC_Configuration();               /* NVIC configuration                                                 */
	CKCU_Configuration();               /* System Related configuration                                       */
	GPIO_Configuration();               /* GPIO Related configuration                                         */
	RETARGET_Configuration();           /* Retarget Related configuration                                     */
	
	ledInit();
	wsInit();
	wsBlinkAll(1000);
//	delay(10000000);
//		
	GPTM1_Configuration();
	TM_Configuration();
	I2C_Configuration();
	ADC_Configuration();
	
	initFlag = TRUE;
	ADC_Cmd(HT_ADC0, ENABLE);
	TM_Cmd(HT_GPTM0, ENABLE);
	TM_Cmd(HT_GPTM1, ENABLE);
	
	/* Analog signal setup */
	analogSwitcherSetup();
	asSetEnable(TRUE);
	asSetSignal(1);
	
	speakerEnable(TRUE);
	
	espUART_Configuration();
	/* UART setup */
	mp3UART_Configuration();
	mp3ResetModule(mp3CmdQueue, &queueSize);
	mp3SetDevice(mp3CmdQueue, &queueSize, 1); // SD Card
	delay(100000);
	mp3SetVolume(mp3CmdQueue, &queueSize, 20);
	mp3Play(mp3CmdQueue, &queueSize, 1);
	delay(100000);
	
	asSetSignal(0);
	
	while (1) {
		if (espFlag) {
			u8 i;
			u16 sum = 0, checksum = 0;
			
			for (i = 1; i < 6; i++) sum += data_from_esp[i];
			checksum = (data_from_esp[6] << 8) + data_from_esp[7];

			printf("\r\n");
			if(!(data_from_esp[0] == 0x95 || data_from_esp[0] == 0x96 || data_from_esp[0] == 0x97)) {
				printf("start byte Error\r\n");
				errorFlag = TRUE;
			}
			if (checksum == sum) {
//				for (i = 0; i < 9; i++) {
//				if (i == 0) {
//						if (data_from_esp[i] == 0x95) printf("Switching Mode: \r\n");
//						else if (data_from_esp[i] == 0x96) printf("Pleasant Mode: \r\n");
//						else if (data_from_esp[i] == 0x97) printf("Music Mode: \r\n");
//					}
//					printf("0x%02X ", data_from_esp[i]);
//				}
				printf("checksum Correct!\r\n");
			} else {
				printf("checksum Error!\r\n");
//				printf("checksum = %04X\r\n", checksum);
//				printf("sum = %04X\r\n", sum);
				
			}
			espFlag = FALSE;
		}
		if (mBtAction == btClick) {
			static bool flag = TRUE;
			mBtAction = btNone;
			printf("click\r\n");
			
			if (mode == 1) mode = 2;
			else if (mode == 2) {
				mode = 1;
				adcIndex = 0;
			}
			wsUpdateMag();
		} else if (mBtAction == btLongClick) {
			mBtAction = btNone;
			printf("long click\r\n");
			if (mode != 0) {
				asSetSignal(1);
				mp3SetVolume(mp3CmdQueue, &queueSize, 20);
				mp3Play(mp3CmdQueue, &queueSize, 2);
				delay(10000000);
				mode = 0;
				wsClearAll();
				wsShow();
				speakerEnable(FALSE);
				printf("off\r\n");
				
			} else {
				mode = 2;
				adcIndex = 0;
				speakerEnable(TRUE);
				asSetSignal(1);
				mp3SetVolume(mp3CmdQueue, &queueSize, 20);
				mp3Play(mp3CmdQueue, &queueSize, 1);
				delay(10000000);
				asSetSignal(0);
				printf("on\r\n");
			}
		}
		
		if (touchKeyDelayFlag) {
			if (!GPIO_ReadInBit(HT_GPIOC, GPIO_PIN_0)) {
				TK_CHECK = TRUE;
				Touch.Data = Touchkey_ButtonRead();
				get_TKLR();
//				printf("\rStatus = %d, DATA = %04X, slideValue = %3d, zoomValue = %3d", status, Touch.Data, slideValue, zoomValue);
				if (status == slide) Slide(TK_L, TK_R, &slideValue);
				else if (status == zoom) Zoom(TK_L, TK_R, &zoomValue);
				
				wsUpdateMag();
				touchKeyDelayFlag = FALSE;
//				uCounter = (HSE_VALUE >> 6);
//				printf("%d\r\n", uCounter);
//				while (uCounter--);
			} else {
				TK_CHECK = FALSE;
				TK_1SEC = TRUE;
				TK_COUNT = 0;
				status = none;
			}
		}
		if (mode == 2) {
			ADC_MainRoutine();
			if (sampleFlag) {
				for (j = 0; j < TEST_LENGTH_SAMPLES; j += 1) fftData[j] = ((float)InputSignal[j]) / 2048.0;
				RUN_FFT();
				wsUpdateMag();
			}
		} else if (mode == 0) {
			
		}
	}
}

void NVIC_Configuration(void) {
	/* Set the Vector Table base location at 0x00000000   */
	NVIC_SetVectorTable(NVIC_VECTTABLE_FLASH, 0x0);
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
	
	// Music mute pin
	GPIO_DirectionConfig(HT_GPIOC, GPIO_PIN_15, GPIO_DIR_OUT);
	
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
	TimeBaseInit.CounterReload = 18000;                  // interrupt in every 500us
	TimeBaseInit.Prescaler = 5;
	TimeBaseInit.PSCReloadTime = TM_PSC_RLD_IMMEDIATE;   // reload immediately
	TM_TimeBaseInit(HT_GPTM1, &TimeBaseInit);            // write the parameters into GPTM1
	TM_ClearFlag(HT_GPTM1, TM_FLAG_UEV);                 // Clear Update Event Interrupt flag
	TM_IntConfig(HT_GPTM1, TM_INT_UEV, ENABLE);          // interrupt by GPTM update
	
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
	AFIO_GPxConfig(GPIO_PA, AFIO_PIN_6, AFIO_FUN_ADC0);

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
		TimeBaseInit.Prescaler = (SystemCoreClock / 25000) - 1;
		TimeBaseInit.CounterReload = 10 - 1;
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
		OutInit.Compare = 5 - 1;
		OutInit.AsymmetricCompare = 0;
		TM_OutputInit(HT_GPTM0, &OutInit);
	}

	TM_IntConfig(HT_GPTM0, TM_INT_CH3CC, ENABLE);
	NVIC_EnableIRQ(GPTM0_IRQn);
}

void espUART_Configuration(void) {
	USART_InitTypeDef USART_InitStructure;
	
	{ /* Enable peripheral clock                                                                              */
		CKCU_PeripClockConfig_TypeDef CKCUClock = {{ 0 }};
		CKCUClock.Bit.USART0 = 1;
		CKCU_PeripClockConfig(CKCUClock, ENABLE);
	}
	
//	/* Turn on UxART Rx internal pull up resistor to prevent unknow state                                     */
//	GPIO_PullResistorConfig(HT_GPIOC, GPIO_PIN_13, GPIO_PR_UP);
	
	AFIO_GPxConfig(GPIO_PA, AFIO_PIN_8, AFIO_FUN_USART_UART); // TX
	AFIO_GPxConfig(GPIO_PA, AFIO_PIN_10, AFIO_FUN_USART_UART); // RX
	
	USART_InitStructure.USART_BaudRate = 9600;
	USART_InitStructure.USART_WordLength = USART_WORDLENGTH_8B;
	USART_InitStructure.USART_StopBits = USART_STOPBITS_1;
	USART_InitStructure.USART_Parity = USART_PARITY_NO;
	USART_InitStructure.USART_Mode = USART_MODE_NORMAL;
	USART_Init(HT_USART0, &USART_InitStructure);
	
	NVIC_EnableIRQ(USART0_IRQn);
	
	USART_IntConfig(HT_USART0, USART_INT_RXDR, ENABLE);
	
	USART_TxCmd(HT_USART0, ENABLE);
	USART_RxCmd(HT_USART0, ENABLE);
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
void ledInit() {
	u8 i, j;
//j=0 { 15,  16,  47,  48,  79,  80, 111, 112, 143},
//    { 14,  17,        ...                       },
//                      ...
//j=15{  0,             ...                       }
//       ^                                      ^
//       i=0                                    i=7
	for(i = 0; i < 9; i++) {
		for(j = 0; j < 16; j++) {
			if(i % 2 == 0) {
				WS_LED[j][i] = i * 16 + (15 - j);
			}else {
				WS_LED[j][i] = i * 16 + j;
			}
//			printf("%4d,", WS_LED[j][i]);
		}
//		printf("\r\n");
	}
}

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

void wsUpdateMag() {
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
			for (j = 0; j < 9; j += 1) {
				if (rows[i] == 1) wsSetColor(WS_LED[i][j], ws_white, ((float)slideValue) / 100.0);
				else wsSetColor(WS_LED[i][j], ws_clean, 0);
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
				{255, 200, 0},
				{255, 128, 0},
				{255, 64, 0},
				{255, 32, 0},
				{255, 0, 0}
		};
		
		for (i = 0; i < 16; i += 1) {
			u8 scale = 2;
			if (OutputSignal[i*scale + 1] < 3) level = 1;
			else if(OutputSignal[i*scale + 1] < 5) level = 2;
			else if(OutputSignal[i*scale + 1] < 8) level = 3;
			else if(OutputSignal[i*scale + 1] < 11) level = 4;
			else if(OutputSignal[i*scale + 1] < 14) level = 5;
			else if(OutputSignal[i*scale + 1] < 17) level = 6;
			else if(OutputSignal[i*scale + 1] < 20) level = 7;
			else if(OutputSignal[i*scale + 1] < 23) level = 8;
			else level = 9;
			
			for (j = 0; j < 9; j += 1) {
				//WS_LED[index][level]
//				if (j < level) wsSetColor(WS_LED[i][j], musicColor[j], ((float)slideValue) / 100.0);
				if (j < level) wsSetColor(WS_LED[i][j], musicColor[j], 0.3);
				else wsSetColor(WS_LED[i][j], musicColor[j], 0);
				if(j == wsLevel[i] - 1)
//					wsSetColor(WS_LED[i][j], musicColor[j], ((float)slideValue) / 100.0);
					wsSetColor(WS_LED[i][j], musicColor[j], 0.3);
			}
			
			if(level > wsLevel[i]) {
				wsLevel[i] = level;
				wsLevelTM[i] = 50;
			}
			if(wsLevelTM[i] == 0) {
				if(wsLevel[i] >= level) wsLevel[i]--;
				wsLevelTM[i] = 8;
			}
		}
	}
	
	startShow = TRUE;
}

void ADC_MainRoutine(void) {
	if (gADC_CycleEndOfConversion) {
		if (adcIndex < TEST_LENGTH_SAMPLES) {
			InputSignal[adcIndex] = gADC_Result;
			InputSignal[adcIndex + 1] = 0;
			adcIndex += 2;
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

void speakerEnable(bool enable) {
	// Music mute pin
	// Enable  -> low
	// Disable -> high
	GPIO_WriteOutBits(HT_GPIOC, GPIO_PIN_15, !enable);
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
