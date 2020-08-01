#include "ht32.h"

#define WS_RELOAD			(60)													// (48000000/800000)	
#define WS_PRESCALER	0
#define WS_PIXEL			8 														
#define WS_LOGIC_1		38														// High Bit Duty
#define WS_LOGIC_0		19														// Low Bit Duty
#define	WS_24BITS			24														// Number of Bits of each LED
#define WS_BLOCK			((WS_PIXEL + 3) * WS_24BITS)	// PDMA transfer blocks

#define WS_GENERAL		80
#define WS_GUIDE			81
#define WS_GAME				82
#define WS_EXIT				10
#define WS_BLINK			18 
#define WS_BREATH			50                                                    


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
