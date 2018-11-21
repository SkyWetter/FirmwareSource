// **********   S * K * Y  |)  W * E * T *
//  -=-=-=-=-=-=-=-=-=-=-=-=-
// turret control firmware for esp32 dev kit C
//  november 20, 2018


// *********   P R E P R O C E S S O R S
#include "stateMachine.h"
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

#define GPIO_INPUT_IO_TRIGGER     0  // There is the Button on GPIO 0
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
	initRainBow();
	checkWakeUpReason();	 // here it goes to see if it a wakeUp event was triggered by a timer or a pushButton event on GPIO_IO_13
	//domeGoHome();			 // M A Y BE DONT COMMENT THIS OUT???! THIS NEED TO BE HERE OR NOT??
	initSleepClock();
}


void loop()
{
	Serial.println("wont ever be here??? Should not be here");
}


void shootSingleSquare()
{
	int targetFlow = squareArray[getSquareID(singleSquareData)][2];
	int targetStep = squareArray[getSquareID(singleSquareData)][3];
}
