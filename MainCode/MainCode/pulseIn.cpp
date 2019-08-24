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


float pulseTime;	// time of pulse for math
int counter1;
int counter2;
int lastMicros = micros();
float freqbuff[4];
float freqsum;

void doPulseIn()
{

	int lastMicros = micros();



	for (int i = 0; i < 4; i++) {

		freqsum = 0;


		if (gpio_get_level(pulsePin) == 1)
		{
			//Serial.println("while loop for pin = 1");
			while (gpio_get_level(pulsePin) == 1)
			{

				counter1 = micros();
				if (counter1 - lastMicros > 500000) { break; }
			}
			pulseTime = counter1 - lastMicros;

			if (500000 > pulseTime && pulseTime > 0)
			{
				freqbuff[i] = 500000 / pulseTime;
			}
			else { freqbuff[1] = 0; freqbuff[2] = 0; freqbuff[3] = 0; freqbuff[4] = 0;}
		}
		//Serial.println("frequency is ");
		//Serial.println(freq);
		lastMicros = micros();

		i++;

		if (gpio_get_level(pulsePin) == 0)
		{
			//Serial.println("while loop for pin = 0");
			while (gpio_get_level(pulsePin) == 0)
			{

				counter2 = micros();
				if (counter2 - lastMicros > 500000) { break; }
			}

			pulseTime = counter2 - lastMicros;

			if (500000 > pulseTime && pulseTime > 0)
			{
				freqbuff[i] = 500000 / pulseTime;
			}
			else { freqbuff[1] = 0; freqbuff[2] = 0; freqbuff[3] = 0; freqbuff[4] = 0; }
		}

		for (int i = 0; i < 4; i++) {
			freqsum += freqbuff[i];
		}
		freq = freqsum / 4;

	}
	//delay(100);
	//Serial.println("frequency is ");
	//Serial.println(freq);
	//Serial.println(gpio_get_level(pulsePin));
}