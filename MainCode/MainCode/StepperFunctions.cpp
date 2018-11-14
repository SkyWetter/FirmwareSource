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
void stepperMove(int numberOfSteps, int speed);

// M A I N    F U N  C T I O N  --- STEPPER GO HOME
void stepperGoHome(byte x, byte y, byte z, byte s)   // x STEP, y DIR, z EN, s HALL
{
	// SET stepper CW
	digitalWrite(z, HIGH);			// ENSURE STEPPER IS NOT IN SLEEP MODE

	while (digitalRead(s) == 1)		// if hallSensor is HIGH the stepper is NOT at HOME
	{
		digitalWrite(x, HIGH);
		delay(5);
		digitalWrite(x, LOW);
		delay(5);
	}

}
// S U B   F U N C T I O N S --- dome and valve go home
void domeGoHome()
{
	stepperDomeDirCCW();
	stepperDomeOneStepHalfPeriod(5);
	stepperDomeOneStepHalfPeriod(5);

	stepperGoHome(stepperDomeStpPin, stepperDomeDirPin, stepperDomeSlpPin, hallSensorDome);		// dome stepper go to home posisition
	stepperDomeOneStepHalfPeriod(10);

	// LOW ON DOME DIR PIN MEANS CW MOVEMENT AND HIGHER VALUE for stepCountDome -- ALWAYS INCREMENT FROM HERE
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
	int finalPosition = targetPosition - currentDomePosition;
	Serial.printf("StprFxns: moveDome (Abs): preparing to move, CurPos: %d | TarPos: %d | PosChange: %d \n", currentDomePosition, targetPosition, finalPosition);
	setDomeDirection(getSign(finalPosition));		//Sets dome direction CW or CCW
	finalPosition = abs(finalPosition);					// Arduino recommends not performing calculations inside the abs function,  
														// as you can a get strange results and errors
	stepperMove(abs(finalPosition), domeDefaultSpeed);

	Serial.printf("StprFxns: moveDome (Abs): Dome moved, current position is now %d \n", currentDomePosition);
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
	if (direction == CW)
	{
		setDomeDirection(CW);
	}
	else
	{
		setDomeDirection(CCW);
	}
	stepperMove(stepsToMove, domeDefaultSpeed);
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
		if (currentDomeDirection == CW && currentDomePosition < 400 ||
			currentDomeDirection == CCW && currentDomePosition > 0)
		{

			stepperDomeOneStepHalfPeriod(speed);
			Serial.printf("StprFxns: stepperMove: curDomePos: %d \n", currentDomePosition);

		}
		else
		{
			if (currentDomeDirection == CW) { Serial.printf("StprFxns: stepperMove: Can't move any further CW"); }
			else { Serial.printf("StprFxns: stepperMove: Can't move any further CCW"); }
			break;
		}
		
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

	currentDomePosition += currentDomeDirection;

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


/* S H O O T  F U N C T I O N S */
/*
void shootSingleSquare()
{
	int thisSquare = getSquareID(singleSquareData);
	int targetFlow = squareArray[thisSquare][2];
	int targetStep = squareArray[thisSquare][3];

	moveDome(targetStep);
	
	while (targetFlow < squareArray[thisSquare][4] || targetFlow > squareArray[thisSquare][5])
	{
		if (targetFlow < squareArray[thisSquare][4])
		{

		}
	}

}*/