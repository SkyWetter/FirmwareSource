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
#include "SPIFFSFunctions.h"

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
#define rgbLedBlue 26
#define rgbLedGreen 25
#define rgbLedRed 27

// solar panel
#define currentSense A6
#define solarPanelVoltage A7

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
#define rgbLedBlue 26
#define rgbLedGreen 25
#define rgbLedRed 27

// solar panel
#define currentSense A6
#define solarPanelVoltage A7


//TESTER HARDCODE VARIABLES

int curDomePos = 0;

void scheduledSprayRoutine()
{
	spiffsFlowPosRead();

	for (int i = 0; i < 500; i++)
	{
		if (sprayFlow[i] == 0 && sprayPos[i] == 0) {
			break;
		}
		else {

			moveToPosition(stepperDomeStpPin, sprayPos[i], 20, 99999, 0);
			makeRain(sprayFlow[i]);

		}

	}
}

// M A I N    F U N  C T I O N  --- STEPPER GO HOME
void stepperGoHome(byte x, byte y, byte z, byte s)                      // x STEP, y DIR, z EN, s HALL
{
	// SET stepper CW
	digitalWrite(z, HIGH);		//makesure sleep mode is off
	int stepcount = 0;
	int limit;

	if (x == 17) { limit = 125; }
	else { limit = 500; }
	//stepperDomeOneStepHalfPeriod(10);
	//stepperDomeOneStepHalfPeriod(10);
	//stepperDomeOneStepHalfPeriod(10);

	while (digitalRead(s) == 1)																// if hallSensor is HIGH the stepper is NOT at HOME
	{
		digitalWrite(x, HIGH);
		delay(20);
		digitalWrite(x, LOW);
		delay(20);
		stepcount++;
	}
	//Serial.print("current dome position");
	//Serial.println(currentDomePosition);
	Serial.print("current valve position");
	Serial.println(currentValvePosition);
	Serial.print("step count back to home was");
	Serial.println(stepcount);

	if (stepcount >= limit) {
		Serial.print("Help! I'm stuck and I cant get up");

		digitalWrite(y, !digitalRead(y));		//change direction and try other way		
		for (int i = 0; i < (limit/5); i++) {
			digitalWrite(x, HIGH);
			delay(4);
			digitalWrite(x, LOW);
			delay(4);
		}
	}
	//digitalWrite(z, HIGH);																	// put stepper back to sleep
	//digitalWrite(y, LOW);	
	// SET STEOPP BACK TO CCW

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
	//SerialBT.println("dome go home");                                           // LOW IS COUNTERCLOCKWISE
}
void valveGoHome()
{
	digitalWrite(stepperValveDirPin, HIGH);		// low is home direction

	stepperGoHome(stepperValveStpPin, stepperValveDirPin, stepperValveSlpPin, hallSensorValve);
	//digitalWrite(stepperValveDirPin, HIGH);
	currentValvePosition = 0;
	digitalWrite(stepperValveSlpPin, LOW);	//turns the valve stepper off after completing a go home
	//SerialBT.println("valve go home");
	//currentValvePosition = 0;
}

//Move valve to flow Function


// Make Rain -- (float)Flow -> 
//
// Takes a desired flow level

void makeRain(float desiredFlow)
{
	
	//valveGoHome();

	digitalWrite(stepperValveSlpPin, HIGH);
	digitalWrite(stepperValveDirPin, LOW);

	for(int i=0; i < 0; i++)		//this compensates for the home position of valve being about 10 steps from hall closed state
	{
		//Serial.println("taking extra steps");
		digitalWrite(stepperValveStpPin, HIGH);
		delay(4);
		//delay(100);
		digitalWrite(stepperValveStpPin, LOW);
		delay(4);
	}

	

	Serial.println("entered makeRain");
	//double desiredFreq = 30;
	//double desiredFreq = 1000000.0 / (double)desiredFlow;
	float desiredFreq = desiredFlow;
	

	//Serial.println(desiredFreq);
	//Serial.println(desiredFreq);

	int fastTime;
	int stepTime;

	int accel = valveStepperDefaults[1];
	int speed = valveStepperDefaults[0];

	speed = map(desiredFreq, 1, 50, 10, speed);

	fastTime = 1000 /speed * 1000;			//multipy because now using microseconds
	stepTime = fastTime * 2;
	////Serial.println("compensated speed is");
	Serial.println(fastTime);
	Serial.println(currentValvePosition);

	while (desiredFreq >= freq+ 0.5 && currentValvePosition <= 80 || desiredFreq <= freq - 0.5)
	{
		Serial.println("doing the while loop");

		setCurrent(stepperValveCrntPin, valveStepperDefaults[2]);

		if (freq <= desiredFreq) { currentValveDirection = LOW; }
		else { currentValveDirection = HIGH; }
		
			digitalWrite(stepperValveDirPin, currentValveDirection);

			digitalWrite(stepperValveStpPin, HIGH);
			delayMicroseconds(stepTime);
			digitalWrite(rgbLedRed, HIGH);
			//delay(100);
			digitalWrite(stepperValveStpPin, LOW);
			delayMicroseconds(stepTime);
			digitalWrite(rgbLedRed, LOW);
			//delay(100);

			//Serial.println("compensated speed iin stepping else is");
			//Serial.println(stepTime);

			if (currentValveDirection == LOW) { currentValvePosition += 1;}
			else { currentValvePosition -= 1;}
			//Serial.println(accel);
			if (stepTime != fastTime) {
				//Serial.println("accelerating");
				stepTime -= accel;
				if (stepTime < fastTime) {
					stepTime = fastTime;
				}		
			}

			//printf("desiredFreq is %f \n", desiredFreq);
//Serial.println(desiredFreq);
//Serial.println(currentValvePosition);
//Serial.println(fastTime);
	//Serial.println(stepTime);
	}
	Serial.println("Steps taken to satisfy flow");
	Serial.println(currentValvePosition);

	Serial.println("the frequency at end of makeRain was");
	Serial.println(freq);
	Serial.println("the desired frequency at end of makeRain was");
	Serial.println(desiredFreq);

	if (currentValvePosition >= 80) 
	{ 
		delay(1000);
		valveGoHome();
		Serial.println("valve overrun went home");
	
	}
	digitalWrite(stepperValveSlpPin, LOW);


}


// M O V E  D O M E  F U N C T I O N S 

//Move dome to a given position (a position is defined as a number of steps away from the home position)
void moveDome(int targetPosition)
{
	targetPosition -= currentDomePosition;   //determines the number of steps from current position to target position
	setDomeDirection(getSign(targetPosition)); //Sets dome direction CW or CCW

}



void moveDome(int dir, int steps)
{
		
}

//general move a gat dang shtepper
void moveToPosition(int stepperpin, int targetPosition, int speed, int accel, int current)
{
	digitalWrite(rgbLedGreen, HIGH);

	if (396 > targetPosition) {
		switch (stepperpin)
		{
		case stepperDomeStpPin:

			//Serial.println("switch case for domeStepper");
			//Serial.println(targetPosition);
			//Serial.println(currentDomePosition);
			printf("move starting at %i", currentDomePosition);
			
			int stepsTaken = 0;

			digitalWrite(stepperDomeSlpPin, HIGH);

			//if statement below checks to see if the dome is supposed to home but isnt
			if (currentDomePosition == 0 && digitalRead(hallSensorDome) == 1 ) {

				digitalWrite(stepperDomeDirPin, 0);

				while (digitalRead(hallSensorDome) == 1 && stepsTaken < 500) {

					

					digitalWrite(stepperDomeStpPin, HIGH);
					delay(5);

					digitalWrite(stepperDomeStpPin, LOW);
					delay(5);

					stepsTaken++;
				}

				for (int i; i < 2; i++) {	//extra steps home incase of early hall
					digitalWrite(stepperDomeStpPin, HIGH);
					delay(5);

					digitalWrite(stepperDomeStpPin, LOW);
					delay(5);

					stepsTaken++;

				}

				if (stepsTaken >= 500) {
					Serial.print("Help! I'm stuck and I cant get up");

					digitalWrite(stepperDomeDirPin, !digitalRead(stepperDomeDirPin));		//change direction and try other way		
					for (int i = 0; i < (500 / 5); i++) {
						digitalWrite(stepperDomeStpPin, HIGH);
						delay(4);
						digitalWrite(stepperDomeStpPin, LOW);
						delay(4);
					}
				}

				digitalWrite(stepperDomeStpPin, HIGH);		//extra step to hit home
				delay(10);

				digitalWrite(stepperDomeStpPin, LOW);
				delay(10);

				currentDomePosition = 0;
				
				Serial.println("taking corrective steps home\n");
			}


			stepsTaken = 0;
			int stepsToGo = targetPosition - currentDomePosition;   //determines the number of steps from current position to target position
			if (getSign(stepsToGo) == 1) { currentDomeDirection = 1; }
			else { currentDomeDirection = 0; }

			digitalWrite(stepperDomeDirPin, currentDomeDirection);
			stepsToGo = abs(stepsToGo);


			if (accel == 0) { accel = domeStepperDefaults[1]; }
			//accel = 100 / accel;
			int  decelUnit = 0;

			if (speed == 0) { speed = domeStepperDefaults[0]; }
			int fastTime;
			fastTime = 1000 / speed * 1000;
			int stepTime;
			stepTime = fastTime * 2;

			if (current == 0) { current = domeStepperDefaults[2]; }		//if it gets a passed 0 use default current
			setCurrent(stepperDomeCrntPin, current);


			
			
			while (currentDomePosition != targetPosition && stepsTaken < 500) {

				digitalWrite(stepperDomeStpPin, HIGH);
				delayMicroseconds(stepTime);
				digitalWrite(stepperDomeStpPin, LOW);
				delayMicroseconds(stepTime);

				if (currentDomeDirection == 1) { currentDomePosition += 1; stepsTaken += 1; }
				else { currentDomePosition -= 1; stepsTaken += 1; }

				if (stepTime != fastTime && stepsToGo - stepsTaken >= decelUnit) {

					stepTime -= accel;
					if (stepTime < fastTime) {
						stepTime = fastTime;

					}
					decelUnit++;
				}

				if (stepsToGo - stepsTaken <= decelUnit) {
					stepTime += accel;
				}

				//if (currentDomeDirection == 0) {
					//if (digitalRead(hallSensorDome) == 0) {
						//currentDomePosition = targetPosition;
						//digitalWrite(stepperDomeSlpPin, LOW);
					//}
				//}

			}
			if (digitalRead(hallSensorDome) == 0 && currentDomePosition == 0) { digitalWrite(stepperDomeSlpPin, LOW); }	//flush the toilet AFTER YOUVE HAD A SHET
		printf("and took %i steps", stepsTaken);
		}
	}

	printf(" ending at %i\n", targetPosition);
	
	digitalWrite(rgbLedGreen, LOW);

}

void executeSquare(int mysquare) {



	int steps2go = squareArray[mysquare][3];

	float targetDist = squareArray[mysquare][2];


	Serial.println("Target flow frequency is");
	Serial.println(targetDist);


	moveToPosition(stepperDomeStpPin,squareArray[mysquare][3],0,0,0);
	delay(100);
	makeRain(targetDist);

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
	//Serial.println(currentValvePosition);
	//SerialBT.println(currentValvePosition);
	//digitalWrite(stepperValveEnPin, LOW);
}

void crazyDomeStepperFromDebugA() // F!Y!I! I TOOK A FUNCTION FROM THE SerialData.CPP debug case statement and moved it here
{
	moveToPosition(stepperDomeStpPin, 10, 0, 0, 0);
	delay(500);		// if active dome count incorrect
	domeGoHome();
	delay(500);		// if active dome count incorrect
	moveToPosition(stepperDomeStpPin, 20, 0, 0, 0);
	delay(500);		// if active dome count incorrect
	//domeGoHome();
	//delay(500);		// if active dome count incorrect
	//moveToPosition(stepperDomeStpPin, 30, 0, 0, 0);
	//delay(500);		// if active dome count incorrect
	//domeGoHome();
	//delay(500);		// if active dome count incorrect
	moveToPosition(stepperDomeStpPin, 40, 0, 0, 0);
	delay(500);		// if active dome count incorrect
	domeGoHome();
	delay(500);		// if active dome count incorrect
	moveToPosition(stepperDomeStpPin, 50, 0, 0, 0);
	delay(500);		// if active dome count incorrect
	domeGoHome();
	delay(500);		// if active dome count incorrect
	moveToPosition(stepperDomeStpPin, 100, 0, 0, 0);
	//delay(500);		// if active dome count incorrect
	//domeGoHome();
	delay(500);		// if active dome count incorrect														// if active dome count incorrect
	moveToPosition(stepperDomeStpPin, 150, 0, 0, 0);
	delay(500);		// if active dome count incorrect
	moveToPosition(stepperDomeStpPin, 20, 0, 0, 0);
	delay(500);		// if active dome count incorrect														// if active dome count incorrect
	moveToPosition(stepperDomeStpPin, 150, 0, 0, 0);
	delay(500);		// if active dome count incorrect
	moveToPosition(stepperDomeStpPin, 20, 0, 0, 0);
	delay(500);		// if active dome count incorrect														// if active dome count incorrect
	moveToPosition(stepperDomeStpPin, 150, 0, 0, 0);
	delay(500);		// if active dome count incorrect
	moveToPosition(stepperDomeStpPin, 20, 0, 0, 0);
}
