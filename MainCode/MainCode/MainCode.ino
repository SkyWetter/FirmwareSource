// **********   S * K * Y  |)  W * E * T *
//  -=-=-=-=-=-=-=-=-=-=-=-=-
// turret control firmware for esp32 dev kit C
//  october 31, 2018


// http://williams.comp.ncat.edu/comp450/pthreadPC.c <--- pthreads example

// *********   P R E P R O C E S S O R S
#include <Stepper.h>
#include <BluetoothSerial.h>
#include <Spiffs.h>
#include <soc\rtc.h>
#include "InitESP.h"
#include <pthread.h>
#include "GlobalVariables.h"
#include "GeneralFunctions.h"
#include "StepperFunctions.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "sys/time.h"
#include "SolarPowerTracker.h"
#include "SerialData.h"
#include "sdkconfig.h"
#include <driver/adc.h>
#include "pthread.h"
#include "config.h"


#define GPIO_INPUT_IO_TRIGGER     0  // There is the Button on GPIO 0
#define GPIO_DEEP_SLEEP_DURATION     10  // sleep 30 seconds and then wake up
#define CCW -1
#define CW  1

// flow meter
#define pulsePin 23
#define SAMPLES 4096

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
#define currentSense A61
#define solarPanelVoltage A7

pthread_t thread1;
int returnValue;
int finished = false;

void *inc_x(void *x_void_ptr)
{
	int *x_ptr = (int *)x_void_ptr;
	while (++(*x_ptr) < 10000);

	printf("x inc finished\n");

	return NULL;
}


void *printThreadID(void *threadid)
{
	Serial.print((int)threadid);
	Serial.println("a ");
}

int testVar1 = 0;
int testVar2 = 0;
pthread_t inc_x_thread;
bool threadCreated = false;

void setup()
{
	Serial.begin(115200); 

	delay(250); // Delay must be input to insure that the Serial
	printf("INIT x: %d, y: %d\n", testVar1, testVar2);
	
	
	

	/*
	printf("y increment finished\n");

	if (pthread_join(inc_x_thread, NULL))
	{
		printf("Error joining thread\n");

	}
	*/

}


void loop()
{
	
	printf("X is: %d\n",testVar1);

	if (!threadCreated)
	{
		if (pthread_create(&inc_x_thread, NULL, inc_x, &testVar1))
		{
			printf("error creating thread\n");

		}
		threadCreated = true;
	}
	
	/*
	
	if (firstRun)
	{
		Serial.println("inFirstRun");
		if (Serial.available())
		{
			Serial.println("Sweet");
			firstRun = false;
			for (int i = 0; i < 4; i++)
			{
				returnValue = pthread_create(&threads[i], NULL, printThreadID, (void *)i);
				if (returnValue)
				{
					Serial.println("An error has occured");
				}
			}
		}
	}

	else
	{

		if (Serial.available())
		{
			char newChar = Serial.read();

			if (newChar == 0)
			{

			}
		}
	}
	//checkSystemState();*/
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

		break;
	}

	case program:
	{

		getSerialData();

		break;
	}

	case water:
	{
		//load correct instruction set for date and time
		//reference temperature and apply modfifier to watering durations
		//open thread for flow sensor
		//run spray program

		break;
	}

	case low_power:
	{
		//close the valve
		//set LED to red
		//allow solar
		//prevent water until battery > 50%
		  //>50% -> perform last spray cycle
	
		break;
	}
	}
}

void shootSingleSquare()
{
	int targetFlow = squareArray[getSquareID(singleSquareData)][2];
	int targetStep = squareArray[getSquareID(singleSquareData)][3];

	
}
