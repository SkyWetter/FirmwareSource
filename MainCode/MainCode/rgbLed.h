// rgbLed.h

#ifndef _RGBLED_h
#define _RGBLED_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif


#endif

uint32_t Wheel(byte WheelPos);
extern void ledBlue(bool onOff);
extern void colorWipe(uint32_t c, uint8_t wait);
extern void rainbow(uint8_t wait);