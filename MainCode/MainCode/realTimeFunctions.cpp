// *********   P R E P R O C E S S O R S
#include "SPIFFSFunctions.h"
#include <SPIFFS.h>
#include <Stepper.h>
#include <BluetoothSerial.h>
#include <soc\rtc.h>
#include "InitESP.h"
#include <pthread.h>
#include "GlobalVariables.h"
#include "GeneralFunctions.h"
#include "StepperFunctions.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include "sys/time.h"
#include "SolarPowerTracker.h"
#include "SerialData.h"
#include "sdkconfig.h"
#include <driver/adc.h>
#include "realTimeFunctions.h"
//#include <freertos/ringbuf.h>
#include <time.h>
#include <esp_clk.h>
#include <esp_sleep.h>
#include <esp_timer.h>




void timeShift()
{
	char incomingTime[16];
	char incomingByte;
	char buf1[4];
	char buf2[2];

	for (int j = 0; j < 16; j++)		// get incoming time string and put it in char array incomingTime[]
	{
		incomingByte = SerialBT.read();
		incomingTime[j] = incomingByte;
	}

	for (int i = 0; i < 4; i++)
	{
		buf1[i] = incomingTime[i];
		usrYear = charToInt(buf1, 4);
	}

	for (int i = 0; i < 2; i++)
	{
		buf2[i] = incomingTime[i + 4];
		usrMon = charToInt(buf2, 2);
	}

	for (int i = 0; i < 2; i++)
	{
		buf2[i] = incomingTime[i + 6];
		usrDay = charToInt(buf2, 2);
	}

	for (int i = 0; i < 2; i++)
	{
		buf2[i] = incomingTime[i + 8];
		usrHour = charToInt(buf2, 2);
	}

	for (int i = 0; i < 2; i++)
	{
		buf2[i] = incomingTime[i + 10];
		usrMin = charToInt(buf2, 2);
	}

	for (int i = 0; i < 2; i++)
	{
		buf2[i] = incomingTime[i + 11];
		usrSec = charToInt(buf2, 2);
	}
	printLocalTime();
}

void printLocalTime()
{

	char buf1[30];
	char buf2[30];

	gettimeofday(&tv, NULL);														//struct timeval tv;
	Serial.printf("gettimeofday()  : %ld : %ld \n", tv.tv_sec, tv.tv_usec);

	time(&time1);																	// time_t time1;	returns numbers seconds since first of jan 1970
	Serial.printf("time()          : %ld \n", time1);
	Serial.printf("ctime()         : %.24s\n", ctime_r(&time1, buf1));				// ctime convert time type into a format string
	Serial.printf("asctime()       : %.24s \n", asctime_r(gmtime(&time1), buf2));	// sane as ctune but taks any structure

	gmtime_r(&time1, &tm1);
	Serial.printf("gmtime()      : %d : %d  : %d : %d : %d : %d \n", tm1.tm_mday, tm1.tm_hour, tm1.tm_min, tm1.tm_sec, tm1.tm_isdst);

	localtime_r(&time1, &tm1);
	Serial.printf("localtime()      %d : %d : %d : %d : %d \n", tm1.tm_mday, tm1.tm_hour, tm1.tm_min, tm1.tm_sec, tm1.tm_isdst);

	tm1.tm_year += (usrYear - 1970);
	tm1.tm_mday += (usrDay - 1);
	tm1.tm_mon += (usrMon - 1);
	tm1.tm_hour += (usrHour);
	tm1.tm_min += usrMin;
	tm1.tm_sec += (usrSec);

	time1 = mktime(&tm1);

	Serial.printf("%.24s \n", asctime(&tm1));										// %A = full weekday name, %B full month name,  %d = day of the month, %Y = year with century, %H = hour (24hr),  %M = minute 900-59), %S= second (00-61)

}

void wateringWakeUp()
{

}