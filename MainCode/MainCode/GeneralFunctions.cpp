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


#define pulsePin GPIO_NUM_23

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

void doPulseIn()
{
	Serial.println("pulseIn begin");

	int pulseStart;			//gets set when the pulse pin goes high
	bool pulseState;		//the current pulse state of the pin starts unknown
	int begin = millis();	// a counter for the entire loop. If no flow then timeout
	int waitTime = 0;		//time spent waiting for pulse to go low
	float pulseTime = 0;	// time of pulse for math
	int newPulseLength = 0;	// continous pulse time

	while (begin + 1000 >= millis()) {					//if time is available do the things
		//while(1){

			//delay(1);
		//initpinState = gpio_get_level(pulsePin);

		if (gpio_get_level(pulsePin) == 0){				//first we wait for the pin to go low
			
			delay(4);									//delays are for debouncing. Shouldnt need to actual sensor
			pulseState = 0;								//set pulse state to match pin

			if (gpio_get_level(pulsePin) == 1) {		//when pin goes from low to high

				pulseState = 1;							//set pulse state
				pulseStart = millis();					//set pulse start time
				newPulseLength = pulseStart;			//set new pulse time
				waitTime = millis();					//set wait timer
				delay(4);								//this delay is only needed for button debugging	
				//Serial.println("pulse detected");
				while (pulseState == gpio_get_level(pulsePin) && waitTime + 1000 >= millis()) {			//while the pin is still high and timer is avail
	
					newPulseLength = millis();			//slot in a new value for pulse length
					//Serial.println("reading pulse");
				}

				pulseTime = newPulseLength - pulseStart;	//math to figure out pulse time float
				pulseStart = millis();						//reset timer incase skips while loop next time
				//Serial.println("pulse complete");
				//Serial.print("pulse length is");
				//Serial.println(pulseTime);
			}
		}
	}
	//Serial.print("pulse length final is ");
	//Serial.println(pulseTime);
	if (pulseTime > 0) {							//only do math if pulse is not zero
		//Serial.print("i try done math ");
		freq = 500 / pulseTime;						// 500 because a pulse is 1/2 cycle
	}
	//Serial.print("frequency is ");
	//Serial.println(freq);
	//Serial.println("exiting pulseIn");


}
