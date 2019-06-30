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
#define rgbLedGreen 25
#define rgbLedBlue 26
#define rgbLedRed 27


// solar panel
#define currentSense A6
#define solarPanelVoltage A7



//test using serial port for commented strings and array for file to open

char testNum1[] = "0001"; //#0001@0028!1,001!2,002!3,003
char testNum2[] = "0002"; //#0002@0022!2,333!3,123
char testNum3[] = "0003"; //#0003@0040!3,000!2,765!3,604!2,111!1,001

float tempVoltage = 5.0;

void getSerialData()
{
	
	if (SerialBT.available() || Serial.available())     //If there is some data waiting in the buffer
	{
		char incomingChar;
		

		if (SerialBT.available())
		{
			incomingChar = SerialBT.read();  //Read a single byte
			
		}
		else if (Serial.available())
		{
			incomingChar = Serial.read();
		}
		
		//Serial.println("incoming leading char is");
		//Serial.println((uint8_t)incomingChar);

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
					Serial.print((uint8_t)incomingChar);
					Serial.printf("incoming char was an illegal character \n");
					singleSquareData[i+1] = '@';
				}
				else
				{
					singleSquareData[i+1] = incomingChar;
				}

				//ignore
			}
		break;

		case '&':
			//Serial.println("debug command"); 
			//("debug command");
			serialState = debugCommand;
		break;

		case '#': 
			//header #0001@0028!data
			if (input2DArrayPosition < 14)
			{
				serialState = parseGarden;
			}
		break;

		case '$':
			if (bootCount == 0)
			{
				// get time
				timeShift();//Following a % timeshift() will parse time from a string in the format YYYYMMDDhhmmss . ex: 19840815042000 is 1984 august 15 04:20.00
				// maybe confirm that it has a good time???
			}
			++bootCount;
			Serial.println("time is money");SerialBT.println("time is money");	
			//Following a % timeshift() will parse time from a string in the format YYYYMMDDhhmmss . ex: 19840815042000 is 1984 august 15 04:20.00
			serialState = doNothing;
		break;
		}
		
	}

	// Check the serial state 
	switch (serialState)
	{
	case doNothing: 
	break;

	case singleSquare:											 //If in singleSquare mode
		checkPacketNumber(&singleSquareData[0]);				 //Check the packet number

		// Check Packet State
		switch (squarePacketState)
		{
		case ok: checkChecksum(&singleSquareData[0]); break;  //if packet is ok, check the checksum
		case ignore: break;                   //do nothing if packet number is old or same as previous successful rx        
		case resend:

			Serial.println("Asking for resend packet in square PacketState");
			
			//SerialBT.write(lastSquarePacketNumber == 999 ? 0 : lastSquarePacketNumber + 1);
			
			break; //Request a missed packet
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

			executeSquare(getSquareID(&singleSquareData[0]));
			delay(2000);
			valveGoHome();
			break;
		case ignore: break;
		case resend: break;
			//SerialBT.write(lastSquarePacketNumber); //If checksum is incorrect, request the same packet from the app
		}
	break;

	case debugCommand:
		//Serial.print("here in debug command");
		debugInputParse(getDebugChar());
	break;

	case parseGarden:

		if (input2DArrayPosition < 14)
		{
			parseInput();
		}
		break;

	default:;
	}
}


// S U B F U N C  T I O N S -- getSerialData
void parseInput()
{
	//Serial.println("test");


	int j = 11;
	int length;
	int packageNum;
	char headerArray[11] = { '#' };
	char lengthArray[4];
	char packageNumArray[4];

	//pull header array
	for (int i = 1; i < 11; i++)
	{
		if (Serial.available())
		{
			headerArray[i] = Serial.read();
		}
		else if(SerialBT.available())
		{
			headerArray[i] = SerialBT.read();
		}
	}

	//pull out string length
	for (int i = 0; i < 4; i++)
	{
		lengthArray[i] = headerArray[(i + 6)];
		packageNumArray[i] = headerArray[(i + 1)];
	}

	length = charToInt(lengthArray, 4);
	packageNum = charToInt(packageNumArray, 4);

	Serial.printf("packageNumArray = %i \n", packageNum);

	//create new array to match
	input2DArray[input2DArrayPosition] = new char[length];

	//replace header chars
	for (int i = 0; i < 11; i++)
	{
		input2DArray[input2DArrayPosition][i] = headerArray[i];
	}

	//pull rest of data - Serial
	while(Serial.available())
	{
		input2DArray[input2DArrayPosition][j] = Serial.read();
		j++;
	}

	//pull rest of data - Serial.BT
	while(SerialBT.available())
	{
		input2DArray[input2DArrayPosition][j] = SerialBT.read();
		j++;
	}

	//print entire string from array
	for (int i = 0; i < length; i++)
	{
		Serial.print(input2DArray[input2DArrayPosition][i]);
	}

	Serial.println();

	//
	spiffsSave(input2DArray[input2DArrayPosition], length, packageNumArray);


	Serial.println();
	Serial.printf("Length was %i, J count is %i \n", length, j);
	Serial.println("Nice");

	input2DArrayPosition++;

	Serial.printf("arrayPosition = %i \n", input2DArrayPosition);
}

// GET SQUARE ID -- gets the id of a single square from 10-byte packet
int getSquareID(char singleSquaredata[])
{

	char thisSquareChar[3];

	for (int i = 0; i < 3; i++)
	{
		thisSquareChar[i] = singleSquareData[i + 4];
	}

	return charToInt(thisSquareChar, 3);

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
			Serial.println("received @ in check packet number");
	
			return;
		}
	}

	//Pulls packet# value for singleSquareData number
	for (int j = 0; j < 3; j++)
	{
		squarePacketNumberChar[j] = singleSquareData[j + 1];
	}

	squarePacketNumberInt = charToInt(squarePacketNumberChar, 3);
	//Serial.println("packet number received is");
	//Serial.println(squarePacketNumberInt);
	//Serial.println("last packet number received is");
	//Serial.println(lastSquarePacketNumber);



	//If this is the first square rx'd, assume its the right packet number, or an inc of last packet (including rollover)
	
	
	//if (squarePacketNumberInt == lastSquarePacketNumber + 1 || firstSingleSquare || (squarePacketNumberInt == 0 && lastSquarePacketNumber == 999))
	//the line below is a temporary fix for what happens when a packet comes out of sequence
	if (squarePacketNumberInt != lastSquarePacketNumber  || firstSingleSquare || (squarePacketNumberInt == 0 && lastSquarePacketNumber == 999))

	{
		//Set this packet to last packet number and set packetState
		lastSquarePacketNumber = squarePacketNumberInt;
		squarePacketState = ok;
		repeatPacketReceived = false;
		firstSingleSquare = false; //First single square no longer in effect
		//printf("checkPacketNumber: state ok: lastpacket = %d \n", lastSquarePacketNumber);
	}

	//If this packet is old (already received) or the same as the last packet, ignore it
	else if (squarePacketNumberInt <= lastSquarePacketNumber)
	{

		//Serial.println("packet is old and already received");
		lastSquarePacketNumber = squarePacketNumberInt;     //If the phone app restarts and restarts packet count this will accept the new count

		//Ignore this packet
		if (!repeatPacketReceived)
		{
			//Serial.println("ignore this packet");
			repeatPacketReceived = true;
		}

		squarePacketState = ignore;
	}

	//If packet received is out of sequence, request resend
	else if (lastSquarePacketNumber == 999 && squarePacketNumberInt != 0 || squarePacketNumberInt > lastSquarePacketNumber + 1)
	{
		lastSquarePacketNumber = squarePacketNumberInt -1;		//this was put here to debug single square
	
		Serial.println("packet out of sequence");
		repeatPacketReceived = false;
		squarePacketState = resend;
	}

}

//CHECK CHECKSUM
//Checks ESP-calculted checksum against rx'd android checksum value, changes checksumState
void checkChecksum(char singleSquareData[])
{
	int espChecksum = 0;								//Value of checksum calculated on esp side
	int androidChecksum = 0;							//Value of checksum sent by android

	for (int i = 0; i < 3; i++)
	{
		squareChecksumChar[i] = singleSquareData[i + 7]; //Get checksum substring from data packet
		espChecksum += singleSquareData[i + 4];			 //Esp calculate checksum  
	}

	androidChecksum = charToInt(squareChecksumChar, 3);	 // Convert checksum substring to int

	if (androidChecksum == espChecksum)					 //If they match, allow the packet data to be used
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

	else if (SerialBT.available())
	{
		return SerialBT.read();
	}

	else
	{
		return '|'; //Returns pipe symbol if the serial monitor no longer has data in it
	}
}

// M A I N   F U N C T I O N --- inputCase statement
void debugInputParse(char debugCommand)
{     // read the incoming byte:

	switch (debugCommand)
	{

	case '0':                             // send dome stepper to home posistion
		moveToPosition(stepperDomeStpPin, 0, 0, 0, 0);
		break;

	case '1':                             // send vavle stepper to home posisiton
		valveGoHome();
		break;

	case '2':
		moveToPosition(stepperDomeStpPin,(currentDomePosition - stepAmount),0,0,0);
		break;

	case '3': 
		moveToPosition(stepperDomeStpPin, (currentDomePosition + stepAmount), 0, 0, 0);
		break;

	case '=':
		stepAmount += 1;
		if (stepAmount > 15)
		{
			stepAmount = 15;
		}
		break;

	case '-':
		stepAmount -= 1;
		if (stepAmount < 1)
		{
			stepAmount = 1;
		}
		break;
	case 'a':
		crazyDomeStepperFromDebugA();
		break;

	case 'b':
		stepperDomeDirCW();					// set dome stepper to CW ---> HIGH IS CLOSEWISE!!!
		break;

	case 'c':
		stepperDomeDirCCW();				// set dome stepper to CCW ---> LOW IS COUNTER CLOCKWISE!!!
		break;

	//case to store position and flow values to spiffs
	case 'd':
		//function to save current flow rate and angle to spiffs
		//spiffsFlowPos(freq, currentDomePosition);
		//function to pull values from spiffs to global arrays for watering cycle
		//spiffsFlowPosRead();

		spiffsDataLog(tempVoltage);
		spiffsDataRead();
		
		Serial.println();


		break;

	case 'e':
		toggleStepperValveDir();
		break;

	case 'f':
		displaySolarCurrent();
		displaySolarVoltage();
		break;

	case 'g':
		solarPowerTracker();
		break;

	case 'h':
		executeSquare(100);
		delay(1000);
		valveGoHome();
		break;

	case 'i':		//test1
		spiffsParse(testNum1);
		extractBedData(bedsToSprayFile);
		spiffsParse(testNum2);
		extractBedData(bedsToSprayFile);
		spiffsParse(testNum3);
		extractBedData(bedsToSprayFile);

		break;

	case 'j':		//test2
		spiffsParse(testNum2);
		extractBedData(bedsToSprayFile);

		break;

	case 'q':
		String flowString = "";
		float desiredFlow = 0;

		flowString = Serial.readString();
		desiredFlow = flowString.toFloat;

		makeRain(desiredFlow);

	case 'r':
		String flowString = "";
		float desiredFlow = 0;
		

		Serial.printf("enter flow value: \n");
		while (!Serial.available()) { ; }
		flowString = Serial.readString();
		desiredFlow = flowString.toFloat;

		makeRain(desiredFlow);

	case 's':
		//Serial.println("debug case: s -> going to sleep...");SerialBT.println("debug case: s -> going to sleep...");
		deepSleep();					// put esp32 to sleep for 15minutes.. add to this function so it wake ups on even time
		break;

	case 't':
		printLocalTime();					// display time
		break;

	case 'z':
		programStateNotDoneFlag = 0;				// display time
		break;
	case 'o':

		digitalWrite(stepperValveSlpPin, HIGH);
		digitalWrite(stepperValveDirPin, LOW);

		digitalWrite(stepperValveStpPin, HIGH);
		delay(4);
		//delay(100);
		digitalWrite(stepperValveStpPin, LOW);
		delay(4);
		break;

	case 'x':
		digitalWrite(stepperValveSlpPin, HIGH);
		digitalWrite(stepperValveDirPin, HIGH);

		digitalWrite(stepperValveStpPin, HIGH);
		delay(4);
		//delay(100);
		digitalWrite(stepperValveStpPin, LOW);
		delay(4);
		break;

	case 'v':
		//storeSpray(freq,currentDomePosition);
		break;
	}
}

void storeSpray(float freq, int pos)
{

}