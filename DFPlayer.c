#include "ht32.h"
#include "DFPlayer.h"

//     7E FF 06 0F 00 01 01 xx xx EF
// No. 0  1  2  3  4  5  6  7  8  9
// 0		->	7E is start code
// 1		->	FF is version
// 2		->	06 is length
// 3		->	0F is command
// 4		->	00 is no receive
// 5 ~ 6	->	01 01 is argument
// 7 ~ 8	->	checksum = 0 - (FF + 06 + 0F + 00 + 01 + 01)
// 9		->	EF is end code

u8 mp3_i;

USART_InitTypeDef USART_InitStructure;

u16 mp3GetChecksum(u8* buf){
	u16 sum = 0;
	for(mp3_i = 1; mp3_i < 7; mp3_i++){
		sum -= buf[mp3_i];
	}
	return sum;
}

void mp3FillChecksum(void) {
	u16 checksum = mp3GetChecksum(send_buf);
	send_buf[7] = (u8)(checksum >> 8);
	send_buf[8] = (u8)checksum;
}

void mp3SendCmd(u8 cmd, u16 high_arg, u16 low_arg) {
//	u8 Tx_Data;
	
	send_buf[3] = cmd;
	send_buf[5] = high_arg;
	send_buf[6] = low_arg;
	mp3FillChecksum();
	
	USART_IntConfig(HT_UART0, USART_INT_TXDE, ENABLE); // Enable the TX inturrupt when data is ready to be transmitted.
	
//	for(mp3_i = 0; mp3_i < 10; mp3_i++) {
//		Tx_Data = send_buf[mp3_i];
//		USART_SendData(HT_UART0, Tx_Data);
//		while(USART_GetFlagStatus(HT_UART0, USART_FLAG_TXC) == RESET);
//	}
	
}

void mp3ReceiveCmd(void) {
//	static u8 RX_index = 0;
//	if (USART_GetFlagStatus(HT_UART0, USART_FLAG_RXDR)) {
//		return_buf[RX_index] = USART_ReceiveData(HT_UART0);
//		if (RX_index >= 9) RX_index = 0;
//		else RX_index += 1;
//	}
	
	switch (return_buf[3]) {
		case 0x3D:
			printf("\r\nmp3Debug: mp3 finished. \r\n");
			break;
		case 0x3A:
			printf("\r\nmp3Debug: push in TF card. \r\n");
			break;
		case 0x3B:
			printf("\r\nmp3Debug: pull out TF card. \r\n");
			break;
		case 0x40:
			if 		(return_buf[6] == 0x00) printf("\r\nmp3Debug: ERROR, Module is busy. \r\n");
			else if (return_buf[6] == 0x01) printf("\r\nmp3Debug: ERROR, A frame data are not all received. \r\n");
			else if (return_buf[6] == 0x02) printf("\r\nmp3Debug: ERROR, Verification error. \r\n");
			break;
		default:
			printf("\rOther: ");
			for (mp3_i = 0; mp3_i < 10; mp3_i++) printf("%02X ", return_buf[mp3_i]);
			break;
	}
}

void mp3ResetModule(void) {
	mp3SendCmd(0x0C, 0, 0);
	
	#if(mp3Debug == 1)
	printf("mp3Debug: %-14s ", "mp3ResetModule");
	for (mp3_i = 0; mp3_i < 10; mp3_i++) printf("%02X ", send_buf[mp3_i]);
	printf("\r\n");
	#endif
}

//0x09 set device 0/1/2/3/4/ U/SD/AUX/SLEEP/FLASH
void mp3SetDevice(u8 device) {
	mp3SendCmd(0x09, 0, device);
	
	#if(mp3Debug == 1)
	printf("mp3Debug: %-14s ", "mp3SetDevice");
	for (mp3_i = 0; mp3_i < 10; mp3_i++) printf("%02X ", send_buf[mp3_i]);
	printf("\r\n");
	#endif
}

void mp3Play(u8 musicNum) {
	
	mp3SendCmd(0x0F, 0x01, musicNum);
	
	#if(mp3Debug == 1)
	printf("mp3Debug: %-14s ", "mp3Play");
	for (mp3_i = 0; mp3_i < 10; mp3_i++) printf("%02X ", send_buf[mp3_i]);
	printf("\r\n");
	#endif
}

void mp3Continue(void) {
	mp3SendCmd(0x0D, 0, 0);
	
	#if(mp3Debug == 1)
	printf("mp3Debug: %-14s ", "mp3Continue");
	for (mp3_i = 0; mp3_i < 10; mp3_i++) printf("%02X ", send_buf[mp3_i]);
	printf("\r\n");
	#endif
}

void mp3Next(void) {
	mp3SendCmd(0x01, 0, 0);
	
	#if(mp3Debug == 1)
	printf("mp3Debug: %-14s ", "mp3Next");
	for (mp3_i = 0; mp3_i < 10; mp3_i++) printf("%02X ", send_buf[mp3_i]);
	printf("\r\n");
	#endif
}

void mp3Prev(void) {
	mp3SendCmd(0x02, 0, 0);
	
	#if(mp3Debug == 1)
	printf("mp3Debug: %14s ", "mp3Prev");
	for (mp3_i = 0; mp3_i < 10; mp3_i++) printf("%02X ", send_buf[mp3_i]);
	printf("\r\n");
	#endif
}

void mp3SetVolume(u8 volume) {
	mp3SendCmd(0x06, 0, volume);
	
	#if(mp3Debug == 1)
	printf("mp3Debug: %-14s ", "mp3SetVolume");
	for (mp3_i = 0; mp3_i < 10; mp3_i++) printf("%02X ", send_buf[mp3_i]);
	printf("\r\n");
	#endif
}

void mp3Pause(void) {
	mp3SendCmd(0x0E, 0, 0);
	
	#if(mp3Debug == 1)
	printf("mp3Debug: %-14s ", "mp3Pause");
	for (mp3_i = 0; mp3_i < 10; mp3_i++) printf("%02X ", send_buf[mp3_i]);
	printf("\r\n");
	#endif
}

void mp3Stop(void) {
	mp3SendCmd(0x16, 0, 0);
	
	#if(mp3Debug == 1)
	printf("mp3Debug: %-14s ", "mp3Stop");
	for (mp3_i = 0; mp3_i < 10; mp3_i++) printf("%02X ", send_buf[mp3_i]);
	printf("\r\n\n");
	#endif
}

void mp3UART_Configuration(void) {
	{ /* Enable peripheral clock                                                                              */
		CKCU_PeripClockConfig_TypeDef CKCUClock = {{ 0 }};
		CKCUClock.Bit.UART0 = 1;
		CKCU_PeripClockConfig(CKCUClock, ENABLE);
	}
	
	/* Turn on UxART Rx internal pull up resistor to prevent unknow state                                     */
	GPIO_PullResistorConfig(HT_GPIOC, GPIO_PIN_5, GPIO_PR_UP);
	
	AFIO_GPxConfig(GPIO_PC, AFIO_PIN_4, AFIO_FUN_USART_UART); // TX
	AFIO_GPxConfig(GPIO_PC, AFIO_PIN_5, AFIO_FUN_USART_UART); // RX
	
	USART_InitStructure.USART_BaudRate = 9600;
	USART_InitStructure.USART_WordLength = USART_WORDLENGTH_8B;
	USART_InitStructure.USART_StopBits = USART_STOPBITS_1;
	USART_InitStructure.USART_Parity = USART_PARITY_NO;
	USART_InitStructure.USART_Mode = USART_MODE_NORMAL;
	USART_Init(HT_UART0, &USART_InitStructure);
	
	NVIC_EnableIRQ(UART0_IRQn);
	
	USART_IntConfig(HT_UART0, USART_INT_RXDR, ENABLE);
	
	USART_TxCmd(HT_UART0, ENABLE);
	USART_RxCmd(HT_UART0, ENABLE);
}
