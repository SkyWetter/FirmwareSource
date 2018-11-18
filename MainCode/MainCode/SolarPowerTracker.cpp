#include <BluetoothSerial.h> 
#include <Stepper.h>
#include <soc\rtc.h>
#include <pthread.h>
#include "GlobalVariables.h"
#include "InitESP.h"
#include "StepperFunctions.h"
#include "driver\adc.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include "sys/time.h"
#include "sdkconfig.h"

#define GPIO_INPUT_IO_TRIGGER     0  // There is the Button on GPIO 0
#define GPIO_DEEP_SLEEP_DURATION     10  // sleep 30 seconds and then wake up
#define CCW -1
#define CW  1
#define TEST 45

// flow meter
#define pulsePin GPIO_NUM_23

// dome stepper
#define stepperDomeDirPin 19
#define stepperDomeStpPin 18
#define stepperDomeSlpPin 2
#define hallSensorDome 16
#define stepperDomeCrntPin 14

// valve stepper
#define stepperValveDirPin 5
#define stepperValveStpPin 17
#define stepperValveSlpPin 15
#define hallSensorValve 4
#define stepperValveCrntPin 12

// wake-up push button
#define wakeUpPushButton GPIO_NUM_13

// rgb led
#define rgbLedBlue 26
#define rgbLedGreen 25
#define rgbLedRed 27

// solar panel
#define currentSense A6
#define solarPanelVoltage A7



void solarPowerTracker()
{
	int peakInsolationSteps = 0;
	static int stepDivider = 20;		//sets number of steps between each measurement. Must divide evenly in to 400

	long delayTimer1, delayTimer2;
	long currentSenseVal1 = 0;
	long currentSenseVal2 = 0;

	bool delayComplete1 = false;
	bool delayComplete2 = false;


	Serial.println("Solar tracking begins");

	domeGoHome();     //send turret to zero position

	while (delayComplete1 == false)
	{

		if (digitalRead(hallSensorDome) == LOW)		//Wait until dome returns home -- CHECK IF ACTIVE HIGH
		{
			Serial.println("Dome has returned to home");

			delayComplete1 = true;    //end loop
			delayTimer1 = 0;      //set timers
			delayTimer2 = 0;


			delay(5);

			//currentSenseVal1 = analogRead(currentSense);

			for (int y = 0; y < 100; y++)
			{
				currentSenseVal1 += adc1_get_raw(ADC1_CHANNEL_6);	//take next voltage reading
				//delay(1);
				//Serial.printf("current reading is %i \n", currentSenseVal2);
			}

			currentSenseVal1 = currentSenseVal1 / 10;

			//currentSenseVal1 = analogRead(currentSense);		//take first voltage reading at zero position

			currentSenseVal2 = 0;

			stepperDomeDirCW();   //set rotation to clockwise 

			Serial.printf("First reading is %i \n", currentSenseVal1);

			for (int i = 1; i <= 400 / stepDivider; i++)    //divide rotation in x number of steps
			{
				for (int x = 0; x < stepDivider; x++)
				{
					stepperDomeOneStepHalfPeriod(6);		//take steps number is the speed

				}

				delayTimer1 = millis();   //start timer

				while (delayComplete2 == false)
				{

					if (delayTimer2 >= delayTimer1 + 10)			//10ms delay per step
					{
						delayComplete2 = true;

						currentSenseVal2 = 0;

						for (int y = 0; y < 100; y++)
						{
							currentSenseVal2 += adc1_get_raw(ADC1_CHANNEL_6);	//take next voltage reading
							//delay(1);
							//Serial.printf("current reading is %i \n", currentSenseVal2);
						}

						currentSenseVal2 = currentSenseVal2 / 10;

						Serial.printf("current reading is %i \n", currentSenseVal2);


						if (currentSenseVal1 < currentSenseVal2)
						{
							currentSenseVal1 = currentSenseVal2;  //if it is greater, save new reading
							peakInsolationSteps = i * stepDivider;    //number of steps taken to reach position
							delayTimer1 = 0;
							delayTimer2 = 0;

							Serial.printf("New highest reading is %i \n", currentSenseVal1);
							Serial.printf("Position is %i steps from zero \n", peakInsolationSteps);
						}
					}

					else
					{
						delayTimer2 = millis();   //increment timer if limit not reached
					}
				}

				delayComplete2 = false;
			}

			Serial.printf("Final highest reading reading is %i \n", currentSenseVal1);
			Serial.printf("Final position is %i steps back from 400 \n", 400 - peakInsolationSteps);

			stepperDomeDirCCW();    //set rotation to ccw

			delayComplete2 = false;   //reset timers
			delayTimer1 = millis();
			delayTimer2 = 0;

			for (int i = 0; i < 400 - peakInsolationSteps; i++)   //return to point of peak insolation
			{
				stepperDomeOneStepHalfPeriod(5);
			}

			while (delayComplete2 == false)
			{
				if (delayTimer2 >= delayTimer1 + 2000)    //wait 2s to arrive, then exit function
				{
					delayComplete2 = true;

					Serial.println("Exiting solar tracker");
				}

				else
				{
					delayTimer2 = millis();   //increment timer if limit not reached
				}
			}
		}
	}
}

void displaySolarVoltage()
{
	solarPanelVoltageVal = (float)analogRead(solarPanelVoltage);
	delay(1);
	solarPanelVoltageVal = (((solarPanelVoltageVal / 4096) * 3.3) * 4.2448);

	Serial.print("solar panel voltage: ");
	Serial.print(solarPanelVoltageVal);
	Serial.println("V");
	SerialBT.print("solar panel voltage: ");
	SerialBT.print(solarPanelVoltageVal);
	SerialBT.println("V");
}

void displaySolarCurrent()
{
	long currentSenseVal1 = 0;

	currentSenseVal1 = analogRead(currentSense);

	Serial.print("current val: ");
	Serial.println(currentSenseVal1);
	SerialBT.print("current val: ");
	SerialBT.println(currentSenseVal1);
}
