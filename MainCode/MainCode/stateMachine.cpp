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
#include "SerialData.h"
#include "SolarPowerTracker.h"
#include "SPIFFSFunctions.h"
#include "stateMachine.h"
#include "StepperFunctions.h"

// rgb led
#define rgbLedBlue 26
#define rgbLedGreen 25
#define rgbLedRed 27


// power on rainbow
// init..
// check status flags, time, schedule tasks, etc --> bool amPmWaterFlag,, int waterAlarm, checkBatteryVoltage(), programFlag, sleepTimer ISR
// case 0 check battery power
// case 1 check if it is watering time
// case 2 do current sense
// ext. ISR if GPIO13 wakeUpButton = HIGH --> enable BT for program mode


void programState()
{
	//bool programStateNotDoneFlag = 1;
	
	initSerialBT();

	while (programStateNotDoneFlag)
	{
		if (freq != oldfreq)
		{
			Serial.println(freq);
			SerialBT.println(freq);
			oldfreq = freq;
		}
		//Serial.println("main code program case");
		getSerialData();
	}

	//PLUS TURN OFF THE OTHER STUFF..??
	// WILL THIS BE A PROBLEM IF BLUETOOTH IS NEVER INIT??
	//SerialBT.end;
	digitalWrite(rgbLedBlue, LOW);
	deepSleep();
}



void timerState()
{
	

	while (wakeUpTimerStateNotDoneFlag)
	{

		switch (sysStateTimerWakeUp)
		{

			case low_power:
			{
				//close the valve
				//set LED to red
				//allow solar
				//prevent water until battery > 50%
				//>50% -> perform last spray cycle

				//checkBatteryCap();
				//if (checkBatteryCap())
				//{
					//powerDownEverything();
					//sysStateTimerWakeUp = solar;
				//}
				//else
				//{
				Serial.println("Im doing low power here");
				digitalWrite(rgbLedRed, HIGH);
				delay(1000);
				digitalWrite(rgbLedRed, LOW);
				sysStateTimerWakeUp = water;
				//}
				break;
			}

			case water:
			{
				//load correct instruction set for date and time
				//reference temperature and apply modfifier to watering durations
				//open thread for flow sensor
				//run spray program
				Serial.println("Im doing watering here");
				delay(1000);
				sysStateTimerWakeUp = solar;
				break;
			}

			case solar:
			{
				// if(solarPanelVoltage > 0)
				Serial.println("Im doing solar here");
				delay(1000);
				solarPowerTracker();
				sysStateTimerWakeUp = sleepy;
				break;
			}

			case sleepy:
			{
				Serial.println("Im doing sleep here");
				delay(1000);
				sysStateTimerWakeUp = low_power;
				wakeUpTimerStateNotDoneFlag = 0;
				deepSleep();
				break;
			}
		}
	}
}