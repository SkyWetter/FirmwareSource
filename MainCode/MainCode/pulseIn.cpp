#include <BluetoothSerial.h> 
#include <Stepper.h>
#include <soc\rtc.h>
#include <pthread.h>
#include "GlobalVariables.h"
#include "InitESP.h"
#include "driver\adc.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include "sys/time.h"
#include "sdkconfig.h"
#include "GeneralFunctions.h"

#define GPIO_INPUT_IO_TRIGGER     0  // There is the Button on GPIO 0
#define GPIO_DEEP_SLEEP_DURATION     10  // sleep 30 seconds and then wake up
#define CCW -1
#define CW  1
#define TEST 45

// flow meter
#define pulsePin GPIO_NUM_23

// dome stepper
#define stepperDomeDirPin 19
#define stepperDomeStpPin 18
#define stepperDomeSlpPin 2
#define hallSensorDome 16
#define stepperDomeCrntPin 14

// valve stepper
#define stepperValveDirPin 5
#define stepperValveStpPin 17
#define stepperValveSlpPin 15
#define hallSensorValve 4
#define stepperValveCrntPin 12

// wake-up push button
#define wakeUpPushButton GPIO_NUM_13

// rgb led
#define rgbLedBlue 26
#define rgbLedGreen 25
#define rgbLedRed 27

// solar panel
#define currentSense A6


void doPulseIn()
{
	//Serial.println("pulseIn begin");

	int pulseStart;			//gets set when the pulse pin goes high
	bool pulseState;		//the current pulse state of the pin starts unknown
	int begin = millis();	// a counter for the entire loop. If no flow then timeout
	int waitTime = 0;		//time spent waiting for pulse to go low
	float pulseTime = 0;	// time of pulse for math
	int newPulseLength = 0;	// continous pulse time
	

	while (begin + 500 >= millis()) {					//if time is available do the things
		//while(1){

			//delay(1);
		//initpinState = gpio_get_level(pulsePin);

		if (gpio_get_level(pulsePin) == 0) {				//first we wait for the pin to go low

			delay(4);									//delays are for debouncing. Shouldnt need to actual sensor
			pulseState = 0;								//set pulse state to match pin

			if (gpio_get_level(pulsePin) == 1) {		//when pin goes from low to high

				pulseState = 1;							//set pulse state
				pulseStart = millis();					//set pulse start time
				newPulseLength = pulseStart;			//set new pulse time
				waitTime = millis();					//set wait timer
				//delay(4);								//this delay is only needed for button debugging	
				//Serial.println("pulse detected");
				while (pulseState == gpio_get_level(pulsePin) && waitTime + 500 >= millis()) {			//while the pin is still high and timer is avail

					newPulseLength = millis();			//slot in a new value for pulse length
					//Serial.println("reading pulse");
				}

				pulseTime = newPulseLength - pulseStart;	//math to figure out pulse time float
				pulseStart = millis();						//reset timer incase skips while loop next time
				//Serial.println("pulse complete");
				//Serial.print("pulse length is");

				if (pulseTime >= 500) { pulseTime = 0; }
				if (pulseTime > 0) {							//only do math if pulse is not zero
					//Serial.print("i try done math ");
					freq = 500 / pulseTime;						// 500 because a pulse is 1/2 cycle
				}
				else {
					freq = 0;
				}
				//Serial.println(pulseTime);
			}
		}
	}
	//Serial.print("pulse length final is ");
	//Serial.println(pulseTime);

	
	//Serial.print("frequency is ");
	//Serial.println(freq);
	//Serial.println("exiting pulseIn");


}
