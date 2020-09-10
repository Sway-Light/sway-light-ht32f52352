#include "ht32.h"
#include "74HC4067.h"

#define debug 0

void analogSwitcherSetup(void) {
	// switch pin setup
	GPIO_DirectionConfig(HT_GPIOD, GPIO_PIN_0, GPIO_DIR_OUT); // s0
	GPIO_DirectionConfig(HT_GPIOD, GPIO_PIN_1, GPIO_DIR_OUT); // s1
	GPIO_DirectionConfig(HT_GPIOD, GPIO_PIN_2, GPIO_DIR_OUT); // s2
	GPIO_DirectionConfig(HT_GPIOD, GPIO_PIN_3, GPIO_DIR_OUT); // s3
	
	// enable pin setup
	GPIO_DirectionConfig(HT_GPIOC, GPIO_PIN_1, GPIO_DIR_OUT);
	#if debug
	printf("asSetup\r\n");
	#endif
}

void asSetSignal(u8 channel) {
	GPIO_WriteOutData(HT_GPIOD, channel);
	#if debug
	printf("asSetSignal(%d)\r\n", channel);
	#endif
}

void asSetEnable(bool enable) {
	// enable  -> low
	// disable -> high
	GPIO_WriteOutBits(HT_GPIOC, GPIO_PIN_1, (FlagStatus)!enable);
	#if debug
	printf("asSetEnable(%d)\r\n", enable);
	#endif
}
