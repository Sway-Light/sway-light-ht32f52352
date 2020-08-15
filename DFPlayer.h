#include "ht32.h"

#define mp3Debug	0

static u8 send_buf[10] = {0x7E, 0xFF, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xEF};
static u8 return_buf[10];

u16 mp3GetChecksum(u8*);

void mp3FillChecksum(void);

void mp3SendCmd(u8, u16, u16);

void mp3ResetModule(void);

void mp3SetDevice(u8);

void mp3Play(u8);

void mp3Continue(void);

void mp3Next(void);

void mp3Prev(void);

void mp3SetVolume(u8); // Only 0 ~ 30

void mp3Pause(void);

void mp3Stop(void);

void mp3ReceiveCmd(void);

void mp3UART_Configuration(void);
