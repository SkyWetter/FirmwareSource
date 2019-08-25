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


#define stepperDomeDirPin 19	//put here for presentaiton
#define stepperDomeStpPin 18
#define stepperDomeSlpPin 2
#define hallSensorDome 16
#define stepperDomeCrntPin 14

//destroy after presntation


// power on rainbow
// init..
// check status flags, time, schedule tasks, etc --> bool amPmWaterFlag,, int waterAlarm, checkBatteryVoltage(), programFlag, sleepTimer ISR
// case 0 check battery power
// case 1 check if it is watering time
// case 2 do current sense
// ext. ISR if GPIO13 wakeUpButton = HIGH --> enable BT for program mode



void programState()
{	
	initSerialBT();

		while (programStateNotDoneFlag) //this is currently where the code loops 
		{

			if (freq != oldfreq)
			{
				Serial.println(freq);
				//SerialBT.println(freq);
				oldfreq = freq;
			}
			//Serial.println("main code program case");
			getSerialData();
		}
	
	//init shutdown from program state
	SerialBT.end();
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
				sysStateTimerWakeUp = water;
				//}
				break;
			}

			case water:
			{
				//load correct instruction set for date and time
				//reference temperature and apply modfifier to watering durations
				//open thread for flow sensor

				// if current Time = schedule Time then do watering
				getLocalTime(&timeinfo);
				if (timeinfo.tm_min == sched_m){
					Serial.println("Im doing watering here");
				scheduledSprayRoutine(); //run spray program
				sysStateTimerWakeUp = solar;
				}
				break;
			}

			case solar:
			{
				// if(solarPanelVoltage > 0)
				Serial.println("Im doing solar here");
				solarWakeUp();
				sysStateTimerWakeUp = sleepy;
				break;
			}

			case sleepy:
			{/*
				Serial.println("Im doing sleep here");
				sysStateTimerWakeUp = low_power;
				wakeUpTimerStateNotDoneFlag = 0;
				deepSleep();*/
				sysStateTimerWakeUp = low_power;
				break;
			}
		}
	}
}

//destroy all below after presentation
