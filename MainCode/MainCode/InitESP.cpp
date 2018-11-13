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
#include "driver\adc.h"
#include "GeneralFunctions.h"

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

int getFlow(int column, int row, int turretColumn, int turretRow, int squareID);
int convertAngleToStep(double angle);
double angleToSquare(int sqCol, int sqRow, int turCol, int turRow);
void createSquareArray(int squaresPerRow);
double distanceToSquare(int sqCol, int sqRow, int turCol, int turRow);

void initESP()
{

	initSerial();
	initPins();
	createSquareArray(SQUARES_PER_ROW);
	systemState = sleeping;
	systemState_previous = water;
	// power management
	esp_sleep_enable_ext0_wakeup(GPIO_NUM_13, 1);
}

void initSerial()
{
	SerialBT.begin("ESP_Dave");
	Serial.begin(serialBaud);
	
	Serial.printf("Serial Intialized with %d baud rate", serialBaud);
}

void initPins()
{
	adc1_config_width(ADC_WIDTH_BIT_12);
	adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_6);

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

// M A I N   F U N C T I O N -- CREATE SQUARE ARRAY
/*
* Initializes the array of squares on startup
* squareArray[squareID][{x,y,distance,angle}]
* Each position in the first dimension of array is a given square id
*
*/

void createSquareArray(int squaresPerRow)
{
	int squareID = 0;
	int turretColumn = (squaresPerRow - 1) / 2;
	int turretRow = turretColumn;

	for (int row = 0; row < squaresPerRow; row++)
	{
		for (int column = 0; column < squaresPerRow; column++)
		{
			//Store col (x) and row (y) data for this square
			squareArray[squareID][0] = column;
			squareArray[squareID][1] = row;

			// Calculates flow needed to reach this square, stores it in array
			squareArray[squareID][2] = getFlow(column, row, turretColumn, turretRow,squareID);

			// Calculates # of steps taken from home needed to make turret face square
			squareArray[squareID][3] = convertAngleToStep(angleToSquare(column, row, turretColumn, turretRow));
			
			double myDistance = distanceToSquare(column, row, turretColumn, turretRow);
			Serial.printf("InitESP: CreateSquareArray: || SquareID: %d | Dist: %f | Flow: %d | lowFlow: %d | highFlow: %d \n",
				squareID,myDistance,squareArray[squareID][2],squareArray[squareID][4],squareArray[squareID][5]);

			squareID += 1;
			
		}
	}

	//Serial.printf("SquareID: %d", squareID);
}

// S U B   F U N C T I O N -- distance to square
/*
* Finds distance between two sets of x,y coordinates
* Returns a double
*/

double distanceToSquare(int sqCol, int sqRow, int turCol, int turRow)
{
	int x = sqCol - turCol;
	int y = sqRow - turRow;

	int squareCoords = (x * x) + (y * y);

	return sqrt((double)squareCoords) * metersPerSquare;
}

// S U B   F U N C T I O N -- angleToSquare
/*
* Finds the angle to a given target square relative to a second square
* give x/y coords, returns a double
*/

double angleToSquare(int sqCol, int sqRow, int turCol, int turRow)
{
	int x = sqCol - turCol;
	int y = sqRow - turRow;
	int quadrant = 0;

	if (x != 0 && y != 0) {

		if (getSign(x) == -1) {
			if (getSign(y) == -1) {
				quadrant = 4;
			}
			else {
				quadrant = 3;
			}
		}
		else {
			if (getSign(y) == -1) {
				quadrant = 1;
			}
			else {
				quadrant = 2;
			}
		}

	}
	else {

		quadrant = 0;

		if (x == 0 && y == 0) {
			return 999.0;
		}
		else if (x == 0) {
			if (getSign(y) == -1) {
				return 0.0;
			}
			else {
				return 180.0;
			}
		}
		else {
			if (getSign(x) == -1) {
				return 270.0;
			}
			else {
				return 90.0;
			}

		}
	}

	double temp = ((double)abs(x) / (double)abs(y));

	switch (quadrant) {
	case 1: return (atan(temp) * 180) / PI; break;
	case 2: return 180 - (atan(temp) * 180) / PI; break;
	case 3: return 180 + (atan(temp) * 180) / PI; break;
	case 4: return 360 - (atan(temp) * 180) / PI; break;
	}
}

// S U B   F U N C T I O N -- convertAngleToStep
/*
* takes angle (as double) and returns the value as a number of steps (int).
* Incorporates and rounds 3 sigfigs past the decimal point.
*/

int convertAngleToStep(double angle)
{
	double anglePerStep = 360.0 / STEPS_PER_FULL_TURN;  //Gets degrees of movement per step 

	if (angle > 400)   //If the turret angle of 999 is found, return same number
	{
		return 999;
	}

	else
	{
		double stepsDouble = angle / anglePerStep;   //Get number of steps to reach this angle starting from home)
		
		int steps = stepsDouble * 1000;

		for (int i = 0; i < 3; i++)   //Rounds the number (including 3 sigfigs after the decimal point)
		{
			if (steps % 10 > 4)
			{
				steps = (steps / 10) + 1;
			}
			else
			{
				steps = steps / 10;
			}
		}
		return steps;   //Return value;
	}
}

// S U B   F U N C T I O N -- Get Flow
/*
*
* Give a distance (x) and returns flow as int
* Currently truncates flow value, but shouldn't be a problem as the system
* isn't capable with that granular of a number in this case
*/


int getFlow(int column, int row, int turretColumn, int turretRow, int squareID)
{

	double squareDistance = distanceToSquare(column, row, turretColumn, turretRow);   //Gets the distance from target square to the central turret square
	double flow = 99857.81 - 23136.9*squareDistance + 1636.316*pow(squareDistance, 2); //Converts the distance value to flow (2nd-Order Polynomial)
	
	double lowDist = squareDistance * 0.98;
	double highDist = squareDistance * 1.02;

	squareArray[squareID][4] = 99857.81 - 23136.9*lowDist + 1636.316*pow(lowDist, 2); //Calc low tolerance for flow value
	squareArray[squareID][5] = 99857.81 - 23136.9*highDist + 1636.316*pow(highDist, 2);//Calc high tolerance for flow value
	return (int)flow;

	// ** Other equations describing our curve **
	// 4PL has the best R^2 value, but might be difficult to tweak.

	//  4PL  
	//  double y = 6944.746 + (94311.32 - 6944.746)/(1 + pow(x/2.473604,1.825548));

	//  3rd-Order Polynomial 
	//  double y = 107475.7 - 31263.28*x + 3826.97*pow(x,2) - 168.5722*pow(x,3);

}

