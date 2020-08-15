#include "ht32.h"

#define mp3Debug	0

#define QUEUE_MAX_SIZE 50

u16 mp3GetChecksum(u8*);

void mp3FillChecksum(void);

void mp3SendCmd(u8 *cmdQueue, u8 *queueSize, u8 cmd, u16 high_arg, u16 low_arg);

void mp3ResetModule(u8 *cmdQueue, u8 *queueSize);

void mp3SetDevice(u8 *cmdQueue, u8 *queueSize, u8);

void mp3Play(u8 *cmdQueue, u8 *queueSize, u8 music);

void mp3Continue(u8 *cmdQueue, u8 *queueSize);

void mp3Next(u8 *cmdQueue, u8 *queueSize);

void mp3Prev(u8 *cmdQueue, u8 *queueSize);

void mp3SetVolume(u8 *cmdQueue, u8 *queueSize, u8 volume); // Only 0 ~ 30

void mp3Pause(u8 *cmdQueue, u8 *queueSize);

void mp3Stop(u8 *cmdQueue, u8 *queueSize);

void mp3ReceiveCmd(void);

void mp3UART_Configuration(void);
