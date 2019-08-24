// *********   P R E P R O C E S S O R S
#include <soc\rtc.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <esp_clk.h>
#include <esp_timer.h>
#include "sys/time.h"

#include "GlobalVariables.h"
#include "GeneralFunctions.h"
#include "realTimeFunctions.h"
#include "SolarPowerTracker.h"

int offMin, offSec;


void timeShift()
{
	char incomingTime[16];
	char incomingByte;
	char buf1[4];
	char buf2[2];

	if (incomingSerialBTFlag == 0)
	{
		for (int j = 0; j < 16; j++)		// get incoming time string and put it in char array incomingTime[]
		{
			incomingByte = Serial.read();
			incomingTime[j] = incomingByte;
		}
	}

	if (incomingSerialBTFlag == 1)
	{
		for (int j = 0; j < 16; j++)		// get incoming time string and put it in char array incomingTime[]
		{
			incomingByte = SerialBT.read();
			incomingTime[j] = incomingByte;
		}
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
		buf2[i] = incomingTime[i + 12];
		usrSec = charToInt(buf2, 2);
	}
	//timeOffsetSinceBoot();
	printLocalTime();
}

void printLocalTime()
{
	char buf1[30];
	char buf2[30];

	time(&time1);			// time_t time1;	returns numbers seconds since first of jan 1970
	ctime_r(&time1, buf1);
	asctime_r(gmtime(&time1), buf2);
	gmtime_r(&time1, &tm1);
	localtime_r(&time1, &tm1);
	
	tm1.tm_year += (usrYear - 1970);
	tm1.tm_wday -= 1;
	tm1.tm_mday += (usrDay - 1);
	tm1.tm_mon += (usrMon - 1);
	tm1.tm_hour += (usrHour);
	tm1.tm_min += usrMin;
	tm1.tm_sec += ((usrSec));

	Serial.printf("%.24s \n", asctime(&tm1));										// %A = full weekday name, %B full month name,  %d = day of the month, %Y = year with century, %H = hour (24hr),  %M = minute 900-59), %S= second (00-61)
}
void timeOffsetSinceBoot()
{
	secsLastBootOffset = (millis() / 1000);
	Serial.printf("Secs since boot: %d\n", secsLastBootOffset);

	offMin = ((secsLastBootOffset / 60) + 1);
	offSec = (60 - (secsLastBootOffset % 60));

	Serial.println(offMin);
	Serial.println(offSec);

	usrMin = (usrMin - offMin);
	usrSec = offSec;

	Serial.println(usrMin);
	Serial.println(usrSec);
}

void wateringWakeUp()
{
	Serial.print("tm1.tm_hour time is: ");
	Serial.println(tm1.tm_hour);

	Serial.print("usr hour: ");
	Serial.println(usrHour);
}

void solarWakeUp()
{
	int q;
	q = tm1.tm_min;
	Serial.println(q);

	if (q == 30)
	{
		//int q = analogRead(currentSense);

		//if ( q > 0)
		//{ 
			Serial.println("im doing a solar power track");
			solarPowerTracker();
		//}
	}
}

int getNowSec()
{
	int nowSec;
	nowSec = tm1.tm_sec;
	return nowSec;
}