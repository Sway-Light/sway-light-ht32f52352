#include "ht32.h"

#define WS_PRESCALER  7
#define WS_PIXEL      128
#define WS_LOGIC_1    0xFC                            // High Bit Duty
#define WS_LOGIC_0    0xE0                            // Low Bit Duty
#define WS_24BITS     24                              // Number of Bits of each LED
#define WS_BLOCK      ((WS_PIXEL + 3) * WS_24BITS)    // PDMA transfer blocks
                                                 
#define WS_FRQ_SIZE   16
#define WS_LEV_SIZE   8

void wsInit(void);
void wsCKCUConfig(void);
void wsAFIOConfig(void);
void wsPDMAConfig(void);
void wsSPIConfig(void);
void wsSetColor(u8 pixelNum, u8 color[], u8 mag);
void wsShow(void);
void wsClearAll(void);
void wsBlinkAll(u32 wait);
