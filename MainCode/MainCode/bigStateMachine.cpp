// *********   P R E P R O C E S S O R S
#include "bigStateMachine.h"
#include "deepSleep.h"
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
#include "bigStateMachine.h"

const byte interruptPin = 13;
volatile int interruptCounter = 0;
int numberOfInterrupts = 0;

portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

// power on rainbow
// init..
// check status flags, time, schedule tasks, etc --> bool amPmWaterFlag,, int waterAlarm, checkBatteryVoltage(), programFlag, sleepTimer ISR
// first check battery power
// 2nd check if it is watering time
// 3rd do current sense
// if button enable BT for program mode

void setupIntterupt()
{

	pinMode(interruptPin, INPUT_PULLUP);
	attachInterrupt(digitalPinToInterrupt(interruptPin), handleInterrupt, FALLING);

	if (interruptCounter > 0)
	{

		portENTER_CRITICAL(&mux);
		interruptCounter--;
		portEXIT_CRITICAL(&mux);

		numberOfInterrupts++;
		Serial.print("An interrupt has occurred. Total: ");
		Serial.println(numberOfInterrupts);
	}
}

void IRAM_ATTR handleInterrupt() 
{
	if (!SerialBT.hasClient)
	{
		// this occurs during operation but is a wake-up push button event
		// goto to program mode
		// pin mode init, dome goes home, etc
		// enable bt
		//blueLedOn


		valveGoHome();
		domeGoHome();

		initPins();
		initSerial();
		initThreads();

		spiffsBegin();
		systemState = idle;
		return;
	}

	portENTER_CRITICAL_ISR(&mux);
	interruptCounter++;
	portEXIT_CRITICAL_ISR(&mux);
}

