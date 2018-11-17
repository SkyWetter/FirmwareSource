// **********   S * K * Y  |)  W * E * T *
//  -=-=-=-=-=-=-=-=-=-=-=-=-
// turret control firmware for esp32 dev kit C
//  october 31, 2018


// *********   P R E P R O C E S S O R S
#include "SPIFFSFunctions.h"
#include <SPIFFS.h>
#include <Stepper.h>
#include <BluetoothSerial.h>
#include <soc\rtc.h>
#include "InitESP.h"
#include <pthread.h>
#include "GlobalVariables.h"
#include "GeneralFunctions.h"
#include "StepperFunctions.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include "sys/time.h"
#include "SolarPowerTracker.h"
#include "SerialData.h"
#include "sdkconfig.h"
#include <driver/adc.h>
#include "realTimeFunctions.h"
//#include <freertos/ringbuf.h>

#define GPIO_INPUT_IO_TRIGGER     0  // There is the Button on GPIO 0
#define GPIO_DEEP_SLEEP_DURATION     10  // sleep 30 seconds and then wake up
#define CCW -1
#define CW  1

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
#define rgbLedBlue 27
#define rgbLedGreen 26
#define rgbLedRed 25

// solar panel
#define currentSense A6
#define solarPanelVoltage A7


void setup()
{
	initESP();  // Configures inputs and outputs/pin assignments, serial baud rate,
				// starting systemState (see InitESP.cpp)
	Serial.println("ESP Initialized...");
	domeGoHome(); 

}


void loop()
{
	checkSystemState();
	//Serial.println(systemState);
}

void checkSystemState()
{
	switch (systemState)
	{
	case sleeping:
	{

		if (SerialBT.available() || Serial.available())
		{
			
			systemState = program;
		}
		break;
	}

	case solar:
	{

		solarPowerTracker();
		systemState = sleeping;
		break;
	}

	case program:
	{
		if (freq != oldfreq) {
			Serial.println(freq);
			SerialBT.println(freq);
			oldfreq = freq;
		}
		Serial.println("main code program case");
		getSerialData();


		systemState = sleeping;

		break;
	}

	case water:
	{
		//load correct instruction set for date and time
		//reference temperature and apply modfifier to watering durations
		//open thread for flow sensor
		//run spray program
		if (systemState_previous != systemState)
		{
			Serial.printf("SystemState: Watering Mode");
		}


		systemState_previous = systemState;

		break;
	}

	case low_power:
	{
		//close the valve
		//set LED to red
		//allow solar
		//prevent water until battery > 50%
		  //>50% -> perform last spray cycle
		if (systemState_previous != systemState)
		{
			Serial.printf("SystemState: Low Power Mode");
		}

		systemState_previous = systemState;

		break;
	}
	}
}

void shootSingleSquare()
{
	int targetFlow = squareArray[getSquareID(singleSquareData)][2];
	int targetStep = squareArray[getSquareID(singleSquareData)][3];

}
