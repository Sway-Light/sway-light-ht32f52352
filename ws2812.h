#include "ht32.h"

#define WS_PRESCALER  7
#define WS_PIXEL      8 														
#define WS_LOGIC_1    0xF8                            // High Bit Duty
#define WS_LOGIC_0    0xC0                            // Low Bit Duty
#define WS_24BITS     24                              // Number of Bits of each LED
#define WS_BLOCK      ((WS_PIXEL + 3) * WS_24BITS)    // PDMA transfer blocks
                                                 
#define WS_LEV_SIZE   16
#define WS_FRQ_SIZE   9

void wsInit(void);
void wsCKCUConfig(void);
void wsAFIOConfig(void);
void wsPDMAConfig(void);
void wsSPIConfig(void);
void wsSetColor(u8 pixelNum, u8 color[], float mag);
void wsShow(void);
void wsClearAll(void);
void wsBlinkAll(u32 wait);
