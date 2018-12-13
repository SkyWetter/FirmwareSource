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

#define pulsePin 23 




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


void printString(char charArray[],int charsPerLine)
{
	int length = getArrayLength(charArray, false);
	int charsPrinted = 0;

	while (length > charsPerLine)
	{
		for (int i = 0; i < charsPerLine; i++)
		{
			printf("%c",charArray[i + charsPrinted]);
		}

		printf("\n");
		charsPrinted += charsPerLine;
		length -= charsPerLine;
	}

	if (length > 0)
	{
		for (int j = 0; j < length; j++)
		{
			printf("%c", charArray[j + charsPrinted]);
		}

		printf("\n");
	}

	else
	{
		printf("\n");
	}

}

int getArrayLength(char charArray[],bool lengthIncludesNull)
{
	int i = 0;
	int length = 0;

	while (charArray[i] != 0x00)
	{
		length++;
		i++;
	}
	/* ///For debug only
	if (charArray[i] == 0x00)
	{
		printf("Found null at pos %d\n", i);
	}*/

	if (!lengthIncludesNull)
	{
		return length;
		//printf("Length of array is %d\n", length);
	}

	else
	{
		return length + 1;
	}
}

void clearArray(char array[], int arrayLength)
{
	for(int i = 0; i < arrayLength; i++)
	{
		array[i] = 0;
	}
}

void clearIntArray(int array[], int arrayLength)
{
	for (int i = 0; i < arrayLength; i++)
	{
		array[i] = 0;
	}
}