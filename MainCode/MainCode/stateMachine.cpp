// 
// 
// 
// *********   P R E P R O C E S S O R S
#include "stateMachine.h"
#include "SPIFFSFunctions.h"
#include "deepSleep.h"

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
#include "stateMachine.h"
#include "deepSleep.h"


// power on rainbow
// init..
// check status flags, time, schedule tasks, etc --> bool amPmWaterFlag,, int waterAlarm, checkBatteryVoltage(), programFlag, sleepTimer ISR
// case 0 check battery power
// case 1 check if it is watering time
// case 2 do current sense
// ext. ISR if GPIO13 wakeUpButton = HIGH --> enable BT for program mode


void programState()
{

	bool programStateNotDoneFlag = 1;

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
	if (SerialBT.hasClient) // wait here while Serial Bluetooth establishes
	{
		SerialBT.end;
	}

	deepSleep();
}


void checkSystemStateWakeUpByTimer()
{
	bool wakeUpTimerStateNotDoneFlag =  1; 

	while(wakeUpTimerStateNotDoneFlag)
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
			sysStateTimerWakeUp = water;
			break;
		}

		case water:
		{
			//load correct instruction set for date and time
			//reference temperature and apply modfifier to watering durations
			//open thread for flow sensor
			//run spray program
			sysStateTimerWakeUp = solar;
			break;
		}

		case solar:
		{
			solarPowerTracker();
			sysStateTimerWakeUp = sleep;
			break;
		}

		case sleep:
		{
			sysStateTimerWakeUp = low_power;
			wakeUpTimerStateNotDoneFlag = 0;
			deepSleep();
			break;
		}
	}

}
