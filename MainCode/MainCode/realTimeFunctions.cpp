#include <esp_clk.h>
#include <esp_sleep.h>
#include <soc/rtc.h>
#include <esp_timer.h>
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  5        /* Time ESP32 will go to sleep (in seconds) */


void timeShift()
{
	char incomingTime[16];
	char incomingByte;
	char buf1[4];
	char buf2[2];

	Serial.println("Enter TIME:  format --> YYYYMMDDhhmmss");

	while (!Serial.available()) {}

	if (Serial.available() > 0)
	{
		for (int j = 0; j < 16; j++)
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

	case 1: Serial.println("Wakeup caused by external signal using RTC_IO");
		timeShift();
		break;

	case 2:
		Serial.println("Wakeup caused by external signal using RTC_CNTL");
		break;
	case 3: Serial.println("Wakeup caused by timer"); break;
	case 4: Serial.println("Wakeup caused by touchpad"); break;
	case 5: Serial.println("Wakeup caused by ULP program"); break;
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

void someFunctions()
{

	if (bootCount == 0)
	{
		timeShift();
	}
	++bootCount;

	printLocalTime();

	//Print the wakeup reason for ESP32

	Serial.println("Boot number: " + String(bootCount)); //Increment boot number and print it every reboot
	Serial.print("# of seconds since last boot: ");
	Serial.println(now.tv_sec);

	print_wakeup_reason();

	// First we configure the wake up source We set our ESP32 to wake up every 5 seconds
	esp_sleep_enable_ext0_wakeup(GPIO_NUM_13, 1);
	esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
	Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) + " Seconds");

	esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
	Serial.println("Configured all RTC Peripherals to be powered on in sleep");

	Serial.println("Going to sleep now");
	Serial.flush();
	esp_deep_sleep_start();
	Serial.println("This will never be printed");

}