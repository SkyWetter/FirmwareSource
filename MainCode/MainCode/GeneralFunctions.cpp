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
#include "GeneralFunctions.h"

#define pulsePin 23


double duration;

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

int charToInt(char *thisChar, int thisCharLength, int startingCharIndex)
{
	  
	int intConversion = 0;
	for (int i = startingCharIndex; i < (thisCharLength + startingCharIndex); i++)
	{                  
		intConversion += ((thisChar[i]) - '0') * pow(10, thisCharLength - startingCharIndex - i);
	
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

void doPulseIn()
{
	//Pulse IN shit
	//changes for example
	duration = float(pulseIn(pulsePin, HIGH));
	//SerialBT.println(duration);

}

long getFlowMeter()
{
	return pulseIn(pulsePin, HIGH);
}

