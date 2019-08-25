// **********   S * K * Y  |)  W * E * T *
//  -=-=-=-=-=-=-=-=-=-=-=-=-
// turret control firmware for esp32 dev kit C
//  may 1, 2019


// *********   P R E P R O C E S S O R S

// WIFI TEST LIBRARIES


#include <WiFi.h>

#include "DataLogging.h"

// standard library includes
#include <Adafruit_NeoPixel.h>
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
#define rgbLedBlue 26
#define rgbLedGreen 25
#define rgbLedRed 27

// solar panel
#define currentSense A6
#define solarPanelVoltage A7
const char* ssid = "chicken_house";
const char* password ="quadra3604";

void setup()
{	
	initRainBow();	// follow the init rainbow....
	
	WiFi.begin(ssid, password);
	/*
	while (WiFi.status() != WL_CONNECTED)
	{
		delay(500);
		Serial.print(".");
	}
	

	printf("... 
		Connected!\n");
	printf("IP Address is: %d\n", WiFi.localIP());
	Serial.println(WiFi.localIP());
	*/
	/*if (WiFi.status() == WL_CONNECTED)
	{
		float d[2] = { 15.0,11.92 };
		int f[2] = { 0,1 };
		logData(d, f, 2);

	}
	*/
	
	delay(5000);

		

	valveGoHome();

	//digitalWrite(stepperValveDirPin, LOW);	// hopefully high means step outwards


	//makes the rainboy panel face south if hose enters from west
	moveToPosition(stepperDomeStpPin, 0, 150, 100, 0); //(int stepperpin, int targetPosition, int speed, int accel, int current)
	delay(500);
	digitalWrite(stepperDomeSlpPin, LOW);
	digitalWrite(stepperValveStpPin, LOW);

	
	checkWakeUpReason();	 // here it goes to see if it a wakeUp event was triggered by a timer or a pushButton event on GPIO_IO_13
	
	Serial.println("i fell out of state machine and now going to sleep");
	deepSleep();			 // should not actually land here unless the program flow fell out of the state Machine
}

void loop()
{
	
}

void shootSingleSquare()
{
	int targetFlow = squareArray[getSquareID(singleSquareData)][2];
	int targetStep = squareArray[getSquareID(singleSquareData)][3];
}
