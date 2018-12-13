// *********   P R E P R O C E S S O R S
// standard library includes
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <soc\rtc.h>
// esp32 periph includes
#include <Stepper.h>
#include <BluetoothSerial.h> 
#include <pthread.h>
#include <SPIFFS.h>
// local includes
#include "sys/time.h"
#include "sdkconfig.h"
#include "driver\adc.h"
#include "driver/gpio.h"
#include "soc/timer_group_struct.h"
#include "soc/timer_group_reg.h"

//#include <freertos/ringbuf.h>
// custom includes
#include  "deepSleep.h"
#include "GeneralFunctions.h"
#include "GlobalVariables.h"
#include "InitESP.h"
#include "pulseIn.h"
#include "realTimeFunctions.h"
#include "rgbLed.h"
#include "SerialData.h"
#include "SolarPowerTracker.h"
#include "SPIFFSFunctions.h"
#include "stateMachine.h"
#include "StepperFunctions.h"
#include <Adafruit_NeoPixel.h>

#define stepperDomeDirPin 19	//put here for presentaiton
#define stepperDomeStpPin 18
#define stepperDomeSlpPin 2
#define hallSensorDome 16
#define stepperDomeCrntPin 14

//destroy after presntation
uint32_t Wheel(byte WheelPos);
void rainbow(uint8_t wait);
void colorWipe(uint32_t c, uint8_t wait);	//destroy after presentation
Adafruit_NeoPixel strip = Adafruit_NeoPixel(3, 27, NEO_GRB + NEO_KHZ800);

// power on rainbow
// init..
// check status flags, time, schedule tasks, etc --> bool amPmWaterFlag,, int waterAlarm, checkBatteryVoltage(), programFlag, sleepTimer ISR
// case 0 check battery power
// case 1 check if it is watering time
// case 2 do current sense
// ext. ISR if GPIO13 wakeUpButton = HIGH --> enable BT for program mode



void programState()
{
	//bool programStateNotDoneFlag = 1;
	
	initSerialBT();

	strip.begin();
	strip.show();
	for (int x = 0; x <= 1; x++) {

		digitalWrite(stepperDomeSlpPin, HIGH);


		digitalWrite(stepperDomeDirPin, HIGH);
		for (int i = 0; i < 50; i++) {

			colorWipe(strip.Color(0, 0, i), 5); // blue
			digitalWrite(stepperDomeStpPin, HIGH);
			delay(1);

			digitalWrite(stepperDomeStpPin, LOW);
			delay(1);

		}
		rainbow(5);

		digitalWrite(stepperDomeDirPin, LOW);

		for (int i = 0; i < 50; i++) {
			colorWipe(strip.Color(0, i, 0), 5); // green
			digitalWrite(stepperDomeStpPin, HIGH);
			delay(1);

			digitalWrite(stepperDomeStpPin, LOW);
			delay(1);

		}
		rainbow(5);

	}
	//for (int x = 0; x < 10; x++)
	//{
	//	rainbow(5);
	//}


	while (programStateNotDoneFlag)
	{
		if (freq != oldfreq)
		{
			Serial.println(freq);
			//SerialBT.println(freq);
			oldfreq = freq;
		}
		//Serial.println("main code program case");
		getSerialData();
		
	}
	ledBlue(1);
	SerialBT.end();
	deepSleep();
}

void timerState()
{

	while (wakeUpTimerStateNotDoneFlag)
	{

		switch (sysStateTimerWakeUp)
		{

			case low_power:
			{
				//close the valve
				//set LED to red
				//allow solar
				//prevent water until battery > 50%
				//>50% -> perform last spray cycle

				//checkBatteryCap();
				//if (checkBatteryCap())
				//{
					//powerDownEverything();
					//sysStateTimerWakeUp = solar;
				//}
				//else
				//{
				Serial.println("Im doing low power here");
				ledRed(1);
				sysStateTimerWakeUp = water;
				//}
				break;
			}

			case water:
			{
				//load correct instruction set for date and time
				//reference temperature and apply modfifier to watering durations
				//open thread for flow sensor
				//run spray program
				Serial.println("Im doing watering here");
				sysStateTimerWakeUp = solar;
				break;
			}

			case solar:
			{
				// if(solarPanelVoltage > 0)
				Serial.println("Im doing solar here");
				solarWakeUp();
				sysStateTimerWakeUp = sleepy;
				break;
			}

			case sleepy:
			{
				Serial.println("Im doing sleep here");
				sysStateTimerWakeUp = low_power;
				wakeUpTimerStateNotDoneFlag = 0;
				deepSleep();
				break;
			}
		}
	}
}

//destroy all below after presentation
void colorWipe(uint32_t c, uint8_t wait) {
	for (uint16_t i = 0; i < strip.numPixels(); i++) {
		strip.setPixelColor(i, c);
		strip.show();
		delay(wait);
	}
}


void rainbow(uint8_t wait) {
	uint16_t i, j;

	for (j = 0; j < 256; j++) {
		for (i = 0; i < strip.numPixels(); i++) {
			strip.setPixelColor(i, Wheel((i + j) & 255));
		}
		strip.show();
		delay(wait);
	}
}

uint32_t Wheel(byte WheelPos) {
	WheelPos = 255 - WheelPos;
	if (WheelPos < 85) {
		return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
	}
	if (WheelPos < 170) {
		WheelPos -= 85;
		return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
	}
	WheelPos -= 170;
	return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}