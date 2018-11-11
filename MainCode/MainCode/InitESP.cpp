 


#include <Stepper.h>
#include <BluetoothSerial.h>
#include <soc\rtc.h>
#include "InitESP.h"
#include <pthread.h>
#include "GlobalVariables.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include "sys/time.h"
#include "sdkconfig.h"



#define GPIO_INPUT_IO_TRIGGER 0  // There is the Button on GPIO 0
#define GPIO_DEEP_SLEEP_DURATION 10  // sleep 30 seconds and then wake up
#define CCW -1
#define CW 1


// ********* P I N   A S S I G N M E N T S
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

// //wake-up push button
#define wakeUpPushButton GPIO_NUM_13

// rgb led
#define rgbLedBlue 27
#define rgbLedGreen 26
#define rgbLedRed 25

// solar panel
#define currentSense A6
#define solarPanelVoltage A7

int serialBaud = 115200;

void initESP()
{
	initPins();
	initSerial();
	systemState = sleeping;
	systemState_previous = water;
	// power management
	esp_sleep_enable_ext0_wakeup(GPIO_NUM_13, 1);
}

void initSerial()
{
	Serial.begin(serialBaud);
	
	Serial.printf("Serial Intialized with %d baud rate", serialBaud);
}

void initPins()
{
	//PWM Stuff for output of duty cycle to current control

	ledcSetup(stepperDomeCrntPin, 500, 8);
	ledcSetup(stepperValveCrntPin, 500, 8);

	ledcAttachPin(stepperValveCrntPin, stepperValveCrntPin);
	ledcAttachPin(stepperDomeCrntPin, stepperDomeCrntPin);



	// pin assignments
	pinMode(pulsePin, INPUT);               // pin to read pulse frequency                    // init timers need for pulseCounters


	pinMode(stepperDomeDirPin, OUTPUT);						// OUTPUT pin setup for MP6500 to control DOME stepper DIRECTION
	pinMode(stepperDomeStpPin, OUTPUT);						// OUTPUT pin setup for MP6500 to control DOME stepper STEP
	pinMode(stepperDomeSlpPin, OUTPUT);						// OUTPUT pin setup for MP6500 to control DOME stepper ENABLE

	pinMode(hallSensorDome, INPUT);
	pinMode(stepperDomeCrntPin, OUTPUT);

	pinMode(stepperValveDirPin, OUTPUT);          // OUTPUT pin setup for MP6500 to control VALVE stepper DIRECTION
	pinMode(stepperValveStpPin, OUTPUT);          // OUTPUT pin setup for MP6500 to control VALVE stepper STEP
	pinMode(stepperValveSlpPin, OUTPUT);          // OUTPUT pin setup for MP6500 to control VALVE stepper ENABLE
	pinMode(hallSensorValve, INPUT);
	pinMode(stepperValveCrntPin, OUTPUT);

	pinMode(wakeUpPushButton, INPUT);

	pinMode(currentSense, INPUT);
	pinMode(solarPanelVoltage, INPUT);

	pinMode(rgbLedBlue, OUTPUT);
	pinMode(rgbLedRed, OUTPUT);
	pinMode(rgbLedGreen, OUTPUT);


	// init pin states
	digitalWrite(stepperDomeStpPin, LOW);
	digitalWrite(stepperValveStpPin, LOW);

	digitalWrite(stepperDomeDirPin, LOW);
	digitalWrite(stepperValveDirPin, LOW);

	digitalWrite(stepperDomeSlpPin, LOW);
	digitalWrite(stepperValveSlpPin, LOW);

	digitalWrite(rgbLedBlue, LOW);
	digitalWrite(rgbLedRed, LOW);
	digitalWrite(rgbLedGreen, LOW);

	ledcWrite(stepperDomeCrntPin, 204);	//sets current limi of dome to ~500mA
	ledcWrite(stepperValveCrntPin, 0);	// no current limit on valve so 2 amp
}