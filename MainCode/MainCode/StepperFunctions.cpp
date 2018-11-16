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
#include "StepperFunctions.h"
#include "GeneralFunctions.h"
#define GPIO_INPUT_IO_TRIGGER     0  // There is the Button on GPIO 0
#define GPIO_DEEP_SLEEP_DURATION     10  // sleep 30 seconds and then wake up
#define CCW -1
#define CW  1
#define TEST 45

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
#define rgbLedBlue 27
#define rgbLedGreen 26
#define rgbLedRed 25

// solar panel
#define currentSense A6
#define solarPanelVoltage A7


// M A I N    F U N  C T I O N  --- STEPPER GO HOME
void stepperGoHome(byte x, byte y, byte z, byte s)                      // x STEP, y DIR, z EN, s HALL
{
	// SET stepper CW
	digitalWrite(z, HIGH);																	// ENSURE STEPPER IS NOT IN SLEEP MODE

	//stepperDomeOneStepHalfPeriod(10);
	//stepperDomeOneStepHalfPeriod(10);
	//stepperDomeOneStepHalfPeriod(10);

	while (digitalRead(s) == 1)																// if hallSensor is HIGH the stepper is NOT at HOME
	{
		digitalWrite(x, HIGH);
		delay(5);
		digitalWrite(x, LOW);
		delay(5);
	}

	//digitalWrite(z, HIGH);																	// put stepper back to sleep
	//digitalWrite(y, LOW);																	// SET STEOPP BACK TO CCW

}
// S U B   F U N C T I O N S --- dome and valve go home
void domeGoHome()
{
	stepperDomeDirCCW();
	stepperDomeOneStepHalfPeriod(5);
	stepperDomeOneStepHalfPeriod(5);


	//digitalWrite(stepperDomeDirPin, HIGH);																				// HIGH IS CLOSEWISE!!!
	stepperGoHome(stepperDomeStpPin, stepperDomeDirPin, stepperDomeSlpPin, hallSensorDome);									// dome stepper go to home posisition

	stepperDomeOneStepHalfPeriod(10);
	//stepperDomeOneStepHalfPeriod(10);

																															//digitalWrite(stepperDomeDirPin, LOW);		
	// LOW ON DOME DIR PIN MEANS CW MOVEMENT AND HIGHER VALUE for stepCountDome -- ALWAYS INCREMENT FROM HERE
	//ledcWrite(stepperDomeCrntPin, 255);			//turn down stepper current once home
	digitalWrite(stepperDomeSlpPin, LOW);		// low means sleep

	currentDomePosition = 0;
	SerialBT.println("dome go home");                                           // LOW IS COUNTERCLOCKWISE
}
void valveGoHome()
{

	digitalWrite(stepperValveDirPin, HIGH);		// high is counter clockwise

	stepperGoHome(stepperValveStpPin, stepperValveDirPin, stepperValveSlpPin, hallSensorValve);
	//digitalWrite(stepperValveDirPin, HIGH);
	currentValvePosition = 0;
	digitalWrite(stepperValveSlpPin, LOW);	//turns the valve stepper off after completing a go home
	SerialBT.println("valve go home");
}

//Move valve to flow Function

void moveValve() {

}

void makeRain() {

}


// M O V E  D O M E  F U N C T I O N S 

//Move dome to a given position (a position is defined as a number of steps away from the home position)
void moveDome(int targetPosition)
{
	targetPosition -= currentDomePosition;   //determines the number of steps from current position to target position
	setDomeDirection(getSign(targetPosition)); //Sets dome direction CW or CCW


}

//general move a gat dang shtepper
void moveToPosition(int stepperpin, int targetPosition, int speed, int accel, int current)
{
	//Serial.println("entered moveToPositionCommand");
	//targetPosition -= currentDomePosition;   //determines the number of steps from current position to target position
	//setDomeDirection(getSign(targetPosition)); //Sets dome direction CW or CCW
	
	switch (stepperpin)
	{
	case stepperDomeStpPin:

		Serial.println("switch case for domeStepper");

		digitalWrite(stepperDomeSlpPin, HIGH);

		int stepsTaken = 0;
		int stepsToGo = targetPosition - currentDomePosition;   //determines the number of steps from current position to target position
		currentDomeDirection = (getSign(stepsToGo));		//1 is open or cw 0 is close or ccw
		digitalWrite(stepperDomeDirPin, currentDomeDirection);
		stepsToGo = abs(stepsToGo);


		if (accel == 0) { accel = domeStepperDefaults[1]; }
		//accel = 100 / accel;
		int  decelUnit = 0;
		int accelTimer = millis();

		if (speed == 0) { speed = domeStepperDefaults[0]; }
		int fastTime;
		fastTime = 1000 / speed *1000;
		int stepTime;
		stepTime = fastTime * 3;

		if (current == 0) { current = domeStepperDefaults[2]; }		//if it gets a passed 0 use default current
		setCurrent(stepperDomeCrntPin, current);


		Serial.println(targetPosition);
		Serial.println(currentDomePosition);
		Serial.println(accel);
		Serial.println(fastTime);
		Serial.println(stepTime);


		while (currentDomePosition != targetPosition) {

			Serial.println("entered while loop");
			Serial.println(targetPosition);
			Serial.println(currentDomePosition);
			Serial.println(accel);
			Serial.println(fastTime);
			Serial.println(stepTime);

			digitalWrite(stepperDomeStpPin, HIGH);
			delayMicroseconds(stepTime);
			//delay(100);
			digitalWrite(stepperDomeStpPin, LOW);
			delayMicroseconds(stepTime);
			//delay(100);

			if (currentDomeDirection == 1) { currentDomePosition += 1; stepsTaken += 1; }
			else { currentDomePosition -= 1; stepsTaken += 1;}

			if (stepTime != fastTime&& stepsToGo - stepsTaken >= decelUnit) {
				accelTimer = millis();
				Serial.println("accelerating");
				stepTime -= accel;
				if (stepTime < fastTime) {
					stepTime = fastTime;
					
				}
				decelUnit++;
			}
			Serial.println(decelUnit);

			if  (stepsToGo - stepsTaken <= decelUnit) {
				stepTime += accel;
				Serial.println("decelerating");
			}


	}
	
	}

	Serial.println("exiting moveToPosition");

}







// M A I N    F U N  C T I O N  --- STEPPER ONE STEP
void stepperOneStepHalfPeriod(byte step, byte dir, byte enable, int *spcnt, int halfFreq)                            //x STEP, y DIR, z EN, q SPCNT, h halFRQ ----!!!!!!check POINTERS!?!??!?!?--------
{
	//h = 500;
	digitalWrite(enable, HIGH);
	delay(1);                                                       // proBablay GeT rId of HTis!!?!?

	digitalWrite(step, HIGH);
	digitalWrite(rgbLedBlue, HIGH);
	//digitalWrite(rgbLedGreen, LOW);
	delay(halfFreq);
	digitalWrite(step, LOW);
	digitalWrite(rgbLedBlue, LOW);
	//digitalWrite(rgbLedGreen, HIGH);
	delay(halfFreq);

	if (digitalRead(dir) == LOW)
	{
		*spcnt--;
	}

	if (digitalRead(dir) == HIGH)
	{
		*spcnt++;                                                       // LOW ON DOME DIR PIN MEANS CW MOVEMENT AND HIGHER VALUE for stepCountDome
	}

}
// S U B   F U N C T I O N S --- dome and valve one step
void stepperDomeOneStepHalfPeriod(int hf)
{
	digitalWrite(stepperDomeSlpPin, HIGH);
	ledcWrite(stepperDomeCrntPin, 204);			//sets domestepper to 450mA of current(max)
	stepperOneStepHalfPeriod(stepperDomeStpPin, stepperDomeDirPin, stepperDomeSlpPin, &domeStepperDefaults[4], hf);
}
void stepperValveOneStepHalfPeriod(int hf)
{
	stepperOneStepHalfPeriod(stepperValveStpPin, stepperValveDirPin, stepperValveSlpPin, &valveStepperDefaults[4], hf);
}

void setDomeDirection(int direction)
{
	if (direction == 1)
	{
		stepperDomeDirCW();
	}
	else if (direction == 0)
	{
		stepperDomeDirCCW();
	}

}
void setCurrent(int pin, int current) {

		int dutyCycle;

		dutyCycle = map(current, 1, 2000, 255, 1);	//high duty cycle = less current
		ledcWrite(pin, dutyCycle);						//maps the dutycycle to the passed in pin

}


void stepperDomeDirCW()
{
	Serial.println("set dome direction CW---> HIGH IS CLOCKWISE!!!");
	SerialBT.println("set direction CW---> HIGH IS CLOCKWISE!!!");
	currentDomeDirection = CW;
	digitalWrite(stepperDomeDirPin, HIGH);
}
void stepperDomeDirCCW()
{
	Serial.println("set dome direction CCW ---> LOW IS COUNTERCLOCKWISE");
	SerialBT.println("set direction CCW---> LOW IS COUNTERCLOCKWISE");
	currentDomeDirection = CCW;
	digitalWrite(stepperDomeDirPin, LOW);
}

void toggleStepperValveDir()
{
	bool valveDir;

	valveDir = digitalRead(stepperValveDirPin);
	digitalWrite(stepperValveDirPin, !valveDir);

	if (valveDir == 1)
	{
		Serial.println("set direction open");
		SerialBT.println("set direction open");
	}
	else
	{
		Serial.println("set direction close");
		SerialBT.println("set direction close");
	}

	digitalWrite(stepperValveDirPin, !valveDir);
}

void valveStepperOneStep()
{
	stepperValveOneStepHalfPeriod(10);
	Serial.println(currentValvePosition);
	SerialBT.println(currentValvePosition);
	//digitalWrite(stepperValveEnPin, LOW);
}
