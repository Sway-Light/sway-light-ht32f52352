#include "ht32.h"

#define WS_RELOAD			(60)													// (48000000/800000)	
#define WS_PRESCALER	0
#define WS_PIXEL			128 														
#define WS_LOGIC_1		38														// High Bit Duty
#define WS_LOGIC_0		19														// Low Bit Duty
#define	WS_24BITS			24														// Number of Bits of each LED
#define WS_BLOCK			(WS_PIXEL * WS_24BITS)	// PDMA transfer blocks
                                                 
#define WS_LEV_SIZE   8
#define WS_FRQ_SIZE   16

//extern u8 ws_green[];
//extern u8 ws_blue[];

void wsInit(void);

void wsCKCUConfig(void);

void wsAFIOConfig(void);

void wsPWMConfig(void);

void wsPDMAConfig(void);

void wsNVICConfig(void);

void wsSetColor(u8 pixelNum, u8 color[], float mag);

void wsShow(void);

void wsClearAll(void);

void wsSetRow(u8 row[], u8 color[]);

void wsSetCol(u8 col[], u8 color[]);

void wsClearBetween(u8 startPixelNum, u8 endPixelNum);

void wsBlinkAll(u32 wait);

void wsPrintBuffer(void);

void wsSetLeftArrow(u8 color[]);

void wsSetRightArrow(u8 color[]);

void wsSetNumber(u8 number);

void wsSetMusicStatus(bool play, u8 color[]);

void wsSetTransAnim(u8 *pictureCount);

void wsSetScoreAnim(u8 *pictrueCount);

void wsSetModeLed(u8 mode);
