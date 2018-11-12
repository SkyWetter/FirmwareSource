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
	digitalWrite(stepperDomeSlpPin, LOW);

	stepCountDome = 0;
	SerialBT.println("dome go home");                                           // LOW IS COUNTERCLOCKWISE
}
void valveGoHome()
{

	digitalWrite(stepperValveDirPin, HIGH);

	stepperGoHome(stepperValveStpPin, stepperValveDirPin, stepperValveSlpPin, hallSensorValve);
	//digitalWrite(stepperValveDirPin, HIGH);
	stepCountValve = 0;
	digitalWrite(stepperValveSlpPin, LOW);	//turns the valve stepper off after completing a go home
	SerialBT.println("valve go home");
}




// M O V E  D O M E  F U N C T I O N S 

//Move dome to a given position (a position is defined as a number of steps away from the home position)
void moveDome(int targetPosition)
{
	int finalPosition = targetPosition -= currentDomePosition;
	finalPosition = abs(finalPosition);  // Arduino recommends not performing calculations inside the abs function, as you can a get strange 
										// results/errors
	while (currentDomePosition != finalPosition)
	{
		targetPosition -= currentDomePosition;   //determines the number of steps from current position to target position
		setDomeDirection(getSign(targetPosition)); //Sets dome direction CW or CCW
		stepperMove(abs(targetPosition), 5);
	}

	
}




//Move dome to a given position with non-default speed, accel and current values
void moveDome(int targetPosition, int speed, int accel, int current)
{
	targetPosition -= currentDomePosition;   //determines the number of steps from current position to target position
	setDomeDirection(getSign(targetPosition)); //Sets dome direction CW or CCW

}

// Move dome a given number of steps in a given direction (takes CW or CCW as second argument)
void moveDome(int stepsToMove, int direction)
{
	;
}


// Move dome a given number of steps in a given direciton, with non-default speed, accel and current values)
void moveDome(int stepsToMove, int direction, int speed, int accel, int current)
{
	;
}


void stepperMove(int numberOfSteps,int speed)
{
	for (int i = 0; i < numberOfSteps; i++)
	{
		stepperDomeOneStepHalfPeriod(speed);
    }
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

	if (currentDomeDirection == CW)
	{
		currentDomePosition += 1;
	}
	
	else if (currentDomePosition == CCW)
	{
		currentDomePosition -= 1;
	}

	if (currentDomePosition > 400) { currentDomePosition = 400; }
	else if (currentDomePosition < 0) { currentDomePosition = 0; }

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
	stepperOneStepHalfPeriod(stepperDomeStpPin, stepperDomeDirPin, stepperDomeSlpPin, &stepCountDome, hf);
}
void stepperValveOneStepHalfPeriod(int hf)
{
	stepperOneStepHalfPeriod(stepperValveStpPin, stepperValveDirPin, stepperValveSlpPin, &stepCountValve, hf);
}

void setDomeDirection(int direction)
{
	if (direction == CW)
	{
		stepperDomeDirCW();
	}
	else if (direction == CCW)
	{
		stepperDomeDirCCW();
	}

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
	Serial.println(stepCountValve);
	SerialBT.println(stepCountValve);
	//digitalWrite(stepperValveEnPin, LOW);
}
