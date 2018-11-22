
// **********   S * K * Y  |)  W * E * T *
//  -=-=-=-=-=-=-=-=-=-=-=-=-
// turret control firmware for esp32 dev kit C
// november 20, 2018
// Make A variable timeeToSleep . so time to sleep => timeToSleep = ( 60s - (timeToSleep))


// *********   P R E P R O C E S S O R S
#include "deepSleep.h"
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

#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  60        /* Time ESP32 will go to sleep (in seconds) */

void deepSleep()
{
	// sleep, rtc and power mangement
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

void programDeepSleep()
{
	// do we need to turn off BT?
	deepSleep();
}