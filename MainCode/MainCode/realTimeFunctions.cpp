#include <esp_clk.h>
#include <esp_sleep.h>
#include <soc/rtc.h>
#include <esp_timer.h>
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


void timeShift()
{
	char incomingTime[16];
	char incomingByte;
	char buf1[4];
	char buf2[2];

	//Serial.println("Enter TIME:  format --> $YYYYMMDDhhmmss");
	//while (!Serial.available()) {}

		if (Serial.available() > 0)
		{

			for (int j = 0; j < 16; j++)		// get incoming time string and put it in char array incomingTime[]
			{

				incomingByte = Serial.read();
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

		//secsLastBootOffset = (now.tv_sec);
	}

	printLocalTime();

}

void print_wakeup_reason()
{
	esp_sleep_wakeup_cause_t wakeup_reason;
	wakeup_reason = esp_sleep_get_wakeup_cause();

	switch (wakeup_reason)
	{

		case 1:
		Serial.println("Wakeup caused by external signal using RTC_IO");
		// if we are here its because there was a wake-up push button event
		// so we will want to enter program mode
		// enable bluetooth
		// goto to sleep when done
		break;

		case 2:
		Serial.println("Wakeup caused by external signal using RTC_CNTL");
		break;

		case 3: 
		Serial.println("Wakeup caused by timer"); 
		//timer should wakeup device every soo often...
		//rainbow will check for solar power
		//rainbow will check to see if it is time to water or not
		break;

		case 4: 
		Serial.println("Wakeup caused by touchpad"); 
		break;

		case 5:
		Serial.println("Wakeup caused by ULP program"); 
		break;

		default: Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason); break;

	}
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

	Serial.printf("%.24s \n", asctime(&tm1));				// %A = full weekday name, %B full month name,  %d = day of the month, %Y = year with century, %H = hour (24hr),  %M = minute 900-59), %S= second (00-61)

}