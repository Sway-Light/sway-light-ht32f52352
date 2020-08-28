/*********************************************************************************************************//**
 * @file    IP/Example/ht32f5xxxx_01_it.c
 * @version $Rev:: 4398         $
 * @date    $Date:: 2019-11-14 #$
 * @brief   This file provides all interrupt service routine.
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

/* Includes ------------------------------------------------------------------------------------------------*/
#include "ht32.h"

#include "ws2812.h"
#include "DFPlayer.h"
#include "74HC4067.h"

/** @addtogroup Project_Template Project Template
  * @{
  */

/** @addtogroup IP_Examples IP
  * @{
  */

/** @addtogroup Example
  * @{
  */

extern u16 adcIndex;

/* Global functions ----------------------------------------------------------------------------------------*/
/*********************************************************************************************************//**
 * @brief   This function handles NMI exception.
 * @retval  None
 ************************************************************************************************************/
void NMI_Handler(void) {
}

/*********************************************************************************************************//**
 * @brief   This function handles Hard Fault exception.
 * @retval  None
 ************************************************************************************************************/
void HardFault_Handler(void) {
	#if 1

	static vu32 gIsContinue = 0;
	/*--------------------------------------------------------------------------------------------------------*/
	/* For development FW, MCU run into the while loop when the hardfault occurred.                           */
	/* 1. Stack Checking                                                                                      */
	/*    When a hard fault exception occurs, MCU push following register into the stack (main or process     */
	/*    stack). Confirm R13(SP) value in the Register Window and typing it to the Memory Windows, you can   */
	/*    check following register, especially the PC value (indicate the last instruction before hard fault).*/
	/*    SP + 0x00    0x04    0x08    0x0C    0x10    0x14    0x18    0x1C                                   */
	/*           R0      R1      R2      R3     R12      LR      PC    xPSR                                   */
	while (gIsContinue == 0) {
	}
	/* 2. Step Out to Find the Clue                                                                           */
	/*    Change the variable "gIsContinue" to any other value than zero in a Local or Watch Window, then     */
	/*    step out the HardFault_Handler to reach the first instruction after the instruction which caused    */
	/*    the hard fault.                                                                                     */
	/*--------------------------------------------------------------------------------------------------------*/

	#else

	/*--------------------------------------------------------------------------------------------------------*/
	/* For production FW, you shall consider to reboot the system when hardfault occurred.                    */
	/*--------------------------------------------------------------------------------------------------------*/
	NVIC_SystemReset();

	#endif
}

/*********************************************************************************************************//**
 * @brief   This function handles SVCall exception.
 * @retval  None
 ************************************************************************************************************/
void SVC_Handler(void) {
}

/*********************************************************************************************************//**
 * @brief   This function handles PendSVC exception.
 * @retval  None
 ************************************************************************************************************/
void PendSV_Handler(void) {
}

/*********************************************************************************************************//**
 * @brief   This function handles SysTick Handler.
 * @retval  None
 ************************************************************************************************************/
void SysTick_Handler(void) {
}

/*********************************************************************************************************//**
 * @brief   This function handles LVD & BOD interrupt.
 * @retval  None
 ************************************************************************************************************/
//void LVD_BOD_IRQHandler(void) {
//}

/*********************************************************************************************************//**
 * @brief   This function handles RTC interrupt.
 * @retval  None
 ************************************************************************************************************/
//void RTC_IRQHandler(void) {
//}

/*********************************************************************************************************//**
 * @brief   This function handles Flash interrupt.
 * @retval  None
 ************************************************************************************************************/
//void FLASH_IRQHandler(void) {
//}

/*********************************************************************************************************//**
 * @brief   This function handles Wake-up interrupt.
 * @retval  None
 ************************************************************************************************************/
//void EVWUP_IRQHandler(void) {
//}

/*********************************************************************************************************//**
 * @brief   This function handles EXTI interrupt.
 * @retval  None
 ************************************************************************************************************/
//void EXTI0_1_IRQHandler(void) {
//}

/*********************************************************************************************************//**
 * @brief   This function handles EXTI interrupt.
 * @retval  None
 ************************************************************************************************************/
//void EXTI2_3_IRQHandler(void) {
//}

/*********************************************************************************************************//**
 * @brief   This function handles EXTI interrupt.
 * @retval  None
 ************************************************************************************************************/
//void EXTI4_15_IRQHandler(void) {
//}

/*********************************************************************************************************//**
 * @brief   This function handles CMP interrupt.
 * @retval  None
 ************************************************************************************************************/
//void COMP_IRQHandler(void) {
//}

/*********************************************************************************************************//**
 * @brief   This function handles ADC interrupt.
 * @retval  None
 ************************************************************************************************************/
extern vu32 gADC_CycleEndOfConversion;
extern u8 slideValue;

void ADC_IRQHandler(void) {
	extern u32 gADC_Result;
	if (ADC_GetIntStatus(HT_ADC0, ADC_INT_SINGLE_EOC) == SET) {
		ADC_ClearIntPendingBit(HT_ADC0, ADC_FLAG_SINGLE_EOC);
	}

	if (ADC_GetIntStatus(HT_ADC0, ADC_INT_CYCLE_EOC) == SET) {
		ADC_ClearIntPendingBit(HT_ADC0, ADC_FLAG_CYCLE_EOC);
		gADC_CycleEndOfConversion = TRUE;
		gADC_Result = (HT_ADC0->DR[0] & 0x0FFF) - 2048;
		gADC_Result *= 32;
	}
}

/*********************************************************************************************************//**
 * @brief   This function handles MCTM interrupt.
 * @retval  None
 ************************************************************************************************************/
//void MCTM0_IRQHandler(void) {
//}

/*********************************************************************************************************//**
 * @brief   This function handles GPTM interrupt.
 * @retval  None
 ************************************************************************************************************/
void GPTM0_IRQHandler(void) {
	if (TM_GetIntStatus(HT_GPTM0, TM_INT_CH3CC) == SET) {
		TM_ClearIntPendingBit(HT_GPTM0, TM_INT_CH3CC);
	}
}

/*********************************************************************************************************//**
 * @brief   This function handles GPTM interrupt.
 * @retval  None
 ************************************************************************************************************/
extern u8 status;
extern const u8 zoom, slide, press, none;

extern bool TK_CHECK, TK_1SEC;
extern u8 TK_L, TK_R;
extern u16 TK_COUNT;

// Button
typedef enum {
	btRelease,
	btPressed
}BtStatus;
extern BtStatus mBtStatus;

typedef enum {
	btNone,
	btClick,
	btLongClick
}BtAction;
extern BtAction mBtAction;
static u8 btPrev = btRelease;
extern u32 btTM;

//u32 time = 0;
//bool startCount = FALSE;
bool extern touchKeyDelayFlag;

void GPTM1_IRQHandler(void) {
	static u8 touchKeyDelayTM = 0;
	extern u8 mode;
	
	//FFT & LED
	extern bool startShow, sampleFlag, initFlag;
	extern u8 wsLevelTM[16];
	static bool btFinish = TRUE;
	
	u8 j = 0;
	
	TM_ClearFlag(HT_GPTM1, TM_FLAG_UEV);
	
	// LED
	if (startShow == TRUE || initFlag == FALSE) {
		wsShow();
		startShow = FALSE;
		adcIndex = 0;
		sampleFlag = FALSE;
	}
	
	for(j = 0; j < 16; j++) {
		if(wsLevelTM[j] > 0) wsLevelTM[j]--;
	}
	
	// Touch Key
	touchKeyDelayTM++;
	if (touchKeyDelayTM > 20){
		touchKeyDelayFlag = TRUE;
		touchKeyDelayTM = 0;
	}
	if (TK_CHECK) {
		if (TK_R - TK_L > 2) {
			status = zoom;
			TK_1SEC = FALSE;
		} else if (TK_R - TK_L <= 2) {
			status = slide;
		}
		
	}
	if (TK_1SEC && status != none) {
		if (TK_COUNT >= (2 * 500)) {
			TK_1SEC = FALSE;
		} else {
			TK_COUNT += 1;
		}
	}
	
	// Button
	if (btFinish) {
		if (!GPIO_ReadInBit(HT_GPIOB, GPIO_PIN_1)) {
			// press
			mBtStatus = btPressed;
		} else {
			// release
			mBtStatus = btRelease;
		}
		if (btPrev != mBtStatus) {
			if(mBtStatus == btRelease) {
				mBtAction = btClick;
				btTM = 0;
				printf("released\r\n");
//				btPrev = mBtStatus;
			} else if (mBtStatus == btPressed){
				printf("pressed\r\n");
			}
			btPrev = mBtStatus;
		}
		if (mBtStatus == btPressed) {
			btTM++;
			if (btTM >= 500) {
				// long click
				mBtAction = btLongClick;
				btTM = 0;
				btFinish = FALSE;
			}
		}
	} else {
		if (GPIO_ReadInBit(HT_GPIOB, GPIO_PIN_1) && mBtAction == btNone) {
			printf("long click release\r\n");
			mBtStatus = btRelease;
			btPrev = mBtStatus;
			btFinish = TRUE;
		}
	}
}

/*********************************************************************************************************//**
 * @brief   This function handles SCTM interrupt.
 * @retval  None
 ************************************************************************************************************/
//void SCTM0_IRQHandler(void) {
//}

/*********************************************************************************************************//**
 * @brief   This function handles SCTM interrupt.
 * @retval  None
 ************************************************************************************************************/
//void SCTM1_IRQHandler(void) {
//}

/*********************************************************************************************************//**
 * @brief   This function handles SCTM interrupt.
 * @retval  None
 ************************************************************************************************************/
//void SCTM2_IRQHandler(void) {
//}

/*********************************************************************************************************//**
 * @brief   This function handles SCTM interrupt.
 * @retval  None
 ************************************************************************************************************/
//void SCTM3_IRQHandler(void) {
//}

/*********************************************************************************************************//**
 * @brief   This function handles SLED0 interrupt.
 * @retval  None
 ************************************************************************************************************/
//void SLED0_IRQHandler(void) {
//}

/*********************************************************************************************************//**
 * @brief   This function handles SLED1 interrupt.
 * @retval  None
 ************************************************************************************************************/
//void SLED1_IRQHandler(void) {
//}

/*********************************************************************************************************//**
 * @brief   This function handles BFTM interrupt.
 * @retval  None
 ************************************************************************************************************/
//void BFTM0_IRQHandler(void) {
//}

/*********************************************************************************************************//**
 * @brief   This function handles BFTM interrupt.
 * @retval  None
 ************************************************************************************************************/
//void BFTM1_IRQHandler(void) {
//}

/*********************************************************************************************************//**
 * @brief   This function handles I2C interrupt.
 * @retval  None
 ************************************************************************************************************/
//void I2C0_IRQHandler(void)
//{
//}

/*********************************************************************************************************//**
 * @brief   This function handles I2C interrupt.
 * @retval  None
 ************************************************************************************************************/
//void I2C1_IRQHandler(void)
//{
//}

/*********************************************************************************************************//**
 * @brief   This function handles I2C interrupt.
 * @retval  None
 ************************************************************************************************************/
//void I2C2_IRQHandler(void)
//{
//}

/*********************************************************************************************************//**
 * @brief   This function handles SPI interrupt.
 * @retval  None
 ************************************************************************************************************/
//void SPI0_IRQHandler(void)
//{
//}

/*********************************************************************************************************//**
 * @brief   This function handles SPI interrupt.
 * @retval  None
 ************************************************************************************************************/
//void SPI1_IRQHandler(void)
//{
//}

/*********************************************************************************************************//**
 * @brief   This function handles USART interrupt.
 * @retval  None
 ************************************************************************************************************/
//void USART0_IRQHandler(void)
//{
//}

/*********************************************************************************************************//**
 * @brief   This function handles USART interrupt.
 * @retval  None
 ************************************************************************************************************/
//void USART1_IRQHandler(void)
//{
//}
void popCmdQueue(u8 *queue, u8 *queueSize) {
	u8 i;
	for(i = 0; i < *queueSize - 1; i++) {
		queue[i] = queue[i + 1];
	}
	(*queueSize)--;
//	printf("queueSize: %d\r\n", *queueSize);
//	printf("pop\r\n");
}
/*********************************************************************************************************//**
 * @brief   This function handles UART interrupt.
 * @retval  None
 ************************************************************************************************************/
extern u8 mp3CmdQueue[QUEUE_MAX_SIZE];
extern u8 queueSize;
void UART0_IRQHandler(void) {
	if (USART_GetIntStatus(HT_UART0, USART_INT_TXDE) && USART_GetFlagStatus(HT_UART0, USART_FLAG_TXDE)) {
//		printf("uart\r\n");
		if(queueSize > 0) {
//			printf("queue size: %d\r\n", queueSize);
			USART_SendData(HT_UART0, mp3CmdQueue[0]);
//			printf("%x " ,mp3CmdQueue[0]);
			popCmdQueue(mp3CmdQueue, &queueSize);
		}else {
//			printf("\r\n");
			USART_IntConfig(HT_UART0, USART_INT_TXDE, DISABLE); // After Transmitting Data is completed, disable the interrupt of UART0.
		}
	}
}

/*********************************************************************************************************//**
 * @brief   This function handles UART interrupt.
 * @retval  None
 ************************************************************************************************************/
const u8 data_length = 9;
extern u8 recieve_index, send_index;
extern u8 data_from_esp[data_length], data_to_esp[data_length];
u8 i;
extern bool espFlag;
void USART0_IRQHandler(void) { 
	if (USART_GetIntStatus(HT_USART0, USART_INT_TXDE) && USART_GetFlagStatus(HT_USART0, USART_FLAG_TXDE)) {
		
	}
	
	if (USART_GetFlagStatus(HT_USART0, USART_FLAG_RXDR)) {
		data_from_esp[recieve_index] = USART_ReceiveData(HT_USART0);
		if (recieve_index >= data_length - 1) {
			recieve_index = 0;
			espFlag = TRUE;
		} else {
			recieve_index += 1;
		}
	}
	
}

/*********************************************************************************************************//**
 * @brief   This function handles UART interrupt.
 * @retval  None
 ************************************************************************************************************/
//void UART2_IRQHandler(void)
//{
//}

/*********************************************************************************************************//**
 * @brief   This function handles UART interrupt.
 * @retval  None
 ************************************************************************************************************/
//void UART3_IRQHandler(void)
//{
//}

/*********************************************************************************************************//**
 * @brief   This function handles UART interrupt.
 * @retval  None
 ************************************************************************************************************/
//void UART0_UART2_IRQHandler(void)
//{
//  #if (RETARGET_PORT == RETARGET_UART0) || (RETARGET_PORT == RETARGET_UART2)
//    #if (RETARGET_INT_MODE == 1)
//    RETARGET_UART_IRQHandler();
//    #endif
//  #endif
//}

/*********************************************************************************************************//**
 * @brief   This function handles UART interrupt.
 * @retval  None
 ************************************************************************************************************/
//void UART1_UART3_IRQHandler(void)
//{
//  #if (RETARGET_PORT == RETARGET_UART1) || (RETARGET_PORT == RETARGET_UART3)
//    #if (RETARGET_INT_MODE == 1)
//    RETARGET_UART_IRQHandler();
//    #endif
//  #endif
//}

/*********************************************************************************************************//**
 * @brief   This function handles SCI interrupt.
 * @retval  None
 ************************************************************************************************************/
//void SCI_IRQHandler(void)
//{
//}

/*********************************************************************************************************//**
 * @brief   This function handles I2S interrupt.
 * @retval  None
 ************************************************************************************************************/
//void I2S_IRQHandler(void)
//{
//}

/*********************************************************************************************************//**
 * @brief   This function handles USB interrupt.
 * @retval  None
 ************************************************************************************************************/
//void USB_IRQHandler(void)
//{
//}

/*********************************************************************************************************//**
 * @brief   This function handles PDMA interrupt.
 * @retval  None
 ************************************************************************************************************/
//void PDMA_CH0_1_IRQHandler(void)
//{
//}

/*********************************************************************************************************//**
 * @brief   This function handles PDMA interrupt.
 * @retval  None
 ************************************************************************************************************/
//void PDMA_CH2_5_IRQHandler(void)
//{
//}


/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */
