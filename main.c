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

#include "time.h"

/** @addtogroup Project_Template Project Template
  * @{
  */

/** @addtogroup IP_Examples IP
  * @{
  */

/** @addtogroup Example
  * @{
  */

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

typedef struct {
	u32 year;
	u32 month;
	u32 day;
	u32 hour;
	u32 minute;
	u32 second;
} Time_T;

/* Settings ------------------------------------------------------------------------------------------------*/
void NVIC_Configuration(void);
void CKCU_Configuration(void);
void GPIO_Configuration(void);
void GPTM1_Configuration(void);
void I2C_Configuration(void);
void ADC_Configuration(void);
void TM_Configuration(void);
void espUART_Configuration(void);
void BLEUART_Configuration(void);
void BFTM0_Configuration(void);

#if (ENABLE_CKOUT == 1)
void CKOUTConfig(void);
#endif

/* Private function prototypes -----------------------------------------------------------------------------*/
void ledInit(u8 offset);
void setLedOffset(uint8_t offset);
u32 Touchkey_ButtonRead(void);
void _I2C_Touchkey_AckPolling(void);
void get_TKLR(void);
void Slide(u32, u32, u8*);
void Zoom(u32, u32, u8*);
u8 *showRows(u8 Value);
void wsUpdateMag(void);
void ADC_MainRoutine(void);
void DefineRefBit(void);
void RUN_FFT(void);
void speakerEnable(bool enable);
void bluetoothEnable(bool enable);
void calculateGradient(u8 i1, u8 i2, u8 Fst_color[], u8 Snd_color[]);
void generateMusicColor(u8 level);
void DataToESP(u8 category, u8 type);
void DataFromESP(u8 command[]);
void DataFromBT(u8 command[]);
void SwitchingMode(u8 status);
void LightingMode(u8 type, u8 data[]);
void MusicMode(u8 type, u8 data[]);
void Setting(u8 syncType, u8 data[]);
void BT_Module(u8 type, u8 data[]);
void On_Effect(u8 wait, bool reverse);
void Switch_Effect(u8 wait, u8 max_brightness);
void Light_Animation(u8 brightness);
void Music_Animation(u8 brightness);
static void delay(u32 count);

/* Global variables ----------------------------------------------------------------------------------------*/
/* Private types -------------------------------------------------------------------------------------------*/
// Switching Mode
u8 mode = 0, curr_mode = 3;

// Lighting Mode
u8 L_Type;
u8 Brightness = 0, Color[3] = {0xFF, 0xFF, 0xFF};

// Music Mode
u8 M_Type, mLevel;
u8 high_color[3] = {255, 0, 0}, mid_color[3] = {255, 255, 0}, low_color[3] = {0, 255, 0};
u8 musicColor[WS_LEV_SIZE][3];
bool Change_musicColor = FALSE;

// Setting
struct tm ts;
u32 Timestamp = 0;
u8 sync, auto_OnOff[2] = {0, 0};
bool realTime_flag = FALSE;
struct tm AutoOn, AutoOff;
u8 weekdayEN[7];

// Touch key
const u8 zoom = 3, slide = 2, press = 1, none = 0;
//typedef struct {
//	u8 slide;
//	u8 zoom;
//	u8 offset;
//} Z_O;
//Z_O Light, Music;
u8 slideValue = 60, zoomValue = 6, offsetValue = 0;
u8 status = none;
TouchKey_TypeDef Touch;
bool TK_CHECK = FALSE, TK_1SEC = FALSE;
u8 TK_L = 0, TK_R = 0;
u16 TK_COUNT = 0, adcIndex = 0;
s16 j = 0;

// WS2812B
u8 ws_white[3] = {255, 255, 255}, ws_clean[3] = {0, 0, 0};
u8 wsLevel[WS_FRQ_SIZE] = {1};
u8 wsLevelTM[WS_FRQ_SIZE] = {0};

// Touch key
#define I2C_TOUCHKEY_SPEED         (100000)          /*!< I2C speed                                          */
#define I2C_TOUCHKEY_DEV_ADDR      (0x50)           /*!< I2C device address                                 */
#define KEYSTATUS_CMD              (0x08)
bool touchKeyDelayFlag = FALSE;

// FFT
#define TEST_LENGTH_SAMPLES 256
u8 fft_mag = 24;
u16 ref_bit;
bool refFlag;
bool sampleFlag = FALSE, startShow = FALSE, initFlag = FALSE;
s32 InputSignal[TEST_LENGTH_SAMPLES];
float32_t fftData[TEST_LENGTH_SAMPLES] = {0};
static float32_t OutputSignal[TEST_LENGTH_SAMPLES / 2];
uint32_t fftSize = TEST_LENGTH_SAMPLES / 2;
uint32_t ifftFlag = 0;
uint32_t doBitReverse = 1;

// ADC
s32 gADC_Result;
vu32 gADC_CycleEndOfConversion;

// LED
u16 WS_LED[WS_FRQ_SIZE][WS_LEV_SIZE];

// Button
bool btPress = FALSE;
bool btReleaseFlag = FALSE;
bool longClick = FALSE;
u32 btTM = 0;

// esp8266
bool espFlag = FALSE, toEspFlag = FALSE;
bool errorFlag = FALSE;
u8 data_from_esp[10], data_to_esp[10];
u8 recieve_index = 0, send_index = 0;

// bluetooth
#define QUEUE_MAX_SIZE 50
u8 queueSize = 0, CmdQueue[QUEUE_MAX_SIZE];
u8 BLE_Flag = FALSE;
u8 BLE_TX[7] = "AT#__\r\n", BLE_RX[9] = "";
bool BT_CON = FALSE, BT_isPlay = FALSE;
u16 BT_VOL = 0;

/* Private variables ---------------------------------------------------------------------------------------*/
/* Global functions ----------------------------------------------------------------------------------------*/
/*********************************************************************************************************//**
  * @brief  Main program.
  * @retval None
  ***********************************************************************************************************/
int main(void) {
	delay(1000000);				// Wait for Operating Voltage to 3.3v
	
	NVIC_Configuration();     	// NVIC configuration
	CKCU_Configuration();     	// System Related configuration
	GPIO_Configuration();     	// GPIO Related configuration
	RETARGET_Configuration(); 	// Retarget Related configuration
	
	generateMusicColor(0x01); 	// High Level
	generateMusicColor(0x02); 	// Medium Level
	generateMusicColor(0x03); 	// Low Level
	
	BFTM0_Configuration();
	
	ledInit(0);
	wsInit();
	
	speakerEnable(FALSE);		// Speaker Off
	bluetoothEnable(FALSE);		// bluetoothEnable Module Off
	
	GPTM1_Configuration();
	TM_Configuration();
	I2C_Configuration();
	ADC_Configuration();
	
	BLEUART_Configuration();
	
	initFlag = TRUE;
	ADC_Cmd(HT_ADC0, ENABLE);
	TM_Cmd(HT_GPTM0, ENABLE);
	TM_Cmd(HT_GPTM1, ENABLE);
	
	/* ESP8266 Setup */
	espUART_Configuration();
	DataToESP(0x01, mode); 		// Sending Status Message
	while(toEspFlag == TRUE);	// Wait Until Data Transferred Complete
	DataToESP(0x02, 0x01); 		// Sending Initial Values of Color
	while(toEspFlag == TRUE);	// Wait Until Data Transferred Complete
	DataToESP(0x02, 0x05); 		// Sending Initial Values of Zoom, Slide, and Offset
	
	DefineRefBit();
	
	while (1) {
		if (realTime_flag == TRUE && Timestamp != 0) {
//			char buf[80];
			Timestamp += 1;
			
			ts = *localtime(&Timestamp);
			
//			strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", &ts);
//			printf("\r%s", buf);
			
			if (weekdayEN[ts.tm_wday - 1] == 1) {
				if (auto_OnOff[1] == 1 && mode != 0) {
					if (AutoOff.tm_hour == ts.tm_hour && AutoOff.tm_min == ts.tm_min && AutoOff.tm_sec == ts.tm_sec) {
						auto_OnOff[1] = 0;
						On_Effect(20, TRUE);
						mode = 0;
						if (toEspFlag != TRUE) DataToESP(0x01, mode); // Sending Power-Off Message
						speakerEnable(FALSE);
						bluetoothEnable(FALSE);
						
						DefineRefBit();
					}
				} else if (auto_OnOff[0] == 1 && mode == 0) {
					if (AutoOn.tm_hour == ts.tm_hour && AutoOn.tm_min == ts.tm_min && AutoOn.tm_sec == ts.tm_sec) {
						auto_OnOff[0] = 0;
						On_Effect(20, FALSE);
						mode = 3;
						mode = 1;
						if (toEspFlag != TRUE) DataToESP(0x01, mode); // Sending Power-On Message
						delay(1000);
						mode = 3;
						if (toEspFlag != TRUE) DataToESP(0x01, mode); // Sending Status Message
						adcIndex = 0;
						speakerEnable(TRUE);
						bluetoothEnable(TRUE);
					}
				}
			}
			
			realTime_flag = FALSE;
		}
		if (espFlag) {
			u8 i;
			u16 sum = 0, checksum = 0;
			
			for (i = 1; i < 7; i++) sum += data_from_esp[i];
			checksum = (data_from_esp[7] << 8) + data_from_esp[8];

//			printf("\r\n");
			if (!(data_from_esp[0] == 0x95)) {
//				printf("start byte Error\r\n");
				errorFlag = TRUE;
			}
			if (checksum == sum) {
				
				DataFromESP(data_from_esp);
				
//				for (i = 0; i < 9; i++) printf("0x%02X ", data_from_esp[i]);
//				printf("checksum Correct!\r\n");
			} else {
//				printf("checksum Error!\r\n");
			}
			espFlag = FALSE;
		}
		if (mBtAction == btClick) {
//			static bool flag = TRUE;
			mBtAction = btNone;
//			printf("click\r\n");
			
			if (mode == 2) {
				Switch_Effect(5, slideValue);
				mode = 3;
				curr_mode = mode;
			} else if (mode == 3) {
				Switch_Effect(5, slideValue);
				mode = 2;
				curr_mode = mode;
				adcIndex = 0;
			}
			if (toEspFlag != TRUE) DataToESP(0x01, mode); // Sending Status Message
			
			
			wsUpdateMag();
		} else if (mBtAction == btLongClick) {
			mBtAction = btNone;
//			printf("long click\r\n");
			
			if (mode != 0) {
				speakerEnable(FALSE);
				ref_bit = 0;
				On_Effect(20, TRUE);
				mode = 0;
				bluetoothEnable(FALSE);
				if (toEspFlag != TRUE) DataToESP(0x01, mode); // Sending Power-Off Message
				
				DefineRefBit();
				
//				printf("off\r\n");
			} else {
				On_Effect(20, FALSE);
				mode = 1;
				if (toEspFlag != TRUE) DataToESP(0x01, mode); // Sending Power-On Message
				delay(1000);
				mode = curr_mode;
				bluetoothEnable(TRUE);
				if (toEspFlag != TRUE) DataToESP(0x01, mode); // Sending Status Message
				
				if (mode == 3) adcIndex = 0;
				else wsUpdateMag();
				
				speakerEnable(TRUE);
				
//				printf("on\r\n");
			}
		}
		
		if (touchKeyDelayFlag) {
			if (!GPIO_ReadInBit(HT_GPIOC, GPIO_PIN_0)) {
				TK_CHECK = TRUE;
				Touch.Data = Touchkey_ButtonRead();
				get_TKLR();
//				printf("\rStatus = %d, DATA = %04X, slideValue = %3d, zoomValue = %3d", status, Touch.Data, slideValue, zoomValue);
				if (status == slide) {
					Slide(TK_L, TK_R, &slideValue);
//					if (mode == 2) Slide(TK_L, TK_R, &Light.slide);
//					else if (mode == 3) Slide(TK_L, TK_R, &Music.slide);
					if (toEspFlag != TRUE) DataToESP(mode, 0x05); // Sending Brightness in Lighting Mode
				} else if (status == zoom) {
					Zoom(TK_L, TK_R, &zoomValue);
//					if (mode == 2) Zoom(TK_L, TK_R, &Light.zoom);
//					else if (mode == 3) Zoom(TK_L, TK_R, &Music.zoom);
					if (toEspFlag != TRUE) DataToESP(mode, 0x05); // Sending Scale in Lighting Mode
				}
				
				wsUpdateMag();
				touchKeyDelayFlag = FALSE;
			} else {
				TK_CHECK = FALSE;
				TK_1SEC = TRUE;
				TK_COUNT = 0;
				status = none;
			}
		}
		
		if (mode == 3 || refFlag == TRUE) {
			ADC_MainRoutine();
			if (sampleFlag) {
				for (j = 0; j < TEST_LENGTH_SAMPLES; j += 1) fftData[j] = ((float)InputSignal[j]) / 2048.0;
				RUN_FFT();
				wsUpdateMag();
			}
		}
		
		if (BLE_Flag == TRUE) {
			DataFromBT(BLE_RX);
			BLE_Flag = FALSE;
		}
	}
}

/* Setup functions -----------------------------------------------------------------------------------------*/
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
	
	// Music Mute Pin
	GPIO_DirectionConfig(HT_GPIOC, GPIO_PIN_15, GPIO_DIR_OUT);
	
	// Bluetooth Module Power Pin
	GPIO_DirectionConfig(HT_GPIOC, GPIO_PIN_8, GPIO_DIR_OUT);
	
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
	
	AFIO_GPxConfig(GPIO_PA, AFIO_PIN_8, AFIO_FUN_USART_UART); // TX
	AFIO_GPxConfig(GPIO_PA, AFIO_PIN_10, AFIO_FUN_USART_UART); // RX
	
	USART_InitStructure.USART_BaudRate = 115200;
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

void BLEUART_Configuration(void) {
	USART_InitTypeDef USART_InitStructure;
	USART_SynClock_InitTypeDef USART_SynClock_InitStructure;
	
	{ /* Enable peripheral clock                                                                              */
		CKCU_PeripClockConfig_TypeDef CKCUClock = {{ 0 }};
		CKCUClock.Bit.UART0 = 1;
		CKCU_PeripClockConfig(CKCUClock, ENABLE);
	}
	
	AFIO_GPxConfig(GPIO_PC, AFIO_PIN_4, AFIO_FUN_USART_UART); // TX
	AFIO_GPxConfig(GPIO_PC, AFIO_PIN_5, AFIO_FUN_USART_UART); // RX
	
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WORDLENGTH_8B;
	USART_InitStructure.USART_StopBits = USART_STOPBITS_1;
	USART_InitStructure.USART_Parity = USART_PARITY_NO;
	USART_InitStructure.USART_Mode = USART_MODE_NORMAL;
	USART_Init(HT_UART0, &USART_InitStructure);
	
	USART_SynClock_InitStructure.USART_ClockEnable = USART_SYN_CLOCK_ENABLE;
	USART_SynClock_InitStructure.USART_ClockPhase = USART_SYN_CLOCK_PHASE_FIRST;
	USART_SynClock_InitStructure.USART_ClockPolarity = USART_SYN_CLOCK_POLARITY_LOW;
	USART_SynClock_InitStructure.USART_TransferSelectMode = USART_LSB_FIRST;
	USART_SynClockInit(HT_UART0, &USART_SynClock_InitStructure);
	
	NVIC_EnableIRQ(UART0_IRQn);
	
	USART_IntConfig(HT_UART0, USART_INT_RXDR, ENABLE);
	
	USART_TxCmd(HT_UART0, ENABLE);
	USART_RxCmd(HT_UART0, ENABLE);
}

void BFTM0_Configuration(void) {
	{ /* Enable peripheral clock                                                                              */
		CKCU_PeripClockConfig_TypeDef CKCUClock = {{ 0 }};
		CKCUClock.Bit.BFTM0 = 1;
		CKCU_PeripClockConfig(CKCUClock, ENABLE);
	}
	
	/* BFTM as Repetitive mode, every 0.5 second to trigger the match interrupt                               */
	BFTM_SetCompare(HT_BFTM0, SystemCoreClock / 10);
	BFTM_SetCounter(HT_BFTM0, 0);
	BFTM_IntConfig(HT_BFTM0, ENABLE);
	BFTM_EnaCmd(HT_BFTM0, ENABLE);

	NVIC_EnableIRQ(BFTM0_IRQn);
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
void ledInit(u8 offset) {
	u8 i, j;
//j=0 {0,  19, 20, ...}
//j=1 {1,  18, 21, ...}
//
//j=9 {9,  10, 29, ...}
//     ^   ^   ^
//     i=0 i=1 i=2

	for (i = 0; i < WS_FRQ_SIZE; i++) {
		for(j = 0; j < WS_LEV_SIZE; j++) {
			u8 ref = i + offset;
			
			if (ref > WS_FRQ_SIZE - 1) ref -= WS_FRQ_SIZE;
			
			if (ref % 2 != 0) {
				WS_LED[i][j] = ref * WS_LEV_SIZE + ((WS_LEV_SIZE - 1) - j);
			}else {
				WS_LED[i][j] = ref * WS_LEV_SIZE + j;
			}
//			printf("%4d,", WS_LED[i][j]);
		}
//		printf("\r\n");
	}
	
	
}

void setLedOffset(uint8_t offset) {
	ledInit(offset);
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
	s16 value;
	
	value = *Value;
	if (L != prevL || R != prevR) {
		if (L > prevL || R > prevR) {
			if (value < 128) (value) -= 2;
			else if (value >= 128) (value) -= 4;
			if (value <= 0) value = 0;
		} else if (L < prevL || R < prevR) {
			if (value < 128) (value) += 2;
			else if (value >= 128) (value) += 4;
			if (value >= 255) value = 255;
		}
		*Value = value;
		prevL = L;
		prevR = R;
	}
}

void Zoom(u32 L, u32 R, u8* Value) {
	static u32 prevL = 0, prevR = 0;
	
	if (L != prevL || R != prevR) {
		if (L > prevL || R < prevR) {
			if (*Value <= 6) *Value = 6;
			else (*Value) -= 1;
		} else if (L < prevL || R > prevR) {
			if (*Value >= WS_FRQ_SIZE) *Value = WS_FRQ_SIZE;
			else (*Value) += 1;
		}
		prevL = L;
		prevR = R;
	}
}

u8 *showRows(u8 Value) {
	u8 i;
	static u8 rows[WS_FRQ_SIZE] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	u8 start_row, end_row;
	if (Value % 2 != 0) {
		start_row = WS_FRQ_SIZE / 2 - (Value / 2 + 1);
		end_row = WS_FRQ_SIZE / 2 + (Value / 2 - 1);
	} else if (Value % 2 == 0) {
		start_row = WS_FRQ_SIZE / 2 - Value / 2;
		end_row = WS_FRQ_SIZE / 2 + (Value / 2 - 1);
	}
	for (i = 0; i < WS_FRQ_SIZE; i++) {
		if (i >= start_row && i <= end_row) rows[i] = 1;
		else rows[i] = 0;
	}
//	printf("rows: ");
//	for (i = 0; i < WS_FRQ_SIZE; i++) printf("%2d ", rows[i]);
//	printf("\r\n");
	
	return rows;
}

void wsUpdateMag() {
	if (mode == 2) Light_Animation(slideValue);		// Light Mode
	else if (mode == 3) Music_Animation(slideValue);	// Music Mode
	
	startShow = TRUE;
}

void ADC_MainRoutine(void) {
	u8 i;
	u32 total = 0;
	if (gADC_CycleEndOfConversion) {
		if (adcIndex < TEST_LENGTH_SAMPLES) {
			InputSignal[adcIndex] = gADC_Result;
			InputSignal[adcIndex + 1] = 0;
			adcIndex += 2;
		} else {
			if (refFlag == TRUE) {
				for (i = 2; i < TEST_LENGTH_SAMPLES; i += 2) total += InputSignal[i];
				ref_bit = total / (TEST_LENGTH_SAMPLES / 2 - 1);
				refFlag = FALSE;
				
//				for (i = 0; i < TEST_LENGTH_SAMPLES; i += 1) printf("%d ", InputSignal[i]);
//				printf("\r\n");
//				printf("ref_bit = %d\r\n", ref_bit);
			} else {
				sampleFlag = TRUE;
			}
		}
		gADC_CycleEndOfConversion = FALSE;
	}
}

void DefineRefBit(void) {
	delay(1000000);
	ref_bit = 0;
	adcIndex = 0;
	refFlag = TRUE;
}

void RUN_FFT(void) {
	/* Process the data through the CFFT/CIFFT module */
	arm_cfft_f32(&arm_cfft_sR_f32_len64, fftData, ifftFlag, doBitReverse);

	/* Process the data through the Complex Magnitude Module for
	calculating the magnitude at each bin */
	arm_cmplx_mag_f32(fftData, OutputSignal, fftSize);
}

void speakerEnable(bool enable) {
	// Music Mute Pin
	// Enable  -> low
	// Disable -> high
	GPIO_WriteOutBits(HT_GPIOC, GPIO_PIN_15, (FlagStatus)!enable);
}

void bluetoothEnable(bool enable) {
	// Bluetooth Module Power Pin
	// Enable -> Bluetooth Module OFF
	// Disable -> Bluetooth Module ON
	GPIO_WriteOutBits(HT_GPIOC, GPIO_PIN_8, (FlagStatus)!enable);
}

void calculateGradient(u8 i1, u8 i2, u8 Fst_color[], u8 Snd_color[]) {
	u8 color_level, color_rgb, value = 0;
	
	for (color_level = i1; color_level <= i2; color_level++) {
		for (color_rgb = 0; color_rgb < 3; color_rgb++) {
			
			if (Snd_color[color_rgb] > Fst_color[color_rgb]) {
				if (i2 > 3) value = Fst_color[color_rgb] + (Snd_color[color_rgb] - Fst_color[color_rgb]) / 4 * (color_level - 4);
				else value = Fst_color[color_rgb] + (Snd_color[color_rgb] - Fst_color[color_rgb]) / 4 * color_level;
			} else if (Snd_color[color_rgb] < Fst_color[color_rgb]) {
				if (i2 > 3) value = Fst_color[color_rgb] - (Fst_color[color_rgb] - Snd_color[color_rgb]) / 4 * (color_level - 4);
				else value = Fst_color[color_rgb] - (Fst_color[color_rgb] - Snd_color[color_rgb]) / 4 * color_level;
			} else {
				value = Fst_color[color_rgb];
			}
			
			musicColor[color_level][color_rgb] = value;
		}
	}
}

void generateMusicColor(u8 level) {
//	u8 color_level;
	u8 color_rgb;
	
	if (initFlag != TRUE) {
		for (color_rgb = 0; color_rgb < 3; color_rgb++) {
			if (level == 0x03) {
				musicColor[0][color_rgb] = low_color[color_rgb];
			} else if (level == 0x02) {
				musicColor[4][color_rgb] = mid_color[color_rgb];
				musicColor[5][color_rgb] = mid_color[color_rgb];
			} else if (level == 0x01) {
				musicColor[9][color_rgb] = high_color[color_rgb];
			}
		}
	}
	if (level == 0x03 || level == 0x02) calculateGradient(1, 3, musicColor[0], musicColor[4]);
	if (level == 0x01 || level == 0x02) calculateGradient(6, 8, musicColor[4], musicColor[9]);
	
//	for (color_level = 0; color_level < WS_LEV_SIZE; color_level++) {
//		printf("musicColor[%2d]: ", color_level);
//		for (color_rgb = 0; color_rgb < 3; color_rgb++) {
//			printf("%3d ", musicColor[color_level][color_rgb]);
//		}
//		printf("\r\n");
//	}
}

void DataToESP(u8 category, u8 type) {
	u8 i;
	u16 checksum = 0;
	
	data_to_esp[0] = 0x95; // Start Bit
	data_to_esp[1] = category;
	data_to_esp[2] = type;
	if (category == 0x01) {
		for (i = 3; i <= 6; i++) data_to_esp[i] = 0x00;
	} else if (category == 0x02) {
		if (type == 0x01) {
//			data_to_esp[3] = slideValue;
			for (i = 4; i <= 6; i++) data_to_esp[i] = Color[i - 4];
		} else if (type == 0x03) {
//			data_to_esp[3] = zoomValue;
			for (i = 4; i <= 6; i++) data_to_esp[i] = 0x00;
		} else if (type == 0x05) {
			data_to_esp[3] = offsetValue;
			data_to_esp[4] = zoomValue;
			
//			if (slideValue <= 0) slideValue = 0;
//			else if (slideValue >= 100) slideValue = 100;
			data_to_esp[5] = (slideValue / 255.0) * 100;
			
			data_to_esp[6] = 0x00;
		}
	} else if (category == 0x03) {
		if (type == 0x05) {
			data_to_esp[3] = offsetValue;
			data_to_esp[4] = zoomValue;
			
//			if (slideValue <= 0) slideValue = 0;
//			else if (slideValue >= 100) slideValue = 100;
			data_to_esp[5] = (slideValue / 255.0) * 100;
			
			data_to_esp[6] = 0x00;
		}
	} else if (category == 0x05) {
		if (type == 0x06) {
			data_to_esp[3] = (u8)BT_CON;
			data_to_esp[4] = (u8)BT_isPlay;
			data_to_esp[5] = (u8)BT_VOL;
			
			data_to_esp[6] = 0x00;
		}
	}
	for (i = 1; i <= 6; i++) checksum += data_to_esp[i];
	data_to_esp[7] = checksum >> 8;
	data_to_esp[8] = checksum & 0x00FF;
	data_to_esp[9] = 0x87; // End Bit
	
//	printf("Sending: [");
//	for (i = 0; i < 10; i++) printf("%02X ", data_to_esp[i]);
//	printf("]\r\n");
	
	send_index = 0;
	toEspFlag = TRUE;
	USART_IntConfig(HT_USART0, USART_INT_TXDE, ENABLE);
}

void DataFromESP(u8 command[]) {
	u8 i;
	static u8 data[4];
	
	for (i = 0; i <= 3; i++) data[i] = command[i + 3];
	
	if (command[1] == 0x01) {			// Switching Mode
//		printf("Switching Mode: \r\n");
		SwitchingMode(command[2]);
	} else if (command[1] == 0x02) {	// Lighting Mode
//		printf("Lighting Mode: \r\n");
		LightingMode(command[2], data);
	} else if (command[1] == 0x03) {	// Music Mode
//		printf("Music Mode: \r\n");
		MusicMode(command[2], data);
	} else if (command[1] == 0x04) {	// Setting
//		printf("Setting: \r\n");
		Setting(command[2], data);
	} else if (command[1] == 0x05) {	// BT_Module
//		printf("BT_Module: \r\n");
		BT_Module(command[2], data);
	}
}

void DataFromBT(u8 command[]) {
	u8 i;
	u8 data[5] = "-----", index = 0;
	u16 volume[2];
	
	for (i = 0; i < 9; i += 1) {
		if (command[i] != '\r' && command[i] != '\n' && command[i] != ' ') {
			data[index] = command[i];
//			printf("%d: '%c'\r\n", index, data[index]);
			index += 1;
		}
	}
//	printf("\n");
	
	if (data[0] == 'E' && data[1] == 'R' && data[2] == 'R') {
//		printf("The command is error.\r\n");
	} else if (data[0] == 'V' && data[1] == 'O' && data[2] == 'L') {
		for (i = 0; i < 2; i += 1) volume[i] = data[i + 3];
		if (volume[1] != '-') BT_VOL = (volume[0] - '0') * 10 + (volume[1] - '0');
		else BT_VOL = (volume[0] - '0');
		printf("Set volume: %d\r\n", BT_VOL);
	} else if (data[0] == 'M' && (data[1] == 'A' || data[1] == 'P')) {
//		printf("Music isn't playing\r\n");
		BT_isPlay = FALSE;
	} else if (data[0] == 'M' && (data[1] == 'B' || data[1] == 'R')) {
//		printf("Music is playing\r\n");
		BT_isPlay = TRUE;
	} else if (data[0] == 'I' && data[1] == 'V') {
//		printf("BT Connected");
		BT_CON = TRUE;
	} else if (data[0] == 'I' && (data[1] == 'A' || data[1] == 'I')) {
//		printf("BT Disconnected");
		BT_CON = FALSE;
	}
	if (toEspFlag == FALSE) DataToESP(0x05, 0x06);
	
//	for (i = 0; i < 9; i += 1) BLE_RX[i] = 0x20;
}

void SwitchingMode(u8 status) {	
	mode = status;
	
	if (mode == 0) {
		On_Effect(20, TRUE);
		speakerEnable(FALSE);
		bluetoothEnable(FALSE);
		
		DefineRefBit();
		
		BT_CON = FALSE;
		BT_isPlay = FALSE;
		if (toEspFlag == FALSE) DataToESP(0x05, 0x06);
		
//		printf("Turn off\r\n");
	} else if (mode == 1) {
		mode = curr_mode;
		
		On_Effect(20, FALSE);
		
		if (toEspFlag != TRUE) DataToESP(0x01, mode); // Sending Status Message
		adcIndex = 0;
		speakerEnable(TRUE);
		bluetoothEnable(TRUE);
		
		wsUpdateMag();
		
//		printf("Turn on\r\n");
	} else if (mode == 2) {
		mode = 3;
		Switch_Effect(5, slideValue);
		mode = 2;
		curr_mode = mode;
		adcIndex = 0;
		
		wsUpdateMag();
	} else if (mode == 3) {
		mode = 2;
		Switch_Effect(5, slideValue);
		mode = 3;
		curr_mode = mode;
	}
}

void LightingMode(u8 type, u8 data[]) {
	u8 i;
	
	L_Type = type;
	if (L_Type == 0x01) {
		Brightness = data[0];
//		slideValue = Brightness;
		
		for (i = 0; i < 3; i++) Color[i] = data[i + 1];
		
	} else if (L_Type == 0x02) {
//		setLedOffset(data[0]);
	} else if (L_Type == 0x03) {
//		zoomValue = data[0];
	} else if (L_Type == 0x05) {
		if (data[0] != offsetValue) {
			offsetValue = data[0];
			setLedOffset(offsetValue);
		}
		if (data[1] != zoomValue) zoomValue = data[1];
		if (data[2] != slideValue) slideValue = (data[2] / 100.0) * 255;
	}
	wsUpdateMag();
}

void MusicMode(u8 type, u8 data[]) {
	u8 i;
	
	M_Type = type;
	mLevel = data[0];
	if (M_Type == 0x01) {
		for (i = 0; i < 3; i++) {
			if (mLevel == 0x03) {
				musicColor[0][i] = data[i + 1];
//				high_color[i] = data[i + 1];
			} else if (mLevel == 0x02) {
				musicColor[4][i] = data[i + 1];
				musicColor[5][i] = data[i + 1];
//				mid_color[i] = data[i + 1];
			} else if (mLevel == 0x01) {
				musicColor[9][i] = data[i + 1];
//				low_color[i] = data[i + 1];
			}
		}
		
		generateMusicColor(mLevel);
	} else if (M_Type == 0x02) {
//		setLedOffset(data[0]);
	} else if (M_Type == 0x04) {
		
	} else if (M_Type == 0x05) {
		if (data[0] != offsetValue) {
			offsetValue = data[0];
			setLedOffset(offsetValue);
		}
		if (data[1] != zoomValue) zoomValue = data[1];
		if (data[2] != slideValue) slideValue = (data[2] / 100.0) * 255;
	}
}

void Setting(u8 syncType, u8 data[]) {
	u8 i;
	u32 time = 0;
	
	sync = syncType;
					
	if (sync == 0xFF) {
		for (i = 0; i <= 3; i++) time += (data[3 - i] << (8 * i));
		Timestamp = time;
	}
	if (sync == 0xFE) {
		fft_mag = data[0];
	}
	if (sync == 0x01 || sync == 0x00) {
		if (sync == 0x01) {
			auto_OnOff[0] = 1;
			AutoOn.tm_hour = data[1];
			AutoOn.tm_min = data[2];
			AutoOn.tm_sec = data[3];
			
//			printf("Set Auto On => Days: %0X, %02d:%02d:%02d\r\n",
//				data[0], 
//				AutoOn.tm_hour, AutoOn.tm_min, AutoOn.tm_sec
//			);
		} else if (sync == 0x00) {
			auto_OnOff[1] = 1;
			AutoOff.tm_hour = data[1];
			AutoOff.tm_min = data[2];
			AutoOff.tm_sec = data[3];
			
//			printf("Set Auto Off => Days: %0X, %02d:%02d:%02d\r\n",
//				data[0], 
//				AutoOff.tm_hour, AutoOff.tm_min, AutoOff.tm_sec
//			);
		}
//		printf("Sun Sat Fri Thu Wed Tue Mon\r\n");
		for (i = 0; i < 7; i++) {
			weekdayEN[6 - i] = (data[0] >> i) & 0x01;
//			printf("%3d ", weekdayEN[6 - i]);
		}
//		printf("\r\n");
	}
//	printf("Timestamp: %d sec\r\n", Timestamp);
}

void BT_Module(u8 type, u8 data[]) {
	u8 i;
	
	if (type == 0x06) {
		
	} else if (type == 0x07) {
		BLE_TX[3] = data[0];
		BLE_TX[4] = data[1];
		for (i = 0; i < 7; i += 1) printf("%c", BLE_TX[i]);
		USART_IntConfig(HT_UART0, USART_INT_TXDE, ENABLE);
	}
}

void On_Effect(u8 wait, bool reverse) {
	const u8 grad_size = 9;
	u8 gradient[WS_LEV_SIZE * 2 + grad_size] = {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		40, 25, 20, 17, 10, 8, 4, 2, 1,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	};
	u8 i, j;
	u8 start_index = 0;
	u32 w = 0;
	if (!reverse) {
		while (start_index < (WS_LEV_SIZE + grad_size)) {
			for (i = 0; i < WS_FRQ_SIZE; i++) {
				for (j = 0; j < WS_LEV_SIZE; j++) {
					wsSetColor(WS_LED[i][j], ws_white, gradient[start_index + j]);
				}
			}
			start_index += 1;
			w = wait * 10000;
			wsShow();
			while(w--);
		}
	} else {
		start_index = 0;
		while (start_index < (WS_LEV_SIZE + grad_size + 1)) {
			for (i = 0; i < WS_FRQ_SIZE; i++) {
				for (j = 0; j < WS_LEV_SIZE; j++) {
					wsSetColor(WS_LED[i][(WS_LEV_SIZE - 1) - j], ws_white, gradient[start_index + j]);
				}
			}
			start_index += 1;
			w = wait * 10000;
			wsShow();
			while(w--);
		}
	}
}

void Switch_Effect(u8 wait, u8 max_brightness) {
	u8 effect, mag;
	u32 w;
	
	effect = 0;
	mag = max_brightness / 16;
	while (effect < max_brightness) {
		if (mode == 2) Light_Animation(max_brightness - effect);
		else Music_Animation(max_brightness - effect);
		effect += mag;
		if (effect >= 254) effect = 254;
		w = (wait * 1000);
		wsShow();
		while(w--);
	}
	effect = 0;
//	if (mode == 2) max_brightness = Music.slide;
//	else max_brightness = Light.slide;
	while (effect < max_brightness) {
		if (mode == 3) Light_Animation(effect);
		else Music_Animation(effect);
		effect += mag;
		if (effect >= 254) effect = 254;
		w = (wait * 1000);
		wsShow();
		while(w--);
	}
}

void Light_Animation(u8 brightness) {
	u8 i, j;
	u8 *rows = showRows(zoomValue);
	
	for (i = 0; i < WS_FRQ_SIZE; i += 1) {
		for (j = 0; j < WS_LEV_SIZE; j += 1) {
			if (rows[i] == 1) wsSetColor(WS_LED[i][j], Color, brightness);
			else wsSetColor(WS_LED[i][j], ws_clean, 0);
		}
	}
}

void Music_Animation(u8 brightness) {
	const u8 o = 2;
	u8 i, j;
	u8 level;
	u8 *rows = showRows(zoomValue);
	u8 scale = WS_FRQ_SIZE / zoomValue;
	u8 exactOutput[WS_FRQ_SIZE] = {0}, n = 0;
	
//		printf("\r");
	for (i = 0; i < WS_FRQ_SIZE; i += 1) {
		if (i % scale == 0) {
			if (OutputSignal[i] < (o + 3)) level = 1;
			else if(OutputSignal[i] < (o + 5)) level = 2;
			else if(OutputSignal[i] < (o + 8)) level = 3;
			else if(OutputSignal[i] < (o + 11)) level = 4;
			else if(OutputSignal[i] < (o + 14)) level = 5;
			else if(OutputSignal[i] < 17) level = 6;
			else if(OutputSignal[i] < 20) level = 7;
			else if(OutputSignal[i] < 23) level = 8;
			else if(OutputSignal[i] < 26) level = 9;
			else level = 10;
			
			exactOutput[i / scale] = level;
//			printf("%-3.0f ", OutputSignal[i]);
		}
	}
		// set fft leds
	for (i = 0; i < WS_FRQ_SIZE; i += 1) {
		for (j = 0; j < WS_LEV_SIZE; j += 1) {
			//WS_LED[index][level]
			if (rows[i] == 1) {
				if (j < exactOutput[n]) wsSetColor(WS_LED[i][j], musicColor[j], brightness);
				else wsSetColor(WS_LED[i][j], musicColor[j], 0);
			} else {
				wsSetColor(WS_LED[i][j], musicColor[j], 0);
			}
			if((j == wsLevel[i] - 1) && rows[i] == 1) wsSetColor(WS_LED[i][j], musicColor[j], brightness);
		}
		// update drop down timer
		if(exactOutput[n] > wsLevel[i]) {
			wsLevel[i] = exactOutput[n];
			wsLevelTM[i] = 65;
		}
		if(wsLevelTM[i] == 0) {
			if(wsLevel[i] >= exactOutput[n]) wsLevel[i]--;
			wsLevelTM[i] = 10;
		}
		
		if (rows[i] == 1) n += 1;
	}
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
