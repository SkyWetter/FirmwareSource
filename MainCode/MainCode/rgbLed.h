// rgbLed.h

#ifndef _RGBLED_h
#define _RGBLED_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif


#endif

extern void ledBlue(bool onOff);
extern void ledRed(bool onOff);
extern void ledsOut();