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
#include "SerialData.h"
#include "SolarPowerTracker.h"
#include "SPIFFSFunctions.h"

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

		case '#':
			//header #0001@0028!data
			if (input2DArrayPosition < 14)
			{
				serialState = parseGarden;
			}

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
		//Serial.print("here in debug command");
		debugInputParse(getDebugChar());

		break;

	case parseGarden:

		parseInput();

		break;

	default:;
	}
}



// S U B F U N C T I O N S -- getSerialData

void parseInput()
{
	//Serial.println("test");


	int j = 11;
	int length;
	char headerArray[11] = { '#' };
	char charNumArray[4];

	//Serial.print("Entering case # - header array: #");

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
		//Serial.print(headerArray[i]);
	}

	//Serial.println();
	//Serial.print("String length, array: ");

	//pull out string length
	for (int i = 0; i < 4; i++)
	{
		charNumArray[i] = headerArray[(i + 6)];
		//Serial.print(charNumArray[i]);
	}

	length = charToInt(charNumArray, 4);

	//Serial.println();
	//Serial.print("String length, conversion: ");
	//Serial.println(length);

	//create new array to match
	input2DArray[input2DArrayPosition] = new char[length];

	//replace header chars
	for (int i = 0; i < 11; i++)
	{
		input2DArray[input2DArrayPosition][i] = headerArray[i];
	}

	//pull rest of data  --- replace Serial w/ Serial.BT
	while(Serial.available())
	{
		input2DArray[input2DArrayPosition][j] = Serial.read();
		j++;
	}

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

	if (input2DArrayPosition == 0)
	{
		spiffsSave(input2DArray[input2DArrayPosition], length);
	}
	else
	{
		spiffsAppend(input2DArray[input2DArrayPosition], length);
	}

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
		domeGoHome();
		break;

	case '1':                             // send vavle stepper to home posisiton
		valveGoHome();
		break;

	case 'a':
		
		
		moveToPosition(stepperDomeStpPin, 10,0, 0, 0);
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
		break;

	case 'b':
		// set dome stepper to CW ---> HIGH IS CLOSEWISE!!!
		stepperDomeDirCW();
		break;

	case 'c':
		// set dome stepper to CCW ---> LOW IS COUNTER CLOCKWISE!!!
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
		//doPulseIn();
		Serial.print("frequency is ");
		Serial.println(freq);
		Serial.println("exiting pulseIn");
		SerialBT.println(freq);
		break;

	case 'i':
		//spiffsBegin();
		//spiffsSave();
		Serial.println("test");
		break;

	case 'j':
		spiffsRead();
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
