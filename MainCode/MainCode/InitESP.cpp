// *********   P R E P R O C E S S O R S
// standard library includes
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

// ********* P I N   A S S I G N M E N T S
// flow meter
#define PCNT_TEST_UNIT      PCNT_UNIT_0
#define PCNT_H_LIM_VAL      10
#define PCNT_L_LIM_VAL     -10
#define pulsePin GPIO_NUM_23  // Pulse Input GPIO

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
#define rgbLedBlue 26
#define rgbLedGreen 25
#define rgbLedRed 27

// solar panel
#define currentSense A6
#define solarPanelVoltage A7

int serialBaud = 115200;

float getFlow(int column, int row, int turretColumn, int turretRow);
int convertAngleToStep(double angle);
double angleToSquare(int sqCol, int sqRow, int turCol, int turRow);

void initRainBow()
{
	initPins();
	initSerial();			//  serial monitor only ===> DOES NOT DO BLUETOOTH ANYMORE
	initThreads();

	createSquareArray(25);
	spiffsBegin();

	Serial.print("RainBow Initialized at ... ");
	printLocalTime();
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
	//pinMode(pulsePin, INPUT);               // pin to read pulse frequency                    // init timers need for pulseCounters
	gpio_pad_select_gpio(pulsePin);
	gpio_set_direction(pulsePin, GPIO_MODE_INPUT);

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

void initSerial()
{
	Serial.begin(serialBaud);
	Serial.printf("Serial Intialized with %d baud rate", serialBaud);
}

void initThreads()
{
	//multiple threads
	TaskHandle_t Task1;				//creating the handle for Task1

	xTaskCreatePinnedToCore(		//creating Task1 and pinning it to core 0
		codeForTask1,
		"Task1",
		1000,
		NULL,
		1,
		&Task1,                   /* Task handle to keep track of created task */
		1);                       /* Core */
}

void initSerialBT()
{
	int chipid = ESP.getEfuseMac();

	//switch (chipid) 
	switch (chipid)
	{
	case 163842596:
		SerialBT.begin("PCB Version sick name bro");
		break;

	default:
		SerialBT.begin("ANDY->ESP HIHIHI");
	}

	ledBlue(1);

	delay(100);

	Serial.println("");

	Serial.print("RainBow Bluetooth Serial Initialized...");
	Serial.println(chipid);
	SerialBT.println("RainBow Bluetooth Serial Initialized...");
	SerialBT.println(chipid);
}

void checkWakeUpReason()
{
	esp_sleep_wakeup_cause_t wakeup_reason;
	wakeup_reason = esp_sleep_get_wakeup_cause();

	switch (wakeup_reason)
	{

	case 1:
		Serial.println("Wakeup caused by external signal using RTC_IO");
		// if we are here its because there was a wake-up push button event
		// init programstate.. thats BT.. 
		programState();
		// goto to sleep when done
		break;

	case 2:
		Serial.println("Wakeup caused by external signal using RTC_CNTL");
		break;

	case 3:
		Serial.println("Wakeup caused by timer");
		//timer event wakeup happens every 60s
		// run wakeUpTimerStateMachine
		// case 0: low battery ?
		// case 1: watering time ?
		// case 2: currentSense ?
		timerState();
		break;

	case 4:
		Serial.println("Wakeup caused by touchpad");
		break;

	case 5:
		Serial.println("Wakeup caused by ULP program");
		break;

	default:
		Serial.printf("Wakeup was not caused by deep sleep -> Entering program state... %d\n", wakeup_reason);
		programState();
		break;
	}
}


void codeForTask1(void * parameter)						//speecial code for task1
{
	while (1)
	{
		TIMERG0.wdt_wprotect = TIMG_WDT_WKEY_VALUE;		//feed the watchdoggy
		TIMERG0.wdt_feed = 1;
		TIMERG0.wdt_wprotect = 0;

		doPulseIn();
	}
}
// M A I N   F U N C T I O N -- CREATE SQUARE ARRAY
/*
* Initializes the array of squares on startup
* squareArray[squareID][{x,y,
,angle}]
* Each position in the first dimension of array is a given square id
*
*/

void createSquareArray(int squaresPerRow)
{
	int squareID = 624;
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
			squareArray[squareID][2] = getFlow(column, row, turretColumn, turretRow);
			//printf("flow requred to hit target %f\n", squareArray[squareID][2]);

			// Calculates # of steps taken from home needed to make turret face square
			squareArray[squareID][3] = convertAngleToStep(angleToSquare(column, row, turretColumn, turretRow));
			//Serial.println(squareArray[squareID][3]);

			squareID -= 1;
			//printf("Square id: %d\n", squareID);
		}
	}
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
	// one square is equal to 0.5 meters and our flow data is measured in meters so 2x one suare = 1 meter
	//x = x;					
	//y = y;

	//printf(" in DistanceToSquare column number is %i\n", sqCol);
	//printf(" in DistanceToSquare turret column number is %i\n", turCol);
	//printf(" in DistanceToSquare row number is %i\n", sqRow);
	//printf(" in DistanceToSquare urent row number is %i\n", turRow);

	//printf(" in DistanceToSquare int x is %i\n", x);
	//printf(" in DistanceToSquareint y number is %i\n", y);


	int squareCoords = (x * x) + (y * y);

	//printf(" in DistanceToSquaresquare coords are %i\n", squareCoords);

	float squareDistance = sqrt(squareCoords);
	//printf(" in DistanceToSquare square distance is%f\n", squareDistance);

	return squareDistance/2;
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
		//Serial.println(stepsDouble);
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


float getFlow(int column, int row, int turretColumn, int turretRow)
{


	float squareDistance = distanceToSquare(column, row, turretColumn, turretRow);   //Gets the distance from target square to the central turret square
	//double flow = 99857.81 - 23136.9*squareDistance + 1636.316*pow(squareDistance, 2); //Converts the distance value to flow (2nd-Order Polynomial)
	
	//printf(" in getFlow square distance prew math is are %f\n", squareDistance);
	
	float flow = squareDistance * 4.54 + 1.69;

	//printf(" in getFlow calculated distance is %f\n", squareDistance);
	//printf(" in getFlow calculated flow is %f\n", flow);


	return flow;

	// ** Other equations describing our curve **
	// 4PL has the best R^2 value, but might be difficult to tweak.

	//  4PL  
	//  double y = 6944.746 + (94311.32 - 6944.746)/(1 + pow(x/2.473604,1.825548));

	//  3rd-Order Polynomial 
	//  double y = 107475.7 - 31263.28*x + 3826.97*pow(x,2) - 168.5722*pow(x,3);

}
