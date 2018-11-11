// **********   S * K * Y  |)  W * E * T *
//  -=-=-=-=-=-=-=-=-=-=-=-=-
// turret control firmware for esp32 dev kit C
//  october 31, 2018


//askdjhsadhiuhehfka error cannot computer test test


// *********   P R E P R O C E S S O R S
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
#include <driver/adc.h>

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




// ********    P R O T O T Y P E S
void debugInputCase(char);

void solarPowerTracker();

void stepperOneStepHalfPeriod(byte x, byte y, byte z, int *q, int h);
void stepperDomeOneStepHalfPeriod(int hf);
void stepperValveOneStepHalfPeriod(int hf);
void stepperGoHome(byte x, byte y, byte z, byte s);
void domeGoHome();
void valveGoHome();

void stepperDomeDirCW();
void stepperDomeDirCCW();

void toggleStepperValveDir();
void valveStepperOneStep();

void getSerialData();
void checkPacketNumber(char *singleSquareData[]);
void checkChecksum(char *singleSquareData[]);
int charToInt(char *thisChar, int thisCharLength);
int getFlow(int column, int row, int turretColumn, int turretRow);
double angleToSquare(int sqCol, int sqRow, int turCol, int turRow);
int convertAngleToStep(double angle);

void setup()
{
	initESP();  // Configures inputs and outputs/pin assignments, serial baud rate,
				// starting systemState (see InitESP.cpp)
	Serial.println("ESP Initialized...");
	domeGoHome(); 

}


void loop()
{

	//state machine

	//interupt: if low battery from BMS -> state = lowPower
	//interupt: if pushbutton depressed -> state = program
	
	switch (systemState)
	{
		case sleeping:
		{
			
			if (SerialBT.available() || Serial.available())
			{
				Serial.println("Changing to program State");
				systemState = program;

			}
	
			break;
		}

		case solar:
		{
			
			solarPowerTracker();

		
			systemState = sleeping;

			break;
		}

		case program:
		{
		
			getSerialData();
			

			
			systemState = sleeping;

			break;
		}

		case water:
		{
			//load correct instruction set for date and time
			//reference temperature and apply modfifier to watering durations
			//open thread for flow sensor
			//run spray program
			if (systemState_previous != systemState)
			{
				Serial.printf("SystemState: Watering Mode");
			}

			
			systemState_previous = systemState;

			break;
		}

		case low_power:
		{
			//close the valve
			//set LED to red
			//allow solar
			//prevent water until battery > 50%
			  //>50% -> perform last spray cycle
			if (systemState_previous != systemState)
			{
				Serial.printf("SystemState: Low Power Mode");
			}

			systemState_previous = systemState;

			break;
		}
	}
}

void realTimeClock()
{

}

// M A I N   F U N C T I O N   ---- SOLAR POWER TRACKING
void solarPowerTracker()
{
	int peakInsolationSteps = 0;
	static int stepDivider = 20;		//sets number of steps between each measurement. Must divide evenly in to 400

	long delayTimer1, delayTimer2;
	long currentSenseVal1 = 0;
	long currentSenseVal2 = 0;

	bool delayComplete1 = false;
	bool delayComplete2 = false;


	Serial.println("Solar tracking begins");

	domeGoHome();     //send turret to zero position

	while (delayComplete1 == false)
	{

		if (digitalRead(hallSensorDome) == LOW)		//Wait until dome returns home -- CHECK IF ACTIVE HIGH
		{
			Serial.println("Dome has returned to home");

			delayComplete1 = true;    //end loop
			delayTimer1 = 0;      //set timers
			delayTimer2 = 0;


			delay(5);

			//currentSenseVal1 = analogRead(currentSense);
	
			for (int y = 0; y < 100; y++)
			{
				currentSenseVal1 += adc1_get_raw(ADC1_CHANNEL_6);	//take next voltage reading
				//delay(1);
				//Serial.printf("current reading is %i \n", currentSenseVal2);
			}
			
			currentSenseVal1 = currentSenseVal1 / 10;

			//currentSenseVal1 = analogRead(currentSense);		//take first voltage reading at zero position

			currentSenseVal2 = 0;

			stepperDomeDirCW();   //set rotation to clockwise 

			Serial.printf("First reading is %i \n", currentSenseVal1);

			for (int i = 1; i <= 400 / stepDivider; i++)    //divide rotation in x number of steps
			{
				for (int x = 0; x < stepDivider; x++)
				{
					stepperDomeOneStepHalfPeriod(6);		//take steps number is the speed

				}

				delayTimer1 = millis();   //start timer

				while (delayComplete2 == false)
				{

					if (delayTimer2 >= delayTimer1+10)			//10ms delay per step
					{
						delayComplete2 = true;
						
						currentSenseVal2 = 0;

						for (int y = 0; y < 100; y++)
						{
							currentSenseVal2 += adc1_get_raw(ADC1_CHANNEL_6);	//take next voltage reading
							//delay(1);
							//Serial.printf("current reading is %i \n", currentSenseVal2);
						}

						currentSenseVal2 = currentSenseVal2 / 10;
						
						Serial.printf("current reading is %i \n", currentSenseVal2);


						if (currentSenseVal1 < currentSenseVal2)
						{
							currentSenseVal1 = currentSenseVal2;  //if it is greater, save new reading
							peakInsolationSteps = i * stepDivider;    //number of steps taken to reach position
							delayTimer1 = 0;
							delayTimer2 = 0;

							Serial.printf("New highest reading is %i \n", currentSenseVal1);
							Serial.printf("Position is %i steps from zero \n", peakInsolationSteps);
						}
					}

					else
					{
						delayTimer2 = millis();   //increment timer if limit not reached
					}
				}

				delayComplete2 = false;
			}

			Serial.printf("Final highest reading reading is %i \n", currentSenseVal1);
			Serial.printf("Final position is %i steps back from 400 \n", 400 - peakInsolationSteps);

			stepperDomeDirCCW();    //set rotation to ccw

			delayComplete2 = false;   //reset timers
			delayTimer1 = millis();
			delayTimer2 = 0;

			for (int i = 0; i < 400 - peakInsolationSteps; i++)   //return to point of peak insolation
			{
				stepperDomeOneStepHalfPeriod(5);
			}

			while (delayComplete2 == false)
			{
				if (delayTimer2 >= delayTimer1 + 2000)    //wait 2s to arrive, then exit function
				{
					delayComplete2 = true;

					Serial.println("Exiting solar tracker");
				}

				else
				{
					delayTimer2 = millis();   //increment timer if limit not reached
				}
			}
		}
	}
}



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
	targetPosition -= currentDomePosition;   //determines the number of steps from current position to target position
	setDomeDirection(getSign(targetPosition)); //Sets dome direction CW or CCW


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

void displaySolarVoltage()
{
	solarPanelVoltageVal = (float)analogRead(solarPanelVoltage);
	delay(1);
	solarPanelVoltageVal = (((solarPanelVoltageVal / 4096) * 3.3) * 4.2448);

	Serial.print("solar panel voltage: ");
	Serial.print(solarPanelVoltageVal);
	Serial.println("V");
	SerialBT.print("solar panel voltage: ");
	SerialBT.print(solarPanelVoltageVal);
	SerialBT.println("V");
}

void displaySolarCurrent()
{
	long currentSenseVal1 = 0;

	currentSenseVal1 = analogRead(currentSense);

	Serial.print("current val: ");
	Serial.println(currentSenseVal1);
	SerialBT.print("current val: ");
	SerialBT.println(currentSenseVal1);
}

void doPulseIn()
{
	//Pulse IN shit
	//changes for example
		duration = float(pulseIn(pulsePin, HIGH));

	//SerialBT.println(duration);

}

// M A I N F U N C T I O N  -- Get Serial Data
/*
*
* Run in loop or state to look for incoming bluetooth serial data and handle it.
*
*/


void getSerialData()
{

	if (SerialBT.available())     //If there is some data waiting in the buffer
	{
		char incomingChar = SerialBT.read();  //Read a single byte

		switch (incomingChar) 
		{
			case '*': 
				Serial.println("Resent from android");  //Debug message to alert through serial monitor that data has been resent on ESP request
				break;

			case '%':
				serialState = singleSquare;  //Puts serialState in singleSquare mode 

				//Grabs a 10-byte single square packet from the serial buffer
				for (int i = 0; i < 9; i++)  //
				{

					incomingChar = SerialBT.read();
					if (incomingChar == ' ' || incomingChar == NULL)
					{
						Serial.printf("incoming char was an illegal character \n");
						singleSquareData[i + 1] = '@';
					}
					else
					{
						singleSquareData[i + 1] = incomingChar;
					}
				}
				break;

			case '&':
				serialState = debugCommand;
				break;
		}
		
	}

	// Check the serial state 

	switch (serialState)
	{
		case doNothing: break;

		case singleSquare:       //If in singleSquare mode
			checkPacketNumber(&singleSquareData[0]); //Check the packet number

			// Check Packet State
			switch (squarePacketState)
			{
				case ok: checkChecksum(&singleSquareData[0]); break;  //if packet is ok, check the checksum
				case ignore: break;                   //do nothing if packet number is old or same as previous successful rx        
				case resend: SerialBT.write(lastSquarePacketNumber == 999 ? 0 : lastSquarePacketNumber + 1); break; //Request a missed packet
			}

			//Check checksum state
			switch (squareChecksumState)
			{
				case ok: getSquareID(&singleSquareData[0]); //If checksum is fine, move turret
					message = false;;
					serialState = doNothing;
					squareChecksumState = ignore;
					squarePacketState = ignore;
					squareIDInt = charToInt(squareID, 3);

					break; 
				case ignore: break;
				case resend: SerialBT.write(lastSquarePacketNumber); //If checksum is incorrect, request the same packet from the app
			}

			break;
		
		case debugCommand:

			debugInputParse(getDebugChar());

			break;

		default:;
	}

}



// S U B F U N C T I O N S -- getSerialData

// GET SQUARE ID -- gets the id of a single square from 10-byte packet
int getSquareID(char singleSquaredata[])
{
	
	char thisSquareChar[3];

	for (int i = 0; i < 3; i++)
	{
		thisSquareChar[i] = singleSquareData[i + 4];
	}

	return charToInt(thisSquareChar,3);

	
}

//CHECK PACKET NUMBER

//Check packet number, changes packetState

void checkPacketNumber(char singleSquareData[])
{

	//Checks for error character '@', if found, exits out of formatSingleSquareData
	for (int i = 0; i < 10; i++)
	{
		if (singleSquareData[i] == '@')
		{
			return;
		}
	}

	//Pulls packet# value for singleSquareData number
	for (int j = 0; j < 3; j++)
	{
		squarePacketNumberChar[j] = singleSquareData[j + 1];
	}

	squarePacketNumberInt = charToInt(squarePacketNumberChar, 3);


	//If this is the first square rx'd, assume its the right packet number, or an inc of last packet (including rollover)
	if (squarePacketNumberInt == lastSquarePacketNumber + 1 || firstSingleSquare || (squarePacketNumberInt == 0 && lastSquarePacketNumber == 999))
	{
		//Set this packet to last packet number and set packetState
		lastSquarePacketNumber = squarePacketNumberInt;
		squarePacketState = ok;
		repeatPacketReceived = false;
		firstSingleSquare = false; //First single square no longer in effect
		printf("checkPacketNumber: state ok: lastpacket = %d \n", lastSquarePacketNumber);
	}

	//If this packet is old (already received) or the same as the last packet, ignore it
	else if (squarePacketNumberInt <= lastSquarePacketNumber)
	{
		//Ignore this packet
		if (!repeatPacketReceived)
		{
			repeatPacketReceived = true;
		}

		squarePacketState = ignore;
	}

	//If packet received is out of sequence, request resend
	else if (lastSquarePacketNumber == 999 && squarePacketNumberInt != 0 || squarePacketNumberInt > lastSquarePacketNumber + 1)
	{
		repeatPacketReceived = false;
		squarePacketState = resend;
	}

}



//CHECK CHECKSUM
//Checks ESP-calculted checksum against rx'd android checksum value, changes checksumState

void checkChecksum(char singleSquareData[])
{
	int espChecksum = 0;      //Value of checksum calculated on esp side
	int androidChecksum = 0;  //Value of checksum sent by android

	for (int i = 0; i < 3; i++)
	{
		squareChecksumChar[i] = singleSquareData[i + 7]; //Get checksum substring from data packet
		espChecksum += singleSquareData[i + 4];     //Esp calculate checksum  

	}

	androidChecksum = charToInt(squareChecksumChar, 3);// Convert checksum substring to int

	if (androidChecksum == espChecksum) //If they match, allow the packet data to be used
	{
		squareChecksumState = ok;
		
	}

	else
	{
		squareChecksumState = resend;  //Otherwise, resend this packet
	}

}

char getDebugChar()
{
	if (Serial.available())
	{
		return Serial.read();
	}

	else
	{
		return '|'; //Returns pipe symbol if the serial monitor no longer has data in it
	}
}

// M A I N  F U N C T I O N -- SHOOT SINGLE SQUARE

void shootSingleSquare()
{
	int targetFlow = squareArray[getSquareID(singleSquareData)][2];
	int targetStep = squareArray[getSquareID(singleSquareData)][3];

	
	
	

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
			squareArray[squareID][2] = getFlow(column, row, turretColumn, turretRow);

			// Calculates # of steps taken from home needed to make turret face square
			squareArray[squareID][3] = convertAngleToStep(angleToSquare(column, row, turretColumn, turretRow));
			Serial.println(squareArray[squareID][3]);

			squareID += 1;
			printf("Square id: %d\n", squareID);
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

	int squareCoords = (x * x) + (y * y);

	return sqrt((double)squareCoords);
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
		Serial.println(stepsDouble);
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


int getFlow(int column, int row, int turretColumn, int turretRow)
{

	double squareDistance = distanceToSquare(column, row, turretColumn, turretRow);   //Gets the distance from target square to the central turret square
	double flow = 99857.81 - 23136.9*squareDistance + 1636.316*pow(squareDistance, 2); //Converts the distance value to flow (2nd-Order Polynomial)

	return (int)flow;

	// ** Other equations describing our curve **
	// 4PL has the best R^2 value, but might be difficult to tweak.

	//  4PL  
	//  double y = 6944.746 + (94311.32 - 6944.746)/(1 + pow(x/2.473604,1.825548));

	//  3rd-Order Polynomial 
	//  double y = 107475.7 - 31263.28*x + 3826.97*pow(x,2) - 168.5722*pow(x,3);

}




// M I S C   F U N C T I O N S

// CHAR TO INT 
/*
* Takes a char array, and the given length of the array
* and returns
*/

int charToInt(char *thisChar, int thisCharLength)
{

	int intConversion = 0;
	for (int i = 0; i < thisCharLength; i++)
	{
		intConversion += ((thisChar[i] - '0') * pow(10, thisCharLength - 1 - i));
	}

	return intConversion;
}

/* Get Sign
 *
 *  takes an int, returns sign (-1 = negative, 1 = positive, 0 = neither)
 *
 */

int getSign(int x)
{
	if (x > 0) { return 1; }
	else if (x < 0) { return -1; }
	else { return 0; }
}

// M A I N   F U N C T I O N --- inputCase statement


void debugInputParse(char debugCommand)
{     // read the incoming byte:

	switch (debugCommand)
	{

	case '0':                             // send dome stepper to home posistion
		domeGoHome();
		break;

	case '1':                             // send vavle stepper to home posisiton
		valveGoHome();
		break;

	case 'a':
		//10 steps on dome stepper
		for (int i = 0; i < 10; i++)
		{
			stepperDomeOneStepHalfPeriod(5);
		}

		Serial.println(stepCountDome);
		SerialBT.println(stepCountDome);
		break;

	case 'b':
		// set dome stepper to CW ---> HIGH IS CLOSEWISE!!!
		stepperDomeDirCW();
		break;

	case 'c':
		// set dome stepper to CW ---> LOW IS COUNTER CLOCKWISE!!!
		stepperDomeDirCCW();
		break;

	case 'd':
		//one step on valveStepper
		valveStepperOneStep();
		break;

	case 'e':
		toggleStepperValveDir();
		break;

	case 'f':

		// panel shit
		displaySolarCurrent();
		displaySolarVoltage();
		break;

	case 'g':
		solarPowerTracker();
		break;

	case 'h':
		doPulseIn();
		break;

	case 's':
		//esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
		esp_deep_sleep_start();
		break;

	case 't':
		gettimeofday(&now, NULL);

		SerialBT.println(now.tv_sec);
		SerialBT.println(last);

		last = now.tv_sec;
		break;

	}
}

